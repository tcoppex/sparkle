#version 430 core

/* Sorting step of the bitonic-sort algorithm.
 * This kernel sort indices by comparing dot products together.
*/

#include "sparkle/interop.h"

//-----------------------------------------------------------------------------

layout(std430, binding = STORAGE_BINDING_DOT_PRODUCTS)
readonly coherent buffer DotProducts {
  float dp[];
};

layout(std430, binding = STORAGE_BINDING_INDICES_FIRST)
readonly buffer ReadIndices {
  uint read_indices[];
};

layout(std430, binding = STORAGE_BINDING_INDICES_SECOND)
writeonly buffer WriteIndices {
  uint write_indices[];
};

//-----------------------------------------------------------------------------

uniform uint uBlockWidth;
uniform uint uMaxBlockWidth;

bool cmp(float a, float b) {
  return a > b;
}

void CompareAndSwap(in uint order, inout uint left, inout uint right) {
  if (bool(order) == cmp(dp[left], dp[right])) {
    left  ^= right;
    right ^= left;
    left  ^= right;
  }
}

layout(local_size_x = PARTICLES_KERNEL_GROUP_WIDTH) in;
void main() {
  uint tid = gl_GlobalInvocationID.x;

  const uint max_block_width = uMaxBlockWidth;
  const uint block_width = uBlockWidth;
  const uint pair_distance = block_width / 2u;

  const uint block_offset = (tid / pair_distance) * block_width;
  const uint left_id = block_offset + (tid % pair_distance);
  const uint right_id = left_id + pair_distance;

  // Data to sort
  uint left_data  = read_indices[left_id];
  uint right_data = read_indices[right_id];

  const uint order = (left_id / max_block_width) & 1;
  CompareAndSwap(order, left_data, right_data);

  write_indices[left_id]  = left_data;
  write_indices[right_id] = right_data;
}
