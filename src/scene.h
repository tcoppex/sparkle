#ifndef SPARKLE_SCENE_H_
#define SPARKLE_SCENE_H_

// ----------------------------------------------------------------------------

#include "opengl.h"
#include "glm/mat4x4.hpp"
#include "glm/vec4.hpp"
class GPUParticle;
class UIView;
namespace views {
class Main;
}

// ----------------------------------------------------------------------------

class Scene {
 public:
  struct DebugParameters_t {
    bool show_grid = true;
    bool show_simulation_volume = false;
    bool freeze = false;
  };

  Scene() :
    gpu_particle_(nullptr)
  {}

  void init();
  void deinit();

  void update(glm::mat4x4 const& view, float const dt);
  void render(glm::mat4x4 const &view, glm::mat4x4 const& viewProj);

  UIView* view() const;

 private:
  void setup_shaders();
  void setup_grid_geometry();
  void setup_wirecube_geometry();
  void setup_sphere_geometry();
  void setup_texture();
  void setup_views();

  void draw_grid(glm::mat4x4 const &mvp);
  void draw_wirecube(glm::mat4x4 const &mvp, const glm::vec4 &color);
  void draw_sphere(glm::mat4x4 const &mvp, const glm::vec4 &color);

  DebugParameters_t debug_parameters_;

  struct {
    struct {
      GLuint vao;
      GLuint vbo;
      GLsizei nvertices;
      unsigned int resolution;
    } grid;

    struct {
      GLuint vao;
      GLuint vbo;
      GLuint ibo;
      GLenum indices_type;
      GLsizei nindices;
    } wirecube;

    struct {
      GLuint vao;
      GLuint vbo;
      GLsizei nvertices;
      unsigned int resolution;
    } sphere;
  } geo_;

  struct {
    GLuint basic;
    GLuint grid;
  } pgm_;

  struct {
    struct {
      GLint mvp;
      GLint color;
    } basic;
    struct {
      GLint mvp;
      GLint scaleFactor;
    } grid;
  } ulocation_;

  GLuint gl_sprite_tex_;
  GPUParticle *gpu_particle_;

  struct {
    views::Main *main;
    UIView *simulation;
    UIView *rendering;
    UIView *debug;
  } views_;
};

// ----------------------------------------------------------------------------

#endif  // SPARKLE_SCENE_H_
