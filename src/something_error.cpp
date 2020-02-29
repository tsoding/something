template <typename T>
T *stec(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError());
        abort();
    }

    return ptr;
}

void stec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL_ttf pooped itself: %s\n", TTF_GetError());
        abort();
    }
}

void sec(int code)
{
    if (code < 0) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }
}

template <typename T>
T *sec(T *ptr)
{
    if (ptr == nullptr) {
        fprintf(stderr, "SDL pooped itself: %s\n", SDL_GetError());
        abort();
    }

    return ptr;
}
