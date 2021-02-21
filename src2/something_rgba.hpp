#ifndef SOMETHING_RGBA_HPP_
#define SOMETHING_RGBA_HPP_

struct RGBA {
    float r, g, b, a;

    RGBA() = default;

    RGBA(float r, float g, float b, float a):
        r(r), g(g), b(b), a(a)
    {}

    static const RGBA RED;
    static const RGBA BLUE;
};

const RGBA RGBA::RED = RGBA(1.0f, 0.0f, 0.0f, 1.0f);
const RGBA RGBA::BLUE = RGBA(0.0f, 0.0f, 1.0f, 1.0f);

#endif  // SOMETHING_RGBA_HPP_
