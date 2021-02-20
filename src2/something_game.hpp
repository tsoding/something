#ifndef SOMETHING_GAME_HPP_
#define SOMETHING_GAME_HPP_

#include "./something_player.hpp"

struct Game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool quit;

    const Uint8 *keyboard;

    Player player;

    void handle_event(const SDL_Event *event);
    void update(Seconds dt);
    void render();
};

extern Game *game;

#endif  // SOMETHING_GAME_HPP_
