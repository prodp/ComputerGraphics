#version 330 core
uniform mat4 MVP;
uniform float time;
in vec3 vpoint;
in vec2 vtexcoord;
out vec2 uv;

mat4 R(float degrees){
    mat4 R = mat4(1);
    float alpha = radians(degrees);
    R[0][0] =  cos(alpha);
    R[0][2] =  sin(alpha);
    R[2][0] = -sin(alpha);
    R[2][2] =  cos(alpha);
    return R;
}

mat4 T(float tx, float ty, float tz){
    mat4 T = mat4(1);
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
    gl_Position =  MVP * R(time/4.0)*T(0.0, 1.5, -0.5)* S(20) * vec4(vpoint,1);
    uv = vtexcoord;
}
