#ifndef SOMETHING_ITEM_HPP_
#define SOMETHING_ITEM_HPP_

enum Item_Type
{
    ITEM_NONE = 0,
    ITEM_HEALTH
};

struct Item
{
    Item_Type type;
    Sprite sprite;
    Vec2f pos;
    float a;

    void update(float delta_time);
    void render(SDL_Renderer *renderer, Camera camera);
};

#endif  // SOMETHING_ITEM_HPP_
