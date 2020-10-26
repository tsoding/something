#ifndef SOMETHING_TEXTURE_HPP_
#define SOMETHING_TEXTURE_HPP_

const char *texture_files[] = {
    "./assets/sprites/Destroy1-sheet.png",
    "./assets/sprites/fantasy_tiles.png",
    "./assets/sprites/spark1-sheet.png",
    "./assets/sprites/walking-12px-zoom.png",
    "./assets/sprites/64.png",
    "./assets/sprites/tsodinw.png",
    "./assets/sprites/parallax-forest-back-trees.png",
    "./assets/sprites/parallax-forest-middle-trees.png",
    "./assets/sprites/parallax-forest-front-trees.png",
    "./assets/sprites/parallax-forest-lights.png",
    "./assets/sprites/golem.png",
    "./assets/sprites/ice.png",
    "./assets/sprites/walking-ice-golem-48px.png",
};
const size_t TEXTURE_COUNT = sizeof(texture_files) / sizeof(texture_files[0]);

struct Texture_Index
{
    size_t unwrap;
};

// TODO(#113): add support for mipmaps for the texture cache

SDL_Texture *textures[TEXTURE_COUNT] = {};
SDL_Surface *surfaces[TEXTURE_COUNT] = {};
SDL_Texture *texture_masks[TEXTURE_COUNT] = {};
SDL_Surface *surface_masks[TEXTURE_COUNT] = {};

SDL_Surface *load_png_file_as_surface(const char *image_filename);
SDL_Surface *load_png_file_as_surface(String_View image_filename);
SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key);

void load_textures(SDL_Renderer *renderer);
Texture_Index texture_index_by_name(String_View filename);

SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key);

#endif  // SOMETHING_TEXTURE_HPP_
