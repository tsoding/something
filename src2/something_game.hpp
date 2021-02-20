#ifndef SOMETHING_GAME_HPP_
#define SOMETHING_GAME_HPP_

typedef float Seconds;
typedef Uint32 Milliseconds;

struct Game
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool quit;

    void handle_event(const SDL_Event *event);
    void update(Seconds dt);
    void render();
};

extern Game *game;

#endif  // SOMETHING_GAME_HPP_
