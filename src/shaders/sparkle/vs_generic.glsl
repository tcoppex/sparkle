#version 450 core

// ----------------------------------------------------------------------------

layout(location=0) in vec3 position;
layout(location=1) in vec3 velocity;
layout(location=2) in vec2 age_info;

uniform mat4 uMVP;
uniform float uMinParticleSize = 1.0f; //
uniform float uMaxParticleSize = 6.0f; //
uniform float uColorMode = 0;
uniform vec3 uBirthGradient = vec3(1.0f, 0.0f, 0.0f);
uniform vec3 uDeathGradient = vec3(0.0f);

out VDataBlock {
  vec3 position;
  vec3 velocity;
  vec3 color;
  float decay;
  float pointSize;
} OUT;

// ----------------------------------------------------------------------------

/* Map a range from [edge0, edge1] to [0, 1]. */
float maprange(float edge0, float edge1, float x) {
  return clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
}

// ----------------------------------------------------------------------------

/* Map a value in [0, 1] to peak at edge. */
float curve_inout(in float x, in float edge) {
  // Coefficient for sub range.
  float a = maprange(0.0f, edge, x);
  float b = maprange(edge, 1.0f, x);

  // Quadratic ease-in / quadratic ease-out.
  float easein = a * (2.0f - a);         // a * a;
  float easeout = 1.0f - b * b;          // b*b - 2.0f*b + 1.0f

  // Makes particles fade-in and out of existence.
  return mix(easein, easeout, step(edge, x));
}

// ----------------------------------------------------------------------------

float compute_size(float z, float decay) {
  const float min_size = uMinParticleSize;
  const float max_size = uMaxParticleSize;

  // tricks to 'zoom-in' the pointsprite, just set to 1 to have normal size.
  const float depth = (max_size-min_size) / (z);

  float size = mix(min_size, max_size, decay * depth);

  return size;
}

// ----------------------------------------------------------------------------

vec3 base_color(in vec3 position, in float decay) {
  // Gradient mode
  if (uColorMode == 1) {
    return mix(uBirthGradient, uDeathGradient, decay);
  }
  // Default mode
  return 0.5f * (normalize(position) + 1.0f);
}

// ----------------------------------------------------------------------------

void main() {
  const vec3 p = position.xyz;

  // Time alived in [0, 1].
  const float dAge = 1.0f - maprange(0.0f, age_info.x, age_info.y);
  const float decay = curve_inout(dAge, 0.52f);

  // Vertex attributes.
  gl_Position = uMVP * vec4(p, 1.0f);
  gl_PointSize = compute_size(gl_Position.z, decay);

  // Output parameters.
  OUT.position = p;
  OUT.velocity = velocity.xyz;
  OUT.color = base_color(position, decay);
  OUT.decay = decay;
  OUT.pointSize = gl_PointSize;
}

// ----------------------------------------------------------------------------
