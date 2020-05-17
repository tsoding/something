const float PI = 3.14159274101f;

template <typename T>
struct Vec2
{
    T x, y;
};

using Vec2f = Vec2<float>;
using Vec2i = Vec2<int>;

template <typename T> Vec2<T> vec2(T x, T y) { return {x, y}; }

//////////////////////////////
// Vector x Vector
//////////////////////////////

template <typename T> Vec2<T> constexpr operator+(Vec2<T> a, Vec2<T> b) { return {a.x + b.x, a.y + b.y}; }
template <typename T> Vec2<T> constexpr operator-(Vec2<T> a, Vec2<T> b) { return {a.x - b.x, a.y - b.y}; }
template <typename T> Vec2<T> constexpr operator*(Vec2<T> a, Vec2<T> b) { return {a.x * b.x, a.y * b.y}; }
template <typename T> Vec2<T> constexpr operator/(Vec2<T> a, Vec2<T> b) { return {a.x / b.x, a.y / b.y}; }
template <typename T> Vec2<T> constexpr &operator+=(Vec2<T> &a, Vec2<T> b) { a = a + b; return a; }
template <typename T> Vec2<T> constexpr &operator-=(Vec2<T> &a, Vec2<T> b) { a = a - b; return a; }
template <typename T> Vec2<T> constexpr &operator*=(Vec2<T> &a, Vec2<T> b) { a = a * b; return a; }
template <typename T> Vec2<T> constexpr &operator/=(Vec2<T> &a, Vec2<T> b) { a = a / b; return a; }

template <typename T>
constexpr
T sqr_len(Vec2<T> p)
{
    return p.x * p.x + p.y * p.y;
}

template <typename T>
constexpr
T sqr_dist(Vec2<T> p0, Vec2<T> p1)
{
    return sqr_len(p0 - p1);
}

template <typename T>
struct Rect
{
    T x, y, w, h;
};

using Rectf = Rect<float>;

template <typename T>
bool rect_contains_vec2(Rect<T> rect, Vec2<T> point)
{
    return rect.x <= point.x && point.x < (rect.x + rect.w)
        && rect.y <= point.y && point.y < (rect.y + rect.h);
}

template <typename T>
Vec2<T> rect_top_left(Rect<T> rect)
{
    return vec2(rect.x, rect.y);
}

//////////////////////////////
// Vector x Scalar
//////////////////////////////

template <typename T> Vec2<T> constexpr operator+(Vec2<T> a, T b) { return {a.x + b, a.y + b}; }
template <typename T> Vec2<T> constexpr operator-(Vec2<T> a, T b) { return {a.x - b, a.y - b}; }
template <typename T> Vec2<T> constexpr operator*(Vec2<T> a, T b) { return {a.x * b, a.y * b}; }
template <typename T> Vec2<T> constexpr operator/(Vec2<T> a, T b) { return {a.x / b, a.y / b}; }
template <typename T> Vec2<T> constexpr &operator+=(Vec2<T> &a, T b) { a = a + b; return a; }
template <typename T> Vec2<T> constexpr &operator-=(Vec2<T> &a, T b) { a = a - b; return a; }
template <typename T> Vec2<T> constexpr &operator*=(Vec2<T> &a, T b) { a = a * b; return a; }
template <typename T> Vec2<T> constexpr &operator/=(Vec2<T> &a, T b) { a = a / b; return a; }

//////////////////////////////
// Scalar x Vector
//////////////////////////////

template <typename T> Vec2<T> constexpr operator+(T a, Vec2<T> b) { return {a + b.x, a + b.y}; }
template <typename T> Vec2<T> constexpr operator-(T a, Vec2<T> b) { return {a - b.x, a - b.y}; }
template <typename T> Vec2<T> constexpr operator*(T a, Vec2<T> b) { return {a * b.x, a * b.y}; }
template <typename T> Vec2<T> constexpr operator/(T a, Vec2<T> b) { return {a / b.x, a / b.y}; }

//////////////////////////////
// Just Vector
//////////////////////////////

template <typename T> Vec2<T> constexpr operator-(Vec2<T> a) { return {-a.x, -a.y}; }

//////////////////////////////
// Rect x Vector
//////////////////////////////

template <typename T>
Rect<T> operator-(Rect<T> a, Vec2<T> b)
{
    return {a.x - b.x, a.y - b.y, a.w, a.h};
}

template <typename T>
Rect<T> operator+(Rect<T> a, Vec2<T> b)
{
    return {a.x + b.x, a.y + b.y, a.w, a.h};
}

//////////////////////////////
// Algorithms
//////////////////////////////

template <typename T>
T abs(T x)
{
    return x < 0 ? -x : x;
}

template <typename T>
constexpr Rect<T> rect(Vec2<T> pos, T w, T h)
{
    return {pos.x, pos.y, w, h};
}

SDL_Rect rectf_for_sdl(Rectf rect)
{
    return {(int) floorf(rect.x),
            (int) floorf(rect.y),
            (int) floorf(rect.w),
            (int) floorf(rect.h)};
}

template <typename U, typename T>
Vec2<U> vec_cast(Vec2<T> v)
{
    return {(U) v.x, (U) v.y};
}

template<>
Vec2<int> vec_cast(Vec2<float> v)
{
    return { (int) floorf(v.x), (int) floorf(v.y) };
}

Vec2<float> normalize(Vec2<float> a)
{
    return a / sqrtf(sqr_len(a));
}

template <typename T> T sgn(T val) {
    return fabsf(val) / val;
    if (val < 0) {
        return -1;
    } else if (val > 0) {
        return 1;
    }

    return 0;
}
