#ifndef SOMETHING_ITEM_HPP_
#define SOMETHING_ITEM_HPP_

enum Item_Type
{
    ITEM_NONE = 0,
    ITEM_HEALTH,
    ITEM_DIRT_BLOCK,
};

struct Item
{
    Item_Type type;
    Sprite sprite;
    Vec2f pos;
    float a;
    Rectf hitbox_local;
    Sample_S16 sound;

    void update(float delta_time);
    void render(SDL_Renderer *renderer, Camera camera,
                RGBA shade = {0, 0, 0, 0}) const;
    void render_debug(SDL_Renderer *renderer, Camera camera) const;
    Rectf hitbox_world() const;
};

Item make_health_item(Vec2f pos);
Item make_dirt_block_item(Vec2f pos);

#endif  // SOMETHING_ITEM_HPP_
