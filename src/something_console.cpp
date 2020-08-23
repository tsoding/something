#include "something_console.hpp"

void Console::render(SDL_Renderer *renderer, Bitmap_Font *font)
{
    if (visible) {
        const float CONSOLE_EDIT_FIELD_ROW = 1.0f;
        const float CONSOLE_HEIGHT = BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE * (CONSOLE_VISIBLE_ROWS + CONSOLE_EDIT_FIELD_ROW);

        // BACKGROUND
        fill_rect(renderer, rect(vec2(0.0f, 0.0f), SCREEN_WIDTH, CONSOLE_HEIGHT), CONSOLE_BACKGROUND_COLOR);

        // ROWS
        for (int i = 0; i < min(CONSOLE_VISIBLE_ROWS, (int) count); ++i) {
            const auto index = mod(begin + count - 1 - i, CONSOLE_ROWS);
            const auto position = vec2(0.0f, (float)((CONSOLE_VISIBLE_ROWS - i - 1) * BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE));
            font->render(renderer, position, vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                         FONT_DEBUG_COLOR, String_View {rows_count[index], rows[index]});
        }

        // EDIT FIELD
        const auto position = vec2(0.0f, (float)(CONSOLE_VISIBLE_ROWS * BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE));
        font->render(renderer, position, vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                     FONT_DEBUG_COLOR, String_View {edit_field_size, edit_field});

        // CURSOR
        const auto cursor_x = edit_field_cursor * BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE;
        fill_rect(renderer,
                  rect(vec2(cursor_x, position.y),
                       (float) (BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE),
                       (float) (BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE)),
                  FONT_DEBUG_COLOR);
        if (edit_field_cursor < edit_field_size) {
            font->render(renderer, vec2(cursor_x, position.y), vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                         CONSOLE_BACKGROUND_COLOR, String_View {1, edit_field + edit_field_cursor});
        }
    }
}

// TODO(#145): Console does not slide down
void Console::update(float dt)
{
    (void) dt;
}

void Console::toggle_visible()
{
    visible = !visible;
}

void Console::println(const char *buffer, size_t buffer_size)
{
    const size_t index = (begin + count) % CONSOLE_ROWS;
    rows_count[index] = min(buffer_size, CONSOLE_COLUMNS);
    memcpy(rows[index], buffer, rows_count[index]);

    if (count < (int) CONSOLE_ROWS) {
        count += 1;
    } else {
        begin = (begin + 1) % CONSOLE_ROWS;
    }
}

void Console::cursor_left()
{
    if (edit_field_cursor > 0) {
        edit_field_cursor -= 1;
    }
}

void Console::cursor_right()
{
    if (edit_field_cursor < edit_field_size) {
        edit_field_cursor += 1;
    }
}

void Console::insert_cstr(const char *cstr)
{
    const size_t remaining = CONSOLE_COLUMNS - edit_field_size;
    const size_t n = min(strlen(cstr), remaining);

    memmove(edit_field + edit_field_cursor + n,
            edit_field + edit_field_cursor,
            edit_field_size - edit_field_cursor);
    memcpy(edit_field + edit_field_cursor, cstr, n);
    edit_field_cursor += n;
    edit_field_size += n;
}

void Console::handle_event(SDL_Event *event)
{
    // TODO(#158): Backtick event bleeds into the Console
    if (visible) {
        // TODO(#146): No support for delete or backspace in console
        // TODO(#159): Console does not integrate with the OS clipboard
        switch (event->type) {
        case SDL_KEYDOWN: {
            switch (event->key.keysym.sym) {
            case SDLK_LEFT: {
                cursor_left();
            } break;

            case SDLK_RIGHT: {
                cursor_right();
            } break;

            case SDLK_v: {
                if (event->key.keysym.mod & KMOD_LCTRL) {
                    insert_cstr(SDL_GetClipboardText());
                }
            } break;

            case SDLK_RETURN: {
                String_View command = {edit_field_size, edit_field};
                if (command == "quit"_sv) {
                    exit(0);
                }
                println(edit_field, edit_field_size);
                edit_field_size = 0;
                edit_field_cursor = 0;
            } break;
            }
        } break;

        case SDL_TEXTINPUT: {
            insert_cstr(event->text.text);
        } break;
        }
    }
}
