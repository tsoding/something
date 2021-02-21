#include "./something_renderer.hpp"

static Fixed_Region<1000 * 1000> shader_buffer;

static GLuint compile_shader_file(const char *file_path, GLenum shader_type)
{
    const auto source = unwrap_or_panic(
                            read_file_as_string_view(file_path, &shader_buffer),
                            "Could not read file `", file_path, "`: ",
                            strerror(errno));
    const GLint source_size = static_cast<GLint>(source.count);

    GLuint shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source.data, &source_size);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        GLchar log[1024];
        GLsizei log_size = 0;
        glGetShaderInfoLog(shader, sizeof(log), &log_size, log);

        const String_View log_sv = {
            (size_t) log_size,
            log
        };

        panic("Failed to compile ", file_path, ":", log_sv);
    }

    return shader;
}

static GLuint link_program(GLuint vert_shader, GLuint frag_shader)
{
    GLuint program = glCreateProgram();

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        GLsizei log_size = 0;
        GLchar log[1024];
        glGetProgramInfoLog(program, sizeof(log), &log_size, log);

        const String_View log_sv = {
            (size_t) log_size,
            log
        };

        panic("Failed to link the shader program: ", log_sv);
    }

    return program;
}

void Renderer::fill_rect(AABB<float> aabb, RGBA color)
{
    glUniform2f(u_rect_position, aabb.pos.x, aabb.pos.y);
    glUniform2f(u_rect_size, aabb.size.x, aabb.size.y);
    glUniform4f(u_color, color.r, color.g, color.b, color.a);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Renderer::init()
{
    float quad[] = {
        0.0, 0.0,
        1.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
    };

    glGenBuffers(1, &quad_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(quad),
                 quad,
                 GL_STATIC_DRAW);

    const auto rect_vert = compile_shader_file("src2/rect.vert", GL_VERTEX_SHADER);
    const auto rect_frag = compile_shader_file("src2/rect.frag", GL_FRAGMENT_SHADER);
    rect_program = link_program(rect_vert, rect_frag);
    glUseProgram(rect_program);

    u_color = glGetUniformLocation(rect_program, "color");
    u_rect_position = glGetUniformLocation(rect_program, "rect_position");
    u_rect_size = glGetUniformLocation(rect_program, "rect_size");

    GLint vertexPosition = glGetAttribLocation(rect_program, "aVertexPosition");

    glVertexAttribPointer(
        vertexPosition,     // index
        2,                  // numComponents
        GL_FLOAT,           // type
        0,                  // normalized
        0,                  // stride
        0                   // offset
    );

    glEnableVertexAttribArray(vertexPosition);
}

