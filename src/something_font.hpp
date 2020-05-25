#ifndef SOMETHING_FONT_HPP_
#define SOMETHING_FONT_HPP_

const size_t CACHED_FONT_CAPACITY = 256;

struct Cached_Font
{
    TTF_Font *font;
    SDL_Texture *glyph_cache[CACHED_FONT_CAPACITY];

    void populate_cache(SDL_Renderer *renderer);
    void render(SDL_Renderer *render, Vec2f position, SDL_Color color, String_View sv);
    void render(SDL_Renderer *render, Vec2f position, SDL_Color color, const char *cstr);
};

const int BITMAP_FONT_ROW_SIZE    = 18;
const int BITMAP_FONT_CHAR_WIDTH  = 7;
const int BITMAP_FONT_CHAR_HEIGHT = 9;

struct Bitmap_Font
{
    SDL_Texture *bitmap;
    Vec2f size;

    void render(SDL_Renderer *renderer, Vec2f position, SDL_Color color, String_View sv);
    void render(SDL_Renderer *renderer, Vec2f position, SDL_Color color, const char *cstr);
    SDL_Rect char_rect(char x);

    Vec2f text_size(String_View sv);
    Vec2f text_size(const char *cstr);
};

#endif  // SOMETHING_FONT_HPP_
