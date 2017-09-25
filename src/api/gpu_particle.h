#ifndef API_GPU_PARTICLE_H
#define API_GPU_PARTICLE_H

/* -------------------------------------------------------------------------- */

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include "opengl.h"
#include "api/random_buffer.h"
#include "api/vector_field.h"

class AppendConsumeBuffer;

/* -------------------------------------------------------------------------- */

/**
 * @brief The GPUParticle class
 *
 * @note As is, the system forces the number of simulated particles to be factor
 * of kThreadsGroupWidth (to avoid condition checking at boundaries).
 * Condition checking is just at emission stage.
 */
class GPUParticle
{
public:
  GPUParticle() :
    num_alive_particles_(0u),
    pbuffer_(nullptr),
    gl_indirect_buffer_id_(0u),
    gl_dp_buffer_id_(0u),
    gl_sort_indices_buffer_id_(0u),
    vao_(0u),
    query_time_(0u),
    simulation_box_size_(kDefaultSimulationBoxSize),
    simulated_(false),
    enable_sorting_(false),
    enable_vectorfield_(true)
  {}

  void init();
  void deinit();

  void update(float const dt, glm::mat4x4 const& view);
  void render(glm::mat4x4 const& view, glm::mat4x4 const& viewProj);

  inline const glm::uvec3& vectorfield_dimensions() const {
    return vectorfield_.dimensions();
  }

  inline float simulation_box_size() const { return simulation_box_size_; }
  inline void simulation_box_size(float size) { simulation_box_size_ = size; }

  inline void enable_sorting(bool status) { enable_sorting_ = status; }
  inline void enable_vectorfield(bool status) { enable_vectorfield_ = status; }

private:
  // [STATIC]
  static unsigned int const kThreadsGroupWidth;

  // [USER DEFINED]
  static unsigned int const kMaxParticleCount   = (1u << 19u);
  static unsigned int const kBatchEmitCount     = 256;//(kMaxParticleCount >> 5u);
  static float constexpr kDefaultSimulationBoxSize = 256.0f;

  //------------------------------

  static
  unsigned int GetThreadsGroupCount(unsigned int const nthreads) {
      return (nthreads + kThreadsGroupWidth-1u) / kThreadsGroupWidth;
  }

  static
  unsigned int FloorParticleCount(unsigned int const nparticles) {
    return kThreadsGroupWidth * (nparticles / kThreadsGroupWidth);
  }

  void _setup_render();

  void _emission(unsigned int const count);
  void _simulation(float const dt);
  void _postprocess();
  void _sorting(glm::mat4x4 const& view);

  //------------------------------

  struct {
    float max_age = 1.0f;
  } params_;

  unsigned int num_alive_particles_;              //< number of particle written and rendered on last frame.
  AppendConsumeBuffer *pbuffer_;                  //< Append / Consume buffer for particles.

  RandomBuffer randbuffer_;                       //< StorageBuffer to hold random values.
  VectorField vectorfield_;                       //< Vector field handler.

  struct {
    GLuint emission;
    GLuint update_args;
    GLuint simulation;
    GLuint fill_indices;
    GLuint calculate_dp;
    GLuint sort_step;
    GLuint sort_final;
    GLuint render_point_sprite;
    GLuint render_stretched_sprite;
  } pgm_;                                         //< Pipeline's shaders.

  struct {
    struct {
      GLint emitCount;
      GLint emitterPosition;
      GLint emitterDirection;
      GLint particleMaxAge;
    } emission;
    struct {
      GLint deltaT;
      GLint vectorFieldSampler;
      GLint bboxSize;
    } simulation;
    struct {
      GLint view;
    } calculate_dp;
    struct {
      GLint blockWidth;
      GLint maxBlockWidth;
    } sort_step;
    struct {
      GLint mvp;
    } render_point_sprite;
    struct {
      GLint view;
      GLint mvp;
      GLint spriteSizeRatio;
    } render_stretched_sprite;
  } ulocation_;                                   //< Programs uniform location.

  ///
  /// @todo instead, use one array in GPUParticle that holds
  /// all storage ids and share them between subclasses.
  ///
  GLuint gl_indirect_buffer_id_;                  //< Indirect Dispatch / Draw buffer.
  GLuint gl_dp_buffer_id_;                        //< DotProduct buffer.
  GLuint gl_sort_indices_buffer_id_;              //< indices buffer (for sorting).

  GLuint vao_;                                    //< VAO for rendering.
  GLuint query_time_;                             //< QueryObject for benchmarking.

  float simulation_box_size_;                     //< Boundary used by the simulation, if any.

  bool simulated_;

  bool enable_sorting_;                           //< True if back-to-front sort is enabled.
  bool enable_vectorfield_;                       //< True if the vector field is used.
};

/* -------------------------------------------------------------------------- */

#endif // API_GPU_PARTICLE_H
