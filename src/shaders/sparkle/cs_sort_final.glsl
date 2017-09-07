#version 430 core

// ============================================================================
/**
  * Reorder particles with respect to their sorted indices.
  *
  * Despite its simplicity this is the most costly kernel of the pipeline.
  * (incoherent bank access prevents optimized gathering, changing for a
  * structure of arrays layout could help better cache use) :
  * eg.
  * vec4 positions[]
  * vec4 velocities[]
  * vec4 attributes[]
  * ....
  * check GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS  (~ 16)
  */

// ============================================================================

#include "sparkle/interop.h"

#if SPARKLE_USE_SOA_LAYOUT

// READ BUFFERs

layout(std430, binding = STORAGE_BINDING_PARTICLE_POSITIONS_B)
readonly buffer PositionBufferB {
  vec4 read_positions[];
};

layout(std430, binding = STORAGE_BINDING_PARTICLE_VELOCITIES_B)
readonly buffer VelocityBufferB {
  vec4 read_velocities[];
};

layout(std430, binding = STORAGE_BINDING_PARTICLE_ATTRIBUTES_B)
readonly buffer AttributeBufferB {
  vec4 read_attributes[];
};

// WRITE BUFFERs

layout(std430, binding = STORAGE_BINDING_PARTICLE_POSITIONS_A)
writeonly buffer PositionBufferA {
  vec4 write_positions[];
};
layout(std430, binding = STORAGE_BINDING_PARTICLE_VELOCITIES_A)
writeonly buffer VelocityBufferA {
  vec4 write_velocities[];
};
layout(std430, binding = STORAGE_BINDING_PARTICLE_ATTRIBUTES_A)
writeonly buffer AttributeBufferA {
  vec4 write_attributes[];
};

#else

layout(std430, binding = STORAGE_BINDING_PARTICLES_SECOND)
readonly buffer ParticleBufferB {
  TParticle read_particles[];
};

layout(std430, binding = STORAGE_BINDING_PARTICLES_FIRST)
writeonly buffer ParticleBufferA {
  TParticle write_particles[];
};

#endif  // SPARKLE_USE_SOA_LAYOUT

layout(std430, binding = STORAGE_BINDING_INDICES_FIRST)
readonly buffer IndexBuffer {
  uint indices[];
};

// ----------------------------------------------------------------------------

layout(local_size_x = PARTICLES_KERNEL_GROUP_WIDTH) in;
void main() {
  const uint tid = gl_GlobalInvocationID.x;

  uint read_id = indices[tid];

#if SPARKLE_USE_SOA_LAYOUT
  write_positions[tid]  = read_positions[read_id];
  write_velocities[tid] = read_velocities[read_id];
  write_attributes[tid] = read_attributes[read_id];
#else
  write_particles[tid] = read_particles[read_id];
#endif
}

// ============================================================================
