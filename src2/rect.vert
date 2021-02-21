attribute vec4 aVertexPosition;

#define RATIO (600.0 / 800.0)

uniform vec2 rect_position;
uniform vec2 rect_size;

void main() {
    gl_Position = vec4(
        (aVertexPosition.xy * rect_size + rect_position) * vec2(RATIO, 1.0),
        0.0, 1.0);
}
