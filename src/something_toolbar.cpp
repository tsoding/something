#include "something_toolbar.hpp"

Rectf Toolbar::button_hitbox(size_t button, Camera camera)
{
    const Vec2f position = {
        TOOLBAR_BUTTON_PADDING,
        camera.height - TOOLBAR_BUTTON_PADDING - TOOLBAR_BUTTON_HEIGHT
    };

    const Rectf hitbox = {
        position.x + button * (TOOLBAR_BUTTON_WIDTH + TOOLBAR_BUTTON_PADDING),
        position.y,
        TOOLBAR_BUTTON_WIDTH,
        TOOLBAR_BUTTON_HEIGHT
    };

    return hitbox;
}

void Toolbar::render(SDL_Renderer *renderer, Camera camera)
{
    for (size_t i = 0; i < buttons_count; ++i) {
        SDL_Color shade = {};

        if (i != active_button) {
            shade = TOOLBAR_INACTIVE_SHADE;
        }

        auto hitbox = button_hitbox(i, camera);
        const auto shade_rect = rectf_for_sdl(hitbox);

        sec(SDL_SetRenderDrawColor(
                renderer,
                TOOLBAR_BUTTON_COLOR.r,
                TOOLBAR_BUTTON_COLOR.g,
                TOOLBAR_BUTTON_COLOR.b,
                TOOLBAR_BUTTON_COLOR.a));
        sec(SDL_RenderFillRect(renderer, &shade_rect));

        buttons[i].icon.render(renderer, rect_shrink(hitbox, TOOLBAR_BUTTON_ICON_PADDING));

        sec(SDL_SetRenderDrawColor(
                renderer,
                shade.r,
                shade.g,
                shade.b,
                shade.a));
        sec(SDL_RenderFillRect(renderer, &shade_rect));
    }
}

bool Toolbar::handle_click_at(Vec2f position, Camera camera)
{
    for (size_t i = 0; i < buttons_count; ++i) {
        if (rect_contains_vec2(button_hitbox(i, camera), position)) {
            active_button = i;
            return true;
        }
    }
    return false;
}

bool Toolbar::handle_mouse_hover(Vec2f position, Camera camera)
{
    for (size_t i = 0; i < buttons_count; ++i) {
        if (rect_contains_vec2(button_hitbox(i, camera), position)) {
            hovered_button = {true, i};
            return true;
        }
    }

    hovered_button = {};
    return false;
}
