#ifndef SOMETHING_SPRITE_HPP_
#define SOMETHING_SPRITE_HPP_

#include "./something_index.hpp"

struct Texture
{
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Surface *surface_mask;
    SDL_Texture *texture_mask;
};

struct Sprite
{
    SDL_Rect srcrect;
    Index<Texture> texture_index;

    void render(SDL_Renderer *renderer,
                Rectf destrect,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0},
                double angle = 0.0,
                Maybe<Vec2f> pivot = {}) const;
    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0},
                double angle = 0.0,
                Maybe<Vec2f> pivot = {}) const;
};

struct Frames
{
    Sprite *sprites;
    size_t count;
    float duration;
};

struct Frames_Animat
{
    Index<Frames> frames_index;

    size_t frame_current;
    float frame_cooldown;

    void reset();

    void render(SDL_Renderer *renderer,
                Rectf dstrect,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0},
                double angle = 0.0) const;

    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                RGBA shade = {0, 0, 0, 0},
                double angle = 0.0) const;

    void update(float dt);

    bool has_finished() const;
};

Frames_Animat frames_animat(Index<Frames> frames_index)
{
    Frames_Animat result = {};
    result.frames_index = frames_index;
    return result;
}

#endif  // SOMETHING_SPRITE_HPP_
