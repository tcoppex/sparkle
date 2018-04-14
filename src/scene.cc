#include "scene.h"

#include <array>
#include <vector>

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "api/gpu_particle.h"
#include "ui/views/views.h"

// ============================================================================

void Scene::init() {
  /* Init shaders */
  setup_shaders();

  /* Init Particles */
  gpu_particle_ = new GPUParticle();
  gpu_particle_->init();

  /* Init geometry */
  setup_grid_geometry();
  setup_wirecube_geometry();
  setup_sphere_geometry();

  setup_texture();

  /* Set OpenGL rendering parameters */
  glClearColor(0.155f, 0.15f, 0.13f, 1.0f);
  glEnable(GL_PROGRAM_POINT_SIZE);

  glBlendEquation(GL_FUNC_ADD);
  //glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);

  setup_views();

  CHECKGLERROR();
}

void Scene::deinit() {
  delete views_.main;
  delete views_.simulation;
  delete views_.rendering;
  delete views_.debug;

  gpu_particle_->deinit();
  delete gpu_particle_;

  glDeleteVertexArrays(1u, &geo_.grid.vao);
  glDeleteVertexArrays(1u, &geo_.wirecube.vao);
  glDeleteVertexArrays(1u, &geo_.sphere.vao);
  glDeleteBuffers(1u, &geo_.grid.vbo);
  glDeleteBuffers(1u, &geo_.wirecube.vbo);
  glDeleteBuffers(1u, &geo_.wirecube.ibo);
  glDeleteBuffers(1u, &geo_.sphere.vbo);
}

void Scene::update(glm::mat4x4 const &view, float const dt) {
  gpu_particle_->update(dt, view);
}

void Scene::render(glm::mat4x4 const &view, glm::mat4x4 const& viewProj) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // -- Grid
  if (debug_parameters_.show_grid) {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    draw_grid(viewProj);
  }

  // -- Particles
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //gpu_particle_->enable_sorting(true);
  gpu_particle_->render(view, viewProj);

  // -- Bounding and test volumes
  glm::mat4x4 mvp;
  glm::vec4 color;
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  const auto &simulation_params = gpu_particle_->simulation_parameters();

  if (debug_parameters_.show_simulation_volume) {
    switch (simulation_params.bounding_volume) {
      case GPUParticle::SPHERE: {
        const float radius = 0.5f * simulation_params.bounding_volume_size;
        glm::mat4 model =   glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 0.0f))
                          * glm::scale(glm::mat4(), glm::vec3(radius));
        mvp = viewProj * model;
        color = glm::vec4(0.5f, 0.5f, 0.5f, 0.1f);
        draw_sphere(mvp, color);
      }
      break;

      case GPUParticle::BOX:
        mvp = glm::scale(viewProj, glm::vec3(simulation_params.bounding_volume_size));
        color = glm::vec4(0.5f, 0.4f, 0.5f, 0.5f);
        draw_wirecube(mvp, color);
      break;

      default:
      break;
    }
  }

  // -- Vector field bounding box
#if 0
  glm::vec3 const& dim = gpu_particle_->vectorfield_dimensions();
  mvp = glm::scale(viewProj, glm::vec3(dim.x, dim.y, dim.z));
  color = glm::vec4(0.5f, 0.5f, 0.1f, 0.3f);
  draw_wirecube(mvp, color);
#endif
}

UIView* Scene::view() const {
  return views_.main;
}

void Scene::setup_shaders() {
  /* Setup programs */
  char *src_buffer = new char[MAX_SHADER_BUFFERSIZE]();
  pgm_.basic = CreateRenderProgram(SHADERS_DIR "/basic/vs_basic.glsl",
                                   SHADERS_DIR "/basic/fs_basic.glsl",
                                   src_buffer);
  pgm_.grid = CreateRenderProgram(SHADERS_DIR "/grid/vs_grid.glsl",
                                  SHADERS_DIR "/grid/fs_grid.glsl",
                                  src_buffer);
  delete [] src_buffer;

  /* Shaders uniform location */
  ulocation_.basic.color        = GetUniformLocation(pgm_.basic, "uColor");
  ulocation_.basic.mvp          = GetUniformLocation(pgm_.basic, "uMVP");
  ulocation_.grid.mvp           = GetUniformLocation(pgm_.grid, "uMVP");
  ulocation_.grid.scaleFactor   = GetUniformLocation(pgm_.grid, "uScaleFactor");
}

void Scene::setup_grid_geometry() {
  // size taken in world space
  float const world_size = 1.0f;

  unsigned int const &res          = 32u; //static_cast<unsigned int>(gpu_particle_->simulation_box_size()) / 2u;
  unsigned int const &num_vertices = 4u * (res + 1u);
  unsigned int const num_component = 2u;
  unsigned int const buffersize    = num_vertices * num_component;
  std::vector<float> vertices(buffersize);

  geo_.grid.resolution = res;
  geo_.grid.nvertices  = static_cast<GLsizei>(num_vertices); //

  float const cell_padding = world_size / res;
  float const offset = cell_padding * (res/2.0f);

  for (unsigned int i=0u; i<=res; ++i) {
    unsigned int const index = 4u * num_component * i;    
    float const cursor = cell_padding * i - offset;

    // horizontal lines
    vertices[index + 0u] = - offset;
    vertices[index + 1u] = cursor;
    vertices[index + 2u] = + offset;
    vertices[index + 3u] = cursor;
    // vertical lines
    vertices[index + 4u] = cursor;
    vertices[index + 5u] = - offset;
    vertices[index + 6u] = cursor;
    vertices[index + 7u] = + offset; 
  }

  // Allocate Storage.
  glGenBuffers(1u, &geo_.grid.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, geo_.grid.vbo);
    GLsizeiptr const bytesize = static_cast<GLsizeiptr>(vertices.size() * sizeof(vertices[0u]));
    glBufferStorage(GL_ARRAY_BUFFER, bytesize, vertices.data(), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0u);

  // Set attribute locations.
  glGenVertexArrays(1u, &geo_.grid.vao);
  glBindVertexArray(geo_.grid.vao);
  {
    size_t const attrib_size = num_component * sizeof(vertices[0u]);
    glBindVertexBuffer(0, geo_.grid.vbo, 0u, attrib_size);
    {
      size_t const attrib_pos = 0u;
      glVertexAttribFormat(attrib_pos, num_component, GL_FLOAT, GL_FALSE, 0);
      glVertexAttribBinding(attrib_pos, 0);
      glEnableVertexAttribArray(attrib_pos);
    }
  }
  glBindVertexArray(0u);
}

void Scene::setup_wirecube_geometry() {
  // Setup the wireframe cube

  float const world_size = 1.0f;
  float const c = 0.5f * world_size;

  std::array<float, 24> const vertices = {
    {+c, +c, +c,   +c, -c, +c,   +c, -c, -c,   +c, +c, -c,
    -c, +c, +c,   -c, -c, +c,   -c, -c, -c,   -c, +c, -c}
  };

  std::array<unsigned char, 24> const indices = {
    {0, 1, 1, 2, 2, 3, 3, 0,
    4, 5, 5, 6, 6, 7, 7, 4,
    0, 4, 1, 5, 2, 6, 3, 7}
  };

  // Vertices storage.
  GLsizeiptr bytesize(0);
  glGenBuffers(1u, &geo_.wirecube.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, geo_.wirecube.vbo);
    bytesize = vertices.size() * sizeof(vertices[0u]);
    glBufferStorage(GL_ARRAY_BUFFER, bytesize, vertices.data(), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0u);

  // Indices storage.
  glGenBuffers(1u, &geo_.wirecube.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo_.wirecube.ibo);
    bytesize = indices.size() * sizeof(indices[0u]);
    glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, bytesize, indices.data(), 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);

  // Rendering attributes.
  glGenVertexArrays(1u, &geo_.wirecube.vao);
  glBindVertexArray(geo_.wirecube.vao);
  {
    // Positions
    unsigned int const attrib_pos = 0u;
    glBindVertexBuffer(attrib_pos, geo_.wirecube.vbo, 0, 3u*sizeof(vertices[0u]));
    glVertexAttribFormat(attrib_pos, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexAttribBinding(attrib_pos, 0);
    glEnableVertexAttribArray(attrib_pos);

    // Enable element array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo_.wirecube.ibo);
  }
  glBindVertexArray(0u);

  // Element array parameters.
  geo_.wirecube.nindices = static_cast<unsigned int>(indices.size());
  geo_.wirecube.indices_type = GL_UNSIGNED_BYTE;

  CHECKGLERROR();
}

void Scene::setup_sphere_geometry() {
  float const world_size = 2.0f; //
  float const radius = 0.5f * world_size;

  unsigned int const res = 32u;
  unsigned int const num_vertices = 2u * res * (res + 2u);

  geo_.sphere.resolution = res;
  geo_.sphere.nvertices = static_cast<GLsizei>(num_vertices); //

  float theta2, phi;    // next theta angle, phi angle
  float ct, st;         // cos(theta), sin(theta)
  float ct2, st2;       // cos(next theta), sin(next theta)
  float cp, sp;         // cos(phi), sin(phi)

  float const Pi    = static_cast<float>(M_PI);
  float const TwoPi = 2.0f * Pi;
  float const Delta = 1.0f / static_cast<float>(geo_.sphere.resolution);

  // Trigonometry base value, base of the spirale sphere.
  ct2 = 0.0f; st2 = -1.0f;

  // Vertices data.
  unsigned int const num_component = 3u;
  std::vector<float> vertices(num_component * num_vertices);

  /* Create a sphere from bottom to top (like a spiral) as a tristrip */
  unsigned int id = 0u;
  for (unsigned int j = 0u; j < geo_.sphere.resolution; ++j) {
    ct = ct2;
    st = st2;

    theta2 = ((j+1u) * Delta - 0.5f) * Pi;
    ct2 = glm::cos(theta2);
    st2 = glm::sin(theta2);

    vertices[id++] = radius * (ct);
    vertices[id++] = radius * (st);
    vertices[id++] = 0.0f;

    for (unsigned int i = 0u; i < geo_.sphere.resolution + 1u; ++i) {
      phi = TwoPi * i * Delta;
      cp = glm::cos(phi);
      sp = glm::sin(phi);

      vertices[id++] = radius * (ct2 * cp);
      vertices[id++] = radius * (st2);
      vertices[id++] = radius * (ct2 * sp);

      vertices[id++] = radius * (ct * cp);
      vertices[id++] = radius * (st);
      vertices[id++] = radius * (ct * sp);
    }
    vertices[id++] = radius * (ct2);
    vertices[id++] = radius * (st2);
    vertices[id++] = 0.0f;
  }

  // Allocate Storage.
  glGenBuffers(1u, &geo_.sphere.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, geo_.sphere.vbo);
    GLsizeiptr const bytesize = static_cast<GLsizeiptr>(vertices.size() * sizeof(vertices[0u]));
    glBufferStorage(GL_ARRAY_BUFFER, bytesize, vertices.data(), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0u);

  // Set attribute locations.
  glGenVertexArrays(1u, &geo_.sphere.vao);
  glBindVertexArray(geo_.sphere.vao);
  {
    size_t const attrib_size = num_component * sizeof(vertices[0u]);
    glBindVertexBuffer(0, geo_.sphere.vbo, 0u, attrib_size);
    {
      unsigned int const attrib_pos = 0u;
      glVertexAttribFormat(attrib_pos, num_component, GL_FLOAT, GL_FALSE, 0);
      glVertexAttribBinding(attrib_pos, 0);
      glEnableVertexAttribArray(attrib_pos);
    }
  }
  glBindVertexArray(0u);
}

void Scene::setup_texture() {
#if 0
  unsigned int const res = sprite_width*sprite_height;
  char *pixels = new char[3u*res];
  char *texdata = new char[res];

  for (unsigned int i=0u; i < res; ++i) {
    char *px = pixels + 3*i;
    HEADER_PIXEL(sprite_data, px);
    texdata[i] = *px;
  }
  delete [] pixels;

  GLsizei const w = static_cast<GLsizei>(sprite_width);
  GLsizei const h = static_cast<GLsizei>(sprite_height);
  glGenTextures(1u, &gl_sprite_tex_);
  glBindTexture(GL_TEXTURE_2D, gl_sprite_tex_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexStorage2D(GL_TEXTURE_2D, 4u, GL_R8, w, h);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, texdata);
    glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0u);

  delete [] texdata;

  // bind here just for testing.
  glBindTexture(GL_TEXTURE_2D, gl_sprite_tex_);

  CHECKGLERROR();
#endif
}

void Scene::setup_views() {
  views_.main = new views::Main();
  views_.simulation = new views::Simulation(gpu_particle_->simulation_parameters());
  views_.rendering = new views::Rendering(gpu_particle_->rendering_parameters());
  views_.debug = new views::Debug(debug_parameters_);

  views_.main->push_view(views_.simulation);
  views_.main->push_view(views_.rendering);
  views_.main->push_view(views_.debug);
}

void Scene::draw_grid(glm::mat4x4 const &mvp) {
  const auto& simulation_params = gpu_particle_->simulation_parameters();

  glUseProgram(pgm_.grid);
  {
    glUniformMatrix4fv(ulocation_.grid.mvp, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform1f(ulocation_.grid.scaleFactor, simulation_params.bounding_volume_size);

    glBindVertexArray(geo_.grid.vao);
      glDrawArrays(GL_LINES, 0, geo_.grid.nvertices);
    glBindVertexArray(0u);
  }
  glUseProgram(0u);

  CHECKGLERROR();
}

void Scene::draw_wirecube(glm::mat4x4 const &mvp, glm::vec4 const &color) {
  glUseProgram(pgm_.basic);
  {
    glUniformMatrix4fv(ulocation_.basic.mvp, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform4fv(ulocation_.basic.color, 1u, glm::value_ptr(color));

    glBindVertexArray(geo_.wirecube.vao);
      glDrawElements(GL_LINES, geo_.wirecube.nindices, geo_.wirecube.indices_type, nullptr);
    glBindVertexArray(0u);
  }
  glUseProgram(0u);

  CHECKGLERROR();
}

void Scene::draw_sphere(glm::mat4x4 const &mvp, glm::vec4 const &color) {
  glUseProgram(pgm_.basic);
  {
    glUniformMatrix4fv(ulocation_.basic.mvp, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform4fv(ulocation_.basic.color, 1u, glm::value_ptr(color));

    glBindVertexArray(geo_.sphere.vao);
      glDrawArrays(GL_LINES, 0, geo_.sphere.nvertices);
    glBindVertexArray(0u);
  }
  glUseProgram(0u);

  CHECKGLERROR();
}

// ============================================================================
