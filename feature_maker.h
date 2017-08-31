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
	virtual int run(const vector<string> & depends, vector<string> & results, const map<string,string> & tmp_map) const { return 0;};
	virtual bool check(const vector<string> & depends) const {return true;}
	virtual ~FeatureFunc(){};
	virtual int check_depends(int expect_cnt, const vector<string> & depends, const map<string,string> & tmp_map) const {
		if (depends.size() != expect_cnt) {
			LOG("fail. depends size " << depends.size());
			return -1;
		}
		for (const auto & d : depends) {
			if (!tmp_map.count(d))  {
				LOG("not found depend:" << d);
				return -1;
			}
		}
		return 0;
	}
};
class FeatureFuncBase: public FeatureFunc{
public:
	virtual int run(const vector<string> & depends, vector<string> & results, const map<string,string> & tmp_map ) const{
		if (check_depends(1, depends, tmp_map) != 0){
			return -1;
		}
		results.push_back(tmp_map.find(depends.at(0))->second);
		return 0;
	}
};
class FeatureFuncBaseList: public FeatureFunc{
public:
	virtual int run(const vector<string> & depends, vector<string> & results, const map<string,string> & tmp_map ) const{
		//TODO
		return 0;
	}
};
class FeatureFuncCross: public FeatureFunc{
public:
	virtual int run(const vector<string> & depends, vector<string> & results, const map<string,string> & tmp_map) const {
		if (check_depends(2, depends, tmp_map) != 0){
			return -1;
		}
		results.push_back(tmp_map.find(depends.at(0))->second + "#" + tmp_map.find(depends.at(1))->second);
		return 0;
	}
};

class FeatureConf{
public:
	FeatureConf(string name, bool output, int prefix, const vector<string> & depends, const FeatureFunc * func) : 
		_name(name), _output(output), _prefix(prefix), _depends(depends), _func(func) , _order(-1){}
	static FeatureConf * create_feature_conf(const string & raw, map<string, FeatureFunc*> & func_map) {
		LOG("xxx");
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
		split(depends, depends_str, is_any_of(","));
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
	string to_string() const {
		string depends  = boost::algorithm::join(_depends, ",");
		return _name + " " + depends + " order:" + std::to_string(_order);
	}
	string _name;
	bool _output;
	int _prefix;
	vector<string> _depends;
	const FeatureFunc * _func;		
	int _order; // -1 unset; -2 waiting depends
	vector<string> _childs;
};
class FeatureStore{
public:
	FeatureStore(const FeatureConf * fc) : _fc(fc) {}
	int run(map<string,string> & tmp_map) {
		int ret = _fc->_func->run(_fc->_depends, _results, tmp_map);
		if (ret != 0) {
			return ret;
		}
		if (!_fc->_childs.empty()) {
			tmp_map[_fc->_name] =  boost::algorithm::join(_results, ",");
		}
		return 0;
	}
	vector<string> _results;
	const FeatureConf * _fc;
};
class WhiteBoard{
public:
	void init(const vector<FeatureConf*> & fcs) {
		for(size_t i = 0; i < fcs.size(); ++i) {
			_feature_stores.push_back(FeatureStore(fcs.at(i)));
		}
	}
	void run(){
		for (int i = 0; i < _feature_stores.size(); ++ i ) {
			auto & store =  _feature_stores.at(i);
			store.run(_tmp_map);
		}
	}
	void output(vector<long> & features){
	}
	string output_detail() const {
		vector<string> features;
		for (const auto & store: _feature_stores) {
			features.push_back(store._fc->_name + ":" + boost::algorithm::join(store._results, ","))	;
		}
		return boost::algorithm::join(features, " ");
	}
	map<string,string> _tmp_map;
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
		_func_map["base"] = new FeatureFuncBase;
		_func_map["cross"] = new FeatureFuncCross;
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
				FeatureConf * conf = FeatureConf::create_feature_conf(tmp, _func_map);
				LOG("xxx");
				if (conf == NULL) {
					LOG("xxx");
					LOG("create_feature_conf fail." << tmp);
					return -1;
				}
				_fc_index_map[conf->_name] = _fcs.size();

				_fcs.push_back(conf);
				LOG("add feature name:" << conf->_name);
			}
			LOG("content:" << tmp);

		}
		if (determin_execute_order() != 0 ) {
			LOG("determin_execute_order fail" );
			return -1;
		}
		return 0;
	}

	int make_features(WhiteBoard & wb);
	int recursive(int i, 
			vector<FeatureConf*> & fcs, 
			const map<string, int> & fc_index_map, 
			vector<FeatureConf*> & fcs_ordered,
			vector<string> & depend_link,
			const string & child
			) {
		FeatureConf* from = fcs.at(i);
		if (!child.empty()) {
			from->_childs.push_back(child);
		}
		LOG("dump feature:" << from->to_string());
		if (from->_order >= 0) {
			return 0;
		}
		depend_link.push_back(from->_name);
		if (from->_order == -2) {
			LOG("exist loop. exist. loop:" << boost::algorithm::join(depend_link, ","));
		}
		const vector<string> & depends  = from->_depends;
		for (int j = 0; j < depends.size(); ++j) {
			const string & depend = depends.at(j);
			//if (depend.compare(0, depend.size(), "i_") == 0) {
			if (strncmp(depend.c_str(), "i_", strlen("i_")) == 0) {
				continue;
			}

			//LOG("from:" << from->_name << " to:" << fcs.at(it->second)->_name);
			LOG("from:" << from->_name << " to:" << depend);
			map<string, int>::const_iterator it = fc_index_map.find(depend);
			if(it == fc_index_map.end()) {
				LOG("not found depend:" << depends.at(j) << " from:" << from->_name );
				return -1;
			}
			if (fcs.at(it->second)->_order >=0){
				continue;
			}
			if (recursive(it->second, fcs, fc_index_map, fcs_ordered,  depend_link, from->_name ) !=0) {
				LOG("recursive fail");
				return -1;
			}
		}
		from->_order = fcs_ordered.size();
		fcs_ordered.push_back(from);
		depend_link.pop_back();
		return 0;
	}
	int determin_execute_order() {
		vector<string> depend_link;
		for (int i = 0; i < _fcs.size(); ++i) {
			if (recursive(i, _fcs, _fc_index_map, _fcs_ordered, depend_link, "")  != 0 ){
				return -1;
			}
		}
		dump_feature(_fcs);
		return 0;
	}
	void dump_feature(vector<FeatureConf*> & fcs) {
		for(const auto & i:  fcs ){
			LOG("fc:" << i->to_string());
		}
	}
private:
	map<string, int> _input_conf;
	map<string, FeatureFunc*> _func_map;
	vector<FeatureConf*> _fcs;
	vector<FeatureConf*> _fcs_ordered;
	map<string, int> _fc_index_map; 
	
};
int FeatureMaker::make_features(WhiteBoard & wb) {
	wb.init(_fcs_ordered);
	wb.run();
	//wb.output(features);
	LOG("total output => " << wb.output_detail());
	return 0;
}
