#version 450 core

// ----------------------------------------------------------------------------

layout(location=0) in vec3 position;
layout(location=1) in vec3 velocity;
layout(location=2) in vec2 age_info;

uniform mat4 uMVP;

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
  const float min_size = 1.0f;
  const float max_size = 6.0f;
  const float depth = 1.0f;//  / (z);

  float size = mix(min_size, max_size, decay * depth);

  return size;
}

// ----------------------------------------------------------------------------

void main() {
  const vec3 p = position.xyz;

  // Time alived in [0, 1].
  const float dAge = 1.0f - maprange(0.0f, age_info.x, age_info.y);
  const float decay = curve_inout(dAge, 0.15f);

  // Vertex attributes.
  gl_Position = uMVP * vec4(p, 1.0f);
  gl_PointSize = compute_size(gl_Position.z, decay);

  // Output parameters.
  OUT.position = p;
  OUT.velocity = velocity.xyz;
  OUT.color = 0.5f * (normalize(p) + 1.0f);
  OUT.decay = decay;
  OUT.pointSize = gl_PointSize;
}

// ----------------------------------------------------------------------------
