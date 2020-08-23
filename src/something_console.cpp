#include "something_console.hpp"

Console::Selection Console::get_selection() const
{
    Selection result = {};

    if (edit_field_selection_begin <= edit_field_cursor) {
        result.begin = edit_field_selection_begin;
        result.end = edit_field_cursor;
    } else {
        result.begin = edit_field_cursor + 1;
        result.end = edit_field_selection_begin + 1;
    }

    assert(result.begin <= result.end);
    return result;
}

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

        // SELECTION
        // text:              the q[uick bro]w[n fox jump]s over the lazy dog
        // cursor:                           ^
        // selection_begin:         ^                   ^
        {
            auto selection = get_selection();

            if (!selection.is_empty()) {
                const float begin_x = (float) selection.begin * BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE;
                const float end_x = (float) selection.end * BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE;
                const float width = end_x - begin_x;

                fill_rect(renderer,
                          rect(vec2(begin_x, position.y),
                               width,
                               (float) (BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE)),
                          CONSOLE_SELECTION_COLOR);

                font->render(renderer, vec2(begin_x, position.y), vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                             CONSOLE_BACKGROUND_COLOR,
                             String_View {selection.end - selection.begin, edit_field + selection.begin});
            }
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

void Console::cursor_left(bool selection)
{
    if (edit_field_cursor > 0) {
        edit_field_cursor -= 1;
        if (!selection) {
            edit_field_selection_begin = edit_field_cursor;
        }
    }
}

void Console::cursor_right(bool selection)
{
    if (edit_field_cursor < edit_field_size) {
        edit_field_cursor += 1;
        if (!selection) {
            edit_field_selection_begin = edit_field_cursor;
        }
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
    edit_field_selection_begin = edit_field_cursor;
    edit_field_size += n;
}

void Console::delete_selection(Selection selection)
{
    assert(selection.begin < edit_field_size);
    assert(selection.end <= edit_field_size);

    memmove(edit_field + selection.begin,
            edit_field + selection.end,
            edit_field_size - selection.end);
    edit_field_cursor = selection.begin;
    edit_field_size -= selection.size();
    edit_field_selection_begin = edit_field_cursor;
}

void Console::delete_char()
{
    if (edit_field_cursor < edit_field_size) {
        const size_t n = edit_field_size - edit_field_cursor - 1;
        memmove(
            edit_field + edit_field_cursor,
            edit_field + edit_field_cursor + 1,
            n);
        edit_field_size -= 1;
        edit_field_selection_begin = edit_field_cursor;
    }
}

void Console::backspace_char()
{
    if (edit_field_cursor > 0) {
        const size_t n = edit_field_size - edit_field_cursor;
        memmove(
            edit_field + edit_field_cursor - 1,
            edit_field + edit_field_cursor,
            n);
        edit_field_cursor -= 1;
        edit_field_size -= 1;
        edit_field_selection_begin = edit_field_cursor;
    }
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
            case SDLK_DELETE: {
                const auto selection = get_selection();
                if (selection.is_empty()) {
                    delete_char();
                } else {
                    delete_selection(selection);
                }
            } break;

            case SDLK_BACKSPACE: {
                const auto selection = get_selection();
                if (selection.is_empty()) {
                    backspace_char();
                } else {
                    delete_selection(selection);
                }
            } break;

            case SDLK_LEFT: {
                cursor_left(event->key.keysym.mod & KMOD_LSHIFT);
            } break;

            case SDLK_RIGHT: {
                cursor_right(event->key.keysym.mod & KMOD_LSHIFT);
            } break;

            case SDLK_v: {
                if (event->key.keysym.mod & KMOD_LCTRL) {
                    insert_cstr(sec(SDL_GetClipboardText()));
                }
            } break;

            case SDLK_c: {
                if (event->key.keysym.mod & KMOD_LCTRL) {
                    auto selection = get_selection();
                    if (!selection.is_empty()) {
                        memcpy(clipboard_buffer, edit_field + selection.begin, selection.size());
                        clipboard_buffer[selection.size()] = '\0';
                        sec(SDL_SetClipboardText(clipboard_buffer));
                    }
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
                edit_field_selection_begin = 0;
            } break;
            }
        } break;

        case SDL_TEXTINPUT: {
            insert_cstr(event->text.text);
        } break;
        }
    }
}
