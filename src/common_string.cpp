template <typename T>
struct Maybe
{
    bool has_value;
    T unwrap;
};

#define unwrap_into(lvalue, maybe)\
    do {\
        auto maybe_var = (maybe);\
        if (!maybe_var.has_value) return {};\
        (lvalue) = maybe_var.unwrap;\
    } while (0)

int hexchar_as_number(int c)
{
    if ('0' <= c && c <= '9') return c - '0';
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    return -1;
}

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

    void chop_back(size_t n)
    {
        count -= min(n, count);
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

    Maybe<float> as_float() const
    {
        char buffer[300] = {};
        memcpy(buffer, data, min(sizeof(buffer) - 1, count));
        char *endptr = NULL;
        float result = strtof(buffer, &endptr);

        if (buffer > endptr || (size_t) (endptr - buffer) != count) {
            return {};
        }

        return {true, result};
    }

    String_View subview(size_t start, size_t size) const
    {
        assert(start + size <= count);
        return String_View {size, data + start};
    }

    template <typename Integer>
    Maybe<Integer> from_hex() const
    {
        Integer result = Integer();

        for (size_t i = 0; i < count; ++i) {
            int x = hexchar_as_number(data[i]);
            if (x < 0) return {};
            result = (Integer) (result * 16 + x);
        }

        return {true, result};
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

String_View file_as_string_view(const char *filepath, char *buffer, size_t buffer_size)
{
    String_View result = {};
    FILE *f = fopen(filepath, "rb");
    if (!f) goto fail;

    result.data = buffer;
    result.count = fread(buffer, 1, buffer_size, f);
    if (ferror(f)) goto fail;

    fclose(f);
    return result;
fail:
    println(stderr, "Could not read file `", filepath, "`: ", strerror(errno));
    abort();
    return {};
}

String_View file_as_string_view(const char *filepath)
{
    assert(filepath);

    size_t n = 0;
    String_View result = {};
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        println(stderr, "Could not open file `", filepath, "`: ",
                strerror(errno));
        abort();
    }

    int code = fseek(f, 0, SEEK_END);
    if (code < 0) {
        println(stderr, "Could find the end of file ", filepath, ": ",
                strerror(errno));
        abort();
    }

    long m = ftell(f);
    if (m < 0) {
        println(stderr, "Could get the end of file ", filepath, ": ",
                strerror(errno));
        abort();
    }
    result.count = (size_t) m;

    code = fseek(f, 0, SEEK_SET);
    if (code < 0) {
        println(stderr, "Could not find the beginning of file ", filepath, ": ",
                strerror(errno));
        abort();
    }

    char *buffer = new char[result.count];
    if (!buffer) {
        println(stderr, "Could not allocate memory for file ", filepath, ": ",
                strerror(errno));
        abort();
    }

    n = fread(buffer, 1, result.count, f);
    if (n != result.count) {
        println(stderr, "Could not read file ", filepath, ": ",
                strerror(errno));
        abort();
    }

    result.data = buffer;

    return result;
}
