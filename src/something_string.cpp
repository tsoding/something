template <typename T>
struct Maybe
{
    bool has_value;
    T unwrap;
};

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

    void chop(size_t n)
    {
        if (n > count) {
            data += count;
            count = 0;
        } else {
            data  += n;
            count -= n;
        }
    }

    String_View chop_by_delim(char delim)
    {
        assert(data);

        size_t i = 0;
        while (i < count && data[i] != delim) i++;
        String_View result = {i, data};
        chop(i + 1);

        return result;
    }

    template <typename Integer>
    Maybe<Integer> as_integer() const
    {
        Integer sign = 1;
        Integer number = {};
        String_View view = *this;

        if (view.count == 0) {
            return {};
        }

        if (*view.data == '-') {
            sign = -1;
            view.chop(1);
        }

        while (view.count) {
            if (!isdigit(*view.data)) {
                return {};
            }
            number = number * 10 + (*view.data - '0');
            view.chop(1);
        }

        return { true, number * sign };
    }
};

void print1(FILE *stream, String_View view)
{
    fwrite(view.data, 1, view.count, stream);
}

String_View cstr_as_string_view(const char *cstr)
{
    return String_View {strlen(cstr), cstr};
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

String_View file_as_string_view(const char *filepath)
{
    assert(filepath);

    size_t n = 0;
    String_View result = {};
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        fprintf(stderr, "Could not open file %s: %s\n",
                filepath, strerror(errno));
        abort();
    }

    int code = fseek(f, 0, SEEK_END);
    if (code < 0) {
        fprintf(stderr, "Could find the end of file %s: %s\n",
                filepath, strerror(errno));
        abort();
    }

    long m = ftell(f);
    if (m < 0) {
        fprintf(stderr, "Could get the end of file %s: %s\n",
                filepath, strerror(errno));
        abort();
    }
    result.count = (size_t) m;

    code = fseek(f, 0, SEEK_SET);
    if (code < 0) {
        fprintf(stderr, "Could not find the beginning of file %s: %s\n",
                filepath, strerror(errno));
        abort();
    }

    char *buffer = new char[result.count];
    if (!buffer) {
        fprintf(stderr, "Could not allocate memory for file %s: %s\n",
                filepath, strerror(errno));
        abort();
    }

    n = fread(buffer, 1, result.count, f);
    if (n != result.count) {
        fprintf(stderr, "Could not read file %s: %s\n",
                filepath, strerror(errno));
        abort();
    }

    result.data = buffer;

    return result;
}
