#ifndef SOMETHING_TOOLBAR_HPP_
#define SOMETHING_TOOLBAR_HPP_

struct Toolbar
{
    enum Button
    {
        Tiles = 0,
        Heals,
        Button_Count
    };

    Button current_button;

    void render(SDL_Renderer *renderer, Camera camera);
    bool handle_click_at(Vec2f position, Camera camera);
    Rectf button_hitbox(Button button, Camera camera);
};


#endif  // SOMETHING_TOOLBAR_HPP_
