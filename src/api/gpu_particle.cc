#include "api/gpu_particle.h"

#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "api/append_consume_buffer.h"
#include "shaders/sparkle/interop.h"

/* ========================================================================== */

unsigned int const GPUParticle::kThreadsGroupWidth = PARTICLES_KERNEL_GROUP_WIDTH;
unsigned int const GPUParticle::kBatchEmitCount;

/* -------------------------------------------------------------------------- */

#define _BENCHMARK(block) \
{ \
  unsigned int const kNumIter = 10000u; \
  static unsigned int iter = 0u; \
  static unsigned int result = 0u; \
  glFinish(); \
  glBeginQuery(GL_TIME_ELAPSED, query_time_); \
  { block; } \
  glEndQuery(GL_TIME_ELAPSED); \
  unsigned int dt; glGetQueryObjectuiv(query_time_, GL_QUERY_RESULT, &dt); \
  result += dt / 100; \
  if (++iter == kNumIter) {fprintf(stderr, "%8u | %32s [%u iter, %u p]\n", result / kNumIter, #block, kNumIter, kMaxParticleCount);} \
}

#define BENCHMARK(x) x

#define DBGMARK       fprintf(stderr, "%3u\t\t%s\n", __LINE__, __FUNCTION__);

#define _SHOWBUFFER()  { \
  GLint params; \
  fprintf(stderr, "%d\n", __LINE__); \
  glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &params);  \
  for (unsigned int i=0; i < COUNT_STORAGE_BINDING; ++i) { \
    glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, i, &params); \
    fprintf(stderr, "%u : %u\n", i, params); \
  } \
  fprintf(stderr, "--------------\n"); \
}

#define SHOWBUFFER()

#define SHOWUINT(x)    fprintf(stderr, "%s : %u\n", #x, x);

/* -------------------------------------------------------------------------- */

// Layout of the indirect buffer (Dispatch + Draw)
struct TIndirectValues {
  unsigned int dispatch_x;
  unsigned int dispatch_y;
  unsigned int dispatch_z;
  unsigned int draw_count;
  unsigned int draw_primCount;
  unsigned int draw_first;
  unsigned int draw_reserved;
};

/* -------------------------------------------------------------------------- */

namespace {

unsigned int GetClosestPowerOfTwo(unsigned int const n) {
  unsigned int r = 1u;
  for (unsigned int i = 0u; r < n; r <<= 1u) ++i;
  return r;
}

unsigned int GetNumTrailingBits(unsigned int const n) {
  unsigned int r = 0u;
  for (unsigned int i = n; i > 1u; i >>= 1u) ++r;
  return r;
}

}  // namespace

/* -------------------------------------------------------------------------- */

void GPUParticle::init() {
  /* Assert than the number of particles will be a factor of threadGroupWidth */
  unsigned int const num_particles = FloorParticleCount(kMaxParticleCount); //
  fprintf(stderr, "[ %u particles, %u per batch ]\n", num_particles , kBatchEmitCount);

  /* Append/Consume Buffer */
  unsigned int const num_attrib_buffer = (sizeof(TParticle) + sizeof(glm::vec4) - 1u) / sizeof(glm::vec4); //
  pbuffer_ = new AppendConsumeBuffer(num_particles, num_attrib_buffer);
  pbuffer_->initialize();

  /* Random value buffer */
  unsigned int const num_randvalues = 3u * num_particles ;
  randbuffer_.initialize(num_randvalues);

  /* VectorField generator */
  if (enable_vectorfield_) {
    vectorfield_.initialize(128u, 128u, 64u);
    vectorfield_.generate_values("velocities.dat");
  }

  /* Compute Shaders */
  char *src_buffer = new char[MAX_SHADER_BUFFERSIZE]();
  pgm_.emission     = CreateComputeProgram(SHADERS_DIR "/sparkle/cs_emission.glsl", src_buffer);
  pgm_.update_args  = CreateComputeProgram(SHADERS_DIR "/sparkle/cs_update_args.glsl", src_buffer);
  pgm_.simulation   = CreateComputeProgram(SHADERS_DIR "/sparkle/cs_simulation.glsl", src_buffer);
  pgm_.fill_indices = CreateComputeProgram(SHADERS_DIR "/sparkle/cs_fill_indices.glsl", src_buffer);
  pgm_.calculate_dp = CreateComputeProgram(SHADERS_DIR "/sparkle/cs_calculate_dp.glsl", src_buffer);
  pgm_.sort_step    = CreateComputeProgram(SHADERS_DIR "/sparkle/cs_sort_step.glsl", src_buffer);
  pgm_.sort_final   = CreateComputeProgram(SHADERS_DIR "/sparkle/cs_sort_final.glsl", src_buffer);
  pgm_.render_point_sprite = CreateRenderProgram(
    SHADERS_DIR "/sparkle/vs_generic.glsl",
    SHADERS_DIR "/sparkle/fs_point_sprite.glsl",
    src_buffer
  );
  pgm_.render_stretched_sprite = CreateRenderProgram(
    SHADERS_DIR "/sparkle/vs_generic.glsl",
    SHADERS_DIR "/sparkle/gs_stretched_sprite.glsl",
    SHADERS_DIR "/sparkle/fs_stretched_sprite.glsl",
    src_buffer
  );
  delete [] src_buffer;

  /* Get uniform locations */
  ulocation_.emission.emitCount        = GetUniformLocation(pgm_.emission, "uEmitCount");
  ulocation_.emission.emitterType      = GetUniformLocation(pgm_.emission, "uEmitterType");
  ulocation_.emission.emitterPosition  = GetUniformLocation(pgm_.emission, "uEmitterPosition");
  ulocation_.emission.emitterDirection = GetUniformLocation(pgm_.emission, "uEmitterDirection");
  ulocation_.emission.emitterRadius    = GetUniformLocation(pgm_.emission, "uEmitterRadius");
  ulocation_.emission.particleMinAge   = GetUniformLocation(pgm_.emission, "uParticleMinAge");
  ulocation_.emission.particleMaxAge   = GetUniformLocation(pgm_.emission, "uParticleMaxAge");

  ulocation_.simulation.timeStep           = GetUniformLocation(pgm_.simulation, "uTimeStep");
  ulocation_.simulation.vectorFieldSampler = GetUniformLocation(pgm_.simulation, "uVectorFieldSampler");
  ulocation_.simulation.bboxSize           = GetUniformLocation(pgm_.simulation, "uBBoxSize");
  ulocation_.simulation.boundingVolume     = GetUniformLocation(pgm_.simulation, "uBoundingVolume");
  ulocation_.simulation.scatteringFactor   = GetUniformLocation(pgm_.simulation, "uScatteringFactor");
  ulocation_.simulation.vectorFieldFactor  = GetUniformLocation(pgm_.simulation, "uVectorFieldFactor");
  ulocation_.simulation.curlNoiseFactor    = GetUniformLocation(pgm_.simulation, "uCurlNoiseFactor");
  ulocation_.simulation.curlNoiseScale     = GetUniformLocation(pgm_.simulation, "uCurlNoiseScale");
  ulocation_.simulation.velocityFactor     = GetUniformLocation(pgm_.simulation, "uVelocityFactor");
  ulocation_.simulation.enableScattering   = GetUniformLocation(pgm_.simulation, "uEnableScattering");
  ulocation_.simulation.enableVectorField  = GetUniformLocation(pgm_.simulation, "uEnableVectorField");
  ulocation_.simulation.enableCurlNoise    = GetUniformLocation(pgm_.simulation, "uEnableCurlNoise");
  ulocation_.simulation.enableVelocityControl = GetUniformLocation(pgm_.simulation, "uEnableVelocityControl");

  ulocation_.calculate_dp.view  = GetUniformLocation(pgm_.calculate_dp, "uViewMatrix");

  ulocation_.sort_step.blockWidth     = GetUniformLocation(pgm_.sort_step, "uBlockWidth");
  ulocation_.sort_step.maxBlockWidth  = GetUniformLocation(pgm_.sort_step, "uMaxBlockWidth");

  ulocation_.render_point_sprite.mvp             = GetUniformLocation(pgm_.render_point_sprite, "uMVP");
  ulocation_.render_point_sprite.minParticleSize = GetUniformLocation(pgm_.render_point_sprite, "uMinParticleSize");
  ulocation_.render_point_sprite.maxParticleSize = GetUniformLocation(pgm_.render_point_sprite, "uMaxParticleSize");
  ulocation_.render_point_sprite.colorMode       = GetUniformLocation(pgm_.render_point_sprite, "uColorMode");
  ulocation_.render_point_sprite.birthGradient   = GetUniformLocation(pgm_.render_point_sprite, "uBirthGradient");
  ulocation_.render_point_sprite.deathGradient   = GetUniformLocation(pgm_.render_point_sprite, "uDeathGradient");
  ulocation_.render_point_sprite.fadeCoefficient = GetUniformLocation(pgm_.render_point_sprite, "uFadeCoefficient");

  ulocation_.render_stretched_sprite.view            = GetUniformLocation(pgm_.render_stretched_sprite, "uView");
  ulocation_.render_stretched_sprite.mvp             = GetUniformLocation(pgm_.render_stretched_sprite, "uMVP");
  ulocation_.render_stretched_sprite.colorMode       = GetUniformLocation(pgm_.render_stretched_sprite, "uColorMode");
  ulocation_.render_stretched_sprite.birthGradient   = GetUniformLocation(pgm_.render_stretched_sprite, "uBirthGradient");
  ulocation_.render_stretched_sprite.deathGradient   = GetUniformLocation(pgm_.render_stretched_sprite, "uDeathGradient");
  ulocation_.render_stretched_sprite.spriteStretchFactor = GetUniformLocation(pgm_.render_stretched_sprite, "uSpriteStretchFactor");
  ulocation_.render_stretched_sprite.fadeCoefficient = GetUniformLocation(pgm_.render_stretched_sprite, "uFadeCoefficient");

  /* One time uniform setting */
  glProgramUniform1i(pgm_.simulation,
                     GetUniformLocation(pgm_.simulation, "uPerlinNoisePermutationSeed"),
                     rand());

  /* Dispatch and Draw Indirect buffer */
  glGenBuffers(1u, &gl_indirect_buffer_id_);
  glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, gl_indirect_buffer_id_);
  TIndirectValues const default_indirect[] = {{
    // Dispatch values
    1u, 1u, 1u,
    // Draw values
    0, 1u, 0u, 0u
  }};
  glBufferStorage(GL_DISPATCH_INDIRECT_BUFFER, sizeof default_indirect, default_indirect, 0);
  glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0u);

  /* Storage buffers */

  // The parallel nature of the sorting algorithm needs power of two sized buffer.
  unsigned int const sort_buffer_max_count = GetClosestPowerOfTwo(kMaxParticleCount); //

  // DotProducts buffer.
  glGenBuffers(1u, &gl_dp_buffer_id_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_dp_buffer_id_);
  GLuint const dp_buffer_size = sort_buffer_max_count * sizeof(GLfloat);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, dp_buffer_size, nullptr, 0);

  // Double-sized buffer for indices sorting.
  /// @note might use short instead.
  glGenBuffers(1u, &gl_sort_indices_buffer_id_);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_sort_indices_buffer_id_);
  GLuint const sort_indices_buffer_size = 2u * sort_buffer_max_count * sizeof(GLuint);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, sort_indices_buffer_size, nullptr, 0);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0u);

  /* Setup rendering buffers */
  _setup_render();

  /* Query used for benchmarking */
  glCreateQueries(GL_TIME_ELAPSED, 1, &query_time_);

  CHECKGLERROR();
}

void GPUParticle::deinit() {
  pbuffer_->deinitialize();
  delete pbuffer_;

  randbuffer_.deinitialize();

  if (enable_vectorfield_) {
    vectorfield_.deinitialize();
  }

  glDeleteProgram(pgm_.emission);
  glDeleteProgram(pgm_.update_args);
  glDeleteProgram(pgm_.simulation);
  glDeleteProgram(pgm_.fill_indices);
  glDeleteProgram(pgm_.calculate_dp);
  glDeleteProgram(pgm_.sort_step);
  glDeleteProgram(pgm_.sort_final);
  glDeleteProgram(pgm_.render_point_sprite);

  glDeleteBuffers(1u, &gl_indirect_buffer_id_);
  glDeleteBuffers(1u, &gl_dp_buffer_id_);
  glDeleteBuffers(1u, &gl_sort_indices_buffer_id_);

  glDeleteVertexArrays(1u, &vao_);
  glDeleteQueries(1, &query_time_);
}

void GPUParticle::update(const float dt, glm::mat4x4 const& view) {
  /* Max number of particles able to be spawned. */
  unsigned int const num_dead_particles = pbuffer_->element_count() - num_alive_particles_;
  /* Number of particles to be emitted. */
  unsigned int const emit_count = std::min(kBatchEmitCount, num_dead_particles); //
  /* Simulation deltatime depends on application framerate and the user input */
  float const time_step = dt * simulation_params_.time_step_factor;

  /* Update random buffer with new values */
  randbuffer_.generate_values();

  pbuffer_->bind_attributes();
  {
    pbuffer_->bind_atomics();
    randbuffer_.bind();
    {
      /* Emission stage : write in buffer A */
      _emission(emit_count);

      /* Simulation stage : read buffer A, write buffer B */
      _simulation(time_step);
    }
    randbuffer_.unbind();
    pbuffer_->unbind_atomics();

    /* Sort particles for alpha-blending. */
    if (enable_sorting_ && simulated_) {
      _sorting(view);
    }
  }
  pbuffer_->unbind_attributes();

  /* PostProcess stage */
  _postprocess();

  CHECKGLERROR();
}

void GPUParticle::render(glm::mat4x4 const& view, glm::mat4x4 const& viewProj) {
  switch(rendering_params_.rendermode) {
    case RENDERMODE_STRETCHED:
      glUseProgram(pgm_.render_stretched_sprite);
      glUniformMatrix4fv(ulocation_.render_stretched_sprite.view, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(ulocation_.render_stretched_sprite.mvp,  1, GL_FALSE, glm::value_ptr(viewProj));
      glUniform1f(ulocation_.render_stretched_sprite.colorMode, rendering_params_.colormode);
      glUniform3fv(ulocation_.render_stretched_sprite.birthGradient, 1, rendering_params_.birth_gradient);
      glUniform3fv(ulocation_.render_stretched_sprite.deathGradient, 1, rendering_params_.death_gradient);
      glUniform1f(ulocation_.render_stretched_sprite.spriteStretchFactor, rendering_params_.stretched_factor);
      glUniform1f(ulocation_.render_stretched_sprite.fadeCoefficient, rendering_params_.fading_factor);
    break;

    case RENDERMODE_POINTSPRITE:
    default:
      glUseProgram(pgm_.render_point_sprite);
      glUniformMatrix4fv(ulocation_.render_point_sprite.mvp,  1, GL_FALSE, glm::value_ptr(viewProj));
      glUniform1f(ulocation_.render_point_sprite.minParticleSize, rendering_params_.min_size);
      glUniform1f(ulocation_.render_point_sprite.maxParticleSize, rendering_params_.max_size);
      glUniform1f(ulocation_.render_point_sprite.colorMode, rendering_params_.colormode);
      glUniform3fv(ulocation_.render_point_sprite.birthGradient, 1, rendering_params_.birth_gradient);
      glUniform3fv(ulocation_.render_point_sprite.deathGradient, 1, rendering_params_.death_gradient);
      glUniform1f(ulocation_.render_point_sprite.fadeCoefficient, rendering_params_.fading_factor);
    break;
  }

  glBindVertexArray(vao_);
    void const *offset = reinterpret_cast<void const*>(offsetof(TIndirectValues, draw_count));
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, gl_indirect_buffer_id_);
    glDrawArraysIndirect(GL_POINTS, offset);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0u);
  glBindVertexArray(0u);

  glUseProgram(0u);

  CHECKGLERROR();
}

// ----------------------------------------------------------------------------

void GPUParticle::_setup_render() {
  glGenVertexArrays(1u, &vao_);
  glBindVertexArray(vao_);

  GLuint const vbo = pbuffer_->first_storage_buffer_id();

#if SPARKLE_USE_SOA_LAYOUT

  unsigned int const attrib_size = 4u * sizeof(float); // vec4
  unsigned int const attrib_buffer_size = pbuffer_->single_attrib_buffer_size();

  GLuint binding_point = 0u;
  GLuint attrib_index = 0u;

  // POSITION
  binding_point = STORAGE_BINDING_PARTICLE_POSITIONS_A;
  glBindVertexBuffer(binding_point, vbo, attrib_index*attrib_buffer_size, attrib_size);
  {
    unsigned int const num_component = 3u;
    glVertexAttribFormat(attrib_index, num_component, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(attrib_index, binding_point);
    glEnableVertexAttribArray(attrib_index);
    ++attrib_index;
  }

  // VELOCITY
  binding_point = STORAGE_BINDING_PARTICLE_VELOCITIES_A;
  glBindVertexBuffer(binding_point, vbo, attrib_index*attrib_buffer_size, attrib_size);
  {
    unsigned int const num_component = 3u;
    glVertexAttribFormat(attrib_index, num_component, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(attrib_index, binding_point);
    glEnableVertexAttribArray(attrib_index);
    ++attrib_index;
  }

  // AGE ATTRIBUTES
  binding_point = STORAGE_BINDING_PARTICLE_ATTRIBUTES_A;
  glBindVertexBuffer(binding_point, vbo, attrib_index*attrib_buffer_size, attrib_size);
  {
    unsigned int const num_component = 2u;
    glVertexAttribFormat(attrib_index, num_component, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(attrib_index, binding_point);
    glEnableVertexAttribArray(attrib_index);
    ++attrib_index;
  }

#else

  unsigned int const binding_index = 0u;
  glBindVertexBuffer(binding_index, vbo, 0u, sizeof(TParticle));
  // Particle's position
  {
    unsigned int const attrib_index = 0u;
    unsigned int const num_component = static_cast<unsigned int>((sizeof TParticle::position) / sizeof(TParticle::position[0u]));
    // Set the attribute format in Vertex Array
    glVertexAttribFormat(attrib_index, num_component, GL_FLOAT, GL_FALSE, offsetof(TParticle, position));
    // Bind attribute to a vertex buffer
    glVertexAttribBinding(attrib_index, binding_index);
    // Activate the attribute
    glEnableVertexAttribArray(attrib_index);
  }
  // velocities
  {
    unsigned int const attrib_index = 1u;
    unsigned int const num_component = static_cast<unsigned int>((sizeof TParticle::velocity) / sizeof(TParticle::velocity[0u]));
    glVertexAttribFormat(attrib_index, num_component, GL_FLOAT, GL_FALSE, offsetof(TParticle, velocity));
    glVertexAttribBinding(attrib_index, binding_index);
    glEnableVertexAttribArray(attrib_index);
  }
  // Particle's age info
  {
    unsigned int const attrib_index = 2u;
    unsigned int const num_component = 2u;
    glVertexAttribFormat(attrib_index, num_component, GL_FLOAT, GL_FALSE, offsetof(TParticle, start_age)); //
    glVertexAttribBinding(attrib_index, binding_index);
    glEnableVertexAttribArray(attrib_index);
  }

#endif

  glBindVertexArray(0u);

  CHECKGLERROR();
}

void GPUParticle::_emission(const unsigned int count) {
  /* Emit only if a minimum count is reached. */
  if (!count) {
    return;
  }
  if (count < kBatchEmitCount) {
    //return;
  }
  //fprintf(stderr, "> %7u particles to emit.\n", count);

  glUseProgram(pgm_.emission);
  {
    glUniform1ui(ulocation_.emission.emitCount, count);
    glUniform1ui(ulocation_.emission.emitterType, simulation_params_.emitter_type);
    glUniform3fv(ulocation_.emission.emitterPosition, 1, simulation_params_.emitter_position);
    glUniform3fv(ulocation_.emission.emitterDirection, 1, simulation_params_.emitter_direction);
    glUniform1f(ulocation_.emission.emitterRadius, simulation_params_.emitter_radius);
    glUniform1f(ulocation_.emission.particleMinAge, simulation_params_.min_age);
    glUniform1f(ulocation_.emission.particleMaxAge, simulation_params_.max_age);

    unsigned int const nGroups = GetThreadsGroupCount(count);
    glDispatchCompute(nGroups, 1u, 1u);
  }
  glUseProgram(0u);

  glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

  /* Number of particles expected to be simulated. */
  num_alive_particles_ += count;

  CHECKGLERROR();
}

void GPUParticle::_simulation(float const time_step) {
  if (num_alive_particles_ == 0u) {
    simulated_ = false;
    return;
  }

  /* Update Indirect arguments buffer for simulation dispatch and draw indirect. */
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDIRECT_ARGS, gl_indirect_buffer_id_);
  glUseProgram(pgm_.update_args);
  {
    glDispatchCompute(1u, 1u, 1u);
  }
  glUseProgram(0u);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDIRECT_ARGS, 0u);

  /* Synchronize the indirect argument buffer */
  glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

  /* Simulation Kernel */
  if (enable_vectorfield_) {
    glBindTexture(GL_TEXTURE_3D, vectorfield_.texture_id());
  }

  glUseProgram(pgm_.simulation);
  {
    glUniform1f(ulocation_.simulation.timeStep, time_step);
    glUniform1i(ulocation_.simulation.vectorFieldSampler, 0);
    glUniform1i(ulocation_.simulation.boundingVolume, simulation_params_.bounding_volume);
    glUniform1f(ulocation_.simulation.bboxSize, simulation_params_.bounding_volume_size);

    glUniform1f(ulocation_.simulation.scatteringFactor, simulation_params_.scattering_factor);
    glUniform1f(ulocation_.simulation.vectorFieldFactor, simulation_params_.vectorfield_factor);
    glUniform1f(ulocation_.simulation.curlNoiseFactor, simulation_params_.curlnoise_factor);
    const float inv_curlnoise_scale = 1.0f / simulation_params_.curlnoise_scale;
    glUniform1f(ulocation_.simulation.curlNoiseScale, inv_curlnoise_scale);
    glUniform1f(ulocation_.simulation.velocityFactor, simulation_params_.velocity_factor);

    glUniform1i(ulocation_.simulation.enableScattering, simulation_params_.enable_scattering);
    glUniform1i(ulocation_.simulation.enableVectorField, simulation_params_.enable_vectorfield);
    glUniform1i(ulocation_.simulation.enableCurlNoise, simulation_params_.enable_curlnoise);
    glUniform1i(ulocation_.simulation.enableVelocityControl, simulation_params_.enable_velocity_control);

    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, gl_indirect_buffer_id_);
      glDispatchComputeIndirect(0);
    glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, 0u);
  }
  glUseProgram(0u);

  glBindTexture(GL_TEXTURE_3D, 0u);

  /* Synchronize operations on buffers. */
  glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

  /* Retrieve the number of alive particle to be used in the next frame. */
  /// @note Needed if we want to emit new particles.
  num_alive_particles_ = pbuffer_->get_num_alive_particles_from_device();

  simulated_ = true;

  CHECKGLERROR();
}


void GPUParticle::_sorting(glm::mat4x4 const& view) {
  /** @note there is probably some remaining issues on kernels boundaries.*/

  /* The algorithm works on buffer sized in power of two. */
  unsigned int const max_elem_count = GetClosestPowerOfTwo(num_alive_particles_);

  /* 1) Intialize indices and dotproducts buffers. */

  // Fill first part of the indices buffer with continuous indices.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDICES_FIRST, gl_sort_indices_buffer_id_);
  glUseProgram(pgm_.fill_indices);
  {
    unsigned int const num_groups = GetThreadsGroupCount(max_elem_count); //
    glDispatchCompute(num_groups, 1u, 1u);
  }
  glUseProgram(0u);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDICES_FIRST, 0u);

  // Clear the dot product buffer.
  float const clear_value = -FLT_MAX;
  glClearNamedBufferSubData(
    gl_dp_buffer_id_, GL_R32F, 0u, max_elem_count * sizeof(GLfloat), GL_RED, GL_FLOAT, &clear_value
  );

  // Compute dot products of particles toward the camera.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_DOT_PRODUCTS, gl_dp_buffer_id_);
  glUseProgram(pgm_.calculate_dp);
  {
    /// @note No kernel boundaries check performed.
    unsigned int const num_groups = GetThreadsGroupCount(num_alive_particles_); //
    glUniformMatrix4fv(ulocation_.calculate_dp.view, 1, GL_FALSE, glm::value_ptr(view));
    glDispatchCompute(num_groups, 1u, 1u);
  }
  glUseProgram(0u);

  /* Synchronize the indices and dotproducts buffers. */
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  /* 2) Sort particle indices through their dot products. */
  // [might be able to optimise early steps with one kernel, when max_block_width <= kernel_size]
  unsigned int const num_threads = max_elem_count / 2u;
  unsigned int const num_groups = GetThreadsGroupCount(num_threads);

  unsigned int const nsteps = GetNumTrailingBits(max_elem_count);
  unsigned int const indices_half_size = max_elem_count * sizeof(GLuint);

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_DOT_PRODUCTS, gl_dp_buffer_id_);
  glUseProgram(pgm_.sort_step);
  unsigned int binding = 0u;
  for (unsigned int step = 0u; step < nsteps; ++step) {
    for (unsigned int stage = 0u; stage < step + 1u; ++stage) {
      // bind read / write indices buffers.
      unsigned int const offset_read  = indices_half_size * binding;
      unsigned int const offset_write = indices_half_size * (binding ^ 1u);
      glBindBufferRange(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDICES_FIRST,  gl_sort_indices_buffer_id_,  offset_read, indices_half_size);
      glBindBufferRange(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDICES_SECOND, gl_sort_indices_buffer_id_, offset_write, indices_half_size);
      binding ^= 1u;

      // compute kernel parameters.
      GLuint const block_width = 2u << (step - stage);
      GLuint const max_block_width = 2u << step;
      glUniform1ui(ulocation_.sort_step.blockWidth, block_width);
      glUniform1ui(ulocation_.sort_step.maxBlockWidth, max_block_width);

      glDispatchCompute(num_groups, 1u, 1u);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
  }
  glUseProgram(0u);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_DOT_PRODUCTS, 0u);

  /* 3) Sort particles datas with their sorted indices. */
  // bind the sorted indices to the first binding slot.
  GLuint const sorted_offset_read = indices_half_size * binding;
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDICES_FIRST, gl_sort_indices_buffer_id_, sorted_offset_read, indices_half_size);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDICES_SECOND, 0u);

  glUseProgram(pgm_.sort_final);
  {
    /// @note could use the DispatchIndirect buffer.
    unsigned int const num_groups = GetThreadsGroupCount(num_alive_particles_);
    glDispatchCompute(num_groups, 1u, 1u);
  }
  glUseProgram(0u);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, STORAGE_BINDING_INDICES_FIRST, 0u);

  CHECKGLERROR();
}

void GPUParticle::_postprocess() {
  if (simulated_) {
    /* Swap atomic counter to have number of alives particles in the first slot */
    pbuffer_->swap_atomics();

    /* Copy non sorted alives particles back to the first buffer. */
    if (!enable_sorting_) {
      pbuffer_->swap_storage();
    }
  }

  /* Copy the number of alive particles to the indirect buffer for drawing. */
  /// @note We use the update_args kernel instead, loosing 1 frame of accuracy.
  glCopyNamedBufferSubData(
    pbuffer_->first_atomic_buffer_id(), gl_indirect_buffer_id_, 0u, offsetof(TIndirectValues, draw_count), sizeof(GLuint)
  );
  CHECKGLERROR();
}
