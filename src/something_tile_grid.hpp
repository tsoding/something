#ifndef TILE_GRID_HPP_
#define TILE_GRID_HPP_

typedef uint32_t Tile;

const Tile TILE_EMPTY         = 0;
const Tile TILE_WALL          = 1;
const Tile TILE_DESTROYABLE_0 = 2;
const Tile TILE_DESTROYABLE_1 = 3;
const Tile TILE_DESTROYABLE_2 = 4;
const Tile TILE_DESTROYABLE_3 = 5;
const Tile TILE_ICE           = 6;
const Tile TILE_COUNT         = 7;

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
    {true, {}, {}},                           // TILE_ICE
};

const float TILE_SIZE = 128.0f * 0.5f;
const float TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

const int ROOM_WIDTH  = 10 * 2;
const int ROOM_HEIGHT = 10 * 2;

template <typename T, size_t Capacity>
struct Queue
{
    T items[Capacity];
    size_t begin;
    size_t count;

    void nq(T x)
    {
        assert(count < Capacity);
        items[(begin + count) % Capacity] = x;
        count++;
    }

    T dq()
    {
        assert(count > 0);
        T result = items[begin];
        begin = (begin + 1) % Capacity;
        count -= 1;
        return result;
    }

    void clear()
    {
        count = 0;
    }

    T &operator[](size_t index)
    {
        return items[(begin + index) % Capacity];
    }

    const T &operator[](size_t index) const
    {
        return items[(begin + index) % Capacity];
    }
};

using Room_Queue = Queue<Vec2i, ROOM_WIDTH * ROOM_HEIGHT>;

struct Tile_Grid
{
    Tile tiles[TILE_GRID_HEIGHT][TILE_GRID_WIDTH];

    void load_from_file(const char *filepath);
    void load_room_from_file(const char *filepath, Vec2i coord);

    void render(SDL_Renderer *renderer, Camera camera, Recti *lock);
    void resolve_point_collision(Vec2f *origin);
    Vec2i abs_to_tile_coord(Vec2f pos);

    Tile get_tile(Vec2i coord);
    void set_tile(Vec2i coord, Tile tile);
    void copy_tile(Vec2i coord_dst, Vec2i coord_src);

    bool is_tile_coord_inbounds(Vec2i coord);
    bool is_tile_empty_tile(Vec2i coord);
    bool is_tile_empty_abs(Vec2f pos);
    Tile *tile_at_abs(Vec2f pos);
    Vec2f abs_center_of_tile(Vec2i coord);
    Rectf rect_of_tile(Vec2i coord);

    int bfs_trace[ROOM_WIDTH][ROOM_HEIGHT];
    void bfs_to_tile(Vec2i src, Recti *lock);
    Maybe<Vec2i> next_in_bfs(Vec2i dst0, Recti *lock);
    void render_debug_bfs_overlay(SDL_Renderer *renderer, Camera *camera, Recti *lock);
    bool a_sees_b(Vec2f a, Vec2f b);
};

#endif  // TILE_GRID_HPP_
