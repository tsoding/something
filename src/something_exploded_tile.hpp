#ifndef SOMETHING_EXPLODED_TILE_H_
#define SOMETHING_EXPLODED_TILE_H_

enum class Exploded_Tile_State
{
    Ded = 0,
    Alive
};

struct Exploded_Tile
{
    Exploded_Tile_State state;
    Tile tile;
    Rectf texbox_local;
    Vec2f pos;
    Vec2f vel;
    Vec2f epicenter;

    inline Rectf texbox_world() const
    {
        Rectf dstrect = {
            texbox_local.x + pos.x,
            texbox_local.y + pos.y,
            texbox_local.w,
            texbox_local.h
        };
        return dstrect;
    }

    void render(SDL_Renderer *renderer, Camera camera) const;
    void update(float dt);
};

Exploded_Tile make_exploded_tile(Vec2f pos, Tile tile, Vec2f epicenter);

#endif  // SOMETHING_EXPLODED_TILE_H_
