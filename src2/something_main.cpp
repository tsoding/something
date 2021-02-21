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
    config.load_file("src2/vars.conf");

    // NOTE: The game object could be too big to put on the stack.
    // So we are allocating it on the heap.
    Game *game = new Game{};
    defer(delete game);

    sec(SDL_Init(SDL_INIT_VIDEO));

    SDL_Window * const window =
        sec(SDL_CreateWindow(
                "Something 2 -- Electric Boogaloo",
                0, 0,
                SCREEN_WIDTH, SCREEN_HEIGHT,
                SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL));
    defer(SDL_DestroyWindow(window));

    SDL_GL_CreateContext(window);

    Renderer *renderer = new Renderer{};
    defer(delete renderer);
    renderer->init();

    game->keyboard = SDL_GetKeyboardState(NULL);

    while (!game->quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            game->handle_event(&event);
        }

        game->update(DELTA_TIME_SECS);

        {
            // TODO: don't recompute the GL viewport on every frame
            int w, h;
            SDL_GetWindowSize(window, &w, &h);

            const float w_width = static_cast<float>(w);
            const float w_height = static_cast<float>(h);
            const float s_width = static_cast<float>(SCREEN_WIDTH);
            const float s_height = static_cast<float>(SCREEN_HEIGHT);

            float a_height = 0.0f;
            float a_width = 0.0f;

            if (w_width > w_height) {
                a_width = s_width * (w_height / s_height);
                a_height = w_height;
            } else {
                a_width = w_width;
                a_height = s_height * (w_width / s_width);
            }

            glViewport(
                w_width * 0.5 - a_width * 0.5,
                w_height * 0.5 - a_height * 0.5,
                a_width, a_height);
        }

        glClearColor(BACKGROUND_COLOR.r,
                     BACKGROUND_COLOR.g,
                     BACKGROUND_COLOR.b,
                     BACKGROUND_COLOR.a);
        glClear(GL_COLOR_BUFFER_BIT);

        renderer->fill_rect(
            AABB(V2(-1.0f), V2(2.0f)),
            RGBA::from_abgr32(0x505050FF));
        game->render(renderer);

        SDL_GL_SwapWindow(window);

        SDL_Delay(DELTA_TIME_MS);
    }

    SDL_Quit();

    return 0;
}
