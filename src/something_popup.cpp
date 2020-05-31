#include "./something_popup.hpp"

void Popup::notify(SDL_Color color, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    buffer_size = vsnprintf(buffer, POPUP_BUFFER_CAPACITY, format, args);
    if (buffer_size < 0) {
        println(stderr, "[WARN] Popup::notify encountered an error");
    }

    a = 1.0f;
    this->color = color;
    va_end(args);
}

Vec2f shadow_offset_dir(float ratio)
{
    return vec2((float) FONT_POPUP_SIZE * ratio, (float) FONT_POPUP_SIZE * ratio);
}

void Popup::render(SDL_Renderer *renderer, const Camera *camera)
{
    if (buffer_size > 0 && a > 1e-6) {
        const auto font_size = vec2(FONT_POPUP_SIZE, FONT_POPUP_SIZE);
        const auto text_size = font.text_size(font_size, buffer);

        const Uint8 alpha = (Uint8) floorf(255.0f * fminf(a, 1.0f));
        const Vec2f position = vec2(camera->width  * 0.5f - (float) text_size.x * 0.5f,
                                    camera->height * 0.5f - (float) text_size.y * 0.5f);

        // SHADOW //////////////////////////////
        SDL_Color shadow_color = FONT_SHADOW_COLOR;
        shadow_color.a         = alpha;
        font.render(renderer, position - shadow_offset_dir(0.5f), font_size, shadow_color, buffer);

        // TEXT   //////////////////////////////
        SDL_Color front_color = color;
        color.a               = alpha;
        font.render(renderer, position, font_size, front_color, buffer);
    }
}

const float POPUP_FADEOUT_RATE = 0.5f;

void Popup::update(float delta_time)
{
    if (a > 0.0f) {
        a -= POPUP_FADEOUT_RATE * delta_time;
    }
}
