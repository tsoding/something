#ifndef SOMETHING_SPRITE_HPP_
#define SOMETHING_SPRITE_HPP_

struct Sprite
{
    SDL_Rect srcrect;
    size_t texture_index;

    void render(SDL_Renderer *renderer,
                Rectf destrect,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                SDL_Color shade = {0, 0, 0, 0}) const;
    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                SDL_Color shade = {0, 0, 0, 0}) const;
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
                SDL_Color shade = {0, 0, 0, 0}) const;

    void render(SDL_Renderer *renderer,
                Vec2f pos,
                SDL_RendererFlip flip = SDL_FLIP_NONE,
                SDL_Color shade = {0, 0, 0, 0}) const;

    void update(float dt);
};

SDL_Surface *load_png_file_as_surface(const char *image_filename);
SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key);

const char *texture_files[] = {
    "./assets/sprites/Destroy1-sheet.png",
    "./assets/sprites/fantasy_tiles.png",
    "./assets/sprites/spark1-sheet.png",
    "./assets/sprites/walking-12px-zoom.png"
};
const size_t TEXTURE_COUNT = sizeof(texture_files) / sizeof(texture_files[0]);

SDL_Texture *textures[TEXTURE_COUNT] = {};
SDL_Texture *texture_masks[TEXTURE_COUNT] = {};

#endif  // SOMETHING_SPRITE_HPP_
