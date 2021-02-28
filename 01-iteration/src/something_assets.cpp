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

void Assets::load_sprite(String_View id, String_View path)
{
    println(stdout, "Loading sprite ", id, " from ", path, "...");
    auto source = read_file_as_string_view(path);
    if (!source.has_value) {
        panic("Could not load sprite file: `", path, "`");
    }
    defer(destroy(source.unwrap));

    String_View input = source.unwrap;
    Sprite result = {};

    for (size_t line_number = 1; input.count > 0; line_number += 1) {
        auto line = input.chop_by_delim('\n').trim();

        if (line.count > 0) {
            const auto key = line.chop_by_delim('=').trim();
            const auto value = line.trim();

            const auto parse_number = [&path, &line_number](String_View value) {
                return unwrap_or_panic(
                    value.as_integer<int>(),
                    path, ":", line_number, ": `", value, "` is not a number");
            };

            if (key == "texture"_sv) {
                result.texture_index = unwrap_or_panic(
                    get_texture_by_id(value),
                    path, ":", line_number, ": Unknown texture id `", value, "`");
            } else if (key == "x"_sv) {
                result.srcrect.x = parse_number(value);
            } else if (key == "y"_sv) {
                result.srcrect.y = parse_number(value);
            } else if (key == "w"_sv) {
                result.srcrect.w = parse_number(value);
            } else if (key == "h"_sv) {
                result.srcrect.h = parse_number(value);
            }
        }
    }

    sprites[sprite_count].unwrap = result;
    sprites[sprite_count].id = id;
    sprites[sprite_count].path = path;
    sprite_count += 1;
}

void Assets::load_frames(String_View id, String_View path)
{
    println(stdout, "Loading animat ", id, " from ", path, "...");

    auto source = read_file_as_string_view(path);
    if (!source.has_value) {
        println(stderr, "Could not load animation file: `", path, "`");
        exit(1);
    }
    defer(free((void*) source.unwrap.data));

    String_View input = source.unwrap;
    Frames frames = {};
    Maybe<Index<Texture>> spritesheet_texture = {};

    for (int line_number = 1; input.count != 0; ++line_number) {
        auto value = input.chop_by_delim('\n');
        auto key = value.chop_by_delim('=').trim();
        if (key.count == 0 || *key.data == '#') continue;
        value = value.trim();

        auto subkey = key.chop_by_delim('.').trim();

        if (subkey == "count"_sv) {
            if (frames.sprites != nullptr) {
                println(stderr, path, ":", line_number, ": `count` provided twice");
                exit(1);
            }

            auto count_result = value.as_integer<int>();
            if (!count_result.has_value) {
                println(stderr, path, ":", line_number, ": `count` is not a number");
                exit(1);
            }

            frames.count = (size_t) count_result.unwrap;
            frames.sprites = new Sprite[frames.count];
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

            frames.duration = (float) result.unwrap / 1000.0f;
        } else if (subkey == "frames"_sv) {
            auto result = key.chop_by_delim('.').trim().as_integer<int>();
            if (!result.has_value) {
                println(stderr, path, ":", line_number, ": frame index is not a number");
                exit(1);
            }

            size_t frame_index = (size_t) result.unwrap;
            if (frame_index >= frames.count) {
                println(stderr, path, ":", line_number, ": frame index is bigger than the `count`");
                exit(1);
            }

            if (!spritesheet_texture.has_value) {
                println(stderr, path, ":", line_number, ": spritesheet was not loaded");
                exit(1);
            }

            frames.sprites[frame_index].texture_index = spritesheet_texture.unwrap;

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
                    frames.sprites[frame_index].srcrect.x = result_value.unwrap;
                } else if (subkey == "y"_sv) {
                    frames.sprites[frame_index].srcrect.y = result_value.unwrap;
                } else if (subkey == "w"_sv) {
                    frames.sprites[frame_index].srcrect.w = result_value.unwrap;
                } else if (subkey == "h"_sv) {
                    frames.sprites[frame_index].srcrect.h = result_value.unwrap;
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

    framesen[framesen_count].id = id;
    framesen[framesen_count].path = path;
    framesen[framesen_count].unwrap = frames;
    framesen_count += 1;
}

Maybe<Index<Sample_S16>> Assets::get_sound_by_id(String_View id)
{
    for (size_t i = 0; i < sounds_count; ++i) {
        if (sounds[i].id == id) {
            return {true, {i}};
        }
    }
    return {};
}

Sample_S16 Assets::get_by_index(Index<Sample_S16> index)
{
    return sounds[index.unwrap].unwrap;
}

Maybe<Index<Texture>> Assets::get_texture_by_id(String_View id)
{
    for (size_t i = 0; i < textures_count; ++i) {
        if (textures[i].id == id) {
            return {true, {i}};
        }
    }

    return {};
}

Texture Assets::get_by_index(Index<Texture> index)
{
    return textures[index.unwrap].unwrap;
}

Maybe<Index<Sprite>> Assets::get_sprite_by_id(String_View id)
{
    for (size_t i = 0; i < sprite_count; ++i) {
        if (sprites[i].id == id) {
            return {true, i};
        }
    }

    return {};
}

Sprite Assets::get_by_index(Index<Sprite> index)
{
    return sprites[index.unwrap].unwrap;
}

Maybe<Index<Frames>> Assets::get_frames_by_id(String_View id)
{
    for (size_t i = 0; i < framesen_count; ++i) {
        if (framesen[i].id == id) {
            return {true, {i}};
        }
    }

    return {};
}

Frames Assets::get_by_index(Index<Frames> index)
{
    return framesen[index.unwrap].unwrap;
}

void Assets::clean()
{
    for (size_t i = 0; i < textures_count; ++i) {
        SDL_FreeSurface(textures[i].unwrap.surface);
        SDL_DestroyTexture(textures[i].unwrap.texture);
        SDL_FreeSurface(textures[i].unwrap.surface_mask);
        SDL_DestroyTexture(textures[i].unwrap.texture_mask);
    }
    textures_count = 0;

    for (size_t i = 0; i < sounds_count; ++i) {
        SDL_FreeWAV((Uint8*) sounds[i].unwrap.audio_buf);
    }
    sounds_count = 0;

    for (size_t i = 0; i < framesen_count; ++i) {
        delete[] framesen[i].unwrap.sprites;
    }
    framesen_count = 0;
}

void Assets::load_conf(SDL_Renderer *renderer, const char *filepath)
{
    clean();

    String_View input = load_file_into_conf_buffer(filepath);

    // TODO(#253): release data pack building based on assets.conf

    parse_vars_conf(input, [&](auto line_number, auto id, auto type, auto asset_path) {
        if (type == "texture"_sv) {
            load_texture(renderer, id, asset_path);
        } else if (type == "sound"_sv) {
            load_sound(id, asset_path);
        } else if (type == "animat"_sv) {
            load_frames(id, asset_path);
        } else if (type == "sprite"_sv) {
            load_sprite(id, asset_path);
        } else {
            println(stderr, asset_path, ":", line_number, ": ",
                    "Unknown type of asset `", type, "`");
            exit(1);
        }

        return parse_success();
    });

    loaded_first_time = true;
}
