#include "./something_exploded_tile.hpp"

void Exploded_Tile::render(SDL_Renderer *renderer, Camera camera) const
{
    tile_defs[tile].bottom_texture.render(renderer, camera.to_screen(texbox_world()));
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

    exploded_tile.texbox_local.w = TILE_SIZE;
    exploded_tile.texbox_local.h = TILE_SIZE;
    exploded_tile.texbox_local.x = exploded_tile.texbox_local.w * -0.5f;
    exploded_tile.texbox_local.y = exploded_tile.texbox_local.h * -0.5f;

    exploded_tile.state = Exploded_Tile_State::Alive;
    exploded_tile.pos = pos;
    exploded_tile.tile = tile;
    exploded_tile.epicenter = epicenter;

    return exploded_tile;
}
