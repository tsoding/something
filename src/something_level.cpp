const float TILE_SIZE = 128.0f;
const float TILE_SIZE_SQR = TILE_SIZE * TILE_SIZE;

enum class Tile
{
    Empty = 0,
    Wall
};

const int ROOM_WIDTH = 10;
const int ROOM_HEIGHT = 10;

const Rectf ROOM_BOUNDARY = {
    0.0f, 0.0f, ROOM_WIDTH * TILE_SIZE, ROOM_HEIGHT * TILE_SIZE
};

struct Room
{
    Tile tiles[ROOM_HEIGHT][ROOM_WIDTH];

    bool is_tile_inbounds(Vec2i p) const
    {
        return 0 <= p.x && p.x < ROOM_WIDTH && 0 <= p.y && p.y < ROOM_HEIGHT;
    }

    bool is_tile_empty(Vec2i p) const
    {
        return !is_tile_inbounds(p) || tiles[p.y][p.x] == Tile::Empty;
    }

    void render(SDL_Renderer *renderer,
                Camera camera,
                Sprite top_ground_texture,
                Sprite bottom_ground_texture)
    {
        for (int y = 0; y < ROOM_HEIGHT; ++y) {
            for (int x = 0; x < ROOM_WIDTH; ++x) {
                switch (tiles[y][x]) {
                case Tile::Empty: {
                } break;

                case Tile::Wall: {
                    const auto dstrect = rect(
                        vec_cast<float>(vec2(x, y)) * TILE_SIZE - camera.pos,
                        TILE_SIZE, TILE_SIZE);
                    if (is_tile_empty(vec2(x, y - 1))) {
                        render_sprite(renderer, top_ground_texture, dstrect);
                    } else {
                        render_sprite(renderer, bottom_ground_texture, dstrect);
                    }
                } break;
                }
            }
        }
    }

    void dump(FILE *stream)
    {
        println(stream, "{");
        for (int y = 0; y < ROOM_HEIGHT; ++y) {
            print(stream, "{");
            for (int x = 0; x < ROOM_WIDTH; ++x) {
                switch (tiles[y][x]) {
                case Tile::Empty: {
                    print(stream, "Tile::Empty, ");
                } break;

                case Tile::Wall: {
                    print(stream, "Tile::Wall, ");
                } break;
                }
            }
            println(stream, "},");
        }
        println(stream, "}\n");
    }
};

Room dummy_room = {
    {
        {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Empty, Tile::Wall, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, Tile::Wall, },
        {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Wall, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
        {Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, Tile::Empty, },
    }
};

const size_t ROOM_ROW_COUNT = 8;
Room room_row[ROOM_ROW_COUNT] = {};
size_t room_current = 0;
