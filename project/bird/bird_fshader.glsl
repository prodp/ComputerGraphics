#version 330 core

uniform sampler2D sky_tex;

in vec2 uv;
in float visibility;
in vec4 clipSpace;
out vec3 color;

void main(){
    vec2 ndc = (clipSpace.xy/clipSpace.w) / 2.0 + 0.5;
    color = mix(texture(sky_tex, ndc).xyz, vec3(0.0, 0.0, 0.0), visibility);
}
