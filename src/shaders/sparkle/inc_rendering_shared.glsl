
uniform sampler2D uSpriteSampler2d;
uniform float uFadeCoefficient = 0.25f;

vec4 compute_color(in vec3 base_color, in float decay, in vec2 texcoord) {
  vec4 color = vec4(base_color, 1.0f);

  // Centered coordinates.
  const vec2 p = 2.0f * (texcoord - 0.5f);
  // Pixel intensity depends on its distance from center.
  float d = 1.0f - abs(dot(p, p));

  // Alpha coefficient.
  float alpha = smoothstep(0.0f, 1.0f, d);

  //color = texture(uSpriteSampler2d, texcoord).rrrr;
  color *= alpha * decay * uFadeCoefficient;

  return color;
}
