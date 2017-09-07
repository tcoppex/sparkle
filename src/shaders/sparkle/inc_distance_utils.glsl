// -----------------------------------------------------------------------------
//
//      Distance functions helpers for simple shapes and CSG operations.
//
//      ref : 'modeling with distance functions' - Inigo Quilez
//
//             This is not a MAIN shader, it must be included.
//
//------------------------------------------------------------------------------

#ifndef SHADER_DISTANCE_UTILS_GLSL_
#define SHADER_DISTANCE_UTILS_GLSL_

//-----------------------------------------------------------------------------

// CSG operations

float opUnion(float d1, float d2) {
  return min(d1, d2);
}

float opSmoothUnion(float d1, float d2, float k) {
  float r = exp(-k*d1) + exp(-k*d2);
  return -log(r) / k;
}

float opIntersection(float d1, float d2) {
  return max(d1, d2);
}

float opSubstraction(float d1, float d2) {
  return max(d1, -d2);
}

vec3 opRepeat(in vec3 p, in vec3 c) {
  return mod(p, c) - 0.5f*c;
}

float opDisplacement(in vec3 p, float d) {
  p = d*p;
  return sin(p.x)*sin(p.y)*sin(p.z);
}

//-----------------------------------------------------------------------------

// PRIMITIVEs

float sdPlane(in vec3 p, in vec4 n) {
  //n.xyz = normalize(n.xyz);
  return n.w + dot(p, n.xyz);
}

float sdSphere(in vec3 p, float r) {
  return length(p) - r;
}

float udRoundBox(in vec3 p, in vec3 b, float r) {
  return length(max(abs(p)-b, 0.0f)) - r;
}

float sdCylinder(in vec3 p, float c) {
  return length(p.xy) - c;
}

float sdCylinder(in vec3 p, vec3 c) {
  return opIntersection(length(p.xz-c.xy) - c.z, abs(p.y)-c.y);
}

float sdTorus(in vec3 p, vec2 t) {
  vec2 q = vec2(length(p.xz) - t.x, p.y);
  return length(q) - t.y;
}

/*
float sdCube(in vec3 p, in vec3 b) {
  vec3 d = abs(p) - b;
  float minDist = min(max( d.x, max(d.y, d.z)), 0.0f);
  return minDist + 0*length(max(d, 0.0f));
}*/

//-----------------------------------------------------------------------------

#endif  // SHADER_DISTANCE_UTILS_GLSL_
