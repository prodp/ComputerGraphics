#version 330 core
uniform mat4 MVP;
uniform float time;
uniform float fog_density;
uniform float fog_gradient;
uniform mat4 model;
uniform mat4 view;
uniform vec2 offset;
uniform float radiusPath;
uniform float phaseWing;

in vec3 vpoint;
in vec2 vtexcoord;

out vec2 uv;
out float visibility;
out vec4 clipSpace;

mat4 R(float alpha){
    mat4 R = mat4(1.0);
    R[0][0] =  cos(alpha);
    R[0][2] =  sin(alpha);
    R[2][0] = -sin(alpha);
    R[2][2] =  cos(alpha);
    return R;
}

mat4 T(float tx, float ty, float tz){
    mat4 T = mat4(1.0);
    T[3][0] = tx;
    T[3][1] = ty;
    T[3][2] = tz;
    return T;
}

mat4 S(float s){
    mat4 S = mat4(s);
    S[3][3] = 1.0;
    return S;
}

void main(){
    float scale = 0.0025;
    mat4 rotation = R(time/3.0);
    mat4 translation = T(0.1-offset.x + radiusPath*cos(time/3.0), 0.7, offset.y+0.2 + radiusPath*sin(time/3.0));//(time-15.0)/15.0 + offset.y);
    float frequency = 3.14/2; // 'how many sinus' do we see in wings
    float wing_speed = 3.14*4;
    mat4 wing_translation = mat4(1.0);
    if (vpoint.x < 0.1 || vpoint.x > 17.9) {
        wing_translation = T(0.0, scale*sin(time*wing_speed + phaseWing), 0.0);
    } else if (vpoint.x < 2.1 || vpoint.x > 15.9) {
        wing_translation = T(0.0, scale*sin(time*wing_speed + frequency / 4.0 + phaseWing), 0.0);
    } else if (vpoint.x < 4.1 || vpoint.x > 13.9) {
        wing_translation = T(0.0, scale*sin(time*wing_speed + 2.0*frequency / 4.0 + phaseWing), 0.0);
    } else if (vpoint.x < 6.1 || vpoint.x > 11.9) {
        wing_translation = T(0.0, scale*sin(time*wing_speed + 3.0*frequency / 4.0 + phaseWing), 0.0);
    }

    mat4 scale_mat = S(scale);
    mat4 transformations = translation * wing_translation * scale_mat * rotation;
    gl_Position =  MVP * transformations * vec4(vpoint,1);
    uv = vtexcoord;

    // Fog
    mat4 MV = transformations * view * model;
    vec4 vpoint_mv = MV * vec4(vpoint, 1.0);
    vec4 positionRelativeToCamera = vpoint_mv;
    float distance = length(positionRelativeToCamera.xyz);
    visibility = exp(-pow((distance * fog_density), fog_gradient));
    visibility = clamp(visibility, 0.0, 1.0);


    clipSpace = gl_Position; // to sample the sky texture in ndc coords
}
