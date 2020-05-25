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

SDL_Rect Bitmap_Font::char_rect(char x)
{
    if (32 <= x && x <= 126) {
        const SDL_Rect rect = {
            ((x - 32) % BITMAP_FONT_ROW_SIZE) * BITMAP_FONT_CHAR_WIDTH,
            ((x - 32) / BITMAP_FONT_ROW_SIZE) * BITMAP_FONT_CHAR_HEIGHT,
            BITMAP_FONT_CHAR_WIDTH,
            BITMAP_FONT_CHAR_HEIGHT
        };
        return rect;
    } else {
        return char_rect('?');
    }
}

void Bitmap_Font::render(SDL_Renderer *renderer, Vec2f position, SDL_Color color, String_View sv)
{
    sec(SDL_SetTextureColorMod(bitmap, color.r, color.g, color.b));
    sec(SDL_SetTextureAlphaMod(bitmap, color.a));

    for (int i = 0, col = 0, row = 0; i < (int) sv.count; ++i, col++) {
        if (sv.data[i] == '\n'){
            col = 0;
            row++;
            continue;
        }
        const SDL_Rect src_rect = char_rect(sv.data[i]);
        const SDL_Rect dest_rect = {
            (int) floorf(position.x) + BITMAP_FONT_CHAR_WIDTH  * col * (int) floorf(size.x),
            (int) floorf(position.y) + BITMAP_FONT_CHAR_HEIGHT * row * (int) floorf(size.y),
            src_rect.w * (int) floorf(size.x),
            src_rect.h * (int) floorf(size.y)
        };
        sec(SDL_RenderCopy(renderer, bitmap, &src_rect, &dest_rect));
    }
}

void Bitmap_Font::render(SDL_Renderer *renderer, Vec2f position, SDL_Color color, const char *cstr)
{
    render(renderer, position, color, cstr_as_string_view(cstr));
}

Vec2f Bitmap_Font::text_size(String_View sv)
{
    size_t lines_count = 0;
    size_t longest_line = 0;
    while (sv.count > 0) {
        String_View line = sv.chop_by_delim('\n');
        lines_count += 1;
        longest_line = max(longest_line, line.count);
    }

    return vec2((float) longest_line * BITMAP_FONT_CHAR_WIDTH * size.x,
                (float) lines_count * BITMAP_FONT_CHAR_HEIGHT * size.y);
}

Vec2f Bitmap_Font::text_size(const char *cstr)
{
    return text_size(cstr_as_string_view(cstr));
}
