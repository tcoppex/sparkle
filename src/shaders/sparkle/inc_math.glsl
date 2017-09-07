#ifndef SHADERS_MATH_GLSL_
#define SHADERS_MATH_GLSL_

float Pi() {
  return 3.141564f;
}

float TwoPi() {
  return 6.283185f;
}

mat3 rotationX(float alpha) {
  const float c = cos(alpha);
  const float s = sin(alpha);
  return mat3(
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, c, s),
    vec3(0.0f, -s, c)
  );
}

mat3 rotationY(float alpha) {
  const float c = cos(alpha);
  const float s = sin(alpha);
  return mat3(
    vec3(c, 0.0f, -s),
    vec3(0.0f, 1.0f, 0.0f),
    vec3(s, 0.0f, c)
  );
}

mat3 rotationZ(float alpha) {
  const float c = cos(alpha);
  const float s = sin(alpha);
  return mat3(
    vec3(c, s, 0.0f),
    vec3(-s, c, 0.0f),
    vec3(0.0f, 0.0f, 1.0f)
  );
}

#endif  // SHADERS_MATH_GLSL_