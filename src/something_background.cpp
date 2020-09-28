#include "./something_background.hpp"

void Background::render(SDL_Renderer *renderer, Camera camera)
{
    for (size_t i = 0; i < BACKGROUND_LAYERS_COUNT; ++i) {
        const float w = (float) layers[i].srcrect.w * BACKGROUND_SCALE_FACTOR;
        const float h = (float) layers[i].srcrect.h * BACKGROUND_SCALE_FACTOR;
        float x =
            camera.to_screen(
                vec2(floorf(camera.pos.x / w) * w - camera.pos.x * BACKGROUND_PARALLAX_FACTOR * (float) i,
                     0.0f)).x;

        while (x + w < 0.0f) {
            x += w;
        }

        while (x > 0.0f) {
            x -= w;
        }

        while (x < (float) SCREEN_WIDTH) {
            layers[i].render(
                renderer,
                rect(vec2(x, 0.0f), w, h));
            x += w;
        }
    }
}
