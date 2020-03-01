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

SDL_Texture *load_texture_from_png_file(SDL_Renderer *renderer, const char *image_filename)
{
    SDL_Surface *image_surface =
        load_png_file_as_surface(image_filename);

    SDL_Texture *image_texture =
        sec(SDL_CreateTextureFromSurface(renderer,
                                         image_surface));
    SDL_FreeSurface(image_surface);

    return image_texture;
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
