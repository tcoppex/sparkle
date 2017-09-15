// -----------------------------------------------------------------------------
//
//      Compute classical Perlin Noise (2D/3D).
//
//      ref : 'Improving Noise' - Ken Perlin
//            'Implementing Improved Perlin Noise' - Ken Perlin
//            'Implementing Improved Perlin Noise' - Simon Green
//            'Advanced Perlin Noise' - Inigo Quilez
//            'Simplex noise demystified' - Stefan Gustavson
//
//      Note : This code is based on Stefan Gustavson & Ian McEwan noise 
//             implementation.
//
//             This is not a MAIN shader, it must be included.
//
//      Todo : implement Simplex Noise.
//
//      version : GLSL 3.1+ core
//
//------------------------------------------------------------------------------
 
#ifndef SHADER_PERLIN_NOISE_SHARED_GLSL_
#define SHADER_PERLIN_NOISE_SHARED_GLSL_


#define NOISE_ENABLE_TILING   0
#define NOISE_TILE_RES        vec3(512.0f)

uniform int uPerlinNoisePermutationSeed;

//------------------------------------------------------------------------------

// Fast computation of x modulo 289
vec3 mod289(in vec3 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0f / 289.0f)) * 289.0f;
}

// Compute indices for the PRNG
vec4 permute(vec4 x) {
  return mod289(((x*34.0f)+1.0f)*x + uPerlinNoisePermutationSeed);
}

// Quintic interpolant
vec2 fade(in vec2 u) {
  return u*u*u*(u*(u*6.0f - 15.0f) + 10.0f);
  
  // Original cubic interpolant (faster, but not 2nd order derivable)
  //return u*u*(3.0f - 2.0f*u);
}

vec3 fade(in vec3 u) {
  return u*u*u*(u*(u*6.0f - 15.0f) + 10.0f);
}

float normalizeNoise(float n) {
  // return noise in [0, 1]
  return 0.5f*(2.44f*n + 1.0f);
}

//------------------------------------------------------------------------------

//#include "sparkle/inc_perlin_2d.glsl"
//#include "sparkle/inc_perlin_3d.glsl"

//------------------------------------------------------------------------------

#endif  // SHADER_PERLIN_NOISE_SHARED_GLSL_
