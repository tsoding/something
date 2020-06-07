#include "./something_texture.hpp"

void load_textures(SDL_Renderer *renderer)
{
    for (size_t i = 0; i < TEXTURE_COUNT; ++i) {
        if (textures[i] == nullptr) {
            SDL_Surface *image_surface = load_png_file_as_surface(texture_files[i]);

            textures[i] = sec(SDL_CreateTextureFromSurface(renderer,
                                                               image_surface));
            sec(SDL_LockSurface(image_surface));
            assert(image_surface->format->format == SDL_PIXELFORMAT_RGBA32);
            for (int row = 0; row < image_surface->h; ++row) {
                uint32_t *pixel_row = (uint32_t*) ((uint8_t *) image_surface->pixels + row * image_surface->pitch);
                for (int col = 0; col < image_surface->w; ++col) {
                    pixel_row[col] = pixel_row[col] | 0x00FFFFFF;
                }
            }
            SDL_UnlockSurface(image_surface);

            texture_masks[i] =
                sec(SDL_CreateTextureFromSurface(renderer, image_surface));

            SDL_FreeSurface(image_surface);
        }
    }
}

size_t texture_index_by_name(String_View filename)
{
    for (size_t i = 0; i < TEXTURE_COUNT; ++i) {
        if (filename == cstr_as_string_view(texture_files[i])) {
            return i;
        }
    }

    println(stderr,
            "[ERROR] Unknown texture file `", filename, "`. ",
            "You may want to add it to the `textures` array.");
    abort();
    return 0;
}

SDL_Texture *load_texture_from_bmp_file(SDL_Renderer *renderer,
                                        const char *image_filepath,
                                        SDL_Color color_key)
{
    SDL_Surface *image_surface = sec(SDL_LoadBMP(image_filepath));

    sec(SDL_SetColorKey(
            image_surface,
            SDL_TRUE,
            SDL_MapRGB(
                image_surface->format,
                color_key.r,
                color_key.g,
                color_key.b)));

    SDL_Texture *image_texture = sec(SDL_CreateTextureFromSurface(renderer, image_surface));
    SDL_FreeSurface(image_surface);
    return image_texture;
}

SDL_Surface *load_png_file_as_surface(const char *image_filename)
{
    int width, height;
    uint32_t *image_pixels = (uint32_t *) stbi_load(image_filename, &width, &height, NULL, 4);

    SDL_Surface* image_surface =
        sec(SDL_CreateRGBSurfaceFrom(image_pixels,
                                     (int) width,
                                     (int) height,
                                     32,
                                     (int) width * 4,
                                     0x000000FF,
                                     0x0000FF00,
                                     0x00FF0000,
                                     0xFF000000));
    return image_surface;
}
