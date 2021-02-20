#include "./something_game.hpp"
#include "./something_sdl.hpp"
#include "./something_config.hpp"

Game *game = nullptr;

void Game::handle_event(const SDL_Event *event)
{
    switch (event->type) {
    case SDL_QUIT: {
        quit = true;
    }
    break;
    }
}

void Game::update(Seconds dt)
{
    player.update(dt);
}

void Game::render()
{
    sec(SDL_SetRenderDrawColor(renderer, RGBA32_UNPACK(BACKGROUND_COLOR)));
    sec(SDL_RenderClear(renderer));

    player.render();

    SDL_RenderPresent(renderer);
}
