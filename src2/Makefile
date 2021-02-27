PKGS=sdl2 glew
CXXFLAGS=-Wall -Wextra -std=c++17 -pedantic -fno-exceptions

something2.debug: $(wildcard something*.cpp) $(wildcard something*.hpp) aids.hpp
	$(CXX) $(CXXFLAGS) `pkg-config --cflags $(PKGS)` -o something2.debug something.cpp `pkg-config --libs $(PKGS)`

atlas_packer: atlas_packer.cpp aids.hpp
	$(CXX) $(CXXFLAGS) -o atlas_packer atlas_packer.cpp
