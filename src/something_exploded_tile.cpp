#include "./something_exploded_tile.hpp"

void Exploded_Tile::render(SDL_Renderer *renderer, Camera camera) const
{
    tile_defs[tile].bottom_texture.render(renderer, camera.to_screen(rect(pos, TILE_SIZE, TILE_SIZE)));
}

void Exploded_Tile::update(float dt)
{
    vel.y += ENTITY_GRAVITY * dt;
    pos += vel * dt;
    // TODO: Exploded tiles can leave rooms
}

Exploded_Tile make_exploded_tile(Vec2f pos, Tile tile, Vec2f epicenter)
{
    Exploded_Tile exploded_tile = {};

    exploded_tile.state = Exploded_Tile_State::Alive;
    exploded_tile.pos = pos;
    exploded_tile.tile = tile;
    exploded_tile.epicenter = epicenter;

    return exploded_tile;
}
