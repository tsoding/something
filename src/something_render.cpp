#include "something_render.hpp"

void render_line(SDL_Renderer *renderer, Vec2f begin, Vec2f end, RGBA color)
{
    SDL_Color sdl_color = rgba_to_sdl(color);
    sec(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    sec(SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x), (int) floorf(begin.y),
            (int) floorf(end.x),   (int) floorf(end.y)));
}

void fill_rect(SDL_Renderer *renderer, Rectf rectf, RGBA color)
{
    SDL_Color sdl_color = rgba_to_sdl(color);
    sec(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    SDL_Rect rect = {
        (int) floorf(rectf.x),
        (int) floorf(rectf.y),
        (int) floorf(rectf.w),
        (int) floorf(rectf.h),
    };
    sec(SDL_RenderFillRect(renderer, &rect));
}

void draw_rect(SDL_Renderer *renderer, Rectf rectf, RGBA color)
{
    SDL_Color sdl_color = rgba_to_sdl(color);
    sec(SDL_SetRenderDrawColor(renderer, sdl_color.r, sdl_color.g, sdl_color.b, sdl_color.a));
    SDL_Rect rect = {
        (int) floorf(rectf.x),
        (int) floorf(rectf.y),
        (int) floorf(rectf.w),
        (int) floorf(rectf.h),
    };
    sec(SDL_RenderDrawRect(renderer, &rect));
}
