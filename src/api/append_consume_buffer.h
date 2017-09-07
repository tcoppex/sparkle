#ifndef API_APPEND_CONSUME_BUFFER_H_
#define API_APPEND_CONSUME_BUFFER_H_

#include "opengl.h"

// ----------------------------------------------------------------------------

/**
 * @brief The AppendConsumeBuffer class
 */
class AppendConsumeBuffer {
public:
  AppendConsumeBuffer(unsigned int const element_count, unsigned int const attrib_buffer_count)
    : element_count_(element_count),

      attrib_buffer_count_(attrib_buffer_count),
      single_attrib_buffer_size_(element_count_ * sizeof(float) * 4u), //
      storage_buffer_size_(single_attrib_buffer_size_ * attrib_buffer_count_), //

      gl_storage_buffer_ids_{0u, 0u},
      gl_atomic_buffer_ids_{0u, 0u}
  {}

  void initialize();
  void deinitialize();

  void bind();
  void unbind();

  void bind_attributes();
  void unbind_attributes();
  void bind_atomics();
  void unbind_atomics();

  void swap_atomics();
  void swap_storage();


  unsigned int get_num_alive_particles_from_device();

  /* Getters */
  unsigned int element_count() const { return element_count_; }
  unsigned int single_attrib_buffer_size() const { return single_attrib_buffer_size_; }
  unsigned int storage_buffer_size() const { return storage_buffer_size_; }

  GLuint first_storage_buffer_id() const { return gl_storage_buffer_ids_[0u]; }   //
  GLuint second_storage_buffer_id() const { return gl_storage_buffer_ids_[1u]; }  //

  GLuint first_atomic_buffer_id() const { return gl_atomic_buffer_ids_[0u]; }   //
  GLuint second_atomic_buffer_id() const { return gl_atomic_buffer_ids_[1u]; }  //

private:
  unsigned int const element_count_;                  //< number of elements in one buffer

  unsigned int const attrib_buffer_count_;
  unsigned int const single_attrib_buffer_size_;
  unsigned int const storage_buffer_size_;            //< one buffer bytesize

  GLuint gl_storage_buffer_ids_[2u];                  //< ShaderStorage buffer (Append and Consume)
  GLuint gl_atomic_buffer_ids_[2u];                   //< AtomicCounter buffer (contains 2 atomic counter)
};

// ----------------------------------------------------------------------------

#endif  // API_APPEND_CONSUME_BUFFER_H_
