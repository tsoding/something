#include "./something_sdl.hpp"
#include "./something_game.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_FPS = 60;

const Seconds DELTA_TIME_SECS = 1.0 / static_cast<Seconds>(SCREEN_FPS);
const Milliseconds DELTA_TIME_MS =
    static_cast<Milliseconds>(floorf(DELTA_TIME_SECS * 1000.0f));

int main()
{
    game = new Game{};
    defer(delete game);

    sec(SDL_Init(SDL_INIT_VIDEO));

    game->window =
        sec(SDL_CreateWindow(
                "Something 2 -- Electric Boogaloo",
                0, 0,
                SCREEN_WIDTH, SCREEN_HEIGHT,
                SDL_WINDOW_RESIZABLE));

    game->renderer =
        SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED);

    while (!game->quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            game->handle_event(&event);
        }

        game->update(DELTA_TIME_SECS);
        game->render();

        SDL_Delay(DELTA_TIME_MS);
    }

    SDL_Quit();

    return 0;
}
