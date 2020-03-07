template <typename T>
T *stec(T *ptr)
{
    if (ptr == nullptr) {
        println(stderr, "SDL_ttf pooped itself: ", TTF_GetError());
        abort();
    }

    return ptr;
}

void stec(int code)
{
    if (code < 0) {
        println(stderr, "SDL_ttf pooped itself: ", TTF_GetError());
        abort();
    }
}

void sec(int code)
{
    if (code < 0) {
        println(stderr, "SDL pooped itself: ", SDL_GetError());
        abort();
    }
}

template <typename T>
T *sec(T *ptr)
{
    if (ptr == nullptr) {
        println(stderr, "SDL pooped itself: ", SDL_GetError());
        abort();
    }

    return ptr;
}
