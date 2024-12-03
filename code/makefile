# simple makefile
#
# macro definitions
# $@ 	= the target
# $?	= all pre-requisites newer than the target
# #^	= all pre-requisites (dependencies)
#

GTEST=~/dev/googletest
CXXFLAGS = -std=c++17 -Wall -O3 -g -I $(GTEST)/googletest/include
LDFLAGS = -L $(GTEST)/lib -lgtest -lgtest_main -lpthread

# by default, the first target is the default target
#
# build the executable from the binary objects

mtest: 001_memory_sizes.o 002_explicit_ctr.o
	$(CXX) $^ -o $@ $(LDFLAGS)
# g++ -o m03 memory_tests.o 
#
# build all binary objects 

001_memory_sizes.o: 001_memory_sizes.cpp
002_explicit_ctr.o: 002_explicit_ctr.cpp
# use make's default rule for building cpp into o
#	 $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $^ -o $@


clean:
	rm -f *.o mtest

