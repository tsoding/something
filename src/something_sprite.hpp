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
                SDL_RendererFlip flip = SDL_FLIP_NONE) const;

    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE) const;

    void update(float dt);
};
#endif  // SOMETHING_SPRITE_HPP_
