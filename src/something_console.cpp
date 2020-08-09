#include "something_console.hpp"

void Console::render(SDL_Renderer *renderer, Bitmap_Font *font)
{
    if (visible) {
        const float CONSOLE_EDIT_FIELD_ROW = 1.0f;
        const float CONSOLE_HEIGHT = BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE * (CONSOLE_VISIBLE_ROWS + CONSOLE_EDIT_FIELD_ROW);

        sec(SDL_SetRenderDrawColor(
                renderer,
                CONSOLE_BACKGROUND_COLOR.r,
                CONSOLE_BACKGROUND_COLOR.g,
                CONSOLE_BACKGROUND_COLOR.b,
                CONSOLE_BACKGROUND_COLOR.a));
        SDL_Rect rect = {
            0, 0,
            (int) SCREEN_WIDTH,
            (int) floorf(CONSOLE_HEIGHT)
        };
        sec(SDL_RenderFillRect(renderer, &rect));

        for (int i = 0; i < min(CONSOLE_VISIBLE_ROWS, count); ++i) {
            const auto index = mod(begin + count - 1 - i, (int) CONSOLE_ROWS);
            const auto position = vec2(0.0f, (float)((CONSOLE_VISIBLE_ROWS - i - 1) * BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE));
            font->render(renderer, position, vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                         FONT_DEBUG_COLOR, String_View {rows_count[index], rows[index]});
        }
    }
}

void Console::update(float dt)
{
    (void) dt;
}

void Console::toggle_visible()
{
    visible = !visible;
}

void Console::println(const char *cstr)
{
    const size_t index = (begin + count) % CONSOLE_ROWS;
    rows_count[index] = min(strlen(cstr), CONSOLE_COLUMNS);
    memcpy(rows[index], cstr, rows_count[index]);

    if (count < (int) CONSOLE_ROWS) {
        count += 1;
    } else {
        begin = (begin + 1) % CONSOLE_ROWS;
    }
}
