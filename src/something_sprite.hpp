#ifndef SOMETHING_SPRITE_HPP_
#define SOMETHING_SPRITE_HPP_

struct Sprite
{
    SDL_Rect srcrect;
    SDL_Texture *texture;

    void render(SDL_Renderer *renderer,
                Rectf destrect,
                SDL_RendererFlip flip = SDL_FLIP_NONE) const;
    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE) const;
};

#endif  // SOMETHING_SPRITE_HPP_
