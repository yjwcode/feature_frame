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
	LOG("Test case start");
	FeatureMaker maker;
	//string path = "./test/conf/feature_maker.conf";
	string path = "/Users/yejingwei/code/work/feature_frame/test/conf/feature_maker.conf";
    REQUIRE( maker.init(path) == 0 );

}
