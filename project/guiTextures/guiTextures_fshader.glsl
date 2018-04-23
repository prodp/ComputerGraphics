#version 330

in vec2 uv;

out vec3 color;

uniform sampler2D tex;
uniform int dimension;
uniform int rotate180;

void main() {
    vec2 uv_new = uv;
    if (rotate180 == 1) {
        uv_new = 1-uv;
    }
    if (dimension == 3) {
        color = texture(tex, uv_new).xyz;
    }
    else {
        color = vec3(texture(tex, uv_new).x);
    }
}
