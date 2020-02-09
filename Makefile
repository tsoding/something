PKGS=sdl2 libpng
CFLAGS=-Wall -Wextra -pedantic -std=c++17 -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

something: main.cpp
	gcc $(CFLAGS) -o something main.cpp $(LIBS)
