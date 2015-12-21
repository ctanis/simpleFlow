-include make.local

CXX ?= g++
CXXFLAGS ?= -fopenmp -O3
LDFLAGS ?= -lgomp

SRC=main.cpp simpleFlow.cpp
HDRS=simpleFlow.hpp
OBJS=$(SRC:.cpp=.o)

all: Make.depend flowtest


%.o: %.cpp 
	$(CXX) -c $(CXXFLAGS) -I$(ROOT)/include ${1} $<

flowtest: main.o simpleFlow.o
	$(CXX) -fopenmp  -o $@ $+  $(LDFLAGS)

clean:
	-rm -f $(OBJS) Make.depend




# for flymake syntax checking
.PHONY: check-syntax
check-syntax:
	$(CXX) -fopenmp -Wall -Wno-long-long -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)


Make.depend: $(SRC)
	$(CXX) $(CXXFLAGS) -M $+ > Make.depend

-include Make.depend
