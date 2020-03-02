struct Sprite
{
    SDL_Rect srcrect;
    SDL_Texture *texture;
};

void render_sprite(SDL_Renderer *renderer,
                   Sprite texture,
                   SDL_Rect destrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    sec(SDL_RenderCopyEx(
            renderer,
            texture.texture,
            &texture.srcrect,
            &destrect,
            0.0,
            nullptr,
            flip));
}

void render_sprite(SDL_Renderer *renderer,
                   Sprite texture,
                   Vec2i pos,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    SDL_Rect destrect = {
        pos.x - texture.srcrect.w / 2, pos.y - texture.srcrect.h / 2,
        texture.srcrect.w, texture.srcrect.h
    };

    sec(SDL_RenderCopyEx(
            renderer,
            texture.texture,
            &texture.srcrect,
            &destrect,
            0.0,
            nullptr,
            flip));
}

struct Animat
{
    Sprite  *frames;
    size_t   frame_count;
    size_t   frame_current;
    uint32_t frame_duration;
    uint32_t frame_cooldown;
};

static inline
void render_animat(SDL_Renderer *renderer,
                   Animat animat,
                   SDL_Rect dstrect,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    render_sprite(
        renderer,
        animat.frames[animat.frame_current % animat.frame_count],
        dstrect,
        flip);
}

static inline
void render_animat(SDL_Renderer *renderer,
                   Animat animat,
                   Vec2i pos,
                   SDL_RendererFlip flip = SDL_FLIP_NONE)
{
    render_sprite(
        renderer,
        animat.frames[animat.frame_current % animat.frame_count],
        pos,
        flip);
}

void update_animat(Animat *animat, uint32_t dt)
{
    assert(animat);

    if (dt < animat->frame_cooldown) {
        animat->frame_cooldown -= dt;
    } else {
        animat->frame_current = (animat->frame_current + 1) % animat->frame_count;
        animat->frame_cooldown = animat->frame_duration;
    }
}

SDL_Surface *load_png_file_as_surface(const char *image_filename)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    // TODO(#6): implement libpng error checker similar to the SDL one
    // TODO(#7): try stb_image.h instead of libpng
    //   https://github.com/nothings/stb/blob/master/stb_image.h
    if (!png_image_begin_read_from_file(&image, image_filename)) {
        fprintf(stderr, "Could not read file `%s`: %s\n",
                image_filename, image.message);
        abort();
    }
    image.format = PNG_FORMAT_RGBA;
    uint32_t *image_pixels = new uint32_t[image.width * image.height];

    if (!png_image_finish_read(&image, nullptr, image_pixels, 0, nullptr)) {
        fprintf(stderr, "libpng pooped itself: %s\n", image.message);
        abort();
    }

    SDL_Surface* image_surface =
        sec(SDL_CreateRGBSurfaceFrom(image_pixels,
                                     image.width,
                                     image.height,
                                     32,
                                     image.width * 4,
                                     0x000000FF,
                                     0x0000FF00,
                                     0x00FF0000,
                                     0xFF000000));
    return image_surface;
}

SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer,
                                        const char *image_filename)
{
    SDL_Surface *image_surface =
        load_png_file_as_surface(image_filename);

    SDL_Texture *image_texture =
        sec(SDL_CreateTextureFromSurface(renderer,
                                         image_surface));
    SDL_FreeSurface(image_surface);

    return image_texture;
}

SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer,
                                        String_View image_filename)
{
    char buffer[256] = {};
    strncpy(buffer, image_filename.data,
            min(sizeof(buffer) - 1, image_filename.count));

    return load_texture_from_png_file(renderer, buffer);
}

Animat load_spritesheet_animat(SDL_Renderer *renderer,
                               size_t frame_count,
                               uint32_t frame_duration,
                               const char *spritesheet_filepath)
{
    Animat result = {};
    result.frames = new Sprite[frame_count];
    result.frame_count = frame_count;
    result.frame_duration = frame_duration;

    SDL_Texture *spritesheet = load_texture_from_png_file(renderer, spritesheet_filepath);
    int spritesheet_w = 0;
    int spritesheet_h = 0;
    sec(SDL_QueryTexture(spritesheet, NULL, NULL, &spritesheet_w, &spritesheet_h));
    int sprite_w = spritesheet_w / frame_count;
    int sprite_h = spritesheet_h; // NOTE: we only handle horizontal sprites

    for (int i = 0; i < (int) frame_count; ++i) {
        result.frames[i].texture = spritesheet;
        result.frames[i].srcrect = {i * sprite_w, 0, sprite_w, sprite_h};
    }

    return result;
}

void dump_animat(Animat animat, const char *sprite_filename, FILE *output)
{
    fprintf(output, "sprite = %s\n", sprite_filename);
    fprintf(output, "count = %lu\n", animat.frame_count);
    fprintf(output, "duration = %u\n", animat.frame_duration);
    fprintf(output, "\n");
    for (size_t i = 0; i < animat.frame_count; ++i) {
        fprintf(output, "frames.%lu.x = %d\n", i, animat.frames[i].srcrect.x);
        fprintf(output, "frames.%lu.y = %d\n", i, animat.frames[i].srcrect.y);
        fprintf(output, "frames.%lu.w = %d\n", i, animat.frames[i].srcrect.w);
        fprintf(output, "frames.%lu.h = %d\n", i, animat.frames[i].srcrect.h);
    }
}

void abort_parse_error(FILE *stream,
                       String_View source, String_View rest,
                       const char *prefix, const char *error)
{
    assert(stream);
    assert(source.data < rest.data);

    size_t n = rest.data - source.data;

    for (size_t line_number = 1; source.count; ++line_number) {
        auto line = source.chop_by_delim('\n');

        if (n <= line.count) {
            fprintf(stream, "%s:%ld: %s\n", prefix, line_number, error);
            fwrite(line.data, 1, line.count, stream);
            fputc('\n', stream);

            for (size_t j = 0; j < n; ++j) {
                fputc(' ', stream);
            }
            fputc('^', stream);
            fputc('\n', stream);
            break;
        }

        n -= line.count + 1;
    }

    for (int i = 0; source.count && i < 3; ++i) {
        auto line = source.chop_by_delim('\n');
        fwrite(line.data, 1, line.count, stream);
        fputc('\n', stream);
    }

    abort();
}

Animat load_animat_file(SDL_Renderer *renderer, const char *animat_filepath)
{
    String_View source = file_as_string_view(animat_filepath);
    String_View input = source;
    Animat animat = {};
    SDL_Texture *spritesheet_texture = nullptr;

    while (input.count != 0) {
        auto value = input.chop_by_delim('\n');
        auto key = value.chop_by_delim('=').trim();
        if (key.count == 0 || *key.data == '#') continue;
        value = value.trim();

        auto subkey = key.chop_by_delim('.').trim();

        if (subkey == "count"_sv) {
            if (animat.frames != nullptr) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "`count` provided twice");
            }

            auto count_result = value.as_number<size_t>();
            if (!count_result.has_value) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "`count` is not a number");
            }

            animat.frame_count = count_result.unwrap;
            animat.frames = new Sprite[animat.frame_count];
        } else if (subkey == "sprite"_sv) {
            // TODO(#20): preload all of the animation sprites outside of load_animat_file
            spritesheet_texture = load_texture_from_png_file(renderer, value);
        } else if (subkey == "duration"_sv) {
            auto result = value.as_number<size_t>();
            if (!result.has_value) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "`duration` is not a number");
            }

            animat.frame_duration = result.unwrap;
        } else if (subkey == "frames"_sv) {
            auto result = key.chop_by_delim('.').trim().as_number<size_t>();
            if (!result.has_value) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "frame index is not a number");
            }

            size_t frame_index = result.unwrap;
            if (frame_index >= animat.frame_count) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "frame index is bigger than the `count`");
            }

            animat.frames[frame_index].texture = spritesheet_texture;

            while (key.count) {
                subkey = key.chop_by_delim('.').trim();

                if (key.count != 0) {
                    abort_parse_error(stderr, source, input, animat_filepath,
                                      "unknown subkey");
                }

                auto result_value = value.as_number<int>();
                if (!result_value.has_value) {
                    abort_parse_error(stderr, source, input, animat_filepath,
                                      "value is not a number");
                }

                if (subkey == "x"_sv) {
                    animat.frames[frame_index].srcrect.x = result_value.unwrap;
                } else if (subkey == "y"_sv) {
                    animat.frames[frame_index].srcrect.y = result_value.unwrap;
                } else if (subkey == "w"_sv) {
                    animat.frames[frame_index].srcrect.w = result_value.unwrap;
                } else if (subkey == "h"_sv) {
                    animat.frames[frame_index].srcrect.h = result_value.unwrap;
                } else {
                    abort_parse_error(stderr, source, input, animat_filepath,
                                      "unknown subkey");
                }
            }
        } else {
            abort_parse_error(stderr, source, input, animat_filepath,
                              "unknown subkey");
        }
    }

    delete[] source.data;

    return animat;
}
