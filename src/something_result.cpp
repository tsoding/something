template <typename T, typename E>
struct Result
{
    bool is_error;
    union {
        T unwrap;
        E error;
    };
};

template <typename T>
struct Result<T, void>
{
    bool is_error;
    T unwrap;
};

template <typename T, typename E>
Result<T, E> fail(E error)
{
    Result<T, E> result = {};
    result.is_error = true;
    result.error = error;
    return result;
}

template <typename T>
Result<T, void> fail(void)
{
    Result<T, void> result = {};
    result.is_error = true;
    return result;
}

template <typename T, typename E>
Result<T, E> ok(T unwrap)
{
    Result<T, E> result = {};
    result.is_error = false;
    result.unwrap = unwrap;
    return result;
}
