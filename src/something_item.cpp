#include "something_item.hpp"

void Item::update(float delta_time)
{
    a = fmodf(a + ITEM_OSC_FREQ * delta_time, 2 * PI);
}

void Item::render(SDL_Renderer *renderer, Camera camera)
{
    if (type != ITEM_NONE) {
        sprite.render(renderer, camera.to_screen(pos + vec2(0.0f, sin(a) * ITEM_AMP_VALUE)));
    }
}
