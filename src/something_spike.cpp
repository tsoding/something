#include "./something_spike.hpp"

void Spike::render(SDL_Renderer *renderer, Camera camera)
{
    auto texture = assets.get_texture_by_index(texture_index).texture;

    int texture_w = 0, texture_h = 0;
    sec(SDL_QueryTexture(texture, NULL, NULL, &texture_w, &texture_h));

    switch (state) {
    case Spike_State::Active: {
        sec(SDL_SetTextureAlphaMod(texture, 255));
        const float effective_h = (float) texture_h * a;
        const auto src_rect =
            rectf_for_sdl(rect(vec2(0.0f, 0.0f), (float) texture_w, effective_h));
        const auto dest_rect =
            rectf_for_sdl(rect(camera.to_screen(pos - vec2((float) texture_w * 0.5f, effective_h * scale)), (float) texture_w * scale, effective_h * scale));
        sec(SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect));
    } break;

    case Spike_State::Poof: {
        const float effective_h = (float) texture_h;
        const auto src_rect =
            rectf_for_sdl(rect(vec2(0.0f, 0.0f), (float) texture_w, effective_h));
        const auto dest_rect =
            rectf_for_sdl(rect(camera.to_screen(pos - vec2((float) texture_w * 0.5f, effective_h * scale)), (float) texture_w * scale, effective_h * scale));
        sec(SDL_SetTextureAlphaMod(texture, (Uint8) floorf(a * 255.0f)));
        sec(SDL_RenderCopy(renderer, texture, &src_rect, &dest_rect));
    } break;

    case Spike_State::Ded: {
    } break;
    }
}

void Spike::update(float dt)
{
    switch (state) {
    case Spike_State::Active: {
        a = a + ICE_SPIKE_APPEAR_SPEED * dt;
        if (a >= 1.0f) {
            state = Spike_State::Poof;
            a = 1.0f;
        }
    } break;

    case Spike_State::Poof: {
        a = a - ICE_SPIKE_POOF_SPEED * dt;
        if (a <= 0.0f) {
            state = Spike_State::Ded;
            a = 0.0f;
        }
    } break;

    case Spike_State::Ded: {
    } break;
    }
}

void Spike::activate()
{
    state = Spike_State::Active;
    a = 0.0f;
}

Spike ice_spike(Vec2f pos)
{
    Spike spike = {};
    spike.pos = pos;
    spike.a = 0.0f;
    spike.texture_index = ICE_SPIKE_INDEX;
    spike.scale = 2.0f;
    return spike;
}

// TODO(#340): Spike Waves don't deal any damage
// TODO(#341): Spike Wave goes through the tiles
//   It should follow the tile surface

void Spike_Wave::update(float dt, Game *game)
{
    if (count > 0) {
        cooldown -= dt;
        if (cooldown <= 0.0f) {
            game->spawn_spike(ice_spike(pos));
            pos += dir;
            cooldown = SPIKE_WAVE_COOLDOWN;
            count -= 1;
        }
    }
}

void Spike_Wave::activate(Vec2f pos, Vec2f dir)
{
    this->pos = pos;
    this->dir = dir;
    this->count = SPIKE_WAVE_MAX_COUNT;
    this->cooldown = 0.0f;
}

// TODO(#329): there is no weapon that uses the spikes mechanics
