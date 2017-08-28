test.exe: test/ff_test.cpp feature_maker.h
	g++  -o $@ test/ff_test.cpp
