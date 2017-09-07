#ifndef API_RANDOM_BUFFER_H_
#define API_RANDOM_BUFFER_H_

#include <random>
#include "opengl.h"

/* -------------------------------------------------------------------------- */

/**
 * @brief Holds a buffer of random floating point values on device.
 */
class RandomBuffer
{
 public:
  RandomBuffer() :
    num_elements_(0u),
    gl_random_buffer_id_(0u),
    mt_(random_device_()),
    min_value_(0.0f),
    max_value_(1.0f)
  {}

  void initialize(unsigned int const nelements);
  void deinitialize();

  void generate_values();

  void bind();
  void unbind();

 private:
  unsigned int num_elements_;
  GLuint gl_random_buffer_id_;
  std::random_device random_device_;
  std::mt19937 mt_;
  float min_value_;
  float max_value_;
};

/* -------------------------------------------------------------------------- */

#endif // API_RANDOM_BUFFER_H_
