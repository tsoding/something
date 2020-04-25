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
    Vec2f position;
    Tile tiles[ROOM_HEIGHT][ROOM_WIDTH];

    bool is_tile_inbounds(Vec2i p) const
    {
        return 0 <= p.x && p.x < ROOM_WIDTH && 0 <= p.y && p.y < ROOM_HEIGHT;
    }

    bool is_tile_empty(Vec2i p) const
    {
        return !is_tile_inbounds(p) || tiles[p.y][p.x] == Tile::Empty;
    }

    bool is_tile_at_abs_p_empty(Vec2f p) const
    {
        return is_tile_empty(vec_cast<int>((p - position) / TILE_SIZE));
    }

    void render(SDL_Renderer *renderer,
                Camera camera,
                Sprite top_ground_texture,
                Sprite bottom_ground_texture,
                SDL_Color blend_color = {0, 0, 0, 0})
    {
        SDL_Color saved_blend_color = {};

        sec(SDL_GetRenderDrawColor(
                renderer,
                &saved_blend_color.r,
                &saved_blend_color.g,
                &saved_blend_color.b,
                &saved_blend_color.a));

        sec(SDL_SetRenderDrawColor(
                renderer,
                blend_color.r,
                blend_color.g,
                blend_color.b,
                blend_color.a));

        for (int y = 0; y < ROOM_HEIGHT; ++y) {
            for (int x = 0; x < ROOM_WIDTH; ++x) {
                switch (tiles[y][x]) {
                case Tile::Empty: {
                } break;

                case Tile::Wall: {
                    const auto dstrect = rect(
                        vec_cast<float>(vec2(x, y)) * TILE_SIZE + position - camera.pos,
                        TILE_SIZE, TILE_SIZE);
                    if (is_tile_empty(vec2(x, y - 1))) {
                        render_sprite(renderer, top_ground_texture, dstrect);
                    } else {
                        render_sprite(renderer, bottom_ground_texture, dstrect);
                    }

                    SDL_Rect rect = rectf_for_sdl(dstrect);
                    SDL_RenderFillRect(renderer, &rect);
                } break;
                }
            }
        }

        sec(SDL_SetRenderDrawColor(
                renderer,
                blend_color.r,
                blend_color.g,
                blend_color.b,
                blend_color.a));
    }

    void fill_with(Tile tile)
    {
        for (size_t row = 0; row < ROOM_HEIGHT; ++row) {
            for (size_t col = 0; col < ROOM_WIDTH; ++col) {
                tiles[row][col] = tile;
            }
        }
    }

    void floor_at(Tile tile, size_t row)
    {
        for (size_t column_index = 0; column_index < ROOM_WIDTH; ++column_index) {
            static_assert(ROOM_HEIGHT >= 0);
            tiles[row][column_index] = tile;
        }
    }

    void dump_file(const char *file_path)
    {
        FILE *room_file = fopen(file_path, "wb");
        if (!room_file) {
            fprintf(stderr, "Could not save room to `%s`: %s\n",
                    file_path, strerror(errno));
            abort();
        }
        dump_stream(room_file);
        fclose(room_file);
    }

    void load_file(const char *file_path)
    {
        FILE *room_file = fopen(file_path, "rb");
        if (!room_file) {
            fprintf(stderr, "Could not load room from `%s`: %s\n",
                    file_path, strerror(errno));
            abort();
        }
        load_stream(room_file);
        fclose(room_file);
    }

    void dump_stream(FILE *stream)
    {
        size_t n = fwrite(tiles, sizeof(Tile), ROOM_WIDTH * ROOM_HEIGHT, stream);
        assert(n == ROOM_WIDTH * ROOM_HEIGHT);
    }

    void load_stream(FILE *stream)
    {
        size_t n = fread(tiles, sizeof(Tile), ROOM_WIDTH * ROOM_HEIGHT, stream);
        assert(n == ROOM_WIDTH * ROOM_HEIGHT);
    }

    void resolve_point_collision(Vec2f *origin)
    {
        assert(origin);

        Vec2f p = *origin  - position;

        const auto tile = vec_cast<int>(p / TILE_SIZE);

        if (is_tile_empty(tile)) {
            return;
        }

        const auto p0 = vec_cast<float>(tile) * TILE_SIZE;
        const auto p1 = vec_cast<float>(tile + 1) * TILE_SIZE;

        struct Side {
            float d;
            Vec2f np;
            Vec2i nd;
            float dd;
        };

        Side sides[] = {
            {sqr_dist<float>({p0.x, 0},    {p.x, 0}),    {p0.x, p.y}, {-1,  0}, TILE_SIZE_SQR},     // left
            {sqr_dist<float>({p1.x, 0},    {p.x, 0}),    {p1.x, p.y}, { 1,  0}, TILE_SIZE_SQR},     // right
            {sqr_dist<float>({0, p0.y},    {0, p.y}),    {p.x, p0.y}, { 0, -1}, TILE_SIZE_SQR},     // top
            {sqr_dist<float>({0, p1.y},    {0, p.y}),    {p.x, p1.y}, { 0,  1}, TILE_SIZE_SQR},     // bottom
            {sqr_dist<float>({p0.x, p0.y}, {p.x, p.y}), {p0.x, p0.y}, {-1, -1}, TILE_SIZE_SQR * 2}, // top-left
            {sqr_dist<float>({p1.x, p0.y}, {p.x, p.y}), {p1.x, p0.y}, { 1, -1}, TILE_SIZE_SQR * 2}, // top-right
            {sqr_dist<float>({p0.x, p1.y}, {p.x, p.y}), {p0.x, p1.y}, {-1,  1}, TILE_SIZE_SQR * 2}, // bottom-left
            {sqr_dist<float>({p1.x, p1.y}, {p.x, p.y}), {p1.x, p1.y}, { 1,  1}, TILE_SIZE_SQR * 2}  // bottom-right
        };
        const int SIDES_COUNT = sizeof(sides) / sizeof(sides[0]);

        int closest = -1;
        for (int current = 0; current < SIDES_COUNT; ++current) {
            for (int i = 1;
                 !is_tile_empty(tile + (sides[current].nd * i)) ;
                 ++i)
            {
                sides[current].d += sides[current].dd;
            }

            if (closest < 0 || sides[closest].d >= sides[current].d) {
                closest = current;
            }
        }

        *origin = sides[closest].np + position;
    }
};

const size_t ROOM_ROW_COUNT = 8;
Room room_row[ROOM_ROW_COUNT] = {};

struct Room_Index
{
    size_t unwrap;
};

Room_Index room_index_at(Vec2f p)
{
    int index = (int) floor(p.x / ROOM_BOUNDARY.w);

    if (index < 0) return {0};
    if (index >= (int) ROOM_ROW_COUNT) return {ROOM_ROW_COUNT - 1};
    return {(size_t) index};
}
