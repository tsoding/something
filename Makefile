WERROR?=-Werror
PKGS=sdl2 SDL2_ttf
CXXFLAGS=-Wall -Wextra $(WERROR) -pedantic -std=c++17 -fno-exceptions -I. $(shell pkg-config --cflags $(PKGS))
CXXFLAGS_DEBUG=$(CXXFLAGS) -O0 -fno-builtin -ggdb
CXXFLAGS_RELEASE=$(CXXFLAGS) -DSOMETHING_RELEASE -O3 -ggdb
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

.PHONY: all
all: something.debug something.release

something.debug: $(wildcard src/something*.cpp) $(wildcard src/something*.hpp)
	$(CXX) $(CXXFLAGS_DEBUG) -o something.debug src/something.cpp $(LIBS)

something.release: $(wildcard src/something*.cpp) $(wildcard src/something*.hpp) baked_config.hpp
	$(CXX) $(CXXFLAGS_RELEASE) -o something.release src/something.cpp $(LIBS)

baked_config.hpp: config_baker ./assets/config.vars
	./config_baker > baked_config.hpp

config_baker: src/config_baker.cpp src/common_string.cpp src/common_config.cpp src/common_print.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -o config_baker src/config_baker.cpp
