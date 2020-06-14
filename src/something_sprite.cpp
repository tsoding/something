#include "./something_sprite.hpp"

void Sprite::render(SDL_Renderer *renderer,
                    Rectf destrect,
                    SDL_RendererFlip flip,
                    SDL_Color shade) const
{
    if (texture_index < TEXTURE_COUNT) {
        SDL_Rect rect = rectf_for_sdl(destrect);

        sec(SDL_RenderCopyEx(
                renderer,
                textures[texture_index],
                &srcrect,
                &rect,
                0.0,
                nullptr,
                flip));

        sec(SDL_SetTextureColorMod(
                texture_masks[texture_index],
                shade.r, shade.g, shade.b));
        sec(SDL_SetTextureAlphaMod(texture_masks[texture_index], shade.a));

        sec(SDL_RenderCopyEx(
                renderer,
                texture_masks[texture_index],
                &srcrect,
                &rect,
                0.0,
                nullptr,
                flip));
    }
}

void Sprite::render(SDL_Renderer *renderer,
                    Vec2f pos,
                    SDL_RendererFlip flip,
                    SDL_Color shade) const
{
    const Rectf destrect = {
        pos.x - (float) srcrect.w * 0.5f,
        pos.y - (float) srcrect.h * 0.5f,
        (float) srcrect.w,
        (float) srcrect.h
    };

    render(renderer, destrect, flip, shade);
}

void Frame_Animat::reset()
{
    frame_current = 0;
}

void Frame_Animat::render(SDL_Renderer *renderer,
                          Rectf dstrect,
                          SDL_RendererFlip flip,
                          SDL_Color shade) const
{
    if (frame_count > 0) {
        frames[frame_current % frame_count].render(renderer, dstrect, flip, shade);
    }
}

void Frame_Animat::render(SDL_Renderer *renderer,
                          Vec2f pos,
                          SDL_RendererFlip flip,
                          SDL_Color shade) const
{
    if (frame_count > 0) {
        frames[frame_current % frame_count].render(renderer, pos, flip, shade);
    }
}

void Frame_Animat::update(float dt)
{
    if (dt < frame_cooldown) {
        frame_cooldown -= dt;
    } else if (frame_count > 0) {
        frame_current = (frame_current + 1) % frame_count;
        frame_cooldown = frame_duration;
    }
}

void dump_animat(Frame_Animat animat, const char *sprite_filename, FILE *output)
{
    println(output, "sprite = ", sprite_filename);
    println(output, "count = ", animat.frame_count);
    println(output, "duration = ", animat.frame_duration);
    println(output);
    for (size_t i = 0; i < animat.frame_count; ++i) {
        println(output, "frames.", i, ".x = ", animat.frames[i].srcrect.x);
        println(output, "frames.", i, ".y = ", animat.frames[i].srcrect.y);
        println(output, "frames.", i, ".w = ", animat.frames[i].srcrect.w);
        println(output, "frames.", i, ".h = ", animat.frames[i].srcrect.h);
    }
}

void abort_parse_error(FILE *stream,
                       String_View source, String_View rest,
                       const char *prefix, const char *error)
{
    assert(stream);
    assert(source.data < rest.data);

    size_t n = (size_t) (rest.data - source.data);

    for (size_t line_number = 1; source.count; ++line_number) {
        auto line = source.chop_by_delim('\n');

        if (n <= line.count) {
            println(stream, prefix, ':', line_number, ": ", error);
            println(stream, line);
            println(stream, Pad {n, ' '}, '^');
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

struct Rubber_Animat
{
    float begin;
    float end;
    float duration;
    float t;

    Rectf transform_rect(Rectf texbox, Vec2f pos) const
    {
        const float offset = begin + (end - begin) * (t / duration);
        const float w = texbox.w + offset * texbox.h;
        const float h = texbox.h - offset * texbox.h;
        return {pos.x - w * 0.5f, pos.y + (texbox.h * 0.5f) - h, w, h};
    }

    void update(float dt)
    {
        if (!finished()) t += dt;
    }

    bool finished() const
    {
        return t >= duration;
    }

    void reset()
    {
        t = 0.0f;
    }
};

template <size_t N>
struct Compose_Rubber_Animat
{
    Rubber_Animat rubber_animats[N];
    size_t current;

    Rectf transform_rect(Rectf texbox, Vec2f pos) const
    {
        return rubber_animats[min(current, N - 1)].transform_rect(texbox, pos);
    }

    void update(float dt)
    {
        if (finished()) return;
        if (rubber_animats[current].finished()) current += 1;
        if (finished()) return;
        rubber_animats[current].update(dt);
    }

    bool finished() const
    {
        return current >= N;
    }

    void reset()
    {
        current = 0;
        for (size_t i = 0; i < N; ++i) {
            rubber_animats[i].reset();
        }
    }
};

// TODO(#52): Replace Squash_Animat with Rubber_Animat
struct Squash_Animat
{
    Sprite sprite;
    float duration;
    float a;

    void render(SDL_Renderer *renderer,
                Vec2f pos,
                Rectf texbox,
                SDL_RendererFlip flip = SDL_FLIP_NONE) const
    {
        const float w = texbox.w + texbox.w * a;
        const float h = texbox.h * (1.0f - a);
        Rectf dstrect = {pos.x - w * 0.5f, pos.y + (texbox.h * 0.5f) - h, w, h};
        sprite.render(renderer, dstrect, flip);
    }

    void update(float dt)
    {
        a += dt / duration;
    }
};

struct Frame_Animat_File {
    const char *file_path;
    Frame_Animat animat;
};

Frame_Animat_File frame_animat_files[] = {
    {"./assets/animats/idle.txt", {}},
    {"./assets/animats/plasma_bolt.txt", {}},
    {"./assets/animats/plasma_pop.txt", {}},
    {"./assets/animats/walking.txt", {}},
    {"./assets/animats/tsodinw.txt", {}}
};
const size_t frame_animat_files_count = sizeof(frame_animat_files) / sizeof(frame_animat_files[0]);

Frame_Animat frame_animat_by_name(String_View file_path)
{
    for (size_t i = 0; i < frame_animat_files_count; ++i) {
        if (file_path == cstr_as_string_view(frame_animat_files[i].file_path)) {
            return frame_animat_files[i].animat;
        }
    }

#ifndef SOMETHING_RELEASE
    println(stderr,
            "Could not find animat `", file_path, "`. ",
            "Did you forget to add it to `frame_animat_files` list?");
    abort();
#endif // SOMETHING_RELEASE

    return {};
}

Frame_Animat load_animat_file(const char *animat_filepath)
{
    String_View source = file_as_string_view(animat_filepath);
    String_View input = source;
    Frame_Animat animat = {};
    Maybe<size_t> spritesheet_texture = {};

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

            auto count_result = value.as_integer<int>();
            if (!count_result.has_value) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "`count` is not a number");
            }

            animat.frame_count = (size_t) count_result.unwrap;
            animat.frames = new Sprite[animat.frame_count];
        } else if (subkey == "sprite"_sv) {
            spritesheet_texture = {true, texture_index_by_name(value)};
        } else if (subkey == "duration"_sv) {
            auto result = value.as_integer<int>();
            if (!result.has_value) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "`duration` is not a number");
            }

            animat.frame_duration = (float) result.unwrap / 1000.0f;
        } else if (subkey == "frames"_sv) {
            auto result = key.chop_by_delim('.').trim().as_integer<int>();
            if (!result.has_value) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "frame index is not a number");
            }

            size_t frame_index = (size_t) result.unwrap;
            if (frame_index >= animat.frame_count) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "frame index is bigger than the `count`");
            }

            if (!spritesheet_texture.has_value) {
                abort_parse_error(stderr, source, input, animat_filepath,
                                  "spritesheet was not loaded");
            }

            animat.frames[frame_index].texture_index = spritesheet_texture.unwrap;

            while (key.count) {
                subkey = key.chop_by_delim('.').trim();

                if (key.count != 0) {
                    abort_parse_error(stderr, source, input, animat_filepath,
                                      "unknown subkey");
                }

                auto result_value = value.as_integer<int>();
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

void load_frame_animat_files()
{
    for (size_t i = 0; i < frame_animat_files_count; ++i) {
        frame_animat_files[i].animat = load_animat_file(frame_animat_files[i].file_path);
    }
}
