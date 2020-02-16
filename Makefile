WERROR?=-Werror
PKGS=sdl2 libpng
CFLAGS=-Wall -Wextra $(WERROR) -pedantic -std=c++17 -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

something: main.cpp
	$(CXX) $(CFLAGS) -o something main.cpp $(LIBS)
