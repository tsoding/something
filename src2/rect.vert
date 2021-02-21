attribute vec4 aVertexPosition;

uniform vec2 rect_position;
uniform vec2 rect_size;

void main() {
    gl_Position = vec4(aVertexPosition.xy * rect_size + rect_position, 0.0, 1.0);
}
