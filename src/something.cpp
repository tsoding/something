#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <SDL.h>
#include <SDL_ttf.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h"

// READ THIS FIRST ---> https://en.wikipedia.org/wiki/Single_Compilation_Unit
#include "common_print.cpp"
#include "something_math.cpp"
#include "common_string.cpp"

#ifndef SOMETHING_RELEASE
#include "common_config.cpp"
#else
#include "./baked_config.hpp"
#define CONFIG_INT(x) x
#define CONFIG_FLOAT(x) x
#define CONFIG_COLOR(x) x
#endif

#include "something_error.cpp"
#include "something_camera.cpp"
#include "something_sprite.cpp"
#include "something_room.cpp"
#include "something_sound.cpp"
#include "something_entity.cpp"
#include "something_game.cpp"
#include "something_main.cpp"
