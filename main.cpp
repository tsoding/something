#include <cstdio>
#include <cstdlib>
#include <SDL.h>

int sdl(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return code;
}

template <typename T>
T *sdl(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return ptr;
}

int main(int argc, char *argv[])
{
    sdl(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS));

    SDL_Window *window =
        sdl(SDL_CreateWindow(
                "New folder 1",
                0, 0, 800, 600,
                SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE));

    SDL_Renderer *renderer =
        sdl(SDL_CreateRenderer(
                window, -1,
                SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED));

    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                quit = true;
            } break;
            }
        }

        sdl(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
        sdl(SDL_RenderClear(renderer));

        SDL_RenderPresent(renderer);
    }

    SDL_Quit();

    return 0;
}
