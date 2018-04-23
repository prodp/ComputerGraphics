#version 330

in vec2 textureCoord;
in vec3 vertexCoord;

out vec2 uv;

void main() {
    gl_Position = vec4(vertexCoord, 1.0);
    uv = textureCoord;
}
