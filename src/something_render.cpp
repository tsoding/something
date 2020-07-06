#include "something_render.hpp"

void render_line(SDL_Renderer *renderer, Vec2f begin, Vec2f end, SDL_Color color)
{
    sec(SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a));
    sec(SDL_RenderDrawLine(
            renderer,
            (int) floorf(begin.x), (int) floorf(begin.y),
            (int) floorf(end.x),   (int) floorf(end.y)));
}
