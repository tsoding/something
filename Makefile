WERROR?=-Werror
PKGS=sdl2
CFLAGS=-Wall -Wextra $(WERROR) -pedantic -I.
CXXFLAGS_WITHOUT_PKGS=$(CFLAGS) -std=c++17 -fno-exceptions -Wno-missing-braces -Wswitch-enum
ifdef OS # https://stackoverflow.com/questions/4058840/makefile-that-distincts-between-windows-and-unix-like-systems	
	CXXFLAGS=$(CXXFLAGS_WITHOUT_PKGS) $(shell pkg-config --cflags $(PKGS))
	LIBS=$(shell pkg-config --libs $(PKGS)) -lm
	EXESUFF=.exe
else
	CXXFLAGS=$(CXXFLAGS_WITHOUT_PKGS) `pkg-config --cflags $(PKGS)`
	LIBS=`pkg-config --libs $(PKGS) ` -lm
	EXESUFF=
endif
CXXFLAGS_DEBUG=$(CXXFLAGS) -O0 -fno-builtin -ggdb
CXXFLAGS_WITHOUT_PKGS_DEBUG=$(CXXFLAGS_WITHOUT_PKGS) -O0 -fno-builtin -ggdb
CXXFLAGS_RELEASE=$(CXXFLAGS) -DSOMETHING_RELEASE -O3 -ggdb

.PHONY: all
all: something.debug something.release

something.debug: $(wildcard src/something*.cpp) $(wildcard src/something*.hpp) stb_image.o config_types.hpp assets_types.hpp
	$(CXX) $(CXXFLAGS_DEBUG) -o $(addsuffix $(EXESUFF),something.debug) src/something.cpp stb_image.o $(LIBS)

something.release: $(wildcard src/something*.cpp) $(wildcard src/something*.hpp) baked_config.hpp assets_types.hpp
	$(CXX) $(CXXFLAGS_RELEASE) -o $(addsuffix $(EXESUFF),something.release) src/something.cpp $(LIBS)

stb_image.o: src/stb_image.h
	$(CC) $(CFLAGS) -x c -ggdb -DSTBI_ONLY_PNG -DSTB_IMAGE_IMPLEMENTATION -c -o stb_image.o src/stb_image.h

baked_config.hpp: config_baker ./assets/vars.conf
	"./config_baker" > baked_config.hpp

config_baker: src/config_baker.cpp src/config_common.cpp config_types.hpp
	$(CXX) $(CXXFLAGS_DEBUG) -o config_baker src/config_baker.cpp $(LIBS)

config_types.hpp: config_typer ./assets/vars.conf
	"./config_typer" ./assets/vars.conf > config_types.hpp

config_typer: src/config_typer.cpp
	$(CXX) $(CXXFLAGS_WITHOUT_PKGS_DEBUG) -o config_typer src/config_typer.cpp

assets_types.hpp: assets_typer ./assets/assets.conf
	"./assets_typer" ./assets/assets.conf > assets_types.hpp

assets_typer: src/assets_typer.cpp
	$(CXX) $(CXXFLAGS_WITHOUT_PKGS_DEBUG) -o assets_typer src/assets_typer.cpp
