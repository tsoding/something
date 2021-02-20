#ifndef SOMETHING_SDL_HPP_
#define SOMETHING_SDL_HPP_

#include "./something_v2.hpp"

template <typename T>
T *sec(T *ptr)
{
    if (ptr == NULL) {
        panic("SDL ERROR: ", SDL_GetError());
    }

    return ptr;
}

void sec(int code)
{
    if (code < 0) {
        panic("SDL ERROR", SDL_GetError());
    }
}

typedef Uint32 RGBA32;

#define RGBA32_UNPACK(color) \
    (color >> (8 * 0)) & 0xFF, \
    (color >> (8 * 1)) & 0xFF, \
    (color >> (8 * 2)) & 0xFF, \
    (color >> (8 * 3)) & 0xFF

typedef float Seconds;
typedef Uint32 Milliseconds;

void fill_rect(SDL_Renderer *renderer,
               RGBA32 color,
               V2<float> pos, V2<float> size);

#endif  // SOMETHING_SDL_HPP_
