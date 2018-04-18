#ifndef SHADERS_MATH_GLSL_
#define SHADERS_MATH_GLSL_

float Pi() {
  return 3.141564f;
}

float TwoPi() {
  return 6.283185f;
}

mat3 rotationX(float c, float s) {
  return mat3(
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, c, s),
    vec3(0.0f, -s, c)
  );
}

mat3 rotationY(float c, float s) {
  return mat3(
    vec3(c, 0.0f, -s),
    vec3(0.0f, 1.0f, 0.0f),
    vec3(s, 0.0f, c)
  );
}

mat3 rotationZ(float c, float s) {
  return mat3(
    vec3(c, s, 0.0f),
    vec3(-s, c, 0.0f),
    vec3(0.0f, 0.0f, 1.0f)
  );
}

mat3 rotationX(float radians) {
  return rotationX(cos(radians), sin(radians));
}

mat3 rotationY(float radians) {
  return rotationY(cos(radians), sin(radians));
}

mat3 rotationZ(float radians) {
  return rotationZ(cos(radians), sin(radians));
}

#endif  // SHADERS_MATH_GLSL_
