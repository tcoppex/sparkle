#ifndef API_VECTOR_FIELD_H_
#define API_VECTOR_FIELD_H_

#include "opengl.h"
#include "glm/glm.hpp"

/* -------------------------------------------------------------------------- */

class VectorField {
 public:
  VectorField()
    : gl_texture_id_(0u)
  {}

  void initialize(unsigned int const width, unsigned int const height, unsigned int const depth);
  void deinitialize();

  /// Generate vector datas.
  /// load them from filename if it exists, otherwise compute them and save on disk.
  void generate_values(char const* filename);

  inline const glm::uvec3& dimensions() const {
    return dimensions_;
  }

  inline const glm::vec3& position() const {
    return position_;
  }

  inline GLuint texture_id() const {
    return gl_texture_id_;
  }

 private:
  glm::vec3 _generate_vector(glm::vec3 const& p) const;

  glm::uvec3 dimensions_;
  glm::vec3 position_;
  GLuint gl_texture_id_;
};

/* -------------------------------------------------------------------------- */

#endif // API_VECTOR_FIELD_H_
