WERROR?=-Werror
PKGS=sdl2
# TODO(#118): try to enable -Weverything and disable things that don't make sense
CFLAGS=-Wall -Wextra $(WERROR) -pedantic -I.
CXXFLAGS=$(CFLAGS) -std=c++17 -fno-exceptions -Wswitch-enum `pkg-config --cflags $(PKGS)`
CXXFLAGS_DEBUG=$(CXXFLAGS) -O0 -fno-builtin -ggdb
CXXFLAGS_RELEASE=$(CXXFLAGS) -DSOMETHING_RELEASE -O3 -ggdb
LIBS=`pkg-config --libs $(PKGS) ` -lm

.PHONY: all
all: something.debug something.release

something.debug: $(wildcard src/something*.cpp) $(wildcard src/something*.hpp) stb_image.o config_types.hpp
	$(CXX) $(CXXFLAGS_DEBUG) -o something.debug src/something.cpp stb_image.o $(LIBS)

something.release: $(wildcard src/something*.cpp) $(wildcard src/something*.hpp) baked_config.hpp
	$(CXX) $(CXXFLAGS_RELEASE) -o something.release src/something.cpp $(LIBS)

stb_image.o: src/stb_image.h
	$(CC) $(CFLAGS) -x c -ggdb -DSTBI_ONLY_PNG -DSTB_IMAGE_IMPLEMENTATION -c -o stb_image.o src/stb_image.h

baked_config.hpp: config_baker ./assets/config.vars
	./config_baker > baked_config.hpp

config_baker: src/config_baker.cpp src/common_config.cpp config_types.hpp
	$(CXX) $(CXXFLAGS_DEBUG) -o config_baker src/config_baker.cpp

config_types.hpp: config_typer ./assets/config.vars
	./config_typer ./assets/config.vars > config_types.hpp

config_typer: src/config_typer.cpp
	$(CXX) $(CXXFLAGS_DEBUG) -o config_typer src/config_typer.cpp
