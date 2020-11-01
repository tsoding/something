#ifndef SOMETHING_SPRITE_HPP_
#define SOMETHING_SPRITE_HPP_

#include "./something_index.hpp"

struct Sprite
{
    SDL_Rect srcrect;
    Texture_Index texture_index;

    void render(SDL_Renderer *renderer,
                Rectf destrect,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0}) const;
    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0}) const;
};

struct Frame_Animat
{
    Sprite *frames;
    size_t  frame_count;
    size_t  frame_current;
    float frame_duration;
    float frame_cooldown;

    void reset();

    void render(SDL_Renderer *renderer,
                Rectf dstrect,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0}) const;

    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0}) const;

    void update(float dt);
};


#endif  // SOMETHING_SPRITE_HPP_
