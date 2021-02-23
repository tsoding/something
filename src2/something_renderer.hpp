#ifndef SOMETHING_RENDERER_HPP_
#define SOMETHING_RENDERER_HPP_

#include "./something_geo.hpp"
#include "./something_rgba.hpp"

struct Renderer {
    static const size_t BATCH_BUFFER_CAPACITY = 1024;

    // The GLSL program that can render a rectangle
    GLuint rect_program;

    // Buffers
    GLuint triangles_buffer_id;
    GLuint colors_buffer_id;

    Triangle<GLfloat> triangles_buffer[BATCH_BUFFER_CAPACITY];
    RGBA colors_buffer[BATCH_BUFFER_CAPACITY * 3];
    size_t batch_buffer_size;

    void init();
    void fill_triangle(Triangle<GLfloat> triangle, RGBA rgba);
    void fill_rect(AABB<float> aabb, RGBA rgba);
    void present();
};

#endif  // SOMETHING_RENDERER_HPP_
