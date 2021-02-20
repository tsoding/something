#ifndef SOMETHING_SDL_HPP_
#define SOMETHING_SDL_HPP_

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

#endif  // SOMETHING_SDL_HPP_
