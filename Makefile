WERROR?=-Werror
PKGS=sdl2 libpng SDL2_ttf
CXXFLAGS=-Wall -Wextra $(WERROR) -pedantic -std=c++17 -fno-exceptions -ggdb $(shell pkg-config --cflags $(PKGS))
LIBS=$(shell pkg-config --libs $(PKGS)) -lm

something: src/something.cpp src/something_main.cpp src/something_vec.cpp src/something_sprite.cpp src/something_error.cpp src/something_projectile.cpp src/something_level.cpp src/something_entity.cpp
	$(CXX) $(CXXFLAGS) -o something src/something.cpp $(LIBS)
