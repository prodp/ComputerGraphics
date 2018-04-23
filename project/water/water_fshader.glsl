#version 330

in vec2 uv;
in vec4 clipSpace;
in vec3 toCameraVector;
in vec3 light_vector;
in float visibility;

out vec4 color;

uniform sampler2D reflectionTexture;
uniform sampler2D refractionTexture;
uniform sampler2D dudv_map;
//uniform sampler2D heightTexture;
uniform sampler2D normal_map;
uniform sampler2D sky_texture;

uniform float move_factor;
uniform int dudv_map_size;
uniform float wave_strenght;
uniform float water_level;
uniform float reflectiveness;
uniform vec3 light_color;
uniform vec3 light_reflectivity;
uniform vec2 offset;

const float shineDamper = 20.0;

void main() {
    vec2 ndc = (clipSpace.xy/clipSpace.w) / 2.0 + 0.5;
    vec2 refractionTextureCoords = vec2(ndc.x, ndc.y);
    vec2 reflectionTextureCoords = vec2(ndc.x, 1-ndc.y);

    float distorsion_1_x = (uv.x + offset.x + move_factor) * dudv_map_size;
    float distorsion_1_y = (uv.y + offset.y) * dudv_map_size;
    vec2 distorsion1 = (texture(dudv_map, vec2(distorsion_1_x, distorsion_1_y)).rg * 2.0 - 1.0) * wave_strenght;
    float distorsion_2_x = (-uv.x - offset.x + move_factor) * dudv_map_size;
    float distorsion_2_y = (uv.y + offset.y + move_factor) * dudv_map_size;
    vec2 distorsion2 = (texture(dudv_map, vec2(distorsion_2_x, distorsion_2_y)).rg * 2.0 - 1.0) * wave_strenght;
    vec2 distorsion = distorsion1 + distorsion2;

    refractionTextureCoords += distorsion;
    refractionTextureCoords = clamp(refractionTextureCoords, 0.001, 0.999);

    reflectionTextureCoords += distorsion;
    reflectionTextureCoords = clamp(reflectionTextureCoords, 0.001, 0.999);

    vec4 reflectionColor = texture(reflectionTexture, reflectionTextureCoords);
    vec4 refractionColor = texture(refractionTexture, refractionTextureCoords);

    // Fresnel effect
    vec3 viewVector = normalize(toCameraVector);
    float refractiveFactor = dot(viewVector, vec3(0.0, 1.0, 0.0));
    refractiveFactor = pow(refractiveFactor, reflectiveness);

    // Normal map for lighting
    vec4 normal_map_color_1 = (texture(normal_map, vec2(distorsion_1_x, distorsion_1_y)));
    vec4 normal_map_color_2 = (texture(normal_map, vec2(distorsion_2_x, distorsion_2_y)));
    vec4 normal_map_color = (normal_map_color_1 + normal_map_color_2)/2;
    vec3 normal = vec3(normal_map_color.r * 2.0 - 1.0, normal_map_color.b, normal_map_color.g * 2.0 - 1.0);
    normal = normalize(normal);

    // 3. specular reflection
    vec3 light_dir = vec3(1.0, 1.0, 1.0);
    vec3 rVector = normalize( 2 * normal * dot(normal, light_dir) - light_dir );
    float light_r_angle_cos = max(0.0, dot(rVector, toCameraVector)); // unitaires -> produit des normes = 1
    vec3 specular_lights = vec3(1.0)*pow(light_r_angle_cos, 1.0f); // ks * Ls * à la place de 1.0

    color = mix(reflectionColor, refractionColor, refractiveFactor);
    color = mix(color, vec4(0.0, 0.3, 0.5, 1.0), 0.2);// + vec4(specular_lights, 0.0);
    color = mix(texture(sky_texture, ndc), color, visibility);

}
