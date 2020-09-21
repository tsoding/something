#include "something_game.hpp"
#include "something_console.hpp"

// TODO(#181): console does not support history
// TODO(#182): console does not support scrolling

void Console::render(SDL_Renderer *renderer, Bitmap_Font *font)
{
    if (slide_position > 0.0f) {
        const float CONSOLE_EDIT_FIELD_ROW = 1.0f;
        const float CONSOLE_HEIGHT = BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE * (CONSOLE_VISIBLE_ROWS + CONSOLE_EDIT_FIELD_ROW);

        const float console_y = -CONSOLE_HEIGHT + CONSOLE_HEIGHT * slide_position * slide_position;

        // BACKGROUND
        fill_rect(renderer, rect(vec2(0.0f, console_y), SCREEN_WIDTH, CONSOLE_HEIGHT), CONSOLE_BACKGROUND_COLOR);

        // ROWS
        for (int i = 0; i < min(CONSOLE_VISIBLE_ROWS, (int) count); ++i) {
            const auto index = mod(begin + count - 1 - i - scroll, CONSOLE_ROWS);
            const auto position = vec2(0.0f, console_y + (float)((CONSOLE_VISIBLE_ROWS - i - 1) * BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE));
            font->render(renderer, position, vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                         FONT_DEBUG_COLOR, String_View {rows_count[index], rows[index]});
        }

        // EDIT FIELD
        const auto prompt_size = font->text_size(vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE), CONSOLE_PROMPT);
        const auto edit_field_position =
            vec2(prompt_size.x, console_y + (float)(CONSOLE_VISIBLE_ROWS * BITMAP_FONT_CHAR_HEIGHT * CONSOLE_FONT_SIZE));
        font->render(renderer, edit_field_position - vec2(prompt_size.x, 0.0f), vec2(CONSOLE_FONT_SIZE, CONSOLE_FONT_SIZE),
                 FONT_DEBUG_COLOR, CONSOLE_PROMPT);
        edit_field.render(renderer, font, edit_field_position);

        // POPUP
        if (completion_popup_enabled) {
            completion_popup.render(renderer, font, edit_field.cursor_position(edit_field_position));
        }
    }
}

void Console::update(float dt)
{
    if (enabled) {
        if (slide_position < 1.0f) slide_position += dt * CONSOLE_SLIDE_SPEED;
        edit_field.update(dt);
    } else {
        if (slide_position > 0.0f) slide_position -= dt * CONSOLE_SLIDE_SPEED;
    }
}

void Console::toggle()
{
    enabled = !enabled;
    if (enabled) {
        SDL_StartTextInput();
    } else {
        SDL_StopTextInput();
    }
}

void Console::start_autocompletion()
{
    completion_popup.clear();
    String_View prefix = {edit_field.edit_field_cursor, edit_field.edit_field};
    for (size_t i = 0; i < commands_count && !completion_popup.full(); ++i) {
        if (commands[i].name.has_prefix(prefix)) {
            completion_popup.push(commands[i].name);
        }
    }
    completion_popup_enabled = true;
}

void Console::handle_event(SDL_Event *event, Game *game)
{
    if (enabled) {
        if (completion_popup_enabled) {
            switch (event->type) {
            case SDL_KEYDOWN: {
                switch (event->key.keysym.sym) {
                case SDLK_ESCAPE: {
                    completion_popup_enabled = false;
                } break;
                case SDLK_UP: {
                    completion_popup.up();
                } break;
                case SDLK_DOWN: {
                    completion_popup.down();
                } break;
                case SDLK_RETURN: {
                    auto s = completion_popup.items[completion_popup.items_cursor];
                    s.chop(edit_field.edit_field_cursor);
                    edit_field.insert_sv(s);
                    completion_popup_enabled = false;
                } break;
                }
            } break;
            }
        } else {
            if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
                scroll = 0;
                String_View command_expr = edit_field.as_string_view().trim();

                this->println(CONSOLE_PROMPT, edit_field.as_string_view());
                history.push(edit_field.as_string_view());
                edit_field.clean();

                if (command_expr.count > 0) {
                    bool command_found = false;
                    const auto command_name = command_expr.chop_word();
                    for (size_t i = 0; !command_found && i < commands_count; ++i) {
                        if (commands[i].name == command_name) {
                            commands[i].run(game, command_expr);
                            command_found = true;
                        }
                    }

                    if (!command_found) {
                        this->println("Command `", command_name, "` not found");
                    }
                }
            } else if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_SPACE && event->key.keysym.mod & KMOD_LCTRL) {
                start_autocompletion();
            } else if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_UP) {
                const auto j = history.current();
                if (j >= 0) {
                    edit_field.copy_from_string_view(String_View {history.entry_sizes[j], history.entries[j]});
                }
                history.up();
            } else if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_DOWN) {
                const auto j = history.current();
                if (j >= 0) {
                    edit_field.copy_from_string_view(String_View {history.entry_sizes[j], history.entries[j]});
                }
                history.down();
            } else if (event->type == SDL_MOUSEWHEEL ) {
                if (event->wheel.y > 0) {
                    scroll += 1;
                } else if (event->wheel.y < 0) {
                    if (scroll > 0) scroll -= 1;
                }
            } else {
                edit_field.handle_event(event);
            }
        }
    }
}

void Console::History::push(String_View entry)
{
    if (entry.count > CONSOLE_COLUMNS) {
        entry.count = CONSOLE_COLUMNS;
    }

    const size_t j = (begin + count) % CONSOLE_HISTORY_CAPACITY;
    memcpy(&entries[j], entry.data, entry.count);
    entry_sizes[j] = entry.count;

    if (count < CONSOLE_HISTORY_CAPACITY) {
        count += 1;
    } else {
        begin = (begin + 1) % CONSOLE_HISTORY_CAPACITY;
    }

    cursor = begin;
}

void Console::History::up()
{
    if (cursor < count) {
        cursor += 1;
    }
}

void Console::History::down()
{
    if (cursor > 0) {
        cursor -= 1;
    }
}

int Console::History::current()
{
    if ((count - cursor) > 0) {
        return mod(begin + count - 1 - cursor, CONSOLE_HISTORY_CAPACITY);
    } else {
        return -1;
    }
}
