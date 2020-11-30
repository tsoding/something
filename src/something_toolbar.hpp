#ifndef SOMETHING_TOOLBAR_HPP_
#define SOMETHING_TOOLBAR_HPP_

#include "./something_entity.hpp"

enum class Tool_Type
{
    Item,
    Tile,
    Entity
};

struct Game;

struct Item_Tool
{
    Item item;

    void handle_event(Game *game, SDL_Event *event);
};

struct Tile_Tool
{
    Tile tile;
    Vec2i coord_down;

    enum State
    {
        Inactive = 0,
        Drawing,
        Erasing,
    };

    State state;

    void handle_event(Game *game, SDL_Event *event);
};

struct Entity_Tool
{
    Entity entity;

    void handle_event(Game *game, SDL_Event *event);
};

struct Tool
{
    Tool_Type type;

    union
    {
        Item_Tool item;
        Tile_Tool tile;
        Entity_Tool entity;
    };

    void handle_event(Game *game, SDL_Event *event);
};


struct Button
{
    Sprite icon;
    String_View tooltip;
    Tool tool;
};

const size_t TOOLBAR_BUTTONS_CAPACITY =  69;

struct Toolbar
{
    Button buttons[TOOLBAR_BUTTONS_CAPACITY];
    size_t buttons_count;
    size_t active_button;
    Maybe<size_t> hovered_button;
    Vec2f tooltip_position;

    void render(SDL_Renderer *renderer, Bitmap_Font font);
    bool handle_click_at(Vec2f position);
    bool handle_mouse_hover(Vec2f position);
    Rectf button_hitbox(size_t button);
};

#endif  // SOMETHING_TOOLBAR_HPP_
