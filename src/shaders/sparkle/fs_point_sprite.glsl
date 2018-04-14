#version 430 core

// ----------------------------------------------------------------------------

#include "sparkle/inc_rendering_shared.glsl"

// ----------------------------------------------------------------------------

in VDataBlock {
  vec3 position; // unused legacy of vs_generic
  vec3 velocity; // idem
  vec3 color;
  float decay;
  float pointSize;
} IN;

out vec4 fragColor;

// ----------------------------------------------------------------------------

void main() {
  fragColor = compute_color(IN.color, gl_PointCoord);
  fragColor *= IN.decay;
}

// ----------------------------------------------------------------------------
