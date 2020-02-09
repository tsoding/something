PKGS=sdl2
CFLAGS=-Wall -Wextra -pedantic -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

'New folder 1': main.cpp
	gcc $(CFLAGS) -o 'New folder 1' main.cpp $(LIBS)
