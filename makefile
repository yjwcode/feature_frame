BASE_FLAGS = -m32  -std=c++11 -lpthread -ldl

# INCLUDE BASE DIRECTORY AND BOOST DIRECTORY FOR HEADERS
LDFLAGS = -I/usr/local/Cellar/boost/1.53.0/include -I/opt/local/include

# INCLUDE BASE DIRECTORY AND BOOST DIRECTORY FOR LIB FILES
LLIBFLAGS = -L/usr/local/Cellar/boost/1.53.0/lib

# SPECIFIY LINK OPTIONS
#LINKFLAGS = -l boost_thread-mt -lboost_system
LINKFLAGS =  -lboost_system-mt 

# FINAL FLAGS -- TO BE USED THROUGHOUT
FLAGS = $(BASE_FLAGS) $(LLIBFLAGS) $(LDFLAGS) $(LINKFLAGS)

test.exe: test/ff_test.cpp feature_maker.h
	g++ $(FLAGS) -o $@ test/ff_test.cpp

