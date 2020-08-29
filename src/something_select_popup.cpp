#include "something_select_popup.hpp"

void Select_Popup::render(SDL_Renderer *renderer, Bitmap_Font *font, Vec2f pos)
{
    size_t longest_length = 0;
    for (size_t i = 0; i < items_size; ++i) {
        longest_length = max(longest_length, items[i].count);
    }

    const float popup_width = longest_length * BITMAP_FONT_CHAR_WIDTH * SELECT_POPUP_FONT_SIZE + SELECT_POPUP_PAD * 2;
    const float item_height = BITMAP_FONT_CHAR_HEIGHT * SELECT_POPUP_FONT_SIZE + SELECT_POPUP_PAD * 2;
    const float popup_height = items_size * item_height;

    fill_rect(renderer, rect(pos, popup_width, popup_height), SELECT_POPUP_BACKGROUND_COLOR);

    // TODO: Select_Popup does not render its cursor
    for (size_t i = 0; i < items_size; ++i) {
        const auto position = pos + vec2(0.0f, i * item_height) + vec2(SELECT_POPUP_PAD, SELECT_POPUP_PAD);
        font->render(renderer, position, vec2(SELECT_POPUP_FONT_SIZE, SELECT_POPUP_FONT_SIZE), SELECT_POPUP_FOREGROUND_COLOR, items[i]);
    }
}

void Select_Popup::update(float)
{
}

void Select_Popup::up()
{
    if (items_cursor > 0) {
        items_cursor -= 1;
    }
}

void Select_Popup::down()
{
    if (items_cursor < items_size - 1) {
        items_cursor += 1;
    }
}

void Select_Popup::push(String_View item)
{
    assert(items_size < SELECT_POPUP_CAPACITY);
    items[items_size++] = item;
}

void Select_Popup::clear()
{
    items_size = 0;
    items_cursor = 0;
}

bool Select_Popup::full()
{
    return items_size >= SELECT_POPUP_CAPACITY;
}
