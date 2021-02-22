#include "./something_player.hpp"

const float PLAYER_SIZE = 0.5f;
const RGBA PLAYER_COLOR = RGBA::from_abgr32(0xFF0000FF);
const float PLAYER_SPEED = 1.0f;

void Player::render(const Game *, Renderer *renderer) const
{
    renderer->fill_rect(
        AABB(pos, V2(PLAYER_SIZE)),
        PLAYER_COLOR);
}

void Player::update(Game *, Seconds dt)
{
    pos += vel * dt;
}

void Player::move(Direction direction)
{
    switch(direction) {
    case Direction::Left:
        vel = V2(-1.0f, 0.0f) * PLAYER_SPEED;
        break;
    case Direction::Right:
        vel = V2(1.0f, 0.0f) * PLAYER_SPEED;
        break;
    default:
        unreachable("Player::move()");
    }
}

void Player::stop()
{
    vel = V2<float>();
}