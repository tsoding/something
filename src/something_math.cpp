#ifndef VEC_HPP_
#define VEC_HPP_

template <typename T>
struct Vec2
{
    T x, y;
};

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
static inline
T sqr_dist(Vec2<T> p0, Vec2<T> p1)
{
    auto d = p0 - p1;
    return d.x * d.x + d.y * d.y;
}

bool rect_contains_vec2i(SDL_Rect rect, Vec2i point)
{
    return rect.x <= point.x && point.x < (rect.x + rect.w)
        && rect.y <= point.y && point.y < (rect.y + rect.h);
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
// Algorithms
//////////////////////////////

template <typename T>
T min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
T abs(T x)
{
    return x < 0 ? -x : x;
}

#endif  // VEC_HPP_
