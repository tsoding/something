#ifndef SOMETHING_ROOM_HPP_
#define SOMETHING_ROOM_HPP_

const int ROOM_WIDTH  = 10 * 2;
const int ROOM_HEIGHT = 10 * 2;


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
    Vec2i coord;
    int bfs_trace[ROOM_WIDTH][ROOM_HEIGHT];

    Vec2f center() const;

    void dump_file(const char *file_path, Tile_Grid *tile_grid);
    void load_file(const char *file_path, Tile_Grid *tile_grid);
    void dump_stream(FILE *stream, Tile_Grid *tile_grid);
    void load_stream(FILE *stream, Tile_Grid *tile_grid);
    void copy_from(Room *room, Tile_Grid *tile_grid);
    bool is_tile_inbounds(Vec2i p) const;

    bool a_sees_b(Vec2f a, Vec2f b, Tile_Grid *tile_grid);
    void bfs_to_tile(Vec2i src, Tile_Grid *tile_grid);
    Maybe<Vec2i> next_in_bfs(Vec2i dst, Tile_Grid *tile_grid);
};

#endif  // SOMETHING_ROOM_HPP_
