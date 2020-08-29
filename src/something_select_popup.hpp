#ifndef SOMETHING_SELECT_POPUP_HPP_
#define SOMETHING_SELECT_POPUP_HPP_

const size_t SELECT_POPUP_CAPACITY = 10;

struct Select_Popup
{
    String_View items[SELECT_POPUP_CAPACITY];
    size_t items_size;
    size_t items_cursor;

    void render(SDL_Renderer *renderer, Bitmap_Font *font, Vec2f pos);
    void update(float dt);

    void up();
    void down();
    void push(String_View item);
    bool full();
    void clear();
};

#endif  // SOMETHING_SELECT_POPUP_HPP_
