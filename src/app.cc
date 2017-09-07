#include "app.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include "GLFW/glfw3.h"

#include "events.h"

// ----------------------------------------------------------------------------

bool App::init(char const* title) {
  /* System parameters */
  std::setbuf(stderr, nullptr);
  std::srand(std::time(0));

  /* Initialize Window Management API */
  if (!glfwInit()) {
    fprintf(stderr, "Error: failed to initialize GLFW.\n");
    return false;
  }

  /* Initialize OpenGL context flags */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, 0);

  /* Compute window resolution from the main monitor's */
  float const scale = 2.0f / 3.0f;
  GLFWvidmode const* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  int const w  = static_cast<int>(scale * mode->width);
  int const h = static_cast<int>(scale * mode->height);

  /* Create the window and OpenGL context */
  window_ = glfwCreateWindow(w, h, title, nullptr, nullptr);
  if (!window_) {
    fprintf(stderr, "Error: failed to create the window.\n");
    glfwTerminate();
    return false;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window_);

  /* Init the event manager */
  InitEvents(window_);

  /* Initialize OpenGL extensions */
  InitGL();

  /* Camera setup. */
  camera_.dolly(295.0f);

  /* Setup the projection matrix */
  float const aspectRatio = w / static_cast<float>(h);
  mat4x4_perspective(matrix_.proj,
    degreesToRadians(60.0f), aspectRatio, 0.01f, 2000.0f
  );

  /* Initialize the scene. */
  scene_.init();

  /* Start the chrono. */
  time_ = std::chrono::steady_clock::now();

  return true;
}

void App::deinit() {
  glfwTerminate();

  window_ = nullptr;
  scene_.deinit();
}

void App::run() {
  /* Mainloop */
  while (!glfwWindowShouldClose(window_)) {
    /* Manage events */
    HandleEvents();

    /* Update and render one frame */
    _frame();

    /* Swap front & back buffers */
    glfwSwapBuffers(window_);
  }
}

void App::_frame() {
  /* Update chrono and calculate deltatim */
  _update_time();

  /* Camera event handling and matrices update */
  _update_camera();

  /* Update scene (eg. particles simulation) */
  scene_.update(matrix_.view, deltatime_);

  /* Render scene */
  scene_.render(matrix_.view, matrix_.viewProj);
}

void App::_update_camera() {
  TEventData const event = GetEventData();

  /* Update camera. */
  camera_.event(
    event.bMouseMove, event.bTranslatePressed, event.bRotatePressed,
    event.mouseX, event.mouseY, event.wheelDelta
  );

  /* Compute the view matrix */
  mat4x4_identity(matrix_.view);

  vec3 eye    = {0.0f, 0.65f*camera_.dolly(), camera_.dolly()};
  vec3 center = {0.0f, 0.0f, 0.0f};
  vec3 up     = {0.0f, 1.0f, 0.0f};
  mat4x4_look_at(matrix_.view, eye, center, up);
  mat4x4_translate_in_place(matrix_.view, camera_.translate_x(), camera_.translate_y(), 0.0f);
  mat4x4 rX;
  mat4x4_rotate_X(rX, matrix_.view, camera_.yaw());
  mat4x4_rotate_Y(matrix_.view, rX, camera_.pitch());

  /* Update the viewproj matrix */
  mat4x4_mul(matrix_.viewProj, matrix_.proj, matrix_.view);
}

void App::_update_time() {
  std::chrono::steady_clock::time_point tick = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tick - time_);
  time_ = tick;

  deltatime_ = (GetEventData().bSpacePressed) ? 0.0f : time_span.count();
  deltatime_ *= 1.0f;
}

// ----------------------------------------------------------------------------
