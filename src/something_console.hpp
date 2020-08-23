#ifndef SOMETHING_CONSOLE_HPP_
#define SOMETHING_CONSOLE_HPP_

const size_t CONSOLE_ROWS = 1024;
const size_t CONSOLE_COLUMNS = 256;

struct Console
{
    struct Selection
    {
        size_t begin;
        size_t end;

        bool is_empty()
        {
            return size() == 0;
        }

        size_t size()
        {
            return end - begin;
        }
    };

    bool visible;

    char rows[CONSOLE_ROWS][CONSOLE_COLUMNS];
    size_t rows_count[CONSOLE_ROWS];

    size_t begin;
    size_t count;

    char clipboard_buffer[CONSOLE_COLUMNS + 1];
    char edit_field[CONSOLE_COLUMNS];
    size_t edit_field_size;
    size_t edit_field_cursor;
    size_t edit_field_selection_begin;

    Selection get_selection() const;

    void render(SDL_Renderer *renderer, Bitmap_Font *font);
    void update(float dt);
    void toggle_visible();

    void cursor_left(bool selection);
    void cursor_right(bool selection);
    void insert_cstr(const char *cstr);
    void backspace_char();
    void delete_char();

    void println(const char *buffer, size_t buffer_size);

    void handle_event(SDL_Event *event);
};

#endif  // SOMETHING_CONSOLE_HPP_
