#ifndef SHADER_PERLIN_NOISE_2D_GLSL_
#define SHADER_PERLIN_NOISE_2D_GLSL_

#include "sparkle/inc_perlin.glsl"

// ----------------------------------------------------------------------------

// Classical Perlin Noise 2D
float pnoise(in vec2 pt, in vec2 scaledTileRes);
float pnoise(in vec2 pt);

// Derivative Perlin Noise 2D
vec3 dpnoise(in vec2 pt, in vec2 scaledTileRes);
vec3 dpnoise(in vec2 pt);

// Classical Perlin Noise fbm 2D
float fbm_pnoise(in vec2 pt,
                 const float zoom,
                 const int numOctave, 
                 const float frequency, 
                 const float amplitude);

// Derivative Perlin Noise fbm 2D
float fbm_dpnoise(in vec2 pt,
                  const float zoom,
                  const int numOctave, 
                  const float frequency, 
                  const float amplitude);

// ----------------------------------------------------------------------------

void pnoise_gradients(in  vec2 pt,
                      in  vec2 scaledTileRes,
                      out vec4 gradients,
                      out vec4 fpt) 
{
  // Retrieve the integral part (for indexation)
  vec4 ipt = floor(pt.xyxy) + vec4(0.0f, 0.0f, 1.0f, 1.0f);

  // Tile the noise (if enabled)
#if NOISE_ENABLE_TILING
    ipt = mod(ipt, scaledTileRes.xyxy);
#endif
  ipt = mod289(ipt);

  // Compute the 4 corners hashed gradient indices
  vec4 ix = ipt.xzxz;
  vec4 iy = ipt.yyww;
  vec4 p = permute(permute(ix) + iy);
  /*
  Fast version for :
  p.x = P(P(ipt.x)      + ipt.y);
  p.y = P(P(ipt.x+1.0f) + ipt.y);
  p.z = P(P(ipt.x)      + ipt.y+1.0f);
  p.w = P(P(ipt.x+1.0f) + ipt.y+1.0f);
  */

  // With 'p', computes Pseudo Random Numbers
  const float one_over_41 = 1.0f / 41.0f; //0.02439f
  vec4 gx = 2.0f * fract(p * one_over_41) - 1.0f;
  vec4 gy = abs(gx) - 0.5f;
  vec4 tx = floor(gx + 0.5f);
  gx = gx - tx;

  // Create unnormalized gradients
  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);

  // 'Fast' normalization
  vec4 dp = vec4(dot(g00, g00), dot(g10, g10), dot(g01, g01), dot(g11, g11));
  vec4 norm = inversesqrt(dp);
  g00 *= norm.x;
  g10 *= norm.y;
  g01 *= norm.z;
  g11 *= norm.w;

  // Retrieve the fractional part (for interpolation)
  fpt = fract(pt.xyxy) - vec4(0.0f, 0.0f, 1.0f, 1.0f);

  // Calculate gradient's influence
  vec4 fx = fpt.xzxz;
  vec4 fy = fpt.yyww;
  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));
  /*
  Fast version for :
  n00 = dot(g00, fpt + vec2(0.0f, 0.0f));
  n10 = dot(g10, fpt + vec2(-1.0f, 0.0f));
  n01 = dot(g01, fpt + vec2(0.0f,-1.0f));
  n11 = dot(g11, fpt + vec2(-1.0f,-1.0f));
  */

  gradients = vec4(n00, n10, n01, n11);
} 

void pnoise_gradients(in  vec2 pt,
                      out vec4 gradients,
                      out vec4 fpt) {
  pnoise_gradients(pt, vec2(0.0f), gradients, fpt);
} 

// Classical Perlin Noise 2D
float pnoise(in vec2 pt, in vec2 scaledTileRes = vec2(0.0f)) {
  vec4 g, fpt;
  pnoise_gradients(pt, scaledTileRes, g, fpt);

  // Interpolate gradients
  vec2 u = fade(fpt.xy);
  float n1 = mix(g.x, g.y, u.x);
  float n2 = mix(g.z, g.w, u.x);
  float noise = mix(n1, n2, u.y);

  return noise;
}

float pnoise(in vec2 pt) {
  return pnoise(pt, vec2(0.0f));
}


// Derivative Perlin Noise 2D
vec3 dpnoise(in vec2 pt, in vec2 scaledTileRes = vec2(0.0f)) {
  vec4 g, fpt;
  pnoise_gradients(pt, scaledTileRes, g, fpt);

  float k0 = g.x;
  float k1 = g.y - g.x;
  float k2 = g.z - g.x;
  float k3 = g.x - g.z - g.y + g.w;
  vec3 res;

  vec2 u = fade(fpt.xy);
  res.x = k0 + k1*u.x + k2*u.y + k3*u.x*u.y;

  vec2 dpt = 30.0f*fpt.xy*fpt.xy*(fpt.xy*(fpt.xy-2.0f)+1.0f);
  res.y = dpt.x * (k1 + k3*u.y);
  res.z = dpt.y * (k2 + k3*u.x);

  return res;
}

vec3 dpnoise(in vec2 pt) {
  return dpnoise(pt, vec2(0.0f));
}


// -- Fractional Brownian Motion function --

/// @note  
/// Tiling does not works with non integer frequency or non power of two zoom.


// Classical Perlin Noise fbm 2D
float fbm_pnoise(in vec2 pt,
                 const float zoom,
                 const int numOctave, 
                 const float frequency, 
                 const float amplitude)
{
  float sum = 0.0f;
  float f = frequency;
  float w = amplitude;

  vec2 v = zoom * pt;
  vec2 scaledTileRes = zoom * NOISE_TILE_RES.xy;

  for (int i = 0; i < numOctave; ++i) {
    sum += w * pnoise(f*v, f*scaledTileRes);
    f *= frequency;
    w *= amplitude;
  }

  return sum;
}

// Derivative Perlin Noise fbm 2D
float fbm_dpnoise(in vec2 pt,
                  const float zoom,
                  const int numOctave, 
                  const float frequency, 
                  const float amplitude)
{
  float sum = 0.0f;

  float f = frequency;
  float w = amplitude;
  vec2 dn = vec2(0.0f);

  vec2 v = zoom * pt;
  vec2 scaledTileRes = zoom * NOISE_TILE_RES.xy;

  for (int i=0; i<numOctave; ++i) {
    vec3 n = dpnoise(f*v, f*scaledTileRes);
    dn += n.yz;
    
    float crestFactor = 1.0f / (1.0f + dot(dn,dn));
    
    sum += w * n.x * crestFactor;
    f *= frequency;
    w *= amplitude;
  }

  return sum;
}

#endif  // SHADER_PERLIN_NOISE_2D_GLSL_
