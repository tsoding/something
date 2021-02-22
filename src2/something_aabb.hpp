#ifndef SOMETHING_AABB_HPP_
#define SOMETHING_AABB_HPP_

#include "./something_v2.hpp"

template <typename T>
struct AABB {
    V2<T> pos;
    V2<T> size;

    AABB() = default;

    AABB(V2<T> pos, V2<T> size):
        pos(pos), size(size)
    {}

    template <typename U>
    AABB<U> cast_to()
    {
        return AABB<U>(pos.template cast_to<U>(),
                       size.template cast_to<U>());
    }

    template <typename U>
    AABB<U> map(U (*f)(T))
    {
        return AABB(pos.map(f),
                    size.map(f));
    }
};

#endif // SOMETHING_AABB_HPP_
