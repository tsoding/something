#include "./something_background.hpp"

void Background::render(SDL_Renderer *renderer, Camera camera)
{
    for (size_t i = 0; i < BACKGROUND_LAYERS_COUNT; ++i) {
        const float w = (float) layers[i].srcrect.w * BACKGROUND_SCALE_FACTOR;
        const float h = (float) layers[i].srcrect.h * BACKGROUND_SCALE_FACTOR;
        const auto s = vec2(w, h);

        auto p = (camera.pos / s).map(floorf) * s + -camera.pos * BACKGROUND_PARALLAX_FACTOR * (float) i;

        while (p.x + w < 0.0f) {
            p.x += w;
        }

        while (p.x > 0.0f) {
            p.x -= w;
        }

        while (p.y + h < 0.0f) {
            p.y += h;
        }

        while (p.y > 0.0f) {
            p.y -= h;
        }

        float the_original_hwy = p.y;
        while (p.x < (float) SCREEN_WIDTH) {
            p.y = the_original_hwy;
            while (p.y < (float) SCREEN_HEIGHT) {
                layers[i].render(renderer, rect(p, w, h));
                p.y += h;
            }
            p.x += w;
        }
    }
}
