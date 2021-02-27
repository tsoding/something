#ifndef SOMETHING_TEXTURE_HPP_
#define SOMETHING_TEXTURE_HPP_

#include <stdint.h>

struct Texture {
    using RGBA32 = uint32_t;

    int width;
    int height;
    RGBA32 *pixels;

    GLuint gl_id;

    static Texture from_file(const char *file_path);
    static Texture from_memory(int width, int height, RGBA32 *pixels);
};

#endif  // SOMETHING_TEXTURE_HPP_
