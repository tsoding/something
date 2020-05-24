#ifndef SOMETHING_FONT_HPP_
#define SOMETHING_FONT_HPP_

const size_t CACHED_FONT_CAPACITY = 256;

struct Cached_Font
{
    TTF_Font *font;
    SDL_Texture *glyph_cache[CACHED_FONT_CAPACITY];

    void populate_cache(SDL_Renderer *renderer);
    void render_sv(SDL_Renderer *render, Vec2f position, SDL_Color color, String_View sv);
    void render_cstr(SDL_Renderer *render, Vec2f position, SDL_Color color, const char *cstr);
};

#endif  // SOMETHING_FONT_HPP_
