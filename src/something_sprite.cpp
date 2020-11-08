#include "./something_sprite.hpp"
#include "./something_assets.hpp"


Sprite sprite_from_texture_index(Texture_Index texture_index)
{
    Sprite result = {};
    result.texture_index = texture_index;
    sec(SDL_QueryTexture(assets.get_texture_by_index(texture_index).texture,
                         NULL,
                         NULL,
                         &result.srcrect.w,
                         &result.srcrect.h));
    return result;
}

void Sprite::render(SDL_Renderer *renderer,
                    Rectf destrect,
                    SDL_RendererFlip flip,
                    RGBA shade) const
{
    if (texture_index.unwrap < assets.textures_count) {
        SDL_Rect rect = rectf_for_sdl(destrect);
        SDL_Color sdl_shade = rgba_to_sdl(shade);

        sec(SDL_RenderCopyEx(
                renderer,
                assets.get_texture_by_index(texture_index).texture,
                &srcrect,
                &rect,
                0.0,
                nullptr,
                flip));

        sec(SDL_SetTextureColorMod(
                assets.get_texture_by_index(texture_index).texture_mask,
                sdl_shade.r, sdl_shade.g, sdl_shade.b));
        sec(SDL_SetTextureAlphaMod(
                assets.get_texture_by_index(texture_index).texture_mask,
                sdl_shade.a));

        sec(SDL_RenderCopyEx(
                renderer,
                assets.get_texture_by_index(texture_index).texture_mask,
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
                    RGBA shade) const
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
                          RGBA shade) const
{
    if (frame_count > 0) {
        frames[frame_current % frame_count].render(renderer, dstrect, flip, shade);
    }
}

void Frame_Animat::render(SDL_Renderer *renderer,
                          Vec2f pos,
                          SDL_RendererFlip flip,
                          RGBA shade) const
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
