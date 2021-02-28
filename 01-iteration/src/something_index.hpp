#ifndef SOMETHING_INDEX_HPP_
#define SOMETHING_INDEX_HPP_

template <typename T>
struct Index
{
    size_t unwrap;

    bool operator==(const Index<T> that) const
    {
        return this->unwrap == that.unwrap;
    }

    bool operator!=(const Index<T> that) const
    {
        return !(*this == that);
    }
};

#endif  // SOMETHING_INDEX_HPP_
