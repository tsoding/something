#include "something_toolbar.hpp"
#include "something_game.hpp"

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
    const SDL_Color tooltip_background_color = rgba_to_sdl(TOOLTIP_BACKGROUND_COLOR);
    sec(SDL_SetRenderDrawColor(
            renderer,
            tooltip_background_color.r,
            tooltip_background_color.g,
            tooltip_background_color.b,
            tooltip_background_color.a));
    sec(SDL_RenderFillRect(renderer, &tooltip_rect));
    font.render(renderer, position + padding, size, TOOLTIP_FOREGROUND_COLOR, tooltip);
}

Rectf Toolbar::button_hitbox(size_t button)
{
    const Vec2f position = {
        TOOLBAR_BUTTON_PADDING,
        SCREEN_HEIGHT - TOOLBAR_BUTTON_PADDING - TOOLBAR_BUTTON_HEIGHT
    };

    const Rectf hitbox = {
        position.x + button * (TOOLBAR_BUTTON_WIDTH + TOOLBAR_BUTTON_PADDING),
        position.y,
        TOOLBAR_BUTTON_WIDTH,
        TOOLBAR_BUTTON_HEIGHT
    };

    return hitbox;
}

void Toolbar::render(SDL_Renderer *renderer, Bitmap_Font font)
{
    for (size_t i = 0; i < buttons_count; ++i) {
        SDL_Color shade = {};

        if (i != active_button) {
            shade = rgba_to_sdl(TOOLBAR_INACTIVE_SHADE);
        }

        auto hitbox = button_hitbox(i);
        const auto shade_rect = rectf_for_sdl(hitbox);

        const SDL_Color toolbar_button_color = rgba_to_sdl(TOOLBAR_BUTTON_COLOR);
        sec(SDL_SetRenderDrawColor(
                renderer,
                toolbar_button_color.r,
                toolbar_button_color.g,
                toolbar_button_color.b,
                toolbar_button_color.a));
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

bool Toolbar::handle_click_at(Vec2f position)
{
    for (size_t i = 0; i < buttons_count; ++i) {
        if (rect_contains_vec2(button_hitbox(i), position)) {
            active_button = i;
            return true;
        }
    }
    return false;
}

bool Toolbar::handle_mouse_hover(Vec2f position)
{
    for (size_t i = 0; i < buttons_count; ++i) {
        if (rect_contains_vec2(button_hitbox(i), position)) {
            hovered_button = {true, i};
            tooltip_position = position;
            return true;
        }
    }

    hovered_button = {};
    return false;
}

void Tool::handle_event(Game *game, SDL_Event *event)
{
    switch (type) {
    case Tool_Type::Item:
        item.handle_event(game, event);
        break;
    case Tool_Type::Tile:
        tile.handle_event(game, event);
        break;
    case Tool_Type::Entity:
        entity.handle_event(game, event);
        break;
    }
}

void Tile_Tool::handle_event(Game *game, SDL_Event *event)
{
    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        Vec2i coord = vec_cast<int>(game->mouse_position / TILE_SIZE);

        if (game->grid.get_tile(coord) == TILE_EMPTY) {
            state = Drawing;
            game->grid.set_tile(coord, tile);
        } else {
            state = Erasing;
            game->grid.set_tile(coord, TILE_EMPTY);
        }
    } break;

    case SDL_MOUSEBUTTONUP: {
        state = Inactive;
    } break;

    case SDL_MOUSEMOTION: {
        switch (state) {
        case Drawing: {
            Vec2i coord = vec_cast<int>(game->mouse_position / TILE_SIZE);
            game->grid.set_tile(coord, tile);
        } break;

        case Erasing: {
            Vec2i coord = vec_cast<int>(game->mouse_position / TILE_SIZE);
            game->grid.set_tile(coord, TILE_EMPTY);
        } break;

        case Inactive:
        default: {}
        }
    } break;
    }

    println(stdout, "Tile_Tool::handle_event()");
}

void Item_Tool::handle_event(Game *game, SDL_Event *event)
{
    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        game->spawn_item_at(item, game->mouse_position);
    } break;
    }
}

void Entity_Tool::handle_event(Game *game, SDL_Event *event)
{
    switch (event->type) {
    case SDL_MOUSEBUTTONDOWN: {
        game->spawn_entity_at(entity, game->mouse_position);
    } break;
    }
}
