#include "./something_game.hpp"
#include "./something_sdl.hpp"
#include "./something_config.hpp"

const RGBA32 BACKGROUND_COLOR = 0xFF181818;

void Game::handle_event(const SDL_Event *event)
{
    if (keyboard[SDL_SCANCODE_D]) {
        player.move(Direction::Right);
    } else if (keyboard[SDL_SCANCODE_A]) {
        player.move(Direction::Left);
    } else {
        player.stop();
    }

    switch (event->type) {
    case SDL_QUIT: {
        quit = true;
    }
    break;
    }
}

void Game::update(Seconds dt)
{
    player.update(this, dt);
}

void Game::render(SDL_Renderer *renderer) const
{
    sec(SDL_SetRenderDrawColor(renderer, RGBA32_UNPACK(BACKGROUND_COLOR)));
    sec(SDL_RenderClear(renderer));

    player.render(this, renderer);

    SDL_RenderPresent(renderer);
}
