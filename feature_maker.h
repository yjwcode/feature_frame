#pragma once
//#include <stdio>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/stat.h> 
#include <fcntl.h>
#include "mmhash.h"

#define FF_LOG(msg) \
	    std::cout << __FILE__ << ":" << __FUNCTION__<< "(" << __LINE__ << "): " << msg << std::dec << std::endl 
#define FF_WARN(msg) \
	    std::cerr << __FILE__ << ":" << __FUNCTION__<< "(" << __LINE__ << "): " << msg << std::dec << std::endl 
using namespace std;
class FeatureConf;

class FeatureFunc{
public:
	virtual int run(const vector<string> & parents, vector<string> & results, const map<string,string> & tmp_map) const { 		return 0;
	};
	virtual int run_with_value(const vector<string> & parents, 
			vector<string> & result_parts, 
			vector<float> & result_values,
			const map<string,string> & tmp_map ) const{
		return 0;
	}
	virtual bool check(const vector<string> & parents) const {return true;}
	virtual bool is_binary() const {
		return true;
	}
	virtual string get_name() const = 0;
	virtual ~FeatureFunc(){};
	virtual int check_parents(int expect_cnt, const vector<string> & parents, const map<string,string> & tmp_map) const {
		if (parents.size() != expect_cnt) {
			FF_WARN("fail. parents size " << parents.size());
			return -1;
		}
		for (const auto & d : parents) {
			if (!tmp_map.count(d))  {
				FF_WARN("not found parent:" << d);
				return -1;
			}
		}
		return 0;
	}
};
class FeatureFuncDirect: public FeatureFunc{
public:
	virtual int run(const vector<string> & parents, vector<string> & result_parts, const map<string,string> & tmp_map ) const{
		if (check_parents(1, parents, tmp_map) != 0){
			return -1;
		}
		result_parts.push_back(tmp_map.find(parents.at(0))->second);
		return 0;
	}
	virtual string get_name() const {
		return "direct";
	}

};
class FeatureFuncMap: public FeatureFunc{
public:
	virtual string get_name() const {
		return "map";
	}
	//virtual int run(const vector<string> & parents, vector<string> & result_parts, const map<string,string> & tmp_map ) const{
	//	if (check_parents(1, parents, tmp_map) != 0){
	//		return -1;
	//	}
	//	using namespace boost::algorithm;
	//	std::vector<std::string> segs;
	//	split(segs, parents.at(0), is_any_of(","));
	//	for (auto & seg: segs)	{
	//		std::vector<std::string> kv;
	//		split(kv, parents.at(0), is_any_of(","));
	//		if (kv.size() == 2) {
	//			result_parts.push_back(tmp_map.find(parents.at(0))->second);
	//		} else {
	//			FF_WARN("invalid data kv:" << seg << " total:" << parents.at(0));
	//		}
	//	}	
	//	return 0;
	//}

	virtual int run_with_value(const vector<string> & parents, 
			vector<string> & result_parts, 
			vector<float> & result_values,
			const map<string,string> & tmp_map ) const{
		if (check_parents(1, parents, tmp_map) != 0){
			return -1;
		}
		using namespace boost::algorithm;
		std::vector<std::string> segs;
		split(segs, tmp_map.find(parents.at(0))->second, is_any_of(","));
		for (auto & seg: segs)	{
			std::vector<std::string> kv;
			split(kv, seg, is_any_of(":"));
			if (kv.size() == 2) {
				result_parts.push_back(kv.at(0));
				result_values.push_back(boost::lexical_cast<float>(kv.at(1)));
			} else {
				FF_WARN("invalid data kv:" << seg << " total:" << tmp_map.find(parents.at(0))->second);
			}
		}	
		return 0;
	}
	virtual bool is_binary() const {
		return false;
	}
};
class FeatureFuncDirectList: public FeatureFunc{
public:
	virtual string get_name() const {
		return "direct_list";
	}
	virtual int run(const vector<string> & parents, vector<string> & result_parts, const map<string,string> & tmp_map ) const{
		//TODO
		return 0;
	}
};
class FeatureFuncCross: public FeatureFunc{
public:
	virtual string get_name() const {
		return "cross";
	}
	virtual int run(const vector<string> & parents, vector<string> & result_parts, const map<string,string> & tmp_map) const {
		FF_WARN("begin FeatureFuncCross run");

		if (check_parents(2, parents, tmp_map) != 0){
			return -1;
		}
		result_parts.push_back(tmp_map.find(parents.at(0))->second + "#" + tmp_map.find(parents.at(1))->second);
		FF_WARN("end FeatureFuncCross run");
		return 0;
	}
};

inline uint16_t parse_hex(const string & h){
	//std::stringstream str;
	//uint16_t value;
	//str << std::hex <<  h;
	//str >> value;
	uint32_t hexNumber;
	sscanf(h.c_str(), "%x", &hexNumber);
	FF_WARN("h:" << h << hex << " value:" << hexNumber);
	return hexNumber;
}
class FeatureConf{
public:
	FeatureConf(string name, bool output, uint16_t field, const vector<string> & parents, const FeatureFunc * func) : 
		_name(name), _output(output), _field(field), _parents(parents), _func(func) , _order(-1){}
	static FeatureConf * create_feature_conf(const string & raw, map<string, FeatureFunc*> & func_map) {
		FF_WARN("xxx");
		using namespace boost::algorithm;
		std::vector<std::string> v;
		split(v, raw, is_any_of("|"));
		if(v.size() != 5) {
			FF_WARN("invalid input:" << raw << " size:" << v.size());
			return NULL;
		}
		string name = boost::trim_copy(v.at(0)); 
		bool output = boost::lexical_cast<bool>(boost::trim_copy(v.at(1)));
		FF_WARN("!!!!out:" << output);
		//int field = boost::lexical_cast<int>(boost::trim_copy(v.at(2)));
		uint16_t field = parse_hex(boost::trim_copy(v.at(2)));
		string parents_str = boost::trim_copy(v.at(4));
		if(parents_str == "SAME") {
			parents_str = "i_" + name;
		} 
		vector<string> parents;
		split(parents, parents_str, is_any_of(","));
		string func_name = boost::trim_copy(v.at(3));
		if (!func_map.count(func_name)) {
			FF_WARN("not found" << func_name);
			return NULL;
		}
		const FeatureFunc * func = func_map[func_name];
		if (!func->check(parents))  {
			FF_WARN("check fail:" << func_name);
			return NULL;
		}
		return new FeatureConf(name, output, field, parents, func);
	}	
	int is_valid(){
		if (!_func->is_binary() && _childs.empty() ) {
			FF_WARN("invalid. no_binary feature should not have child. " << to_string());
			return -1;
		} else {
			return 0;
		}
	}
	string to_string() const {
		string parents  = boost::algorithm::join(_parents, ",");
		ostringstream oss;
		oss 
			<< " order:" << std::to_string(_order) 
			<< " name:" << _name 
			<< " parents:[" << parents << "]"
			<< " out:" << _output 
			<< " childs:[" << boost::algorithm::join(_childs, ",") << "]";
		return oss.str();
	}
	string _name;
	bool _output;
	uint16_t _field;
	vector<string> _parents;
	const FeatureFunc * _func;		
	int _order; // -1 unset; -2 waiting parents
	vector<string> _childs;
};
class FeatureInstance{
public:
	FeatureInstance(const FeatureConf * fc) : _fc(fc) , _hash(0){}
	int run(map<string,string> & tmp_map) {
		int ret = 0;
		if (_fc->_func->is_binary()) {
			_fc->_func->run(_fc->_parents, _result_parts, tmp_map);
		} else {
			_fc->_func->run_with_value(_fc->_parents, _result_parts,_result_values, tmp_map);
		}
		if (ret != 0) {
			return ret;
		}

		FF_WARN("begin " << _fc->_name << " result_part.size:" 
				<< _result_parts.size() << " output:" << _fc->_output
				<< " child:" << _fc->_childs.size()
				);
		if (!_result_parts.empty()){
			if (_fc->_output) {
				for (auto & part : _result_parts) {
					uint64_t hash = MurMurHash::Hash(part);	
					uint64_t tmp_field = _fc->_field;
					FF_WARN("hash-------->" << hex << hash << " field:" << tmp_field);
					hash = (hash & 0xFFFFFFFF0000FFFF ) | (tmp_field << 16);
					_hashes.push_back(hash);
				}
			}
			_result =  boost::algorithm::join(_result_parts, ",");
			if (!_fc->_childs.empty()) {//TODO
				tmp_map[_fc->_name] = _result;
			}
		}
		FF_WARN("end:" << _result_parts.size());

		return 0;
	}

	string to_string() const {
		ostringstream oss;
		oss <<  _fc->to_string() + ", instance result:" << _result 
			<< " result_parts:" << _result_parts.size() 
			<< " hashes:["  ;
		bool first = true;
		for (auto h : _hashes) {
			if (!first) {
				oss << ",";
			}
			oss << hex << h ;
			first = false;
		}
		oss << "]";
		return oss.str();
	}

	vector<string> _result_parts;
	vector<float> _result_values;
	string _result;
	const FeatureConf * _fc;
	uint64_t _hash;
	vector<uint64_t> _hashes;
};
class WhiteBoard{
public:
	WhiteBoard():_label(-1){}
	void init(const vector<FeatureConf*> & fcs) {
		for(size_t i = 0; i < fcs.size(); ++i) {
			_feature_instances.push_back(FeatureInstance(fcs.at(i)));
		}
	}
	void run(){
		for (int i = 0; i < _feature_instances.size(); ++ i ) {
			auto & instance =  _feature_instances.at(i);
			instance.run(_tmp_map);
		}
	}
	void get(map<string, uint64_t> & content){
		for (const auto & instance: _feature_instances) {
			content[instance._fc->_name] = instance._hash;
			FF_WARN("get " << instance.to_string() );
			//FF_WARN("get instance._fc->_name:" << instance._fc->to_string() << " hash:" << hex <<  instance._hash  );
			//features.push_back(instance._fc->_name + ":" + boost::algorithm::join(instance._result_parts, ","))	;
		}
	}
	void get_sample(string & line) {
		ostringstream oss;	
		oss << _label ;
		for (const auto & instance: _feature_instances) {
			for (int i = 0; i < instance._hashes.size(); ++i) {
				oss << " " << hex << instance._hashes.at(i) << dec;
				if (!instance._fc->_func->is_binary() and i <instance._result_values.size()) {
					oss << ":" << instance._result_values.at(i);
				}
			}
		}
		line = oss.str();
	}
	string output_detail() const {
		vector<string> features;
		for (const auto & instance: _feature_instances) {
			features.push_back(instance._fc->_name + ":" + boost::algorithm::join(instance._result_parts, ","))	;
		}
		return boost::algorithm::join(features, " ");
	}
	map<string,string> _tmp_map;
	vector<FeatureInstance> _feature_instances;
	map<string,string> _input_map;
	int _label;
};

inline int parse_input_conf(const string & conf, map<string, int> & conf_map){
	using namespace boost::algorithm;
	std::vector<std::string> v;
	split(v, conf, is_any_of(","));
	for (int i = 0; i < v.size(); ++i) {
		boost::trim(v.at(i));
		conf_map["i_" + v.at(i)] = i;
		FF_WARN("conf " << v.at(i) << " i:"<< i);
	}
	return 0;
}
class FeatureMaker{
public:
	FeatureMaker () {
		{FeatureFunc * func =  new FeatureFuncDirect; _func_map[func->get_name()] = func;}
		{FeatureFunc * func =  new FeatureFuncCross; _func_map[func->get_name()] = func;}
		{FeatureFunc * func =  new FeatureFuncMap; _func_map[func->get_name()] = func;}
	}

	int init(const string & path){
		FILE * fp = fopen(path.c_str(), "r");
		if (!fp ) {
			FF_WARN("open fail. file:" << path.c_str());
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
				FF_WARN("xxx");
				if (conf == NULL) {
					FF_WARN("xxx");
					FF_WARN("create_feature_conf fail." << tmp);
					return -1;
				}
				_fc_index_map[conf->_name] = _fcs_orig.size();

				_fcs_orig.push_back(conf);
				FF_WARN("add feature name:" << conf->_name);
			}
			FF_WARN("content:" << tmp);

		}
		if (determin_execute_order() != 0 ) {
			FF_WARN("determin_execute_order fail !!!" );
			return -1;
		}
		return 0;
	}

	int make_features(WhiteBoard & wb);
	int recursive(int cur_idx, 
			vector<FeatureConf*> & fcs, 
			const map<string, int> & fc_index_map, 
			vector<FeatureConf*> & fcs_ordered,
			vector<string> & parent_link,
			const string & child
			) {
		FeatureConf* cur = fcs.at(cur_idx);
		FF_WARN("dump feature:" << cur->to_string() << " add child:" << child);
		if (!child.empty()) {
			cur->_childs.push_back(child);
		}
		if (cur->_order >= 0) {
			return 0;
		}
		parent_link.push_back(cur->_name);
		if (cur->_order == -2) {
			FF_WARN("exist loop. exist. loop:" << boost::algorithm::join(parent_link, ","));
		}
		const vector<string> & parents  = cur->_parents;
		for (int j = 0; j < parents.size(); ++j) {
			const string & parent = parents.at(j);
			//if (parent.compare(0, parent.size(), "i_") == 0) {
			if (strncmp(parent.c_str(), "i_", strlen("i_")) == 0) {
				continue;
			}

			FF_WARN("cur:" << cur->_name << " to:" << parent);
			map<string, int>::const_iterator it = fc_index_map.find(parent);
			if(it == fc_index_map.end()) {
				FF_WARN("!!!!not found parent:" << parents.at(j) << " cur:" << cur->_name );
				return -1;
			}
			if (fcs.at(it->second)->_order >=0){
				fcs.at(it->second)->_childs.push_back(cur->_name);
				continue;
			}
			if (recursive(it->second, fcs, fc_index_map, fcs_ordered,  parent_link, cur->_name ) !=0) {
				FF_WARN("recursive fail");
				return -1;
			}
		}
		cur->_order = fcs_ordered.size();
		fcs_ordered.push_back(cur);
		parent_link.pop_back();
		return 0;
	}
	int determin_execute_order() {
		vector<string> parent_link;
		for (int i = 0; i < _fcs_orig.size(); ++i) {
			if (recursive(i, _fcs_orig, _fc_index_map, _fcs_ordered, parent_link, "")  != 0 ){
				FF_WARN("recursive fail");
				return -1;
			}
		}

		dump_feature(_fcs_ordered);
		return 0;
	}
	
	void dump_feature(vector<FeatureConf*> & fcs) {
		for(const auto & i:  fcs ){
			FF_WARN("fc:" << i->to_string());
		}
	}
private:
	map<string, int> _input_conf;
	map<string, FeatureFunc*> _func_map;
	vector<FeatureConf*> _fcs_orig;
	vector<FeatureConf*> _fcs_ordered;
	map<string, int> _fc_index_map; 
	
};

int FeatureMaker::make_features(WhiteBoard & wb) {
	wb.init(_fcs_ordered);
	wb.run();
	FF_WARN("total output => " << wb.output_detail());
	map<string, uint64_t> out;
	wb.get(out);
	string sample;
	wb.get_sample(sample);
	FF_WARN("sample:" << sample);
	cout << sample << endl;

	//uint64_t l = MurMurHash::MurMur3_32("abx", 1);
	//uint64_t h = MurMurHash::MurMur3_32("abx", 2);
	//uint64_t c = MurMurHash::Hash("abx");
	//cout << "l:"  << hex << l << endl;
	//cout << "h:"  << hex << h << endl;
	//cout << "c:"  << hex << c << endl;

	//uint64_t c = ( (h << 32 ) | l ) & 0xFFFFFFFF ;
	//uint64_t c =  (h << 32 ) | l  ;
	//uint64_t d =( ( (h << 32 ) | l ) & 0xFFFFFFFF00000000  ) >> 32;
	//FF_WARN("l:" << l << " h:" << h << " c:" << c);	

	return 0;
}
