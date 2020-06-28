#include "something_toolbar.hpp"

static void render_tooltip(SDL_Renderer *renderer,
                           Bitmap_Font font,
                           String_View tooltip,
                           Vec2f position)
{
    const Vec2f padding = vec2(TOOLTIP_PADDING, TOOLTIP_PADDING);
    const Vec2f size = vec2(FONT_DEBUG_SIZE, FONT_DEBUG_SIZE);
    const Vec2f tooltip_box = font.text_size(size, tooltip) + padding * 2.0f;

    const SDL_Rect tooltip_rect = {
        (int) floorf(position.x),
        (int) floorf(position.y),
        (int) floorf(tooltip_box.x),
        (int) floorf(tooltip_box.y)
    };
    sec(SDL_SetRenderDrawColor(
            renderer,
            TOOLTIP_BACKGROUND_COLOR.r,
            TOOLTIP_BACKGROUND_COLOR.g,
            TOOLTIP_BACKGROUND_COLOR.b,
            TOOLTIP_BACKGROUND_COLOR.a));
    sec(SDL_RenderFillRect(renderer, &tooltip_rect));
    font.render(renderer, position + padding, size, TOOLTIP_FOREGROUND_COLOR, tooltip);
}


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

void Toolbar::render(SDL_Renderer *renderer, Camera camera, Bitmap_Font font)
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

    if (hovered_button.has_value) {
        render_tooltip(renderer, font,
                       buttons[hovered_button.unwrap].tooltip,
                       tooltip_position);
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
            tooltip_position = position;
            return true;
        }
    }

    hovered_button = {};
    return false;
}
