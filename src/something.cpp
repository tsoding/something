#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>
#include <SDL_ttf.h>

#include <png.h>

template <typename T>
T min(T a, T b)
{
    return a < b ? a : b;
}

// READ THIS FIRST ---> https://en.wikipedia.org/wiki/Single_Compilation_Unit
#include "something_error.cpp"
#include "something_vec.cpp"
#include "something_result.cpp"
#include "something_string_view.cpp"
#include "something_sprite.cpp"
#include "something_level.cpp"
#include "something_projectile.cpp"
#include "something_entity.cpp"
#include "something_main.cpp"
