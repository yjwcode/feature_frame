#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "../catch.hpp"
#include "../feature_maker.h"

unsigned int Factorial( unsigned int number ) {
    return number <= 1 ? number : Factorial(number-1)*number;
}

TEST_CASE( "Factorials are computed", "[factorial]" ) {
    REQUIRE( Factorial(1) == 1 );
    REQUIRE( Factorial(2) == 2 );
    REQUIRE( Factorial(3) == 6 );
    REQUIRE( Factorial(10) == 3628800 );
}

TEST_CASE( "maker", "1" ) {
	FF_LOG("Test case start");
	FeatureMaker maker;
	//string path = "./test/conf/feature_maker.conf";
	string path = "/Users/yejingwei/code/work/feature_frame/test/conf/feature_maker.conf";
    REQUIRE( maker.init(path) == 0 );

}

TEST_CASE( "maker2", "2" ) {
	FF_LOG("Test case start");
	FeatureMaker maker;
	//string path = "./test/conf/feature_maker.conf";
	string path = "/Users/yejingwei/code/work/feature_frame/test/conf/feature_maker.conf";
    REQUIRE( maker.init(path) == 0 );
	//@i_user_id,i_user_province,i_item_id,i_user_tags,
	WhiteBoard wb;
	wb._tmp_map["i_user_id"] = "1";
	wb._tmp_map["i_user_province"] = "2";
	wb._tmp_map["i_item_id"] = "3";
	wb._tmp_map["i_item_media"] = "4";
	wb._tmp_map["i_user_os"] = "android";
	wb._tmp_map["i_user_long_sc_top20"] = "1:10.1,2:20.2";
	wb._tmp_map["i_user_media_top20"] = "马鞍山生活君:0.004431848414242268,女人健身频道:0.004431848414242268";
	maker.make_features	(wb);
}
