#pragma once
//#include <stdio>
#include <vector>
#include <map>
#include <boost/algorithm/string.hpp>

#define LOG(msg) \
	    std::cout << __FILE__ << "(" << __LINE__ << "): " << msg << std::endl 
using namespace std;
class FeatureConf;

class FeatureFunc{
public:
	virtual int run(const vector<string> & depends, vector<string> & results) const ;


};
class FeatureFuncDirect{
public:
	virtual int run(const vector<string> & depends, vector<string> & results ) const{
		return 0;
	}
};
class FeatureFuncJoin{
public:
	virtual int run(const vector<string> & depends, vector<string> & results) const {
		return 0;
	}
};

class FeatureConf{
public:
	FeatureConf(bool output, int prefix, const vector<string> & depends, const FeatureFunc * func) : 
		_output(output), _prefix(prefix), _depends(depends), _func(func) {}
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

class FeatureMaker{
public:
	int init(const string & path){return 0;}
	int make_features(WhiteBoard & wb, vector<long> & features);
private:
	vector<FeatureConf> _fcs;
};
//int FeatureMaker::make_features(WhiteBoard & wb, vector<long> & features) {
//	wb.init(_fcs);
//	wb.run();
//	wb.output(features);
//}
