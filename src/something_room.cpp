#include "something_room.hpp"

Vec2f Room::center() const
{
    assert(0 && "TODO: Room::center() needs to be reimplemented");
    return {};
}

bool Room::is_tile_inbounds(Vec2i p) const
{
    return 0 <= p.x && p.x < ROOM_WIDTH && 0 <= p.y && p.y < ROOM_HEIGHT;
}

void Room::fill_with(Tile tile, Tile_Grid *tile_grid)
{
    assert(0 && "TODO: Room::fill_with() is not implemented");
}

void Room::floor_at(Tile tile, size_t row, Tile_Grid *tile_grid)
{
    assert(0 && "TODO: Room::floor_at() is not implemented");
}

void Room::dump_file(const char *file_path, Tile_Grid *tile_grid)
{
    FILE *room_file = fopen(file_path, "wb");
    if (!room_file) {
        fprintf(stderr, "Could not save room to `%s`: %s\n",
                file_path, strerror(errno));
        abort();
    }
    dump_stream(room_file, tile_grid);
    fclose(room_file);
}

void Room::load_file(const char *file_path, Tile_Grid *tile_grid)
{
    FILE *room_file = fopen(file_path, "rb");
    if (!room_file) {
        fprintf(stderr, "Could not load room from `%s`: %s\n",
                file_path, strerror(errno));
        abort();
    }
    load_stream(room_file, tile_grid);
    fclose(room_file);
}

void Room::dump_stream(FILE *stream, Tile_Grid *tile_grid)
{
    assert(0 && "TODO: Room::dump_stream() is not implemented");
}

void Room::load_stream(FILE *stream, Tile_Grid *tile_grid)
{
    Tile tmp[ROOM_WIDTH * ROOM_HEIGHT];
    fread(tmp, sizeof(Tile), ROOM_WIDTH * ROOM_HEIGHT, stream);

    for (int y = 0; y < ROOM_HEIGHT; ++y) {
        for (int x = 0; x < ROOM_WIDTH; ++x) {
            tile_grid->set_tile(coord + vec2(x, y), tmp[y * ROOM_WIDTH + x]);
        }
    }
}

void Room::copy_from(Room *room, Tile_Grid *tile_grid)
{
    assert(0 && "TODO: Room::copy_from() is not implemented");
}

bool Room::a_sees_b(Vec2f a, Vec2f b, Tile_Grid *tile_grid)
{
    // TODO: Room::a_sees_b is not particularly smart
    //   It is implemented using a very simple ray marching which sometimes skips
    //   the corners. We need to evaluate whether this is important or not
    Vec2f d = normalize(b - a);
    float s = TILE_SIZE * 0.5f;
    float n = sqrtf(sqr_dist(a, b)) / s;
    for (float i = 0; i < n; i += 1.0f) {
        Vec2f p = a + d * s * i;
        if (!tile_grid->is_tile_empty_abs(p)) {
            return false;
        }
    }

    return true;
}

void Room::bfs_to_tile(Vec2i src, Tile_Grid *grid)
{
    assert(0 && "Room::bfs_to_tile() is not implemented");
#if 0
    Room_Queue bfs_q = {};
    memset(bfs_trace, 0, sizeof(bfs_trace));

    bfs_q.nq(src);
    bfs_trace[src.y][src.x] = 1;
    while (bfs_q.count > 0) {
        Vec2i p0 = bfs_q.dq();
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if ((dy == 0) != (dx == 0)) {
                    Vec2i p1 = {p0.x + dx, p0.y + dy};
                    if (is_tile_inbounds(p1) &&
                        tiles[p1.y][p1.x] == TILE_EMPTY &&
                        bfs_trace[p1.y][p1.x] == 0)
                    {
                        bfs_trace[p1.y][p1.x] = bfs_trace[p0.y][p0.x] + 1;
                        bfs_q.nq(p1);
                    }
                }
            }
        }
    }
#endif
}

Maybe<Vec2i> Room::next_in_bfs(Vec2i dst)
{
    assert(0 && "Room::next_in_bfs() is not implemented");
#if 0
    if (bfs_trace[dst.y][dst.x] > 0) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                if ((dy == 0) != (dx == 0)) {
                    Vec2i dst1 = {
                        dst.x + dx,
                        dst.y + dy
                    };

                    if (is_tile_inbounds(dst1) &&
                        tiles[dst1.y][dst1.x] == TILE_EMPTY &&
                        bfs_trace[dst1.y][dst1.x] < bfs_trace[dst.y][dst.x])
                    {
                        return {true, dst1};
                    }
                }
            }
        }
    }
#endif

    return {};
}
