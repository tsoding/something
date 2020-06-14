#ifndef SOMETHING_TEXTURE_HPP_
#define SOMETHING_TEXTURE_HPP_

const char *texture_files[] = {
    "./assets/sprites/Destroy1-sheet.png",
    "./assets/sprites/fantasy_tiles.png",
    "./assets/sprites/spark1-sheet.png",
    "./assets/sprites/walking-12px-zoom.png",
    "./assets/sprites/64.png",
    "./assets/sprites/tsodinw.png"
};
const size_t TEXTURE_COUNT = sizeof(texture_files) / sizeof(texture_files[0]);

// TODO(#113): add support for mipmaps for the texture cache

SDL_Texture *textures[TEXTURE_COUNT] = {};
SDL_Texture *texture_masks[TEXTURE_COUNT] = {};

SDL_Surface *load_png_file_as_surface(const char *image_filename);
SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key);

void load_textures(SDL_Renderer *renderer);
size_t texture_index_by_name(String_View filename);

SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key);

SDL_Surface *load_png_file_as_surface(const char *image_filename);

#endif  // SOMETHING_TEXTURE_HPP_
