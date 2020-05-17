template <typename T>
constexpr T min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
constexpr T max(T x, T y)
{
    return x > y ? x : y;
}

template <typename T>
constexpr T clamp(T x, T low, T high)
{
    return min(max(low, x), high);
}
