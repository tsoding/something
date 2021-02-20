#include "./something_player.hpp"

const float PLAYER_SIZE = 100.0f;
const RGBA32 PLAYER_COLOR = 0xFF0000FF;

void Player::render()
{
    fill_rect(
        game->renderer,
        PLAYER_COLOR,
        pos,
        V2(PLAYER_SIZE));
}

void Player::update(Seconds) {}

