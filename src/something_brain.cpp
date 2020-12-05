#include "./something_game.hpp"
#include "./something_brain.hpp"
#include "./something_entity.hpp"

void shooter_think(Game *game, Entity_Index entity_index, Recti *lock)
{
    auto &entity = game->entities[entity_index.unwrap];
    auto &player = game->entities[PLAYER_ENTITY_INDEX];

    if (entity.state == Entity_State::Alive) {
        if (game->grid.a_sees_b(entity.pos, player.pos)) {
            entity.stop();
            entity.point_gun_at(player.pos);
            game->entity_shoot(entity_index);
        } else {
            auto entity_tile = game->grid.abs_to_tile_coord(entity.pos);
            auto next = game->grid.next_in_bfs(entity_tile, lock);
            if (next.has_value) {
                auto d = next.unwrap - entity_tile;

                if (d.y < 0) {
                    entity.jump();
                }
                if (d.x > 0) {
                    entity.move(Entity::Right);
                }
                if (d.x < 0) {
                    entity.move(Entity::Left);
                }
                if (d.x == 0) {
                    entity.stop();
                }
            } else {
                entity.stop();
            }
        }
    }
}

void stomper_think(Game *, Entity_Index, Recti *)
{
    assert(0 && "TODO: stomper_think is not implemented");
}
