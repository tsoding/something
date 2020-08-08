#ifndef SOMETHING_CONSOLE_HPP_
#define SOMETHING_CONSOLE_HPP_

struct Console
{
    bool visible;

    void render(SDL_Renderer *renderer);
    void update(float dt);
};

#endif  // SOMETHING_CONSOLE_HPP_
