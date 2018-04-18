#ifndef SHADERS_MATH_GLSL_
#define SHADERS_MATH_GLSL_

float Pi() {
  return 3.141564f;
}

float TwoPi() {
  return 6.283185f;
}

float GoldenAngle() {
  return 2.399963f;
}

float cbrt(float x) {
  return pow(x, 0.33333f);
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

vec3 disk_distribution(float radius, vec2 rn) {
  const float r = radius * rn.x;
  const float theta = TwoPi() * rn.y;
  return vec3(
    r * cos(theta),
    0.0f,
    r * sin(theta)
  );
}

vec3 disk_even_distribution(float radius, uint id, uint total) {
  // ref : http://blog.marmakoide.org/?p=1
  const float theta = id * GoldenAngle();
  const float r = radius *  sqrt(id / float(total));
  return vec3(
    r * cos(theta),
    0.0f,
    r * sin(theta)
  );
}

vec3 sphere_distribution(float radius, vec2 rn) {
  // ref : https://www.cs.cmu.edu/~mws/rpos.html
  //       https://gist.github.com/dinob0t/9597525
  const float phi = TwoPi() * rn.x;
  const float z = radius * (2.0f * rn.y - 1.0f);
  const float r = sqrt(radius * radius - z * z);
  return vec3(
    r * cos(phi),
    r * sin(phi),
    z
  );
}

vec3 ball_distribution(float radius, vec3 rn) {
  // ref : so@5408276
  const float costheta = 2.0f * rn.x - 1.0f;
  const float phi = TwoPi() * rn.y;
  const float theta = acos(costheta);
  const float r = radius * cbrt(rn.z);
  const float s = sin(theta);

  return r * vec3(
    s * cos(phi),
    s * sin(phi),
    costheta
  );
}

#endif  // SHADERS_MATH_GLSL_
