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

void Tile_Grid::render(SDL_Renderer *renderer, Camera camera)
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

            if (is_tile_empty_tile(vec2(coord.x, coord.y - 1))) {
                tile_defs[tile].top_texture.render(renderer, dstrect);
            } else {
                tile_defs[tile].bottom_texture.render(renderer, dstrect);
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
