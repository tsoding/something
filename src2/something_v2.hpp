#ifndef SOMETHING_V2_HPP_
#define SOMETHING_V2_HPP_

template <typename T>
struct V2 {
    T x, y;

    V2() = default;

    V2(T a):
        x(a), y(a)
    {}

    V2(T x, T y):
        x(x), y(y)
    {}

    template <typename U>
    V2<U> cast_to()
    {
        return V2<U>(static_cast<U>(x),
                     static_cast<U>(y));
    }

    template <typename U>
    V2<U> map(U (*f)(T))
    {
        return V2<U>(f(x), f(y));
    }
};

template <typename T>
V2<T> operator+(V2<T> a, V2<T> b)
{
    return V2<T>(a.x + b.x, a.y + b.y);
}

template <typename T>
V2<T> &operator+=(V2<T> &a, V2<T> b)
{
    a.x += b.x;
    a.y += b.y;
    return a;
}

template <typename T>
V2<T> operator-(V2<T> a, V2<T> b)
{
    return V2<T>(a.x - b.x, a.y - b.y);
}

template <typename T>
V2<T> operator*(V2<T> a, T s)
{
    return V2<T>(a.x * s, a.y * s);
}

template <typename T>
V2<T> operator*(T s, V2<T> a)
{
    return V2<T>(a.x * s, a.y * s);
}

#endif  // SOMETHING_V2_HPP_
