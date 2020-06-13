#include "something_item.hpp"

void Item::update(float delta_time)
{
    a = fmodf(a + ITEM_OSC_FREQ * delta_time, 2 * PI);
}

void Item::render(SDL_Renderer *renderer, Camera camera) const
{
    if (type != ITEM_NONE) {
        sprite.render(renderer, camera.to_screen(pos + vec2(0.0f, sin(a) * ITEM_AMP_VALUE)));
    }
}

void Item::render_debug(SDL_Renderer *renderer, Camera camera) const
{
    if (type != ITEM_NONE) {
        auto rect = rectf_for_sdl(camera.to_screen(hitbox_world()));
        sec(SDL_SetRenderDrawColor(
                renderer,
                ITEM_DEBUG_HITBOX_COLOR.r,
                ITEM_DEBUG_HITBOX_COLOR.g,
                ITEM_DEBUG_HITBOX_COLOR.b,
                ITEM_DEBUG_HITBOX_COLOR.a));
        sec(SDL_RenderDrawRect(renderer, &rect));
    }
}

Rectf Item::hitbox_world() const
{
    return hitbox_local + pos;
}

Item make_health_item(Vec2f pos)
{
    Item item = {};
    item.pos = pos;
    item.type = ITEM_HEALTH;
    item.sprite.texture_index = texture_index_by_name("./assets/sprites/64.png"_sv);
    item.sprite.srcrect = {0, 0, 64, 64};
    item.hitbox_local = {
        ITEM_HITBOX_WIDTH * -0.5f,
        ITEM_HITBOX_HEIGHT * -0.5f,
        ITEM_HITBOX_WIDTH,
        ITEM_HITBOX_HEIGHT
    };
    item.sound = sample_s16_by_name("./assets/sounds/pop-48000.wav"_sv);

    return item;
}
