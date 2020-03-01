template <typename T>
struct String_View
{
    size_t count;
    const T *data;
};

template <typename T>
String_View<T> string_view_of_cstr(const char *cstr)
{
    String_View<T> result = {};
    result.count = strlen(cstr);
    result.data = cstr;
    return result;
}

String_View<char> operator ""_sv (const char *data, size_t count)
{
    String_View<char> result;
    result.count = count;
    result.data = data;
    return result;
}

template <typename T, typename Is_Space>
String_View<T> trim_begin(String_View<T> view, Is_Space is_space)
{
    while (view.count != 0 && is_space(*view.data)) {
        view.data  += 1;
        view.count -= 1;
    }
    return view;
}

template <typename T, typename Is_Space>
String_View<T> trim_end(String_View<T> view, Is_Space is_space)
{
    while (view.count != 0 && is_space(*(view.data + view.count - 1))) {
        view.count -= 1;
    }
    return view;
}

template <typename T, typename Is_Space>
String_View<T> trim(String_View<T> view, Is_Space is_space)
{
    return trim_end(trim_begin(view, is_space), is_space);
}

template <typename T>
String_View<T> chop_by_delim(String_View<T> *view, T delim)
{
    assert(view);
    assert(view->data);

    size_t i = 0;
    while (i < view->count && view->data[i] != delim) i++;

    String_View<T> result;
    result.count = i;
    result.data = view->data;

    if (i < view->count) {
        view->count -= i + 1;
        view->data  += i + 1;
    } else {
        view->count -= i;
        view->data  += i;
    }

    return result;
}

template <typename T>
bool operator==(String_View<T> view1, String_View<T> view2)
{
    if (view1.count != view2.count) return false;
    return memcmp(view1.data, view2.data, view1.count) == 0;
}

template <typename T>
bool operator==(String_View<T> view, const T *cstr)
{
    return view == string_view_of_cstr<T>(cstr);
}

template <typename T>
Result<T, void> as_number(String_View<char> view)
{
    T result = {};

    for (size_t i = 0; i < view.count; ++i) {
        if (!isdigit(view.data[i])) {
            return fail<T>();
        }

        result = result * 10 + (view.data[i] - '0');
    }

    return ok<T, void>(result);
}

String_View<char> file_as_string_view(const char *filepath)
{
    assert(filepath);

    size_t n = 0;
    String_View<char> result = {};
    FILE *f = fopen(filepath, "rb");
    assert(f);

    fseek(f, 0, SEEK_END);
    long m = ftell(f);
    fseek(f, 0, SEEK_SET);
    result.count = (size_t) m;
    char *buffer = new char[result.count];
    n = fread(buffer, 1, result.count, f);
    assert(n == result.count);
    result.data = buffer;

    return result;
}
