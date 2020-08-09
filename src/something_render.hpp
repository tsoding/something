#ifndef _SOMETHING_RENDER_HPP
#define _SOMETHING_RENDER_HPP

void render_line(SDL_Renderer *renderer, Vec2f begin, Vec2f end,
    SDL_Color color);

void fill_rect(SDL_Renderer *renderer, Rectf rect, SDL_Color color);

#endif // _SOMETHING_RENDER_HPP
