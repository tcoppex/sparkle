#version 430 core

// ============================================================================
/*
 * First Stage of the particle system.
 * Initialize particles and add them to a buffer
*/
// ============================================================================

#include "sparkle/interop.h"
#include "sparkle/inc_math.glsl"

//-----------------------------------------------------------------------------

layout(location=0) uniform uint uEmitCount;
layout(location=1) uniform uint uEmitterType;
layout(location=2) uniform vec3 uEmitterPosition;
layout(location=3) uniform vec3 uEmitterDirection;
layout(location=4) uniform float uEmitterRadius;
layout(location=5) uniform float uParticleMinAge;
layout(location=6) uniform float uParticleMaxAge;

//-----------------------------------------------------------------------------

layout(binding = ATOMIC_COUNTER_BINDING_FIRST)
uniform atomic_uint write_count;

#if SPARKLE_USE_SOA_LAYOUT

layout(std430, binding = STORAGE_BINDING_PARTICLE_POSITIONS_A)
writeonly buffer PositionBufferA {
  vec4 positions[];
};

layout(std430, binding = STORAGE_BINDING_PARTICLE_VELOCITIES_A)
writeonly buffer VelocityBufferA {
  vec4 velocities[];
};

layout(std430, binding = STORAGE_BINDING_PARTICLE_ATTRIBUTES_A)
writeonly buffer AttributeBufferA {
  vec4 attributes[];
};

#else

layout(std430, binding = STORAGE_BINDING_PARTICLES_FIRST)
writeonly buffer ParticleBufferA {
  TParticle particles[];
};

#endif

layout(std430, binding = STORAGE_BINDING_RANDOM_VALUES)
readonly buffer RandomBuffer {
  float randbuffer[];
};

// ----------------------------------------------------------------------------

void PushParticle(in vec3 position,
                  in vec3 velocity,
                  in float age)
{
  // Emit particle id.
  const uint id = atomicCounterIncrement(write_count);

#if SPARKLE_USE_SOA_LAYOUT
  positions[id]  = vec4(position, 1.0f);
  velocities[id] = vec4(velocity, 0.0f);
  attributes[id] = vec4(age, age, 0.0f, uintBitsToFloat(id));
#else
  TParticle p;
  p.position = vec4(position, 1.0f);
  p.velocity = vec4(velocity, 0.0f);
  p.start_age = age;
  p.age = age;
  p.id = id;

  particles[id] = p;
#endif
}

// ----------------------------------------------------------------------------

void CreateParticle(const uint gid) {
  // Random vector.
  const uint rid = 3u * gid;
  const vec3 rn = vec3(randbuffer[rid], randbuffer[rid+1u], randbuffer[rid+2u]);

  // Position
  vec3 pos = uEmitterPosition;
  if (uEmitterType == 1) {
    //pos += disk_distribution(uEmitterRadius, rn.xy);
    pos += disk_even_distribution(uEmitterRadius, gid, uEmitCount);
  } else if (uEmitterType == 2) {
    pos += sphere_distribution(uEmitterRadius, rn.xy);
  } else if (uEmitterType == 3) {
    pos += ball_distribution(uEmitterRadius, rn);
  }

  // Velocity
  vec3 vel = uEmitterDirection;

  // Age
  // The age is set by thread groups to assure we have a number of particles
  // factors of groupWidth, this method is safe but prevents continuous emission.
  const float group_rand = randbuffer[gl_WorkGroupID.x];
  // [As the threadgroup are not full, some dead particles might appears if not
  // skipped in following stages].
  const float single_rand = randbuffer[gid];

  const float age = mix( uParticleMinAge, uParticleMaxAge, single_rand);

  PushParticle(pos, vel, age);
}

// ----------------------------------------------------------------------------

layout(local_size_x = PARTICLES_KERNEL_GROUP_WIDTH) in;
void main() {
  const uint gid = gl_GlobalInvocationID.x;

  if (gid < uEmitCount) {
    CreateParticle(gid);
  }
}

