#include "./something_font.hpp"

void Cached_Font::populate_cache(SDL_Renderer *renderer)
{
    if (font) {
        for (Uint16 glyph = 1; glyph < CACHED_FONT_CAPACITY; ++glyph) {
            if (TTF_GlyphIsProvided(font, glyph)) {
                SDL_Surface *surface = stec(TTF_RenderGlyph_Blended(font, glyph, {255, 255, 255, 255}));
                glyph_cache[glyph] = sec(SDL_CreateTextureFromSurface(renderer, surface));
                SDL_FreeSurface(surface);
            }
        }
    }
}

void Cached_Font::render(SDL_Renderer *renderer,
                         Vec2f position, SDL_Color color,
                         String_View sv)
{
    if (font) {
        int pen_x = 0;
        for (size_t i = 0; i < sv.count; ++i) {
            const Uint16 glyph = sv.data[i];
            if (0 < glyph && glyph < CACHED_FONT_CAPACITY && glyph_cache[glyph]) {
                int minx, maxx, miny, maxy, advance;
                stec(TTF_GlyphMetrics(font, glyph, &minx, &maxx, &miny, &maxy, &advance));
                const int w = maxx - minx;
                const int h = maxy - miny;
                const SDL_Rect srcrect = {
                    minx, TTF_FontAscent(font) - maxy,
                    w, h
                };
                const SDL_Rect dstrect = {
                    (int) floorf(position.x) + pen_x + minx,
                    (int) floorf(position.y) + miny + TTF_FontAscent(font) - maxy,
                    w, h
                };

                sec(SDL_SetTextureColorMod(glyph_cache[glyph], color.r, color.g, color.b));
                sec(SDL_SetTextureAlphaMod(glyph_cache[glyph], color.a));
                sec(SDL_RenderCopy(renderer, glyph_cache[glyph], &srcrect, &dstrect));
                pen_x += advance;
            }
        }
    }
}

void Cached_Font::render(SDL_Renderer *renderer,
                         Vec2f position, SDL_Color color,
                         const char *cstr)
{
    render(renderer, position, color, cstr_as_string_view(cstr));
}
