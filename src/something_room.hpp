#ifndef SOMETHING_ROOM_HPP_
#define SOMETHING_ROOM_HPP_

const int ROOM_WIDTH = 10;
const int ROOM_HEIGHT = 10;

enum Tile
{
    TILE_EMPTY = 0,
    TILE_WALL,

    TILE_COUNT
};

const float TILE_SIZE = 128.0f;
const float TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

const Rectf ROOM_BOUNDARY = {
    0.0f, 0.0f, ROOM_WIDTH * TILE_SIZE, ROOM_HEIGHT * TILE_SIZE
};

struct Room
{
    Vec2f position;
    Tile tiles[ROOM_HEIGHT][ROOM_WIDTH];

    Vec2f center() const;
    bool is_tile_inbounds(Vec2i p) const;
    bool is_tile_empty(Vec2i p) const;
    bool is_tile_at_abs_p_empty(Vec2f p) const;

    void render(SDL_Renderer *renderer,
                Camera camera,
                Sprite top_ground_texture,
                Sprite bottom_ground_texture,
                SDL_Color blend_color = {0, 0, 0, 0});
    void fill_with(Tile tile);
    void floor_at(Tile tile, size_t row);
    void dump_file(const char *file_path);
    void load_file(const char *file_path);
    void dump_stream(FILE *stream);
    void load_stream(FILE *stream);
    void copy_from(Room *room);
    void resolve_point_collision(Vec2f *origin);
};

#endif  // SOMETHING_ROOM_HPP_
