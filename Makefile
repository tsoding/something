PKGS=sdl2
CFLAGS=-Wall -Wextra -pedantic -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

something: main.cpp
	gcc $(CFLAGS) -o something main.cpp $(LIBS)
