WERROR?=-Werror
PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra $(WERROR) -pedantic -std=c++17 -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

something: src/main.cpp
	$(CXX) $(CXXFLAGS) -o something src/main.cpp $(LIBS)
