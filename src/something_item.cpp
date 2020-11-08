#include "something_item.hpp"

void Item::update(float delta_time)
{
    a = fmodf(a + ITEM_OSC_FREQ * delta_time, 2 * PI);
}

void Item::render(SDL_Renderer *renderer, Camera camera, RGBA shade) const
{
    if (type != ITEM_NONE) {
        sprite.render(
            renderer,
            camera.to_screen(texbox_world() + vec2(0.0f, sin(a) * ITEM_AMP_VALUE)),
            SDL_FLIP_NONE,
            shade);
    }
}

void Item::render_debug(SDL_Renderer *renderer, Camera camera) const
{
    if (type != ITEM_NONE) {
        auto rect = rectf_for_sdl(camera.to_screen(hitbox_world()));
        const SDL_Color item_debug_hitbox_color = rgba_to_sdl(ITEM_DEBUG_HITBOX_COLOR);
        sec(SDL_SetRenderDrawColor(
                renderer,
                item_debug_hitbox_color.r,
                item_debug_hitbox_color.g,
                item_debug_hitbox_color.b,
                item_debug_hitbox_color.a));
        sec(SDL_RenderDrawRect(renderer, &rect));
    }
}

Rectf Item::hitbox_world() const
{
    return hitbox_local + pos;
}

Rectf Item::texbox_world() const
{
    return texbox_local + pos;
}

Item make_health_item(Vec2f pos)
{
    Item item = {};
    item.pos = pos;
    item.type = ITEM_HEALTH;
    item.sprite.texture_index = HEALTH_ITEM_TEXTURE_INDEX;
    item.sprite.srcrect = {0, 0, 64, 64};
    item.hitbox_local = {
        ITEM_HITBOX_WIDTH * -0.5f,
        ITEM_HITBOX_HEIGHT * -0.5f,
        ITEM_HITBOX_WIDTH,
        ITEM_HITBOX_HEIGHT
    };
    item.texbox_local = {
        ITEM_TEXBOX_WIDTH * -0.5f,
        ITEM_TEXBOX_HEIGHT * -0.5f,
        ITEM_TEXBOX_WIDTH,
        ITEM_TEXBOX_HEIGHT
    };
    item.sound = POP_SOUND_INDEX;

    return item;
}

Item make_dirt_block_item(Vec2f pos)
{
    Item item = {};
    item.pos = pos;
    item.type = ITEM_DIRT_BLOCK;
    item.sprite = tile_defs[TILE_DIRT_0].top_texture;
    item.hitbox_local = {
        ITEM_HITBOX_WIDTH * -0.5f,
        ITEM_HITBOX_HEIGHT * -0.5f,
        ITEM_HITBOX_WIDTH,
        ITEM_HITBOX_HEIGHT
    };
    item.texbox_local = {
        ITEM_TEXBOX_WIDTH * -0.5f,
        ITEM_TEXBOX_HEIGHT * -0.5f,
        ITEM_TEXBOX_WIDTH,
        ITEM_TEXBOX_HEIGHT
    };
    item.sound = POP_SOUND_INDEX;
    return item;
}

Item make_ice_block_item(Vec2f pos)
{
    Item item = {};
    item.pos = pos;
    item.type = ITEM_ICE_BLOCK;
    item.sprite = tile_defs[TILE_ICE_0].top_texture;
    item.hitbox_local = {
        ITEM_HITBOX_WIDTH * -0.5f,
        ITEM_HITBOX_HEIGHT * -0.5f,
        ITEM_HITBOX_WIDTH,
        ITEM_HITBOX_HEIGHT
    };
    item.texbox_local = {
        ITEM_TEXBOX_WIDTH * -0.5f,
        ITEM_TEXBOX_HEIGHT * -0.5f,
        ITEM_TEXBOX_WIDTH,
        ITEM_TEXBOX_HEIGHT
    };
    item.sound = POP_SOUND_INDEX;
    return item;
}
