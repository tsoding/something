#ifndef SOMETHING_CONSOLE_HPP_
#define SOMETHING_CONSOLE_HPP_

#include "something_select_popup.hpp"
#include "something_edit_field.hpp"

struct Game;

struct Console
{
    struct History
    {
        char entries[CONSOLE_HISTORY_CAPACITY][CONSOLE_COLUMNS];
        size_t entry_sizes[CONSOLE_HISTORY_CAPACITY];
        int end;
        int count;
        int cursor;

        void push(String_View entry);
        void up();
        void down();
        String_View current();
    };

    bool enabled;
    float slide_position;

    char rows[CONSOLE_ROWS][CONSOLE_COLUMNS];
    size_t rows_count[CONSOLE_ROWS];

    size_t begin;
    size_t count;
    size_t scroll;

    History history;

    Edit_Field edit_field;
    bool completion_popup_enabled;
    Select_Popup completion_popup;

    void render(SDL_Renderer *renderer, Bitmap_Font *font);
    void update(float dt);
    void toggle();

    void start_autocompletion();


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
