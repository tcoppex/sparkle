#ifndef DEMO_SCENE_H_
#define DEMO_SCENE_H_

// ----------------------------------------------------------------------------

#include "opengl.h"
#include "linmath.h"

class GPUParticle;

// ----------------------------------------------------------------------------

class Scene {
 public:
  Scene() :
    gpu_particle_(nullptr)
  {}

  void init();
  void deinit();

  void update(mat4x4 const& view, float const dt);
  void render(mat4x4 const &view, mat4x4 const& viewProj);

 private:
  void setup_shaders();
  void setup_grid_geometry();
  void setup_wirecube_geometry();
  void setup_sphere_geometry();
  void setup_texture();


  void draw_grid(mat4x4 const &mvp);
  void draw_wirecube(mat4x4 const &mvp, const vec4 &color);
  void draw_sphere(mat4x4 const &mvp, const vec4 &color);

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
};

// ----------------------------------------------------------------------------

#endif  // DEMO_SCENE_H_
