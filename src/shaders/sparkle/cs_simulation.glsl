#version 430 core

// ============================================================================

/* Second Stage of the particle system :
 * - Update particle position and velocity,
 * - Apply curl noise,
 * - Apply vector field,
 * - Performs time integration,
 * - Handle collision detection,
 */

// ============================================================================


#include "sparkle/interop.h"
#include "sparkle/inc_curlnoise.glsl"

// ----------------------------------------------------------------------------

#define ENABLE_SCATTERING         0
#define ENABLE_VECTORFIELD        0
#define ENABLE_CURLNOISE          1

// ----------------------------------------------------------------------------

// Time integration step.
uniform float uDeltaT;
// Vector field sampler.
uniform sampler3D uVectorFieldSampler;
// Simulation box dimension.
uniform float uBBoxSize;

// ----------------------------------------------------------------------------

layout(binding = ATOMIC_COUNTER_BINDING_FIRST)
uniform atomic_uint read_count;

layout(binding = ATOMIC_COUNTER_BINDING_SECOND)
uniform atomic_uint write_count;

// ----------------------------------------------------------------------------

#if SPARKLE_USE_SOA_LAYOUT

// READ BUFFERs

layout(std430, binding = STORAGE_BINDING_PARTICLE_POSITIONS_A)
readonly buffer PositionBufferA {
  vec4 read_positions[];
};
layout(std430, binding = STORAGE_BINDING_PARTICLE_VELOCITIES_A)
readonly buffer VelocityBufferA {
  vec4 read_velocities[];
};
layout(std430, binding = STORAGE_BINDING_PARTICLE_ATTRIBUTES_A)
readonly buffer AttributeBufferA {
  vec4 read_attributes[];
};

// WRITE BUFFERs

layout(std430, binding = STORAGE_BINDING_PARTICLE_POSITIONS_B)
writeonly buffer PositionBufferB {
  vec4 write_positions[];
};
layout(std430, binding = STORAGE_BINDING_PARTICLE_VELOCITIES_B)
writeonly buffer VelocityBufferB {
  vec4 write_velocities[];
};
layout(std430, binding = STORAGE_BINDING_PARTICLE_ATTRIBUTES_B)
writeonly buffer AttributeBufferB {
  vec4 write_attributes[];
};


#else

layout(std430, binding = STORAGE_BINDING_PARTICLES_FIRST)
readonly buffer ParticleBufferA {
  TParticle read_particles[];
};

layout(std430, binding = STORAGE_BINDING_PARTICLES_SECOND)
writeonly buffer ParticleBufferB {
  TParticle write_particles[];
};

#endif  // SPARKLE_USE_SOA_LAYOUT

layout(std430, binding = STORAGE_BINDING_RANDOM_VALUES)
readonly buffer RandomBuffer {
  float randbuffer[];
};

// ----------------------------------------------------------------------------

TParticle PopParticle() {
  const uint index = gl_GlobalInvocationID.x;
  atomicCounterDecrement(read_count);

  TParticle p;

#if SPARKLE_USE_SOA_LAYOUT
  p.position   = read_positions[index];
  p.velocity   = read_velocities[index];
  vec4 attribs = read_attributes[index];

  p.start_age  = attribs.x;
  p.age        = attribs.y;
  p.id         = floatBitsToUint(attribs.w);
#else
  p = read_particles[index];
#endif

  return p;
}

void PushParticle(in TParticle p) {
  const uint index = atomicCounterIncrement(write_count);

#if SPARKLE_USE_SOA_LAYOUT
  write_positions[index]  = p.position;
  write_velocities[index] = p.velocity;
  write_attributes[index] = vec4(p.start_age, p.age, 0.0f, uintBitsToFloat(p.id));
#else
  write_particles[index] = p;
#endif
}

// ----------------------------------------------------------------------------

float UpdateAge(in const TParticle p) {
  const float decay = 0.01*uDeltaT;
  float age = clamp(p.age - decay, 0.0f, p.start_age);
  return age;
}

void UpdateParticle(inout TParticle p,
                    in vec3 pos,
                    in vec3 vel,
                    in float age) {
  p.position.xyz = pos;
  p.velocity.xyz = vel;
  p.age = age;
}

// ----------------------------------------------------------------------------

vec3 ApplyForces() {
  vec3 force = vec3(0.0f);

#if ENABLE_SCATTERING
  // Add a random force to each particles.
  const float scattering = 0.45f;
  const uint gid = gl_GlobalInvocationID.x;
  vec3 randforce = 2.0f * vec3(randbuffer[gid], randbuffer[gid+1u], randbuffer[gid+2u]) - 1.0f;
  force += scattering * randforce;
#endif

  return force;
}

// ----------------------------------------------------------------------------

vec3 ApplyRepulsion(in const TParticle p) {

  vec3 push = vec3(0.0f);
/*
  // IDEA
  const vec3 vel = p.velocity.xyz;
  const vec3 pos = p.position.xyz;
  const float MAX_INFLUENCE_DISTANCE = 8.0f;

  vec3 n;
  float d = compute_gradient(pos, n);
  float coeff = smoothstep(0.0f, MAX_INFLUENCE_DISTANCE, abs(d));
  push = coeff * (n);
  //vec3 side = cross(cross(n, normalize(vel + vec3(1e-5))), n);
  //push = mix(push, side, coeff*coeff);
*/
  return push;
}

// ----------------------------------------------------------------------------

vec3 ApplyTargetMesh(in const TParticle p) {
  vec3 pull = vec3(0.0f);

  /*
  // IDEA
  vec3 anchor = anchors_buffer[p.anchor_id];
  mat4x4 anchorModel = achor_models_buffer[p.anchor_id];
  vec3 target = anchorModel * vec4(anchor, 1.0f);
  vec3 pull = target - p.position;
  float length_pull = length(pull);
  pull *= inversesqrt(length_pull));
  float factor = MAX_PULL_FACTOR * smoothstep(0.0f, MAX_PULL_FACTOR, length_pull);
  pull *= factor;
  */

  return pull;
}

// ----------------------------------------------------------------------------

vec3 ApplyVectorField(in const TParticle p) {
  vec3 vfield = vec3(0.0f);

#if ENABLE_VECTORFIELD
  const vec3 pt = p.position.xyz;

  const ivec3 texsize = textureSize(uVectorFieldSampler, 0).xyz;
  const vec3 extent = 0.5f * vec3(texsize.x, texsize.y, texsize.z);
  const vec3 texcoord = (pt + extent) / (2.0f * extent);

  vfield = texture(uVectorFieldSampler, texcoord).xyz;

  /* Custom GL_CLAMP_TO_BORDER */
  vec3 clamp_to_border = step(-extent, pt) * step(pt, +extent);
  bool b = any(lessThan(clamp_to_border, vec3(1.0f)));
  vfield = mix(vfield, vec3(0.0f), float(b));
#endif

  return vfield;
}

// ----------------------------------------------------------------------------

vec3 GetCurlNoise(in const TParticle p) {
  vec3 curl = vec3(0.0f);

#if ENABLE_CURLNOISE
  const float effect = 1.0f;
  const float scale = 1.0f / 128.0f;
  curl = effect * compute_curl(p.position.xyz * scale);
#endif

  return curl;
}

// ----------------------------------------------------------------------------

void CollideSphere(float r, in vec3 center, inout vec3 pos, inout vec3 vel) {
  const vec3 p = pos - center;

  const float dp = dot(p, p);
  const float r2 = r*r;

  if (dp > r2) {
    vec3 n = -p * inversesqrt(dp);
    vel = reflect(vel, n);

    pos = center - r*n;
  }
}

void CollideBox(in vec3 corner, in vec3 center, inout vec3 pos, inout vec3 vel) {
  vec3 p = pos - center;

  if (p.x < -corner.x) {
    p.x = -corner.x;
    vel = reflect(vel, vec3(1.0f,0.0f,0.0f));
  }

  if (p.x > corner.x) {
    p.x = corner.x;
    vel = reflect(vel, vec3(-1.0f,0.0f,0.0f));
  }

  if (p.y < -corner.y) {
    p.y = -corner.y;
    vel = reflect(vel, vec3(0.0f,1.0f,0.0f));
  }

  if (p.y > corner.y) {
    p.y = corner.y;
    vel = reflect(vel, vec3(0.0f,-1.0f,0.0f));
  }

  if (p.z < -corner.z) {
    p.z = -corner.z;
    vel = reflect(vel, vec3(0.0f,0.0f,1.0f));
  }

  if (p.z > corner.z) {
    p.z = corner.z;
    vel = reflect(vel, vec3(0.0f,0.0f,-1.0f));
  }

  pos = p + center;
}

void CollisionHandling(inout vec3 pos, inout vec3 vel) {
  const float r = 0.5f * uBBoxSize;

  CollideSphere(r, vec3(0.0f), pos, vel);
  CollideBox(vec3(r), vec3(0.0f), pos, vel);
}

// ----------------------------------------------------------------------------

layout(local_size_x = PARTICLES_KERNEL_GROUP_WIDTH) in;
void main() {
  const vec3 dt = vec3(uDeltaT);

  // Local copy of the particle.
  TParticle p = PopParticle();

  // Update age
  /// [ dead particles still have a positive age, but are not push in render buffer.
  ///   Still set their age to zero ? ]
  float age = UpdateAge(p);

  if (age > 0.0f) {
    // Apply external forces.
    vec3 force = ApplyForces();

    // Apply repulsions on simple objects.
    force += ApplyRepulsion(p);

    // Apply mesh targeting.
    force += ApplyTargetMesh(p);

    // Apply vector field.
    force += ApplyVectorField(p);

    // Integrate velocity.
    vec3 vel = fma(force, dt, p.velocity.xyz);

    // Get curling noise.
    vec3 noise_vel = GetCurlNoise(p);

    // ----------
    vel += noise_vel; noise_vel *= 0.0f;
    vel = 10*normalize(vel);
    // ----------

    // Integrate position.
    vec3 pos = fma(vel + noise_vel, dt, p.position.xyz);

    // Handle collision.
    CollisionHandling(pos, vel);

    // Update particle.
    UpdateParticle(p, pos, vel, age);
    PushParticle(p);
  }

}
