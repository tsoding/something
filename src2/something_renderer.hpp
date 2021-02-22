#ifndef SOMETHING_RENDERER_HPP_
#define SOMETHING_RENDERER_HPP_

#include "./something_geo.hpp"
#include "./something_rgba.hpp"

struct Renderer {
    // The GLSL program that can render a rectangle
    GLuint rect_program;
    // The buffer that contains the quad for drawing rectangle
    GLuint quad_buffer;
    // Uniform location of the color
    GLuint u_color;
    GLuint u_rect_position;
    GLuint u_rect_size;

    void init();
    void fill_rect(AABB<float> aabb, RGBA rgba);
};

#endif  // SOMETHING_RENDERER_HPP_
