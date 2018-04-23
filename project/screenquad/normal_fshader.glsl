#version 330

in vec2 uv;

out vec3 color;

uniform sampler2D tex; // The noise texture (height map)
uniform float tex_width;
uniform float tex_height;


void main() {

    float height_x1 = texture(tex, uv+vec2(-1/tex_width,0)).r;
    float height_x2 = texture(tex, uv+vec2(1/tex_width,0)).r;
    vec3 dx = vec3(2/tex_width, 0, height_x2 - height_x1);

    float height_y1 = texture(tex, uv+vec2(0, -1/tex_height)).r;
    float height_y2= texture(tex, uv+vec2(0, 1/tex_height)).r;
    vec3 dy = vec3(0, 2/tex_height, height_y2 - height_y1);

    vec3 n = normalize(cross(dx, dy));
    // If the normal points down, we take the opposite direction
    if (n.z < 0) {
        n*=-1;
    }

    color = n;
}
