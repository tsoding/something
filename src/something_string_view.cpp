struct String_View
{
    size_t count;
    const char *data;

    String_View trim_begin(void) const
    {
        String_View view = *this;

        while (view.count != 0 && isspace(*view.data)) {
            view.data  += 1;
            view.count -= 1;
        }
        return view;
    }

    String_View trim_end(void) const
    {
        String_View view = *this;

        while (view.count != 0 && isspace(*(view.data + view.count - 1))) {
            view.count -= 1;
        }
        return view;
    }

    String_View trim(void) const
    {
        return trim_begin().trim_end();
    }

    String_View chop_by_delim(char delim)
    {
        assert(data);

        size_t i = 0;
        while (i < count && data[i] != delim) i++;

        String_View result;
        result.count = i;
        result.data = data;

        if (i < count) {
            count -= i + 1;
            data  += i + 1;
        } else {
            count -= i;
            data  += i;
        }

        return result;
    }
};

String_View string_view_of_cstr(const char *cstr)
{
    String_View result = {};
    result.count = strlen(cstr);
    result.data = cstr;
    return result;
}

String_View operator ""_sv (const char *data, size_t count)
{
    String_View result;
    result.count = count;
    result.data = data;
    return result;
}

bool operator==(String_View view1, String_View view2)
{
    if (view1.count != view2.count) return false;
    return memcmp(view1.data, view2.data, view1.count) == 0;
}

bool operator==(String_View view, const char *cstr)
{
    return view == string_view_of_cstr(cstr);
}

template <typename T>
Result<T, void> as_number(String_View view)
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

String_View file_as_string_view(const char *filepath)
{
    assert(filepath);

    size_t n = 0;
    String_View result = {};
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
