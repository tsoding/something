WERROR?=-Werror
PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra $(WERROR) -pedantic -std=c++17 -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

something: src/scu.cpp src/main.cpp src/vec.cpp src/sprite.cpp src/error.cpp src/projectile.cpp src/level.cpp src/entity.cpp
	$(CXX) $(CXXFLAGS) -o something src/scu.cpp $(LIBS)
