#ifndef SOMETHING_COLOR_HPP_
#define SOMETHING_COLOR_HPP_

struct RGBA;
struct HSLA;

struct HSLA
{
    float h, s, l, a;

    RGBA to_rgba() const;
};

struct RGBA
{
    float r, g, b, a;

    HSLA to_hsla() const;
};

RGBA sdl_to_rgba(SDL_Color sdl_color);
SDL_Color rgba_to_sdl(RGBA rgba);

#endif  // SOMETHING_COLOR_HPP_
