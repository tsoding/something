#ifndef SOMETHING_INDEX_HPP_
#define SOMETHING_INDEX_HPP_

template <typename That>
struct Index
{
    size_t unwrap;

    bool operator==(const That that) const
    {
        return this->unwrap == that.unwrap;
    }

    bool operator!=(const That that) const
    {
        return !(*this == that);
    }
};

struct Entity_Index: public Index<Entity_Index> {};
struct Projectile_Index: public Index<Projectile_Index> {};
struct Texture_Index: public Index<Texture_Index> {};
struct Sample_S16_Index: public Index<Sample_S16_Index> {};
struct Frame_Animat_Index: public Index<Frame_Animat_Index> {};

#endif  // SOMETHING_INDEX_HPP_
