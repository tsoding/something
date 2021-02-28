#include "./something_font.hpp"

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

void Bitmap_Font::render(SDL_Renderer *renderer, Vec2f position, Vec2f size, RGBA color, String_View sv)
{
    SDL_Color sdl_color = rgba_to_sdl(color);
    sec(SDL_SetTextureColorMod(bitmap, sdl_color.r, sdl_color.g, sdl_color.b));
    sec(SDL_SetTextureAlphaMod(bitmap, sdl_color.a));

    for (int row = 0; sv.count > 0; ++row) {
        auto line = sv.chop_by_delim('\n');

        for (int col = 0; (size_t) col < line.count; ++col) {
            const SDL_Rect src_rect = char_rect(line.data[col]);
            const SDL_Rect dest_rect = {
                (int) floorf(position.x + BITMAP_FONT_CHAR_WIDTH  * col * size.x),
                (int) floorf(position.y + BITMAP_FONT_CHAR_HEIGHT * row * size.y),
                (int) floorf(src_rect.w * size.x),
                (int) floorf(src_rect.h * size.y)
            };
            sec(SDL_RenderCopy(renderer, bitmap, &src_rect, &dest_rect));
        }
    }
}

void Bitmap_Font::render(SDL_Renderer *renderer, Vec2f position, Vec2f size, RGBA color, const char *cstr)
{
    render(renderer, position, size, color, cstr_as_string_view(cstr));
}

Vec2f Bitmap_Font::text_size(Vec2f size, String_View sv)
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

Vec2f Bitmap_Font::text_size(Vec2f size, const char *cstr)
{
    return text_size(size, cstr_as_string_view(cstr));
}
