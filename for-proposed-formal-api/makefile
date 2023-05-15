ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
else
    UNAME_S := $(shell uname -s)
endif


ifeq ($(UNAME_S),Linux)
	CXXFLAGS = -g -rdynamic -Wl,--no-as-needed -ldl -Wall
	CXXFLAGSNOLINKER = -g -Wl,--no-as-needed -ldl -Wall
endif
ifeq ($(UNAME_S),Darwin)
	CXXFLAGS = -std=c++14 -g -rdynamic -ldl -Wall
	CXXFLAGSNOLINKER = -std=c++14 -g -Wall
endif

# TODO: handle Windows (mingw?)

all: helloworld hh_sim vclamp sections

helloworld: helloworld.cpp neuron_api.o neuron_api.h
	$(CXX) $(CXXFLAGS) helloworld.cpp neuron_api.o -o helloworld

hh_sim: hh_sim.cpp neuron_api.o neuron_api.h
	$(CXX) $(CXXFLAGS) hh_sim.cpp neuron_api.o -o hh_sim

neuron_api.o: neuron_api.cpp neuron_api.h
	$(CXX) $(CXXFLAGSNOLINKER) -c neuron_api.cpp

vclamp: vclamp.cpp neuron_api.o neuron_api.h
	$(CXX) $(CXXFLAGS) vclamp.cpp neuron_api.o -o vclamp

sections: sections.cpp neuron_api.o neuron_api.h
	$(CXX) $(CXXFLAGS) sections.cpp neuron_api.o -o sections
