#version 430 core

layout(location = 0) in vec3 inPosition;

uniform mat4x4 uMVP;
uniform float uScaleFactor = 1.0f;

void main() {
  vec3 pos = uScaleFactor * inPosition.xyz;
  gl_Position = uMVP * vec4(pos, 1.0f);
}
