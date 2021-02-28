#!/bin/sh

set -xe

CXX="${CXX:-g++}"
PKGS="sdl2 glew"
CXXFLAGS="-Wall -Wextra -std=c++17 -pedantic -fno-exceptions"

rm -rf build/

mkdir -p build/assets/

$CXX $CXXFLAGS `pkg-config --cflags $PKGS` -o build/something2.debug something.cpp `pkg-config --libs $PKGS`
