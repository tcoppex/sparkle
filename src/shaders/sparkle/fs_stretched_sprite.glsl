#version 430 core

// ----------------------------------------------------------------------------

#include "sparkle/inc_rendering_shared.glsl"

// ----------------------------------------------------------------------------

uniform sampler2D uSpriteSampler2d;

layout(location = 0) out vec4 fragColor;

in GDataBlock {
  vec3 color;
  vec2 texcoord;
  float decay;
} IN;

// ----------------------------------------------------------------------------

void main() {
  fragColor = compute_color(IN.color, IN.texcoord);
  fragColor *= IN.decay * 0.25;
}

// ----------------------------------------------------------------------------
