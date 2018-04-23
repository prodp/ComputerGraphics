#version 330

in vec2 position;

out vec2 uv;
out vec4 clipSpace;
out vec3 toCameraVector;
out vec3 light_vector;
out float visibility;

uniform mat4 MVP;
uniform float time;
uniform vec2 offset;
uniform float water_level;
uniform vec3 camPos;
uniform mat4 model;
uniform mat4 view;
uniform vec3 light_pos;
uniform float amplitude;

// Fog parameters
uniform float fog_density;
uniform float fog_gradient;


void main() {
    uv = (position + vec2(1.0, 1.0)) * 0.5;


    float height = water_level;//0.002*sin(mod(uv.x+offset.x + uv.y+offset.y, 1) * 40 * PI + time) + 0.4;
    vec3 pos_3d = vec3(position.x, height, -position.y);

    vec4 worldPosition = model * vec4(pos_3d, 1.0);

    clipSpace = MVP * vec4(pos_3d, 1.0);
    gl_Position = clipSpace;

    toCameraVector = camPos - worldPosition.xyz;
    light_vector = worldPosition.xyz - light_pos;

    // Fog
    mat4 MV = view * model;
    vec4 positionRelativeToCamera = MV * vec4(pos_3d, 1.0);
    float distance = length(positionRelativeToCamera.xyz);
    visibility = exp(-pow((distance * fog_density), fog_gradient));
    visibility = clamp(visibility, 0.0, 1.0);
}
