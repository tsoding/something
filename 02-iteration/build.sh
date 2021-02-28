#!/bin/sh

set -xe

CXX="${CXX:-g++}"
PKGS="sdl2 glew"
CXXFLAGS="-Wall -Wextra -std=c++17 -pedantic -fno-exceptions -ggdb"

rm -rf build/

mkdir -p build/

$CXX $CXXFLAGS `pkg-config --cflags $PKGS` -o build/something2.debug src/something.cpp `pkg-config --libs $PKGS`
