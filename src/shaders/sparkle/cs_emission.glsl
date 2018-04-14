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
layout(location=1) uniform vec3 uEmitterPosition;
layout(location=2) uniform vec3 uEmitterDirection;
layout(location=3) uniform float uParticleMinAge;
layout(location=4) uniform float uParticleMaxAge;

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
  attributes[id] = vec4(age, age, 0.0f, uintBitsToFloat(id)); //
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

  // ---------------------
  vec3 pos = uEmitterPosition;
  vec3 vel = uEmitterDirection;

  float r = 60.0f * rn.x;
  vec3 theta = rn * TwoPi();

#if 0
  // Position in a sphere with omnidirectional velocity.
  pos = r * uEmitterDirection * rotationZ(theta.z) * rotationY(theta.y);
  //vel = normalize(pos);
#else
  // Position on a plate going up.
  pos.x = r * cos(theta.y);
  pos.y = 0.1f * rn.z;
  pos.z = r * sin(theta.y);
  //vel = vec3(0.0f);
#endif

  // The age is set by group to assure to have a number of particles factor of groupWidth.
  float age = mix( uParticleMinAge, uParticleMaxAge, randbuffer[gl_WorkGroupID.x]);
  // ---------------------

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

