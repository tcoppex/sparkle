#version 430 core

in VDataBlock {
  vec4 color;
} IN;

layout(location = 0) out vec4 fragColor;

void main() {
  fragColor = IN.color;
}
