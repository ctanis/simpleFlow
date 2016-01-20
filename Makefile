-include make.$(MODE)
MODE ?= gcc
CXX ?= gcc
CXXFLAGS ?= -fopenmp -O3
LDFLAGS ?= -lgomp
EXE ?= flowtest.$(MODE)

SRC=main.cpp simpleFlow.cpp
HDRS=simpleFlow.hpp
OBJS=$(SRC:.cpp=.o)

all: Make.depend $(EXE)


%.o: %.cpp 
	$(CXX) -c $(CXXFLAGS) -I$(ROOT)/include ${1} $<

$(EXE): main.o simpleFlow.o
	$(CXX)  -o $@ $+  $(LDFLAGS)

clean:
	-rm -f $(OBJS) flowtest* Make.depend


# for flymake syntax checking
.PHONY: check-syntax
check-syntax:
	$(CXX) -fopenmp -Wall -Wno-long-long -Wextra -pedantic -fsyntax-only $(CHK_SOURCES)


Make.depend: $(SRC)
	$(CXX) $(CXXFLAGS) -M $+ > Make.depend

-include Make.depend
