#version 430 core

// ----------------------------------------------------------------------------

layout(location = 0) in vec2 inPosition;

uniform mat4x4 uMVP;
uniform vec4 uGridColor = vec4(vec3(0.25f), 0.75f);
uniform float uScaleFactor;

out VDataBlock {
  vec4 color;
} OUT;

// ----------------------------------------------------------------------------

void main() {
  vec3 pos = uScaleFactor * vec3(inPosition.x, 0.0f, inPosition.y);
  bool main_axis = any(equal(inPosition.xy, vec2(0.0f)));
  vec3 axis_color = (vec3(1.0f) - step(vec3(0.0f), -abs(pos))) * 0.75f;
  OUT.color.rgb = (main_axis) ? axis_color : uGridColor.rgb;
  OUT.color.a = uGridColor.a;

  gl_Position = uMVP * vec4(pos, 1.0f);
}

// ----------------------------------------------------------------------------
