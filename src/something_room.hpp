#ifndef SOMETHING_ROOM_HPP_
#define SOMETHING_ROOM_HPP_

const int ROOM_WIDTH  = 10 * 2;
const int ROOM_HEIGHT = 10 * 2;

typedef uint32_t Tile;

#define TILE_EMPTY         0
#define TILE_WALL          1
#define TILE_DESTROYABLE_0 2
#define TILE_DESTROYABLE_1 3
#define TILE_DESTROYABLE_2 4
#define TILE_DESTROYABLE_3 5
#define TILE_COUNT         6

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

const Rectf ROOM_BOUNDARY = {
    0.0f, 0.0f, ROOM_WIDTH * TILE_SIZE, ROOM_HEIGHT * TILE_SIZE
};

struct Room_Index
{
    size_t unwrap;
};

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

struct Room
{
    Room_Index index;
    Tile tiles[ROOM_HEIGHT][ROOM_WIDTH];

    Vec2f position() const;
    Vec2f center() const;
    bool is_tile_inbounds(Vec2i p) const;
    bool is_tile_empty(Vec2i p) const;
    bool is_tile_at_abs_p_empty(Vec2f p) const;
    Vec2i tile_coord_at(Vec2f p);

    void render(SDL_Renderer *renderer,
                Camera camera,
                SDL_Color blend_color = {0, 0, 0, 0});
    void fill_with(Tile tile);
    void floor_at(Tile tile, size_t row);
    void dump_file(const char *file_path);
    void load_file(const char *file_path);
    void dump_stream(FILE *stream);
    void load_stream(FILE *stream);
    void copy_from(Room *room);
    void resolve_point_collision(Vec2f *origin);
    Tile *tile_at(Vec2f p);
    bool a_sees_b(Vec2f a, Vec2f b);
    void bfs(Vec2i src, Vec2i dst, Room_Queue *path);
};

#endif  // SOMETHING_ROOM_HPP_
