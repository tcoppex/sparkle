#include "app.h"

#include <cstdlib>
#include <cstdio>
#include <ctime>

#include "glm/gtc/matrix_transform.hpp"
#include "glfw.h"
#include "events.h"

// ----------------------------------------------------------------------------

bool App::init(char const* title) {
  /* System parameters */
  std::setbuf(stderr, nullptr);
  std::srand(static_cast<uint32_t>(std::time(nullptr)));

  /* Initialize Window Management API */
  if (!glfwInit()) {
    fprintf(stderr, "Error: failed to initialize GLFW.\n");
    return false;
  }

  /* Initialize OpenGL context flags */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, 0);

  /* Compute window resolution from the main monitor's */
  float const scale = 4.0f / 5.0f;
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
  glfwSwapInterval(1);

  /* Initialize OpenGL extensions */
  InitGL();

  /* User Interface init */
  ui_.init(window_);

  /* Init the event manager */
  InitEvents(window_);

  /* Camera setup. */
  camera_.dolly(295.0f);

  /* Setup the projection matrix */
  float const aspectRatio = w / static_cast<float>(h);
  matrix_.proj = glm::perspective(
    glm::radians(60.0f), aspectRatio, 0.01f, 2000.0f
  );

  /* Initialize the scene. */
  scene_.init();
  ui_.set_mainview(scene_.view());

  /* Start the chrono. */
  time_ = std::chrono::steady_clock::now();

  return true;
}

void App::deinit() {
  scene_.deinit();

  ui_.deinit();

  glfwTerminate();
  window_ = nullptr;
}

void App::run() {
  /* Mainloop */
  while (!glfwWindowShouldClose(window_)) {
    /* Manage events */
    HandleEvents();

    /* Update UI events */
    ui_.update();

    /* Update and render one frame */
    _frame();

    /* Render UI */
    ui_.render();

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
  matrix_.view = glm::mat4(1.0f);
  matrix_.view = glm::lookAt(glm::vec3(0.0f, 0.65f*camera_.dolly(), camera_.dolly()),
                             glm::vec3(0.0f, 0.0f, 0.0f),
                             glm::vec3(0.0f, 1.0f, 0.0f));
  glm::translate(matrix_.view, glm::vec3(camera_.translate_x(), camera_.translate_y(), 0.0f));
  matrix_.view = glm::rotate(
    glm::rotate(matrix_.view, camera_.yaw(), glm::vec3(1.0f, 0.0f, 0.0f)),
    camera_.pitch(), glm::vec3(0.0f, 1.0f, 0.0f)
  );

  /* Update the viewproj matrix */
  matrix_.viewProj = matrix_.proj * matrix_.view;
}

void App::_update_time() {
  std::chrono::steady_clock::time_point tick = std::chrono::steady_clock::now();
  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tick - time_);
  time_ = tick;

  deltatime_ = static_cast<float>(time_span.count());
}

// ----------------------------------------------------------------------------
