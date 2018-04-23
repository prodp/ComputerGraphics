#version 330

// IO
in vec2 uv;
in float height;
in vec3 view_dir;
in float visibility;
in vec4 clipSpace;
in vec4 clipSpaceNormal;

out vec3 color;


// uniforms
uniform float water_level;          // uniform parameters
uniform float gridSizeExp;
uniform vec2 offset;
uniform int flatTerrainActivated;
uniform float steepness_threshold_mixedrock;
uniform float steepness_threshold_purerock;
uniform float drawFogEnabled;

uniform sampler2D noise_tex;        // procedural textures
uniform sampler2D normal_tex;
uniform sampler2D sky_tex;
uniform sampler2D bedrock_tex;      // terrain textures
uniform sampler2D seaside_tex;
uniform sampler2D plain_tex;
uniform sampler2D mountainside_tex;
uniform sampler2D rock_tex;
uniform sampler2D snowyrock_tex;
uniform sampler2D snow_tex;

/*
 * Local parameters
 */

vec2 ofst_pos;
vec3 normal;

float tex_resolution = 10.0;

// textures thresholds ordered from bottom to top:
float threshold_bedrock = water_level - 0.06; // rouge
float threshold_mix_bedrock_seaside = threshold_bedrock + 0.05;
float threshold_seaside = threshold_bedrock + 0.065;
float threshold_mix_seaside_plain = threshold_seaside + 0.003;
float threshold_plain = threshold_seaside + 0.043;
float threshold_mix_plain_mountainside = threshold_plain + 0.03;
float threshold_mountainside = threshold_plain + 0.06;
float threshold_mix_mountainside_snow = threshold_mountainside + 0.05;
// threshold for snow is anything above mountainside


/*
 *  Functions Forward declarations
 */
float slope_steepness();
float slope_steepness_proportion();
float proportion(float inputValue, float inputRangeMin, float inputRangeMax);
float rand(vec2 co, float seed);
vec3 texture_snow();
vec3 texture_snow_mix();
vec3 texture_rock();
vec3 texture_mountainside();
vec3 texture_plain();
vec3 texture_seaside();
vec3 texture_bedrock();



void main() {
    // position parameters
    ofst_pos = uv + offset;
    normal = texture(normal_tex, uv).xyz;

    vec2 ndc = (clipSpace.xy/clipSpace.w) / 2.0 + 0.5;
    vec2 ndcNormal = (clipSpaceNormal.xy / clipSpaceNormal.w) / 2.0 + 0.5;
    float adaptationFactor = pow(2, gridSizeExp-9);

    float amplitude = 1.5;
    vec3 light_dir = vec3(1.0, 1.0, 1.0);
    vec3 Ld = vec3(1.0, 1.0, 1.0);
    vec3 Kd = vec3(1.0, 1.0, 1.0);

    // 2. diffuse reflection
    vec3 diffuse = Kd * Ld * max(0.0, dot(light_dir, normal));


    // apply terrain textures according to theight and terrain slope
    vec3 color_terrain;
    if (height < threshold_bedrock){
        color_terrain = texture_bedrock();
    }
    else if ( height < threshold_mix_bedrock_seaside){
        color_terrain = mix(texture_bedrock(), texture_seaside(), proportion(height, threshold_bedrock, threshold_mix_bedrock_seaside));
    }
    else if (height < threshold_seaside) {
        color_terrain = texture_seaside();
    }
    else if (height < threshold_mix_seaside_plain) {
        vec3 color_seaside = texture_seaside();
        vec3 color_plain = texture_plain();
        color_terrain = mix(color_seaside, color_plain, proportion(height, threshold_seaside, threshold_mix_seaside_plain));
    }
    else if (height < threshold_plain) {
        color_terrain = texture_plain();
    }
    else if (height < threshold_mix_plain_mountainside){
        color_terrain = mix(texture_plain(), texture_mountainside(), proportion(height, threshold_plain, threshold_mix_plain_mountainside));
    }
    else if(height < threshold_mountainside){
        color_terrain = texture_mountainside();
    }
    else if (height < threshold_mix_mountainside_snow){
        color_terrain = texture_snow_mix();
    }
    else {
        color_terrain = texture_snow();
    }

    color = 0.8 * diffuse * vec3((height-0.1)*amplitude) * color_terrain +
            0.2 * vec3((height-0.1)*amplitude) * color_terrain;

    if (drawFogEnabled < 0.0) {
        color = mix(texture(sky_tex, ndcNormal).xyz, color, visibility);
    }
    else {
        color = mix(texture(sky_tex, ndc).xyz, color, visibility);
    }
}




/*
 *      TEXTURE FUNCTIONS
 */

// Textures snow or snowy rock depending on slope steepness
vec3 texture_snow(){
    float slope_steepness = slope_steepness();
    if (slope_steepness < steepness_threshold_mixedrock) {
        return texture(snow_tex, ofst_pos).xyz;
    }
    else if (slope_steepness < steepness_threshold_purerock ){
        return mix(texture(snow_tex, ofst_pos).xyz, texture_rock(), slope_steepness_proportion());
    }
    else { // purerock
        return texture_rock();
    }
}


// Interpolates plain/snow textures, by randomly generating "snow packs" here and there instead of linear interpolation
vec3 texture_snow_mix(){
    // parameters
    float score_threshold_no_snow = 0.48;
    float score_threshold_mixed_snow = 0.52;
    float noise_query_resolution = 8.0;

    float height_proportion = proportion(height, threshold_mountainside, threshold_mix_mountainside_snow);
    float noise = texture(noise_tex, ofst_pos*noise_query_resolution).x;
    float snowyness_score = 0.55*noise + 0.45*height_proportion;

    if (snowyness_score < score_threshold_no_snow) {
        return texture_mountainside();
    }
    else if (snowyness_score < score_threshold_mixed_snow){
        float snow_proportion = (snowyness_score - score_threshold_no_snow)*10.0; // arbitrary *10.0 ?
        vec3 color_mountainside = texture_mountainside();
        vec3 color_snow = texture_snow();
        return mix(color_mountainside, color_snow, snow_proportion);
    }
    else { // snowyness_score > threshold_to_be_snow+0.05
        return texture_snow();
    }
}


vec3 texture_rock(){
    float res_query_rock = 15.0;
    return texture(rock_tex, ofst_pos*res_query_rock).xyz;
}

vec3 texture_mountainside(){
    float res_query_mountainside = 20.0;
    vec3 terrain_mountainside = texture(mountainside_tex, ofst_pos*res_query_mountainside).xyz;
    if ( slope_steepness() < steepness_threshold_mixedrock){
        return terrain_mountainside;
    }
    else if ( slope_steepness() < steepness_threshold_purerock ){
        vec3 temp = mix(terrain_mountainside, texture_rock(), slope_steepness_proportion());
        return 0.8 * temp + 0.2 * terrain_mountainside;
    }
    else {
        return texture_rock()*0.9 + terrain_mountainside*0.1;
    }
}

vec3 texture_plain(){
    float resolution_tex_plain = 25.0;
    vec3 terrain_plain =  texture(plain_tex, ofst_pos*resolution_tex_plain).xyz;
    if ( slope_steepness() < steepness_threshold_mixedrock){
        return terrain_plain;
    }
    else if ( slope_steepness() < steepness_threshold_purerock ){
        vec3 temp = mix(terrain_plain, texture_rock(), slope_steepness_proportion());
        return 0.8 * temp + 0.2 * terrain_plain;
    }
    else {
        return texture_rock()*0.9 + terrain_plain*0.1;
    }
}

vec3 texture_seaside(){
    vec3 terrain_seaside = texture(seaside_tex, ofst_pos*tex_resolution).xyz;
    if ( slope_steepness() < steepness_threshold_mixedrock){
        return terrain_seaside;
    }
    else if ( slope_steepness() < steepness_threshold_purerock ){
        vec3 temp = mix(terrain_seaside, texture_rock(), slope_steepness_proportion());
        return 0.8 * temp + 0.2 * terrain_seaside;
    }
    else {
        return texture_rock()*0.9 + terrain_seaside*0.1;
    }
}

vec3 texture_bedrock(){
    return texture(bedrock_tex, ofst_pos).xyz;
}




/*
 *      HELPERS
 */

// returns the slope steepness
float slope_steepness(){
    return 1-dot(normal, vec3(normal.x, 0, normal.z));
}

// returns the slope_steepness proportion
float slope_steepness_proportion(){
    return proportion(slope_steepness(), steepness_threshold_mixedrock, steepness_threshold_purerock);
}

// returns the linear proportion of input range
float proportion(float inputValue, float inputRangeMin, float inputRangeMax){
    return ((inputValue-inputRangeMin)/(inputRangeMax-inputRangeMin));
}

// returns a pseudorandom float in ]0,1[
float rand(vec2 co, float seed){
    // http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
    return fract(sin(dot(co.xy * seed, vec2(12.9898,78.233))) * 43758.5453);
}
