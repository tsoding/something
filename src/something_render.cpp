#include "something_render.hpp"

void render_line(SDL_Renderer *renderer, Vec2f begin, Vec2f end, SDL_Color color)
{
    sec(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a));
    sec(SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x), (int) floorf(begin.y),
            (int) floorf(end.x),   (int) floorf(end.y)));
}

void fill_rect(SDL_Renderer *renderer, Rectf rectf, SDL_Color color)
{
    sec(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a));
    SDL_Rect rect = {
        (int) floorf(rectf.x),
        (int) floorf(rectf.y),
        (int) floorf(rectf.w),
        (int) floorf(rectf.h),
    };
    sec(SDL_RenderFillRect(renderer, &rect));
}
