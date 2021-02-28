#ifndef SOMETHING_TEXTURE_HPP_
#define SOMETHING_TEXTURE_HPP_


// TODO(#113): add support for mipmaps for the texture cache
SDL_Surface *load_png_file_as_surface(const char *image_filename);
SDL_Surface *load_png_file_as_surface(String_View image_filename);
SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key);

void load_textures(SDL_Renderer *renderer);

SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key);

#endif  // SOMETHING_TEXTURE_HPP_
