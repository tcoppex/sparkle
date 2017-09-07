#version 430 core

// ----------------------------------------------------------------------------

#include "sparkle/inc_rendering_shared.glsl"

// ----------------------------------------------------------------------------

uniform sampler2D uSpriteSampler2d;

layout(location = 0) out vec4 fragColor;

in VDataBlock {
  vec3 position; // unused legacy of vs_generic
  vec3 velocity; // idem
  vec3 color;
  float decay;
  float pointSize;
} IN;

// ----------------------------------------------------------------------------

void main() {
  fragColor = compute_color(IN.color, gl_PointCoord);
  fragColor *= IN.decay;
}

// ----------------------------------------------------------------------------
