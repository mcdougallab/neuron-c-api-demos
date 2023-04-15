ifeq ($(OS),Windows_NT)
    UNAME_S := Windows
else
    UNAME_S := $(shell uname -s)
endif


ifeq ($(UNAME_S),Linux)
	CXXFLAGS = -g -rdynamic -Wl,--no-as-needed -ldl
endif
ifeq ($(UNAME_S),Darwin)
	CXXFLAGS = -g -rdynamic -ldl -Wno-c++11-extensions
endif

# TODO: handle Windows (mingw?)

plotshape: plotshape.cpp ../neuron_api_headers.h
	$(CXX) $(CXXFLAGS) plotshape.cpp -o plotshape
