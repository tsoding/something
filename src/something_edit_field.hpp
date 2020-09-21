#ifndef SOMETHING_EDIT_FIELD_HPP_
#define SOMETHING_EDIT_FIELD_HPP_

const size_t CONSOLE_ROWS = 1024;
const size_t CONSOLE_COLUMNS = 256;
const int CONSOLE_HISTORY_CAPACITY = 69;

struct Edit_Field
{
    char clipboard_buffer[CONSOLE_COLUMNS + 1];
    char edit_field[CONSOLE_COLUMNS];
    size_t edit_field_size;
    size_t edit_field_cursor;
    size_t edit_field_selection_begin;
    bool completion_popup_enabled;
    float blink_angle;

    void render(SDL_Renderer *renderer, Bitmap_Font *font, Vec2f position);
    void update(float dt);
    void handle_event(SDL_Event *event);

    Vec2f cursor_position(Vec2f edit_field_position);

    String_View as_string_view();
    void clean();
    void copy_from_string_view(String_View s);

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

    Selection get_selection() const;
    void cursor_left(bool selection);
    void cursor_right(bool selection);
    void insert_cstr(const char *cstr);
    void insert_sv(String_View sv);
    void backspace_char();
    void delete_char();
    void delete_selection(Selection selection);
};

#endif  // SOMETHING_EDIT_FIELD_HPP_
