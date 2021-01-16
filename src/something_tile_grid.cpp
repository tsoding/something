#include "something_tile_grid.hpp"

Vec2i Tile_Grid::abs_to_tile_coord(Vec2f pos)
{
    return vec2(
        (int) floorf(pos.x / TILE_SIZE),
        (int) floorf(pos.y / TILE_SIZE));
}

bool Tile_Grid::is_tile_coord_inbounds(Vec2i coord)
{
    return 0 <= coord.x && coord.x < (int) TILE_GRID_WIDTH &&
           0 <= coord.y && coord.y < (int) TILE_GRID_HEIGHT;
}

Tile Tile_Grid::get_tile(Vec2i coord)
{
    if (is_tile_coord_inbounds(coord))  {
        return tiles[coord.y][coord.x];
    }

    return TILE_EMPTY;
}

void Tile_Grid::set_tile(Vec2i coord, Tile tile)
{
    if (is_tile_coord_inbounds(coord)) {
        tiles[coord.y][coord.x] = tile;
    }
}

void Tile_Grid::copy_tile(Vec2i coord_dst, Vec2i coord_src)
{
    if (is_tile_coord_inbounds(coord_dst) && is_tile_coord_inbounds(coord_src)) {
        tiles[coord_dst.y][coord_dst.x] = tiles[coord_src.y][coord_src.x];
    }
}

bool Tile_Grid::is_tile_empty_tile(Vec2i coord)
{
    return !tile_defs[get_tile(coord)].is_collidable;
}

bool Tile_Grid::is_tile_empty_abs(Vec2f pos)
{
    return is_tile_empty_tile(abs_to_tile_coord(pos));
}

Tile *Tile_Grid::tile_at_abs(Vec2f pos)
{
    const Vec2i coord = abs_to_tile_coord(pos);
    if (is_tile_coord_inbounds(coord)) {
        return &tiles[coord.y][coord.x];
    }

    return NULL;
}

void Tile_Grid::render(SDL_Renderer *renderer, Camera camera, Recti *lock)
{
    const Vec2i begin = abs_to_tile_coord(
        camera.pos - vec2(SCREEN_WIDTH, SCREEN_HEIGHT) * 0.5f);
    const Vec2i end = abs_to_tile_coord(
        camera.pos + vec2(SCREEN_WIDTH, SCREEN_HEIGHT) * 0.5f);

    for (int y = begin.y; y <= end.y; ++y) {
        for (int x = begin.x; x <= end.x; ++x) {
            const auto coord = vec2(x, y);
            const auto tile = get_tile(coord);
            const auto dstrect = rect(
                camera.to_screen(vec2((float) x, (float) y) * TILE_SIZE),
                TILE_SIZE, TILE_SIZE);

            RGBA shade_color = ROOM_NEIGHBOR_DIM_COLOR;

            if (lock && rect_contains_vec2(*lock, coord)) {
                shade_color = {0, 0, 0, 0};
            }

            if (is_tile_empty_tile(vec2(coord.x, coord.y - 1))) {
                tile_defs[tile].top_texture.render(renderer, dstrect, SDL_FLIP_NONE, shade_color);
            } else {
                tile_defs[tile].bottom_texture.render(renderer, dstrect, SDL_FLIP_NONE, shade_color);
            }
        }
    }
}

void Tile_Grid::resolve_point_collision(Vec2f *origin)
{
    Vec2f p = *origin;

    const auto tile = vec_cast<int>(p / TILE_SIZE);

    if (is_tile_empty_tile(tile)) {
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
             !is_tile_empty_tile(tile + (sides[current].nd * i)) ;
             ++i)
        {
            sides[current].d += sides[current].dd;
        }

        if (closest < 0 || sides[closest].d >= sides[current].d) {
            closest = current;
        }
    }

    *origin = sides[closest].np;
}
void Tile_Grid::bfs_to_tile(Vec2i src, Recti *lock)
{
    if (rect_contains_vec2(*lock, src)) {
        Room_Queue bfs_q = {};
        memset(bfs_trace, 0, sizeof(bfs_trace));

        bfs_q.nq(src);
        bfs_trace[src.y - lock->y][src.x - lock->x] = 1;
        while (bfs_q.count > 0) {
            Vec2i p0 = bfs_q.dq();
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    if ((dy == 0) != (dx == 0)) {
                        Vec2i p1 = {p0.x + dx, p0.y + dy};
                        if (rect_contains_vec2(*lock, p1) &&
                            is_tile_empty_tile(p1) &&
                            bfs_trace[p1.y - lock->y][p1.x - lock->x] == 0)
                        {
                            bfs_trace[p1.y - lock->y][p1.x - lock->x] = bfs_trace[p0.y - lock->y][p0.x - lock->x] + 1;
                            bfs_q.nq(p1);
                        }
                    }
                }
            }
        }
    }
}


Maybe<Vec2i> Tile_Grid::next_in_bfs(Vec2i dst, Recti *lock)
{

    if (rect_contains_vec2(*lock, dst) && bfs_trace[dst.y - lock->y][dst.x - lock->x] > 0) {
        Vec2i directions[4];
        directions[0] = {dst.x + 1, dst.y    };
        directions[1] = {dst.x - 1, dst.y    };
        directions[2] = {dst.x    , dst.y - 1};
        directions[3] = {dst.x    , dst.y + 1};

        for (auto &dst1 : directions) {
            if (rect_contains_vec2(*lock, dst1) &&
                is_tile_empty_tile(dst1) &&
                bfs_trace[dst1.y - lock->y][dst1.x - lock->x] < bfs_trace[dst.y - lock->y][dst.x - lock->x])
            {
                return {true, dst1};
            }
        }
    }

    return {};
}

void fill_rect(SDL_Renderer *renderer, Camera *camera,
               Rectf rectf, RGBA color)
{
    SDL_Color sdl_color = rgba_to_sdl(color);
    sec(SDL_SetRenderDrawColor(
            renderer,
            sdl_color.r,
            sdl_color.g,
            sdl_color.b,
            sdl_color.a));
    SDL_Rect rect = rectf_for_sdl(camera->to_screen(rectf));
    sec(SDL_RenderFillRect(renderer, &rect));
}

void Tile_Grid::render_debug_bfs_overlay(SDL_Renderer *renderer, Camera *camera, Recti *lock)
{
    for (int y = 0; y < lock->h; ++y) {
        for (int x = 0; x < lock->w; ++x) {
            fill_rect(
                renderer,
                camera,
                rect(vec2((float) (lock->x + x) * TILE_SIZE,
                          (float) (lock->y + y) * TILE_SIZE),
                     TILE_SIZE,
                     TILE_SIZE),
                {1.0f, 0.0f, 0.0f, clamp(1.0f - bfs_trace[y][x] * bfs_trace[y][x] / 255.0f, 0.0f, 1.0f)});
        }
    }
}

bool Tile_Grid::a_sees_b(Vec2f a, Vec2f b)
{
    // TODO(#133): Tile_Grid::a_sees_b is not particularly smart
    //   It is implemented using a very simple ray marching which sometimes skips
    //   the corners. We need to evaluate whether this is important or not
    Vec2f d = normalize(b - a);
    float s = TILE_SIZE * 0.5f;
    float n = sqrtf(sqr_dist(a, b)) / s;
    for (float i = 0; i < n; i += 1.0f) {
        Vec2f p = a + d * s * i;
        if (!is_tile_empty_abs(p)) {
            return false;
        }
    }

    return true;
}

void Tile_Grid::load_from_file(const char *filepath)
{
    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        println(stderr, "Could not load from file `", filepath, "`: ", strerror(errno));
        abort();
    }

    size_t n = fread(tiles, sizeof(Tile), TILE_GRID_WIDTH * TILE_GRID_HEIGHT, f);
    assert(n == TILE_GRID_WIDTH * TILE_GRID_HEIGHT);

    fclose(f);
}

void Tile_Grid::load_room_from_file(const char *filepath, Vec2i coord)
{
    Tile tmp[ROOM_HEIGHT][ROOM_WIDTH] = {};

    FILE *f = fopen(filepath, "rb");
    if (f == NULL) {
        println(stderr, "Could not load from file `", filepath, "`: ", strerror(errno));
        abort();
    }

    size_t n = fread(tmp, sizeof(Tile), ROOM_WIDTH * ROOM_HEIGHT, f);
    assert(n == ROOM_WIDTH * ROOM_HEIGHT);

    for (size_t dy = 0; dy < ROOM_HEIGHT; ++dy) {
        for (size_t dx = 0; dx < ROOM_WIDTH; ++dx) {
            size_t x = coord.x + dx;
            size_t y = coord.y + dy;
            if (x < (size_t) TILE_GRID_WIDTH && y < (size_t) TILE_GRID_HEIGHT) {
                tiles[y][x] = tmp[dy][dx];
            }
        }
    }
}

Vec2f Tile_Grid::abs_center_of_tile(Vec2i coord)
{
    return vec_cast<float>(coord) * TILE_SIZE + vec2(TILE_SIZE, TILE_SIZE) * 0.5f;
}

Rectf Tile_Grid::rect_of_tile(Vec2i coord)
{
    return rect(vec_cast<float>(coord) * TILE_SIZE, TILE_SIZE, TILE_SIZE);
}

Vec2f Tile_Grid::find_floor(Vec2f abs_pos)
{
    const size_t THRESHOLD = 10;

    if (is_tile_empty_abs(abs_pos)) {
        Vec2i tile_pos = abs_to_tile_coord(abs_pos);
        for (size_t i = 0; is_tile_empty_tile(tile_pos) && i < THRESHOLD; ++i) {
            tile_pos.y += 1;
        }
        return vec2(abs_pos.x, tile_pos.y * TILE_SIZE);
    } else {
        Vec2i tile_pos = abs_to_tile_coord(abs_pos);
        for (size_t i = 0; !is_tile_empty_tile(tile_pos) && i < THRESHOLD; ++i) {
            tile_pos.y -= 1;
        }
        return vec2(abs_pos.x, (tile_pos.y + 1) * TILE_SIZE);
    }
}
