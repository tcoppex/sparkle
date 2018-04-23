#version 430 core

uniform mat4 uMVP;
uniform mat4 uView;
uniform float uSpriteStretchFactor;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VDataBlock {
  vec3 position;
  vec3 velocity;
  vec3 color;
  float decay;
  float pointSize;
} IN[];

out GDataBlock {
  vec3 color;
  vec2 texcoord;
  float decay;
} OUT;

void main() {
  const mat3 view = mat3(uView);

  // view space velocity
  vec3 u = view * IN[0].velocity;
  const float dp_u = dot(u, u);

  // NOTE : we should use the dot product between particle_to_eye and velocity instead.
  // dp_u and nz are false approximation

  // closer to 1, the particle velocity face the camera
  float nz = (u*inversesqrt(dp_u)).z;
  nz *= nz;

  // stretched billboard dimensions.
  const float w = 0.2f;

  // when face to the camera, the particle is not stretched.
  const float speed = smoothstep(0.0f, 1.0f/w, dp_u);
  float h = mix(0.1f, uSpriteStretchFactor, speed);
        h = mix(h, 1.0f, nz) * w;

  // Origin height of the quad to create
  float origin_y = 0.0;

  //const float u_step = step(0.9, nz);
  //origin_y = mix(0.0f, -h, u_step);
  //h = mix(h, w, u_step);

  // screen-space velocity
#if 0
  // Try to blend between top view and side view [wip]
  vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
  vec3   top = vec3(view[1][0], view[1][1], view[2][1]);

  u = mix(u / u.z, cross(right, top), u_step);
#else
  // incorrect
  u.z = 0.0;
  u = normalize(u);
#endif

  // orthogonal screen-space vector.
  const vec3 v = vec3(-u.y, u.x, 0.0f);

  // Change of basis matrix.
  const vec3 a = normalize(v * view);
  const vec3 b = normalize(u * view);
  const vec3 c = normalize(cross(a, b));
  const mat3 basis = mat3(a, b, c);

  // Offset vectors.
  const vec3 O = basis * vec3(0,  origin_y, 0);
  const vec3 N = basis * vec3(0,  h, 0);
  const vec3 W = basis * vec3(w,  0, 0);

  // Emit the quad primitive.
  OUT.color = IN[0].color;
  OUT.decay = IN[0].decay;

  vec3 p = IN[0].position;
  OUT.texcoord = vec2(0.0f, 0.0f); gl_Position = uMVP * vec4(p+W+O, 1.0f); EmitVertex();
  OUT.texcoord = vec2(1.0f, 0.0f); gl_Position = uMVP * vec4(p-W+O, 1.0f); EmitVertex();
  OUT.texcoord = vec2(0.0f, 1.0f); gl_Position = uMVP * vec4(p+W+O+2*N, 1.0f); EmitVertex();
  OUT.texcoord = vec2(1.0f, 1.0f); gl_Position = uMVP * vec4(p-W+O+2*N, 1.0f); EmitVertex();
  EndPrimitive();
}
