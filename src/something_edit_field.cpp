#include "something_edit_field.hpp"

void Edit_Field::render(SDL_Renderer *renderer, Bitmap_Font *font, Vec2f edit_field_position)
{
    // EDIT FIELD
    font->render(renderer, edit_field_position, vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                 FONT_DEBUG_COLOR, String_View {edit_field_size, edit_field});

    // CURSOR
    const auto cursor_pos = cursor_position(edit_field_position);
    {
        SDL_Color cursor_color = FONT_DEBUG_COLOR;
        const float blink_alpha = cosf(blink_angle * CONSOLE_BLINK_FREQUENCY) * 0.5f + 0.5f;
        cursor_color.a = (Uint8) floorf(blink_alpha * 255.0f);
        fill_rect(renderer,
                  rect(cursor_pos,
                       (float) (BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE),
                       (float) (BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE)),
                  cursor_color);
        if (edit_field_cursor < edit_field_size) {
            SDL_Color overlay_text_color = CONSOLE_BACKGROUND_COLOR;
            overlay_text_color.a = cursor_color.a;
            font->render(renderer, cursor_pos, vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                         overlay_text_color, String_View {1, edit_field + edit_field_cursor});
        }
    }

    // SELECTION
    // text:              the q[uick bro]w[n fox jump]s over the lazy dog
    // cursor:                           ^
    // selection_begin:         ^                   ^
    {
        auto selection = get_selection();

        if (!selection.is_empty()) {
            const float begin_x = (float) selection.begin * BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE + edit_field_position.x;
            const float end_x = (float) selection.end * BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE + edit_field_position.x;
            const float width = end_x - begin_x;

            fill_rect(renderer,
                      rect(vec2(begin_x, edit_field_position.y),
                           width,
                           (float) (BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE)),
                      CONSOLE_SELECTION_COLOR);

            font->render(renderer, vec2(begin_x, edit_field_position.y), vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                         CONSOLE_BACKGROUND_COLOR,
                         String_View {selection.end - selection.begin, edit_field + selection.begin});
        }
        }
}

Vec2f Edit_Field::cursor_position(Vec2f edit_field_position)
{
    const auto cursor_x = edit_field_cursor * BITMAP_FONT_CHAR_WIDTH * CONSOLE_FONT_SIZE + edit_field_position.x;
    return {cursor_x, edit_field_position.y};
}

void Edit_Field::update(float dt)
{
    blink_angle = fmodf(blink_angle + 2 * PI * dt, 2 * PI);
}

void Edit_Field::handle_event(SDL_Event *event)
{
    blink_angle = 0.0;
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
        }
    } break;


    case SDL_TEXTINPUT: {
        insert_cstr(event->text.text);
    } break;
    }
}

Edit_Field::Selection Edit_Field::get_selection() const
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
void Edit_Field::cursor_left(bool selection)
{
    if (edit_field_cursor > 0) {
        edit_field_cursor -= 1;
        if (!selection) {
            edit_field_selection_begin = edit_field_cursor;
        }
    }
}

void Edit_Field::cursor_right(bool selection)
{
    if (edit_field_cursor < edit_field_size) {
        edit_field_cursor += 1;
        if (!selection) {
            edit_field_selection_begin = edit_field_cursor;
        }
    }
}
void Edit_Field::insert_sv(String_View sv)
{
    const size_t remaining = CONSOLE_COLUMNS - edit_field_size;
    const size_t n = min(sv.count, remaining);

    memmove(edit_field + edit_field_cursor + n,
            edit_field + edit_field_cursor,
            edit_field_size - edit_field_cursor);
    memcpy(edit_field + edit_field_cursor, sv.data, n);
    edit_field_cursor += n;
    edit_field_selection_begin = edit_field_cursor;
    edit_field_size += n;
}

void Edit_Field::insert_cstr(const char *cstr)
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

void Edit_Field::delete_selection(Selection selection)
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

void Edit_Field::delete_char()
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

void Edit_Field::backspace_char()
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

String_View Edit_Field::as_string_view()
{
    return String_View {edit_field_size, edit_field};
}

void Edit_Field::clean()
{
    edit_field_size = 0;
    edit_field_cursor = 0;
    edit_field_selection_begin = 0;
}

void Edit_Field::copy_from_string_view(String_View s)
{
    memcpy(edit_field, s.data, s.count);
    edit_field_size = s.count;
    edit_field_cursor = s.count;
    edit_field_selection_begin = edit_field_cursor;
}
