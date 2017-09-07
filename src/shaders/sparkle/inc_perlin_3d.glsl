#ifndef SHADER_PERLIN_NOISE_3D_GLSL_
#define SHADER_PERLIN_NOISE_3D_GLSL_

#include "sparkle/inc_perlin.glsl"

// ----------------------------------------------------------------------------

// Classical Perlin Noise 3D
float pnoise(in vec3 pt, in vec3 scaledTileRes);
float pnoise(in vec3 pt);

// Classical Perlin Noise 2D + time
float pnoise_loop(in vec2 u, float dt);

// Classical Perlin Noise fbm 3D
float fbm_pnoise(in vec3 pt, 
                 const float zoom,
                 const int numOctave, 
                 const float frequency, 
                 const float amplitude);

// ----------------------------------------------------------------------------


// Classical Perlin Noise 3D
float pnoise(in vec3 pt, in vec3 scaledTileRes) {
  // Retrieve the integral part (for indexation)
  vec3 ipt0 = floor(pt);
  vec3 ipt1 = ipt0 + vec3(1.0f);

  // Tile the noise
#if NOISE_ENABLE_TILING
  ipt0 = mod(ipt0, scaledTileRes);
  ipt1 = mod(ipt1, scaledTileRes);
#endif

  ipt0 = mod289(ipt0);
  ipt1 = mod289(ipt1);

  // Compute the 8 corners hashed gradient indices
  vec4 ix = vec4(ipt0.x, ipt1.x, ipt0.x, ipt1.x);
  vec4 iy = vec4(ipt0.yy, ipt1.yy);
  vec4 p = permute(permute(ix) + iy);
  vec4 p0 = permute(p + ipt0.zzzz);
  vec4 p1 = permute(p + ipt1.zzzz);

  // Compute Pseudo Random Numbers
  vec4 gx0 = p0 * (1.0f / 7.0f);
  vec4 gy0 = fract(floor(gx0) * (1.0f / 7.0f)) - 0.5f;
  gx0 = fract(gx0);  
  vec4 gz0 = vec4(0.5f) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0f));
  gx0 -= sz0 * (step(0.0f, gx0) - 0.5f);
  gy0 -= sz0 * (step(0.0f, gy0) - 0.5f);

  vec4 gx1 = p1 * (1.0f / 7.0f);
  vec4 gy1 = fract(floor(gx1) * (1.0f / 7.0f)) - 0.5f;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5f) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0f));
  gx1 -= sz1 * (step(0.0f, gx1) - 0.5f);
  gy1 -= sz1 * (step(0.0f, gy1) - 0.5f);


  // Create unnormalized gradients
  vec3 g000 = vec3(gx0.x, gy0.x, gz0.x);
  vec3 g100 = vec3(gx0.y, gy0.y, gz0.y);
  vec3 g010 = vec3(gx0.z, gy0.z, gz0.z);
  vec3 g110 = vec3(gx0.w, gy0.w, gz0.w);
  vec3 g001 = vec3(gx1.x, gy1.x, gz1.x);
  vec3 g101 = vec3(gx1.y, gy1.y, gz1.y);
  vec3 g011 = vec3(gx1.z, gy1.z, gz1.z);
  vec3 g111 = vec3(gx1.w, gy1.w, gz1.w);

  // 'Fast' normalization
  vec4 dp = vec4(dot(g000, g000), dot(g100, g100), dot(g010, g010), dot(g110, g110));
  vec4 norm = inversesqrt(dp);
  g000 *= norm.x;
  g100 *= norm.y;
  g010 *= norm.z;
  g110 *= norm.w;

  dp = vec4(dot(g001, g001), dot(g101, g101), dot(g011, g011), dot(g111, g111));
  norm = inversesqrt(dp);
  g001 *= norm.x;
  g101 *= norm.y;
  g011 *= norm.z;
  g111 *= norm.w;

  // Retrieve the fractional part (for interpolation)
  vec3 fpt0 = fract(pt);
  vec3 fpt1 = fpt0 - vec3(1.0f);

  // Calculate gradient's influence
  float n000 = dot(g000, fpt0);
  float n100 = dot(g100, vec3(fpt1.x, fpt0.yz));
  float n010 = dot(g010, vec3(fpt0.x, fpt1.y, fpt0.z));
  float n110 = dot(g110, vec3(fpt1.xy, fpt0.z));
  float n001 = dot(g001, vec3(fpt0.xy, fpt1.z));
  float n101 = dot(g101, vec3(fpt1.x, fpt0.y, fpt1.z));
  float n011 = dot(g011, vec3(fpt0.x, fpt1.yz));
  float n111 = dot(g111, fpt1);

  // Interpolate gradients
  vec3 u = fade(fpt0);
  float nxy0 = mix(mix(n000, n100, u.x), mix(n010, n110, u.x), u.y);
  float nxy1 = mix(mix(n001, n101, u.x), mix(n011, n111, u.x), u.y);
  float noise = mix(nxy0, nxy1, u.z);

  return noise;
}

float pnoise(in vec3 pt) {
  return pnoise(pt, vec3(0.0f));
}

// Classical Perlin Noise 2D + time
float pnoise_loop(in vec2 u, float dt) {
  vec3 pt1 = vec3(u, dt);
  vec3 pt2 = vec3(u, dt-1.0f);

  return mix(pnoise(pt1), pnoise(pt2), dt);
}

// Classical Perlin Noise fbm 3D
float fbm_pnoise(in vec3 pt, 
                 const float zoom,
                 const int numOctave, 
                 const float frequency, 
                 const float amplitude)
{
  float sum = 0.0f;
  float f = frequency;
  float w = amplitude;

  vec3 v = zoom * pt;
  vec3 scaledTileRes = zoom * NOISE_TILE_RES;

  for (int i = 0; i < numOctave; ++i) {
    sum += w * pnoise(f*v, f*scaledTileRes);
    
    f *= frequency;
    w *= amplitude;
  }
  
  return sum;
}

/*
float fbm_3d(in vec3 ws) {
  const float N = 128.0f;
  const float zoom = 1/N;
  const int octave = 4;
  const float freq = 2.0f;
  const float w    = 0.45f;

  return N * fbm_pnoise(ws, zoom, octave, freq, w);
}
*/

// ----------------------------------------------------------------------------

#endif  // SHADER_PERLIN_NOISE_3D_GLSL_
