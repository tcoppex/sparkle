#include "api/vector_field.h"

#include <cfloat>
#include <cmath>
#include <cstdio>
#include <vector>

#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/noise.hpp"

/* -------------------------------------------------------------------------- */

void VectorField::initialize(unsigned int const width, unsigned int const height, unsigned int const depth) {
  dimensions_ = glm::vec3(width, height, depth);

  /// @bug
  /// Velocity fields are 3d textures where only particles in the texture volume
  /// are affected. Therefore particles outside the volume should be clamped to
  /// zeroes when the texture is sampled.
  /// However there is some strange and buggy behavior when specifying a border
  /// of 3 or more values, so here only two are specified for the 3d texture.
  /// Wrap mode is set to clamp_to_edge for now.

  GLint const filter_mode = GL_LINEAR;
  GLint const wrap_mode = GL_CLAMP_TO_EDGE;
  //GLfloat const border[4u] = {0.0f, 0.0f, 0.0f, 0.0f};

  glGenTextures(1u, &gl_texture_id_);
  glBindTexture(GL_TEXTURE_3D, gl_texture_id_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, filter_mode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, filter_mode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_mode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_mode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_mode);
    //glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, border);
  glBindTexture(GL_TEXTURE_3D, 0u);

  CHECKGLERROR();
}

void VectorField::deinitialize() {
  glDeleteTextures(1u, &gl_texture_id_);
}

void VectorField::generate_values(char const* filename) {
  unsigned int const W = dimensions_.x;
  unsigned int const H = dimensions_.y;
  unsigned int const D = dimensions_.z;
  float const dW = 1.0f / static_cast<float>(W);
  float const dH = 1.0f / static_cast<float>(H);
  float const dD = 1.0f / static_cast<float>(D);

  // velocity data buffer.
  std::vector<float> data(3u*W*H*D);

  // If true, the datas will be recalculated.
  bool bForceCalculate = false;

  // Read data from a file if it exists.
  FILE *fd = fopen(filename, "rb");

  if (fd) {
    size_t nreads = fread(&data[0u], sizeof(float), data.size(), fd);
    if (nreads != data.size()) {
      fprintf(stderr, "Velocity Field: incorrect velocity file \"%s\", recalculating.\n", filename);
      bForceCalculate = true;
    }
  }

  // Or recalculate the data.
  if (!fd || bForceCalculate) {
    std::vector<float>::iterator pData = data.begin();
    for (unsigned int z=0u; z < D; ++z) {
      float const dz = z * dD;
      for (unsigned int y=0u; y < H; ++y) {
        float const dy = y * dH;
        for (unsigned int x=0u; x < W; ++x) {
          float const dx = x * dW;
          //unsigned int const index = 3u * z * (height_ * width_) + (y * width_) + x;

          glm::vec3 p = glm::vec3(dx, dy, dz);
          glm::vec3 v = _generate_vector(p);
          *pData++ = v.x;
          *pData++ = v.y;
          *pData++ = v.z;
        }
      }
      unsigned int percent = static_cast<unsigned int>(100u * (z+1u) * dD);
      fprintf(stdout, ">> Calculating velocity field data : %2u%%\r", percent);
    }

    // Save on disk.5
    fd = fopen(filename, "wb");
    fwrite(&data[0u], sizeof(float), data.size(), fd);
  }
  fclose(fd);

  // Transfer pixels to device.
  glBindTexture(GL_TEXTURE_3D, gl_texture_id_);
  glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGB32F, W, H, D);
  glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, W, H, D, GL_RGB, GL_FLOAT, data.data());
  glBindTexture(GL_TEXTURE_3D, 0u);

  CHECKGLERROR();
}

// ----------------------------------------------------------------------------

glm::vec3 VectorField::_generate_vector(glm::vec3 const& p) const {
#if  0
  float scale = 1.0f;
  float const n1 = glm::simplex(scale * p);
  float const n2 = glm::simplex(scale * (p + glm::vec3(230.4f, 640.7f, -150.1f)));
  float const TwoPi = glm::two_pi<float>();

  glm::vec3 v = glm::vec3(1.0f, 1.0f, 0.0f) *
                glm::angleAxis(n1*TwoPi, glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::angleAxis(n2*TwoPi, glm::vec3(0.0f, 1.0f, 0.0f));
#else
  glm::vec3 pos = p - glm::vec3(0.5f);
  glm::vec3 v = glm::vec3(pos.y, -pos.x, 0.0f);

  // Stangely, normalizing the value seems to bring error in the shader stage.
  // (particle disappearing)
  //v = glm::normalize(v);
#endif

  return v;
}


/* -------------------------------------------------------------------------- */
