#include "./something_player.hpp"

const float PLAYER_SIZE = 100.0f;
const RGBA32 PLAYER_COLOR = 0xFF0000FF;
const float PLAYER_SPEED = 1000.0f;

void Player::render(const Game *, SDL_Renderer *renderer) const
{
    fill_rect(
        renderer,
        PLAYER_COLOR,
        pos,
        V2(PLAYER_SIZE));
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
