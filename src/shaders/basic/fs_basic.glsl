#version 430 core

layout(location = 0) out vec4 fragColor;

uniform vec4 uColor = vec4(1.0f);

void main() {
  fragColor = uColor;
}
