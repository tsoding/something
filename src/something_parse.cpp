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

    template <typename Number>
    Maybe<Number> as_number()
    {
        // TODO: as_number() does not support negative numbers
        Number number = {};

        while (count) {
            if (!isdigit(*data)) return {};
            number = number * 10 + (*data - '0');
            chop(1);
        }

        return { true, number };
    }
};

template <typename T>
struct Parse_Result
{
    bool is_error;
    String_View rest;
    T unwrap;
    const char *error;

    template <typename U>
    Parse_Result<U> refail()
    {
        Parse_Result<U> result = {};
        result.is_error = is_error;
        result.rest = rest;
        result.error = error;
        return result;
    }
};

template <typename T>
Parse_Result<T> parse_fail(String_View rest, const char *error)
{
    Parse_Result<T> result = {};
    result.is_error = true;
    result.rest = rest;
    result.error = error;
    return result;
}

template <typename T>
Parse_Result<T> parse_ok(String_View rest, T unwrap)
{
    Parse_Result<T> result = {};
    result.rest = rest;
    result.unwrap = unwrap;
    return result;
}


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
