#ifndef SOMETHING_PRINT_HPP_
#define SOMETHING_PRINT_HPP_

#include "./something_particles.hpp"

void sprint1(String_Buffer *sbuffer, SDL_Color color)
{
    sprint(sbuffer, "{", color.r, ",", color.g, ",", color.b, ",", color.a, "}");
}

void print1(FILE *stream, SDL_Color color)
{
    print(stream, "{", color.r, ",", color.g, ",", color.b, ",", color.a, "}");
}

template <typename T>
void print1(FILE *stream, Vec2<T> v)
{
    print(stream, '(', v.x, ',', v.y, ')');
}

void sprint1(String_Buffer *sbuffer, Particles::State state)
{
    switch (state) {
    case Particles::DISABLED:
        sprint(sbuffer, "DISABLED");
        break;
    case Particles::EMITTING:
        sprint(sbuffer, "EMITTING");
        break;
    }
}

#endif  // SOMETHING_PRINT_HPP_
