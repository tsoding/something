#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <SDL.h>
#include <SDL_ttf.h>

#include <png.h>

void print1(FILE *stream, const char *cstr)
{
    fputs(cstr, stream);
}

template <typename... T>
void println(FILE *stream, T... args)
{
    (print1(stream, args), ...);
    fputc('\n', stream);
}

// READ THIS FIRST ---> https://en.wikipedia.org/wiki/Single_Compilation_Unit
#include "something_error.cpp"
#include "something_math.cpp"
#include "something_string.cpp"
#include "something_sprite.cpp"
#include "something_level.cpp"
#include "something_projectile.cpp"
#include "something_entity.cpp"
#include "something_main.cpp"
