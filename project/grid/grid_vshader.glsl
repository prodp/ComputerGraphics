#version 330

in vec2 position;

out vec2 uv;
out float height;
out vec3 view_dir;
out float visibility;
out vec4 clipSpace;
out vec4 clipSpaceNormal;

uniform mat4 MVPReversed;
uniform mat4 MVPNormal;
uniform float water_level;
uniform float gridSizeExp;
uniform float amplitude;
uniform vec4 plane;
uniform vec2 offset;
uniform float fog_density;
uniform float fog_gradient;

uniform mat4 model;
uniform mat4 viewNormal;

uniform sampler2D tex;      // heightmap
uniform sampler2D noise_tex; // For flatness of terrain



// forward declarations
float query_smaller_fbm();


void main() {
    uv = (position + vec2(1.0, 1.0)) * 0.5;

    mat4 MV = viewNormal * model;


    // When the size of the grid grow, we have to adjust the height for the mountains
    // to have the same look, for this we use the adaptationFactor
    float adaptationFactor = pow(2, gridSizeExp-9);
    height = ((texture(tex, uv).x - 0.5) / adaptationFactor) + 0.5;

    vec3 pos_3d = vec3(position.x, height*amplitude, -position.y);
    vec4 vpoint_mv = MV * vec4(pos_3d, 1.0);

    // If the distance is negative, then the vertex is outside the clipping
    // plane and thus will not be rendered. The distance between the point
    // and the plane is compute simply by taking the dot product.
    //gl_ClipDistance[0] = height - water_level;
    gl_ClipDistance[0] = dot(vec4(pos_3d, 1.0), plane);

    view_dir = normalize( -vpoint_mv.xyz );

    gl_Position = MVPReversed * vec4(pos_3d, 1.0);

    // Fog
    vec4 positionRelativeToCamera = vpoint_mv;
    float distance = length(positionRelativeToCamera.xyz);
    visibility = exp(-pow((distance * fog_density), fog_gradient));
    visibility = clamp(visibility, 0.0, 1.0);

    clipSpace = gl_Position; // to sample the sky texture in ndc coords
    clipSpaceNormal = MVPNormal * vec4(pos_3d, 1.0);
}


/*
 *      HELPERS
 */

float query_smaller_fbm(){
    float resolution_param = 0.1;
    vec2 newCoords = vec2(0.5) - (vec2(0.5 - uv.x, 0.5-uv.y) * resolution_param);
    return texture(tex, newCoords).x;
}
