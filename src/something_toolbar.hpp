#ifndef SOMETHING_TOOLBAR_HPP_
#define SOMETHING_TOOLBAR_HPP_

struct Button
{
    Sprite icon;
    String_View tooltip;
};

const size_t TOOLBAR_BUTTONS_CAPACITY =  69;

struct Toolbar
{
    Button buttons[TOOLBAR_BUTTONS_CAPACITY];
    size_t buttons_count;
    size_t active_button;
    Maybe<size_t> hovered_button;
    Vec2f tooltip_position;

    void render(SDL_Renderer *renderer, Camera camera, Bitmap_Font font);
    bool handle_click_at(Vec2f position, Camera camera);
    bool handle_mouse_hover(Vec2f position, Camera camera);
    Rectf button_hitbox(size_t button, Camera camera);
};

#endif  // SOMETHING_TOOLBAR_HPP_
