#ifndef SOMETHING_BACKGROUND_HPP_
#define SOMETHING_BACKGROUND_HPP_

const size_t BACKGROUND_LAYERS_COUNT = 4;

struct Background
{
    Sprite layers[BACKGROUND_LAYERS_COUNT];

    void render(SDL_Renderer *renderer, Camera camera);
};

#endif  // SOMETHING_BACKGROUND_HPP_
