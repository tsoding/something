#ifndef SOMETHING_CONSOLE_HPP_
#define SOMETHING_CONSOLE_HPP_

#include "something_select_popup.hpp"

const size_t CONSOLE_ROWS = 1024;
const size_t CONSOLE_COLUMNS = 256;

struct Game;

struct Console
{
    struct Selection
    {
        size_t begin;
        size_t end;

        bool is_empty() const
        {
            return size() == 0;
        }

        size_t size() const
        {
            return end - begin;
        }
    };

    bool enabled;
    float a;

    char rows[CONSOLE_ROWS][CONSOLE_COLUMNS];
    size_t rows_count[CONSOLE_ROWS];

    size_t begin;
    size_t count;

    char clipboard_buffer[CONSOLE_COLUMNS + 1];
    char edit_field[CONSOLE_COLUMNS];
    size_t edit_field_size;
    size_t edit_field_cursor;
    size_t edit_field_selection_begin;
    Select_Popup popup;
    bool popup_enabled;

    Selection get_selection() const;

    void render(SDL_Renderer *renderer, Bitmap_Font *font);
    void update(float dt);
    void toggle();

    void start_autocompletion();
    void cursor_left(bool selection);
    void cursor_right(bool selection);
    void insert_cstr(const char *cstr);
    void insert_sv(String_View sv);
    void backspace_char();
    void delete_char();
    void delete_selection(Selection selection);

    template <typename ... Types>
    void println(Types... args)
    {
        const size_t index = (begin + count) % CONSOLE_ROWS;
        String_Buffer sbuffer = {CONSOLE_COLUMNS, rows[index], rows_count[index]};
        sprintln(&sbuffer, args...);
        rows_count[index] = sbuffer.size;

        if (count < (int) CONSOLE_ROWS) {
            count += 1;
        } else {
            begin = (begin + 1) % CONSOLE_ROWS;
        }
    }

    void handle_event(SDL_Event *event, Game *game);
};

#endif  // SOMETHING_CONSOLE_HPP_
