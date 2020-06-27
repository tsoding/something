#include "something_toolbar.hpp"

Rectf Toolbar::button_hitbox(Button button, Camera camera)
{
    const Vec2f position = {
        TOOLBAR_BUTTON_PADDING,
        camera.height - TOOLBAR_BUTTON_PADDING - TOOLBAR_BUTTON_HEIGHT
    };

    const Rectf hitbox = {
        position.x + (int) button * (TOOLBAR_BUTTON_WIDTH + TOOLBAR_BUTTON_PADDING),
        position.y,
        TOOLBAR_BUTTON_WIDTH,
        TOOLBAR_BUTTON_HEIGHT
    };

    return hitbox;
}

void Toolbar::render(SDL_Renderer *renderer, Camera camera)
{
    Sprite heals_sprite =
        sprite_from_texture_index(
            texture_index_by_name(
                TOOLBAR_BUTTON_TEXTURE));

    Sprite tiles_sprite = {};
    tiles_sprite.texture_index = texture_index_by_name("./assets/sprites/fantasy_tiles.png"_sv);
    // TODO: move tiles srcrect dimention to config.vars
    //   That may require add a new type to the config file.
    //   Might be a good opportunity to simplify adding new types to the system.
    tiles_sprite.srcrect = {120, 128, 16, 16};

    // TODO: Toolbar buttons should have tooltips explaining buttons' purpose

    for (int i = 0; i < Button_Count; ++i) {
        SDL_Color shade = {};

        if (i != (int) current_button) {
            shade = TOOLBAR_INACTIVE_SHADE;
        }

        switch ((Button) i) {
        case Tiles: {
            tiles_sprite.render(renderer, button_hitbox((Button) i, camera), SDL_FLIP_NONE, shade);
        } break;

        case Heals: {
            heals_sprite.render(renderer, button_hitbox((Button) i, camera), SDL_FLIP_NONE, shade);
        } break;

        case Button_Count:
        default: {}
        }
    }
}

bool Toolbar::handle_click_at(Vec2f position, Camera camera)
{
    for (int i = 0; i < Button_Count; ++i) {
        if (rect_contains_vec2(button_hitbox((Button) i, camera), position)) {
            current_button = (Button) i;
            return true;
        }
    }
    return false;
}
