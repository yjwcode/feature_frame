#pragma once
//#include <stdio>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/stat.h> 
#include <fcntl.h>

#define LOG(msg) \
	    std::cout << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl 
using namespace std;
class FeatureConf;

class FeatureFunc{
public:
	virtual int run(const vector<string> & depends, vector<string> & results) const { return 0;};
	virtual bool check(const vector<string> & depends) const {return true;}
	virtual ~FeatureFunc(){};
};
class FeatureFuncBase: public FeatureFunc{
public:
	int run(const vector<string> & depends, vector<string> & results ) const{
		return 0;
	}
};
class FeatureFuncCross: public FeatureFunc{
public:
	int run(const vector<string> & depends, vector<string> & results) const {
		return 0;
	}
};

class FeatureConf{
public:
	FeatureConf(string name, bool output, int prefix, const vector<string> & depends, const FeatureFunc * func) : 
		_name(name), _output(output), _prefix(prefix), _depends(depends), _func(func) {}
	static FeatureConf * create_feature_conf(const string & raw, map<string, FeatureFunc*> & func_map) {
		using namespace boost::algorithm;
		std::vector<std::string> v;
		split(v, raw, is_any_of("|"));
		if(v.size() != 5) {
			LOG("invalid input:" << raw << " size:" << v.size());
			return NULL;
		}
		string name = boost::trim_copy(v.at(0)); 
		bool output = boost::lexical_cast<bool>(boost::trim_copy(v.at(1)));
		int prefix = boost::lexical_cast<int>(boost::trim_copy(v.at(2)));
		string depends_str = boost::trim_copy(v.at(4));
		vector<string> depends;
		split(depends, depends_str, is_any_of("|"));
		string func_name = boost::trim_copy(v.at(3));
		if (!func_map.count(func_name)) {
			LOG("not found" << func_name);
			return NULL;
		}
		const FeatureFunc * func = func_map[func_name];
		if (!func->check(depends))  {
			LOG("check fail:" << func_name);
			return NULL;
		}
		return new FeatureConf(name, output, prefix, depends, func);
	}	
	string _name;
	bool _output;
	int _prefix;
	vector<string> _depends;
	const FeatureFunc * _func;		
};
class FeatureStore{
public:
	FeatureStore(const FeatureConf * fc) : _fc(fc) {}
	int run() {
		return _fc->_func->run(_fc->_depends, _results);
	}
private:
	vector<string> _results;
	const FeatureConf * _fc;
};
class WhiteBoard{
public:
	void init(const vector<FeatureConf> & fcs) {
		for(size_t i = 0; i < fcs.size(); ++i) {
			_feature_stores.push_back(FeatureStore(&fcs.at(i)));
		}
	}
	map<string,string> _data_source;
	vector<FeatureStore> _feature_stores;
	map<string,string> _input_map;
};

inline int parse_input_conf(const string & conf, map<string, int> & conf_map){
	using namespace boost::algorithm;
	std::vector<std::string> v;
	split(v, conf, is_any_of(","));
	for (int i = 0; i < v.size(); ++i) {
		boost::trim(v.at(i));
		conf_map[v.at(i)] = i;
		LOG("conf " << v.at(i) << " i:"<< i);
	}
	return 0;
}
class FeatureMaker{
public:
	FeatureMaker () {
		func_map["base"] = new FeatureFuncBase;
		func_map["cross"] = new FeatureFuncCross;
	}

	int init(const string & path){
		FILE * fp = fopen(path.c_str(), "r");
		if (!fp ) {
			LOG("open fail. file:" << path.c_str());
			return -1;
		}

		int BUF_SIZE = 10240;
		char buffer[BUF_SIZE];
		ssize_t ret_in = 0;
		map<string, int> input_conf;
		while (fgets(buffer, sizeof(buffer), fp)) {
			string tmp = buffer;
			boost::trim(tmp);
			if(tmp.empty() or tmp[0] == '#') {
				continue;
			}
			if(tmp[0] == '@') {
				parse_input_conf(tmp.substr(1), _input_conf);
			} else {
				FeatureConf * conf = FeatureConf::create_feature_conf(tmp, func_map);
				if (conf == NULL) {
					LOG("create_feature_conf fail." << tmp);
					return -1;
				}
				_fcs.push_back(conf);
			}
			LOG("content:" << tmp);

		}
		return 0;
	}
	int make_features(WhiteBoard & wb, vector<long> & features);
private:
	map<string, int> _input_conf;
	map<string, FeatureFunc*> func_map;
	vector<FeatureConf*> _fcs;
	
};
//int FeatureMaker::make_features(WhiteBoard & wb, vector<long> & features) {
//	wb.init(_fcs);
//	wb.run();
//	wb.output(features);
//}
