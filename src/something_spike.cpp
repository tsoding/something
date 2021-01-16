#include "./something_spike.hpp"

void Spike::render(SDL_Renderer *renderer, Camera camera)
{
    auto sprite  = assets.get_by_index(sprite_index);
    auto texture = assets.get_by_index(sprite.texture_index).texture;

    switch (state) {
    case Spike_State::Active: {
        sec(SDL_SetTextureAlphaMod(texture, 255));
        const float effective_h = (float) sprite.srcrect.h * a;
        sprite.srcrect.h = (int) floorf(effective_h);
        const auto dest_rect = rect(camera.to_screen(pos - vec2((float) sprite.srcrect.w * 0.5f, effective_h * scale)), (float) sprite.srcrect.w * scale, effective_h * scale);
        sprite.render(renderer, dest_rect);
    } break;

    case Spike_State::Poof: {
        const float effective_h = (float) sprite.srcrect.h;
        const auto dest_rect = rect(camera.to_screen(pos - vec2((float) sprite.srcrect.w * 0.5f, effective_h * scale)), (float) sprite.srcrect.w * scale, effective_h * scale);
        sec(SDL_SetTextureAlphaMod(texture, (Uint8) floorf(a * 255.0f)));
        sprite.render(renderer, dest_rect);
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

Spike ice_spike(Vec2f pos,
                Index<Sprite> sprite_index,
                float scale)
{
    Spike spike = {};
    spike.pos = pos;
    spike.a = 0.0f;
    spike.sprite_index = sprite_index;
    spike.scale = scale;
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
            const auto scale_step = (SPIKE_WAVE_SCALE_HIGH - SPIKE_WAVE_SCALE_LOW) / SPIKE_WAVE_MAX_COUNT;
            pos = game->grid.find_floor(pos);
            game->spawn_spike(
                ice_spike(pos,
                          rand() % 2 == 0 ? ICE_SPIKE_1_SPRITE_INDEX : ICE_SPIKE_2_SPRITE_INDEX,
                          scale));
            scale += scale_step;
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
    this->scale = SPIKE_WAVE_SCALE_LOW;
}

// TODO(#329): there is no weapon that uses the spikes mechanics
