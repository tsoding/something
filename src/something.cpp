#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <SDL.h>
#include <SDL_ttf.h>

#ifdef SOMETHING_RELEASE
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "./stb_image.h"

// READ THIS FIRST ---> https://en.wikipedia.org/wiki/Single_Compilation_Unit
#include "common_algo.cpp"
#include "common_print.cpp"
#include "common_string.cpp"

#include "something_math.cpp"

#ifndef SOMETHING_RELEASE
#include "common_config.cpp"
#else
#include "./baked_config.hpp"
#endif

#include "something_error.cpp"
#include "something_font.cpp"
#include "something_camera.cpp"
#include "something_sprite.cpp"
#include "something_room.cpp"
#include "something_sound.cpp"
#include "something_entity.cpp"
#include "something_game.cpp"
#include "something_main.cpp"
