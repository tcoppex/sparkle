// -----------------------------------------------------------------------------
//
//      Compute curl noise.
//
//      ref : 'Curl-Noise for Procedural Fluid Flow' - Robert Bridson & al
//            'Turbulent Particles demo' - Phillip Rideout
//
//      Note : This code is based on Bridson & Rideout implementation.
//
//             This is not a MAIN shader, it must be included.
//
//------------------------------------------------------------------------------

#ifndef SHADER_CURLNOISE_GLSL_
#define SHADER_CURLNOISE_GLSL_

#include "sparkle/inc_perlin_3d.glsl"
#include "sparkle/inc_distance_func.glsl"

// ----------------------------------------------------------------------------

vec3 compute_curl(in vec3 p);
vec3 sample_potential(in vec3 p);
void match_boundary(in float inv_noise_scale, in float d, in vec3 normal, inout vec3 psi);

// higher order smoothstep.
float smoothstep_2(float edge0, float edge1, float x);
// smoothed a value in [-1, 1]
float ramp(float x);
// return a vector of noise values.
vec3 noise3d(in vec3 seed);

// ----------------------------------------------------------------------------

vec3 compute_curl(in vec3 p) {
  const float eps = 1e-4f;

  const vec3 dx = vec3(eps, 0.0f, 0.0f);
  const vec3 dy = dx.yxy;
  const vec3 dz = dx.yyx;

  vec3 p00 = sample_potential(p + dx);
  vec3 p01 = sample_potential(p - dx);
  vec3 p10 = sample_potential(p + dy);
  vec3 p11 = sample_potential(p - dy);
  vec3 p20 = sample_potential(p + dz);
  vec3 p21 = sample_potential(p - dz);

  vec3 v;
  v.x = p11.z - p10.z - p21.y + p20.y;
  v.y = p21.x - p20.x - p01.z + p00.z;
  v.z = p01.y - p00.y - p11.x + p10.x;
  v /= (2.0f*eps);

  return v;
}

// [ User customized sampling function ]
vec3 sample_potential(in vec3 p) {
  const uint num_octaves = 4u;

  // Potential
  vec3 psi = vec3(0.0f);

  // Compute normal and retrieve distance from colliders.
  vec3 normal;
  float distance = compute_gradient(p, normal);

  /*
  // --------
  const float PlumeCeiling = (0);
  const float PlumeBase = (-3.0);
  const float PlumeHeight = (80);
  const float RingRadius = (10.25f);
  const float RingSpeed = (0.3f);
  const float RingsPerSecond = (0.125f);
  const float RingMagnitude = (10);
  const float RingFalloff = (0.7f);
  //---------
  /**/

  float height_factor = 1.0;//ramp((p.y - PlumeBase)/ PlumeHeight);

  // Add turbulence octaves that respects boundaries.
  float noise_gain = 1.0f;
  for(uint i=0u; i < num_octaves; ++i, noise_gain *= 0.5f) {
    const float noise_scale = 0.42f * noise_gain;
    const float inv_noise_scale = 1.0f / noise_gain;

    vec3 s = p * inv_noise_scale;
    vec3 n = noise3d(s);

    match_boundary(inv_noise_scale, distance, normal, psi);
    psi += height_factor * noise_gain * n;
  }

  // [ add custom potentials ]

  // --------

  /*
  vec3 rising_force = vec3(-p.z, 0.0f, p.x);

  float ring_y = PlumeCeiling;
  float d = ramp(abs(distance) / RingRadius);

  while (ring_y > PlumeBase) {
    float ry = p.y - ring_y;
    float rr = sqrt(dot(p.xz, p.xz));
    vec3 v = vec3(rr-RingRadius, rr+RingRadius, ry);
    float rmag = RingMagnitude / (dot(v,v) + RingFalloff);
    vec3 rpsi = rmag * rising_force;
    psi += mix(dot(rpsi, normal)*normal, psi, d);
    ring_y -= RingSpeed / RingsPerSecond;
  }
  */

  return psi;
}

void match_boundary(in float inv_noise_scale, in float d, in vec3 normal, inout vec3 psi) {
   float alpha = ramp(abs(d) * inv_noise_scale);
   float dp = dot(psi, normal);
   psi = mix(dp * normal, psi, alpha);
}

// ----------------------------------------------------------------------------

float smoothstep_2(float edge0, float edge1, float x) {
  float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
  return t * t * t * (10.0f + t *(-15.0f + 6.0f * t));
}

float ramp(float x) {
  return smoothstep_2(-1.0f, 1.0f, x) * 2.0f - 1.0f;
}

vec3 noise3d(in vec3 seed) {
  return vec3(
    pnoise(seed),
    pnoise(seed + vec3(31.416f, -47.853f, 12.793f)),
    pnoise(seed + vec3(-233.145f, -113.408f, -185.31f))
  );
}

// ----------------------------------------------------------------------------

#endif  // SHADER_CURLNOISE_GLSL_
