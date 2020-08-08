#include "something_console.hpp"

void Console::render(SDL_Renderer *renderer)
{
    if (visible) {
        sec(SDL_SetRenderDrawColor(
                renderer,
                CONSOLE_BACKGROUND_COLOR.r,
                CONSOLE_BACKGROUND_COLOR.g,
                CONSOLE_BACKGROUND_COLOR.b,
                CONSOLE_BACKGROUND_COLOR.a));
        SDL_Rect rect = {
            0, 0,
            (int) SCREEN_WIDTH,
            CONSOLE_HEIGHT
        };
        sec(SDL_RenderFillRect(renderer, &rect));
    }
}

void Console::update(float dt)
{
    (void) dt;
}
