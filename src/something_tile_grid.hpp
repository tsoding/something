#ifndef TILE_GRID_HPP_
#define TILE_GRID_HPP_

typedef uint32_t Tile;

const Tile TILE_EMPTY         = 0;
const Tile TILE_WALL          = 1;
const Tile TILE_DESTROYABLE_0 = 2;
const Tile TILE_DESTROYABLE_1 = 3;
const Tile TILE_DESTROYABLE_2 = 4;
const Tile TILE_DESTROYABLE_3 = 5;
const Tile TILE_COUNT         = 6;

const size_t TILE_GRID_WIDTH = 4096;
const size_t TILE_GRID_HEIGHT = 4096;

struct Tile_Def
{
    bool is_collidable;
    Sprite top_texture;
    Sprite bottom_texture;
};

Tile_Def tile_defs[TILE_COUNT] = {
    {false, {}, {}},                          // TILE_EMPTY
    {true, {}, {}},                           // TILE_WALL
    {true, {}, {}},                           // TILE_DESTROYABLE_0
    {true, {}, {}},                           // TILE_DESTROYABLE_1
    {true, {}, {}},                           // TILE_DESTROYABLE_2
    {true, {}, {}},                           // TILE_DESTROYABLE_3
};

const float TILE_SIZE = 128.0f * 0.5f;
const float TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

struct Tile_Grid
{
    Tile tiles[TILE_GRID_HEIGHT][TILE_GRID_WIDTH];

    void render(SDL_Renderer *renderer, Camera camera);
    void resolve_point_collision(Vec2f *origin);
    Vec2i abs_to_tile_coord(Vec2f pos);
    Tile get_tile(Vec2i coord);
    bool is_tile_empty(Vec2i coord);
};

#endif  // TILE_GRID_HPP_
