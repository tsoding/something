#!/bin/sh

set -xe

CXX="${CXX:-g++}"
PKGS="sdl2 glew"
CXXFLAGS="-Wall -Wextra -std=c++17 -pedantic -fno-exceptions"

rm -rf build/

mkdir -p build/assets/

$CXX $CXXFLAGS -DTESTING -o build/atlas_packer_test atlas_packer.cpp
./build/atlas_packer_test

$CXX $CXXFLAGS -o build/atlas_packer atlas_packer.cpp
./build/atlas_packer -o ./build/assets/ ./assets/atlas.conf
$CXX $CXXFLAGS `pkg-config --cflags $PKGS` -o build/something2.debug something.cpp `pkg-config --libs $PKGS`
