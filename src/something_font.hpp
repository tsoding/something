#ifndef SOMETHING_FONT_HPP_
#define SOMETHING_FONT_HPP_

const int BITMAP_FONT_ROW_SIZE    = 18;
const int BITMAP_FONT_CHAR_WIDTH  = 7;
const int BITMAP_FONT_CHAR_HEIGHT = 9;

struct Bitmap_Font
{
    SDL_Texture *bitmap;

    void render(SDL_Renderer *renderer, Vec2f position, Vec2f size, RGBA color, String_View sv);
    void render(SDL_Renderer *renderer, Vec2f position, Vec2f size, RGBA color, const char *cstr);
    SDL_Rect char_rect(char x);

    Vec2f text_size(Vec2f size, String_View sv);
    Vec2f text_size(Vec2f size, const char *cstr);
};

#endif  // SOMETHING_FONT_HPP_
