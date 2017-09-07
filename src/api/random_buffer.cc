#include "api/random_buffer.h"

#include "linmath.h" //
#include "shaders/sparkle/interop.h"

/* -------------------------------------------------------------------------- */

void RandomBuffer::initialize(unsigned int const nelements) {
  num_elements_ = nelements;

  glGenBuffers(1u, &gl_random_buffer_id_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_random_buffer_id_);

  glBufferStorage(GL_SHADER_STORAGE_BUFFER, num_elements_ * sizeof(float), nullptr,
                  GL_MAP_WRITE_BIT
  );

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);

  CHECKGLERROR();
}

void RandomBuffer::deinitialize() {
  glDeleteBuffers(1u, &gl_random_buffer_id_);
}

void RandomBuffer::generate_values() {
  std::uniform_real_distribution<float> distrib(min_value_, max_value_);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_random_buffer_id_);

  float *buffer = (float*)glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0u, num_elements_ * sizeof(float),
    GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT
  );

  for (unsigned int i=0u; i<num_elements_; ++i) {
    buffer[i] = distrib(mt_);
  }

  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);

  CHECKGLERROR();
}

void RandomBuffer::bind() {
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_RANDOM_VALUES, gl_random_buffer_id_);
}

void RandomBuffer::unbind() {
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_RANDOM_VALUES, 0u);
}
