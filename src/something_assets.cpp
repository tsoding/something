#include "./something_assets.hpp"

Assets assets = {};

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

Maybe<String_View> read_file_as_string_view(String_View filename)
{
    char *filename_cstr = (char*) malloc(filename.count + 1);
    defer(free(filename_cstr));
    memcpy(filename_cstr, filename.data, filename.count);
    filename_cstr[filename.count] = '\0';
    return read_file_as_string_view(filename_cstr);
}

void Assets::load_animat(String_View id, String_View path)
{
    println(stdout, "Loading animat ", id, " from ", path, "...");

    auto source = read_file_as_string_view(path);
    if (!source.has_value) {
        println(stderr, "Could not load animation file: `", path, "`");
        exit(1);
    }
    defer(free((void*) source.unwrap.data));

    String_View input = source.unwrap;
    Frame_Animat animat = {};
    Maybe<Texture_Index> spritesheet_texture = {};

    for (int line_number = 1; input.count != 0; ++line_number) {
        auto value = input.chop_by_delim('\n');
        auto key = value.chop_by_delim('=').trim();
        if (key.count == 0 || *key.data == '#') continue;
        value = value.trim();

        auto subkey = key.chop_by_delim('.').trim();

        if (subkey == "count"_sv) {
            if (animat.frames != nullptr) {
                println(stderr, path, ":", line_number, ": `count` provided twice");
                exit(1);
            }

            auto count_result = value.as_integer<int>();
            if (!count_result.has_value) {
                println(stderr, path, ":", line_number, ": `count` is not a number");
                exit(1);
            }

            animat.frame_count = (size_t) count_result.unwrap;
            animat.frames = new Sprite[animat.frame_count];
        } else if (subkey == "texture"_sv) {
            auto maybe_texture = get_texture_by_id(value);
            if (!maybe_texture.has_value) {
                println(stderr, path, ":", line_number, ": could not find a texture by id `", value, "`");
                exit(1);
            }

            spritesheet_texture = maybe_texture;
        } else if (subkey == "duration"_sv) {
            auto result = value.as_integer<int>();
            if (!result.has_value) {
                println(stderr, path, ":", line_number, ": `duration` is not a number");
                exit(1);
            }

            animat.frame_duration = (float) result.unwrap / 1000.0f;
        } else if (subkey == "frames"_sv) {
            auto result = key.chop_by_delim('.').trim().as_integer<int>();
            if (!result.has_value) {
                println(stderr, path, ":", line_number, ": frame index is not a number");
                exit(1);
            }

            size_t frame_index = (size_t) result.unwrap;
            if (frame_index >= animat.frame_count) {
                println(stderr, path, ":", line_number, ": frame index is bigger than the `count`");
                exit(1);
            }

            if (!spritesheet_texture.has_value) {
                println(stderr, path, ":", line_number, ": spritesheet was not loaded");
                exit(1);
            }

            animat.frames[frame_index].texture_index = spritesheet_texture.unwrap;

            while (key.count) {
                subkey = key.chop_by_delim('.').trim();

                if (key.count != 0) {
                    println(stderr, path, ":", line_number, ": unknown subkey `", subkey, "`");
                    exit(1);
                }

                auto result_value = value.as_integer<int>();
                if (!result_value.has_value) {
                    println(stderr, path, ":", line_number, ": value is not a number");
                    exit(1);
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
                    println(stderr, path, ":", line_number, ": unknown subkey `", subkey, "`");
                    exit(1);
                }
            }
        } else {
            println(stderr, path, ":", line_number, ": unknown subkey `", subkey, "`");
            exit(1);
        }
    }

    animats[animats_count].id = id;
    animats[animats_count].path = path;
    animats[animats_count].unwrap = animat;
    animats_count += 1;
}

Maybe<Sample_S16> Assets::get_sound_by_id(String_View id)
{
    for (size_t i = 0; i < sounds_count; ++i) {
        if (sounds[i].id == id) {
            return {true, sounds[i].unwrap};
        }
    }
    return {};
}

Sample_S16 Assets::get_sound_by_id_or_panic(String_View id)
{
    return unwrap_or_panic(
        get_sound_by_id(id),
        "Could not find sound with id `", id, "`");
}

Maybe<Texture_Index> Assets::get_texture_by_id(String_View id)
{
    for (size_t i = 0; i < textures_count; ++i) {
        if (textures[i].id == id) {
            return {true, {i}};
        }
    }

    return {};
}

Texture_Index Assets::get_texture_by_id_or_panic(String_View id)
{
    return unwrap_or_panic(
        get_texture_by_id(id),
        "Could not find texture with id `", id, "`");
}

Maybe<Frame_Animat> Assets::get_animat_by_id(String_View id)
{
    for (size_t i = 0; i < animats_count; ++i) {
        if (animats[i].id == id) {
            return {true, animats[i].unwrap};
        }
    }

    return {};
}

Frame_Animat Assets::get_animat_by_id_or_panic(String_View id)
{
    return unwrap_or_panic(
        get_animat_by_id(id),
        "Could not find animat with id `", id, "`");
}

void Assets::load_conf(SDL_Renderer *renderer, const char *filepath)
{
    String_View input = load_file_into_conf_buffer(filepath);

    // TODO(#251): assets.conf does not support comments
    // TODO(#252): there is no way to reload assets at runtime
    // TODO(#253): release data pack building based on assets.conf

    while (input.count > 0) {
        String_View line = input.chop_by_delim('\n').trim();

        if (line.count == 0) continue; // Skip empty lines
        if (*line.data == '#') continue; // Skip single line comments

        String_View asset_type = line.chop_by_delim('[').trim();
        String_View asset_id = line.chop_by_delim(']').trim();
        line.chop_by_delim('=');
        String_View asset_path = line.chop_by_delim('#').trim();

        if (asset_type == "textures"_sv) {
            load_texture(renderer, asset_id, asset_path);
        } else if (asset_type == "sounds"_sv) {
            load_sound(asset_id, asset_path);
        } else if (asset_type == "animats"_sv) {
            load_animat(asset_id, asset_path);
        } else {
            println(stderr, "Unknown asset type `", asset_type, "`");
            exit(1);
        }
    }
}
