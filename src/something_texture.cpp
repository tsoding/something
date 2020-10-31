#include "./something_texture.hpp"

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
    if (image_pixels == NULL) {
        println(stderr, "[ERROR] Could not load `", image_filename, "` as PNG");
        abort();
    }

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

SDL_Surface *load_png_file_as_surface(String_View image_filename)
{
    char *filepath_cstr = (char*) malloc(image_filename.count + 1);
    assert(filepath_cstr != NULL);
    memcpy(filepath_cstr, image_filename.data, image_filename.count);
    filepath_cstr[image_filename.count] = '\0';
    auto result = load_png_file_as_surface(filepath_cstr);
    free(filepath_cstr);
    return result;
}
