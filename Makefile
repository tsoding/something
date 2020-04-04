WERROR?=-Werror
PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra -Wconversion $(WERROR) -O3 -pedantic -std=c++17 -fno-exceptions -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

something: $(wildcard src/something*.cpp)
	$(CXX) $(CXXFLAGS) -o something src/something.cpp $(LIBS)
