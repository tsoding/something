#include "./something_game.hpp"
#include "./something_brain.hpp"
#include "./something_entity.hpp"

void follower_think(Game *game, Index<Entity> entity_index, Recti *lock)
{
    auto &entity = game->entities[entity_index.unwrap];
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

void shooter_think(Game *game, Index<Entity> entity_index, Recti *lock)
{
    auto &entity = game->entities[entity_index.unwrap];
    auto &player = game->entities[PLAYER_ENTITY_INDEX];

    if (entity.state == Entity_State::Alive) {
        if (game->grid.a_sees_b(entity.pos, player.pos)) {
            entity.stop();
            entity.point_gun_at(player.pos);
            game->entity_shoot(entity_index);
        } else {
            follower_think(game, entity_index, lock);
        }
    }
}

void stomper_think(Game *game, Index<Entity> entity_index, Recti *lock)
{
    auto &entity = game->entities[entity_index.unwrap];
    auto &player = game->entities[PLAYER_ENTITY_INDEX];

    if (entity.state == Entity_State::Alive) {
        if (fabsf(entity.pos.x - player.pos.x) < STOMPER_ATTACK_X_THRESHOLD) {
            if (player.pos.y - entity.pos.y > STOMPER_ATTACK_Y_THRESHOLD) {
                entity.stomp(&game->grid);
            } else {
                entity.jump();
            }
        } else {
            follower_think(game, entity_index, lock);
        }
    }
}

void shooter_stomper_think(Game *game, Index<Entity> entity_index, Recti *lock)
{
    const float STOMPER_DISTANCE_THRESHOLD = 600.0;
    auto &entity = game->entities[entity_index.unwrap];
    auto &player = game->entities[PLAYER_ENTITY_INDEX];
    if (entity.state == Entity_State::Alive) {
        if (sqr_dist(entity.pos, player.pos) <= STOMPER_DISTANCE_THRESHOLD * STOMPER_DISTANCE_THRESHOLD) {
            stomper_think(game, entity_index, lock);
        } else {
            shooter_think(game, entity_index, lock);
        }
    }
}
