#include "something_color.hpp"

HSLA RGBA::to_hsla() const
{
    const float max = fmaxf(r, fmaxf(g, b));
    const float min = fminf(r, fminf(g, b));
    const float c = max - min;
    float hue = 0.0f;
    float saturation = 0.0f;
    const float lightness = (max + min) * 0.5f;

    if (fabsf(c) > 1e-6) {
        if (fabs(max - r) <= 1e-6) {
            hue = 60.0f * fmodf((g - b) / c, 6.0f);
        } else if (fabs(max - g) <= 1e-6) {
            hue = 60.0f * ((b - r) / c + 2.0f);
        } else {
            hue = 60.0f * ((r - g) / c + 4.0f);
        }

        saturation = c / (1.0f - fabsf(2.0f * lightness - 1.0f));
    }

    return HSLA {hue, saturation, lightness, a};
}

RGBA HSLA::to_rgba() const
{
    const float norm_h = fmodf(h, 360.0f);

    const float c = (1.0f - fabsf(2.0f * l - 1.0f)) * s;
    const float x = c * (1 - fabsf(fmodf(norm_h / 60.0f, 2.0f) - 1.0f));
    const float m = l - c / 2.0f;

    RGBA rgba = {0.0f, 0.0f, 0.0f, a};

    if (0.0f <= norm_h && norm_h < 60.0f) {
        rgba = {c, x, 0.0f, a};
    } else if (60.0f <= norm_h && norm_h < 120.0f) {
        rgba = {x, c, 0.0f, a};
    } else if (120.0f <= norm_h && norm_h < 180.0f) {
        rgba = {0.0f, c, x, a};
    } else if (180.0f <= norm_h && norm_h < 240.0f) {
        rgba = {0.0f, x, c, a};
    } else if (240.0f <= norm_h && norm_h < 300.0f) {
        rgba = {x, 0.0f, c, a};
    } else if (300.0f <= norm_h && norm_h < 360.0f) {
        rgba = {c, 0.0f, x, a};
    }

    rgba.r += m;
    rgba.g += m;
    rgba.b += m;

    return rgba;
}

RGBA sdl_to_rgba(SDL_Color sdl_color)
{
    RGBA rgba = {};
    rgba.r = (float) sdl_color.r / 255.0f;
    rgba.g = (float) sdl_color.g / 255.0f;
    rgba.b = (float) sdl_color.b / 255.0f;
    rgba.a = (float) sdl_color.a / 255.0f;
    return rgba;
}

SDL_Color rgba_to_sdl(RGBA rgba)
{
    SDL_Color sdl_color = {};
    sdl_color.r = (Uint8) roundf(rgba.r * 255.0f);
    sdl_color.g = (Uint8) roundf(rgba.g * 255.0f);
    sdl_color.b = (Uint8) roundf(rgba.b * 255.0f);
    sdl_color.a = (Uint8) roundf(rgba.a * 255.0f);
    return sdl_color;
}
