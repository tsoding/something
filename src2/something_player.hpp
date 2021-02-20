#ifndef SOMETHING_PLAYER_HPP_
#define SOMETHING_PLAYER_HPP_

#include "./something_sdl.hpp"
#include "./something_v2.hpp"

struct Player
{
    V2<float> pos;

    void render();
    void update(Seconds dt);
};

#endif // SOMETHING_PLAYER_HPP_
