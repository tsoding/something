#ifndef SOMETHING_PLAYER_HPP_
#define SOMETHING_PLAYER_HPP_

#include "./something_sdl.hpp"

struct Player
{
    void render();
    void update(Seconds dt);
};

#endif // SOMETHING_PLAYER_HPP_
