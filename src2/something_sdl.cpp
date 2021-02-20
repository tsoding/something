#include "./something_sdl.hpp"

void fill_rect(SDL_Renderer *renderer,
               RGBA32 color,
               V2<float> pos, V2<float> size)
{
    const auto posi = pos.map(floorf).cast_to<int>();
    const auto sizei = size.map(floorf).cast_to<int>();
    const SDL_Rect rect = {
        posi.x,
        posi.y,
        sizei.x,
        sizei.y,
    };

    sec(SDL_SetRenderDrawColor(renderer, RGBA32_UNPACK(color)));
    sec(SDL_RenderFillRect(renderer, &rect));
}
