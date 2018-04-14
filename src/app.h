#ifndef SPARKLE_APP_H_
#define SPARKLE_APP_H_

// ----------------------------------------------------------------------------

#include <chrono>
#include "glm/mat4x4.hpp"

#include "arcball_camera.h"
#include "scene.h"
#include "ui/controller.h"

struct GLFWwindow;

// ----------------------------------------------------------------------------

class App {
 public:
  App() :
    window_(nullptr),
    deltatime_(0.0f)
  {}
  
  bool init(char const* title);
  void deinit();
  
  void run();
  
 private:
  void _frame();
  void _update_camera();
  void _update_time();

  std::chrono::steady_clock::time_point time_;
  GLFWwindow *window_;

  UIController ui_;
  ArcBallCamera camera_;

  struct {
    glm::mat4x4 view;
    glm::mat4x4 proj;
    glm::mat4x4 viewProj;
  } matrix_;

  Scene scene_;

  float deltatime_;
};

// ----------------------------------------------------------------------------

#endif  // SPARKLE_APP_H_
