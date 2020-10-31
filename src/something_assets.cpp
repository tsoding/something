#include "./something_assets.hpp"

String_View Assets::load_file_into_conf_buffer(const char *filepath)
{
    FILE *conf_file = fopen(filepath, "rb");
    if (!conf_file) {
        println(stderr, "Could not load file `", filepath, "`: ", strerror(errno));
        exit(1);
    }
    defer(fclose(conf_file));

    if (fseek(conf_file, 0, SEEK_END) != 0) {
        println(stderr, "Could not load file `", filepath, "`: ", strerror(errno));
        exit(1);
    }

    long m = ftell(conf_file);
    if (m < 0) {
        println(stderr, "Could not load file `", filepath, "`: ", strerror(errno));
        exit(1);
    }

    if ((size_t) m > ASSETS_CONF_BUFFER_CAPACITY) {
        println(stderr, "Could not fit file `", filepath, "` into the memory");
        exit(1);
    }

    if (fseek(conf_file, 0, SEEK_SET) != 0) {
        println(stderr, "Could not load file `", filepath, "`: ", strerror(errno));
        exit(1);
    }

    size_t n = fread(conf_buffer, 1, m, conf_file);
    if (ferror(conf_file)) {
        println(stderr, "Could not load file `", filepath, "`: ", strerror(errno));
        exit(1);
    }

    assert(n == (size_t) m);

    return {(size_t) m, conf_buffer};
}

void Assets::load_texture(SDL_Renderer *renderer, String_View id, String_View path)
{
    assert(textures_count < ASSETS_TEXTURES_CAPACITY);

    println(stdout, "Loading texture ", id, " from ", path, "...");

    Texture asset = {};
    asset.surface = load_png_file_as_surface(path);
    asset.texture = sec(SDL_CreateTextureFromSurface(renderer, asset.surface));
    asset.surface_mask = load_png_file_as_surface(path);

    sec(SDL_LockSurface(asset.surface_mask));
    assert(asset.surface_mask->format->format == SDL_PIXELFORMAT_RGBA32);
    for (int row = 0; row < asset.surface_mask->h; ++row) {
        uint32_t *pixel_row = (uint32_t*) ((uint8_t *) asset.surface_mask->pixels + row * asset.surface_mask->pitch);
        for (int col = 0; col < asset.surface_mask->w; ++col) {
            pixel_row[col] = pixel_row[col] | 0x00FFFFFF;
        }
    }
    SDL_UnlockSurface(asset.surface_mask);

    asset.texture_mask = sec(SDL_CreateTextureFromSurface(renderer, asset.surface_mask));

    textures[textures_count].id = id;
    textures[textures_count].path = path;
    textures[textures_count].unwrap = asset;
    textures_count += 1;
}

void Assets::load_sound(String_View id, String_View path)
{
    println(stdout, "Loading sound ", id, " from ", path, "...");
    sounds[sounds_count].id = id;
    sounds[sounds_count].path = path;
    sounds[sounds_count].unwrap = load_wav_as_sample_s16(path);
    sounds_count += 1;
}

void Assets::load_animat(String_View, String_View)
{
    assert(0 && "TODO: Assets::load_animat is not implemented");
}

void Assets::load_conf(SDL_Renderer *renderer, const char *filepath)
{
    String_View input = load_file_into_conf_buffer(filepath);

    while (input.count > 0) {
        String_View line = input.chop_by_delim('\n').trim();
        if (line.count == 0) continue;

        String_View asset_type = line.chop_by_delim('[').trim();
        String_View asset_id = line.chop_by_delim(']').trim();
        line.chop_by_delim('=');
        String_View asset_path = line.trim();

        if (asset_type == "textures"_sv) {
            load_texture(renderer, asset_id, asset_path);
        } else if (asset_type == "sounds"_sv) {
            load_sound(asset_id, asset_path);
        } else if (asset_type == "animats"_sv) {
            // load_animat(asset_id, asset_path);
        } else {
            println(stderr, "Unknown asset type `", asset_type, "`");
            exit(1);
        }
    }
}
