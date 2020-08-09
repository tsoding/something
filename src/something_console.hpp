#ifndef SOMETHING_CONSOLE_HPP_
#define SOMETHING_CONSOLE_HPP_

const size_t CONSOLE_ROWS = 1024;
const size_t CONSOLE_COLUMNS = 256;

struct Console
{
    bool visible;
    char rows[CONSOLE_ROWS][CONSOLE_COLUMNS];
    size_t rows_count[CONSOLE_ROWS];
    int begin;
    int count;

    void render(SDL_Renderer *renderer, Bitmap_Font *font);
    void update(float dt);
    void toggle_visible();
    void println(const char *cstr);
};

#endif  // SOMETHING_CONSOLE_HPP_
