#ifndef SOMETHING_POPUP_HPP_
#define SOMETHING_POPUP_HPP_

const size_t POPUP_BUFFER_CAPACITY = 256;

struct Popup
{
    char buffer[POPUP_BUFFER_CAPACITY];
    int buffer_size;
    Bitmap_Font font;
    SDL_Color color;
    float a;

    void notify(SDL_Color color, const char *format, ...);
    void render(SDL_Renderer *renderer);
    void update(float delta_time);
};


#endif  // SOMETHING_POPUP_HPP_
