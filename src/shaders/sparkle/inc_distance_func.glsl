#ifndef SHADER_DISTANCE_FUNC_GLSL_
#define SHADER_DISTANCE_FUNC_GLSL_

#include "sparkle/inc_distance_utils.glsl"

//-----------------------------------------------------------------------------

// Return the distance to the closest object and its normal.
float compute_gradient(in vec3 p, out vec3 normal);
// Return the distance to the closest object.
float sample_distance(in vec3 p);

//-----------------------------------------------------------------------------

float compute_gradient(in vec3 p, out vec3 normal) {
  const float d = sample_distance(p);

  const vec2 eps = vec2(1e-2f, 0.0f);
  normal.x = sample_distance(p + eps.xyy) - d;
  normal.y = sample_distance(p + eps.yxy) - d;
  normal.z = sample_distance(p + eps.yyx) - d;
  normal = normalize(normal);

  return d;
}

// [USER DEFINED]
float sample_distance(in vec3 p) {
  return p.y; //sdSphere(p - vec3(30.0f, 110.0f, 0.0f), 64.0);
}

//-----------------------------------------------------------------------------

#endif  // SHADER_DISTANCE_FUNC_GLSL_
