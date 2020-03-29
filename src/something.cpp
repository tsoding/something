#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cmath>
#include <SDL.h>
#include <SDL_ttf.h>

#include <algorithm>

#include <png.h>

void print1(FILE *stream, char c)
{
    fputc(c, stream);
}

void print1(FILE *stream, int x)
{
    fprintf(stream, "%d", x);
}

void print1(FILE *stream, float x)
{
    fprintf(stream, "%f", x);
}

void print1(FILE *stream, long unsigned int x)
{
    fprintf(stream, "%lu", x);
}

void print1(FILE *stream, uint32_t x)
{
    fprintf(stream, "%u", x);
}

void print1(FILE *stream, const char *cstr)
{
    fputs(cstr, stream);
}

struct Pad
{
    size_t n;
    char c;
};

void print1(FILE *stream, Pad pad)
{
    for (size_t i = 0; i < pad.n; ++i) {
        fputc(pad.c, stream);
    }
}

template <typename... T>
void print([[maybe_unused]] FILE *stream, T... args)
{
    (print1(stream, args), ...);
}

template <typename... T>
void println(FILE *stream, T... args)
{
    print(stream, args...);
    fputc('\n', stream);
}

// READ THIS FIRST ---> https://en.wikipedia.org/wiki/Single_Compilation_Unit
#include "something_error.cpp"
#include "something_math.cpp"

// TODO(#31): free camera support is not implemented
struct Camera
{
    Vec2f pos;
};

#include "something_string.cpp"
#include "something_sprite.cpp"
#include "something_room.cpp"
#include "something_entity.cpp"
#include "something_projectile.cpp"
#include "something_main.cpp"
