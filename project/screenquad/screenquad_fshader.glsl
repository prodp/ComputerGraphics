#version 330

// inputs/outputs
in vec2 uv;
out float color;

// uniforms
uniform int SEED;
uniform int OCTAVES_NB;                 // how many octaves do we want
uniform int GRID_SIZE_EXP;            // grid will have size =  2 ^ GRID_SIZE_EXP
uniform int CELL_SIZE_EXP = 9;
uniform float LACUNARITY = 2;
uniform float H = 1;
uniform float RIDGED_MFRACTAL_OFFSET = 2.0;
uniform vec2 offset;                    // used to simulate camera moving around
uniform int flatTerrainActivated;
uniform float noise_density;
uniform float noise_gradient;
uniform float noise_size;
uniform float water_level;

uniform sampler2D noise_tex;

//globals
float SIZE_GRID = pow(2, GRID_SIZE_EXP);                              // will be divided by powers of 2
float CELL_SIZE = pow(2, CELL_SIZE_EXP);

const float PI = 3.14159265;


// functions forward declarations
float fractalBrownianMotion(vec2 point);
float turbulence(vec2 point);
float ridgedMultifractal(vec2 point);
//helper functions
float perlin(vec2 point);
vec2 getRandomGradient(float xCorner, float yCorner);
float rand(vec2 co, float seed);
float interpolate_smooth(float t);
float mix(float x, float y, float alpha);
float computeBrownianTerm(float weight, float cells_size);




void main(){
    color = fractalBrownianMotion(uv.xy+offset);
//    color = turbulence(uv.xy+offset);
//    color = ridgedMultifractal(uv.xy+offset);

    // If activated, we make the terrain more flat at some location
    if (flatTerrainActivated != 0) {
        float attenuationForFlatTerrain =
                exp(-pow((texture(noise_tex, (uv + offset) / noise_size).x * noise_density), noise_gradient));

        color = mix(color, water_level+0.01, attenuationForFlatTerrain);
    }
}


/*
 *      COMPOUND NOISE FUNCTIONS
 */

// Ridge multifractal noise
float ridgedMultifractal(vec2 point){
    float noise = 0.0;
    for( int i = 0; i < OCTAVES_NB; i+=1) {
        noise *= (perlin(point)+RIDGED_MFRACTAL_OFFSET) * pow(LACUNARITY, -H*i);
        point *= LACUNARITY;
    }
    return noise;
}

// Turbulence noise
float turbulence(vec2 point){
    float noise = 0.0;
    for( int i = 0; i < OCTAVES_NB; i+=1) {
        noise += abs(perlin(point)) * pow(LACUNARITY, -H*i);
        point *= LACUNARITY;
    }
    return noise;
}

// Fractal Brownian motion noise
float fractalBrownianMotion(vec2 point){
    float noise = 0.0;
    for( int i = 0; i < OCTAVES_NB; i+=1) {
        noise += perlin(point) * pow(LACUNARITY, -H*i);
        point *= LACUNARITY;
    }
    return (noise+1)/2.0;   // +1 /2 to obtain a result in [0,1]
}


/*
 *      VANILLA PERLIN NOISE
 */

// queries/computes the perlin noise at a given point
float perlin(vec2 point){
    // Coordinates of the unit cell that contains the point
    float px = (SIZE_GRID/CELL_SIZE) * (point.x); //
    float py = (SIZE_GRID/CELL_SIZE) * (point.y);

    // Coordinates of the corners of the cell that contains the point
    float x0 = floor(px);
    float x1 = x0+1.0;
    float y0 = floor(py);
    float y1 = y0+1.0;

    // Coordinates of the point inside the cell
    float x = fract(px);
    float y = fract(py);

    // Random gradients at cell corners
    vec2 g00 = getRandomGradient(x0, y0);
    vec2 g01 = getRandomGradient(x0, y1);
    vec2 g10 = getRandomGradient(x1, y0);
    vec2 g11 = getRandomGradient(x1, y1);

    // Compute the difference vectors from cells corners to p
    vec2 a = vec2(x, y);
    vec2 b = vec2(x-1.0, y);
    vec2 c = vec2(x, y-1.0);
    vec2 d = vec2(x-1.0, y-1.0);

    // We compute scalar values for the corners
    float s = dot(g00, a);
    float t = dot(g10, b);
    float u = dot(g01, c);
    float v = dot(g11, d);

    // We compute the final noise
    float fx = interpolate_smooth(x);
    float st_mix = mix(s, t, fx);
    float uv_mix = mix(u, v, fx);
    float noise = mix(st_mix, uv_mix, interpolate_smooth(y));

    return noise;
}


/*
 *      HELPER FUNCTIONS
 */

// returns a pseudorandom float in ]0,1[
float rand(vec2 co, float seed){
    // http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
    return fract(sin(dot(co.xy * seed, vec2(12.9898,78.233))) * 43758.5453);
}

// computes the pseudorandom gradient at a given cell corner
vec2 getRandomGradient(float xCorner, float yCorner){
    float norm = 2 * rand(vec2(xCorner, yCorner), SEED*2);
    float angle = rand(vec2(xCorner, yCorner), SEED*2+1) * (2 * PI);
    vec2 gradient =  norm * vec2(cos(angle), sin(angle));
    return gradient;
}

// smooth interpolation function
float interpolate_smooth(float t) {
    return t*t*t * ( t * (t*6 - 15) + 10);
}

// linearily interpolates parameters
float mix(float x, float y, float alpha) {
    return (1.0 - alpha) * x + alpha * y;
}


