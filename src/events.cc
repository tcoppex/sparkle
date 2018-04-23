#include "events.h"
#include "imgui.h"
#include "glfw.h"

/* -------------------------------------------------------------------------- */

/* Event data accessed by callbacks */
static TEventData s_Global;

/* -------------------------------------------------------------------------- */

static
void keyboard_cb(GLFWwindow *window, int key, int scancode, int action, int mods);

static
void char_cb(GLFWwindow*, unsigned int c);

static
void mouse_button_cb(GLFWwindow* window, int button, int action, int mods);

static
void mouse_move_cb(GLFWwindow* window, double xpos, double ypos);

static
void scroll_cb(GLFWwindow* window, double x, double y);

/* -------------------------------------------------------------------------- */

extern
void InitEvents(GLFWwindow* window) {
  s_Global.wheelDelta = 0.0f;
  s_Global.mouseX = 0.0f;
  s_Global.mouseY = 0.0f;
  s_Global.bMouseMove = false;
  s_Global.bRotatePressed = false;
  s_Global.bTranslatePressed = false;

  glfwSetKeyCallback(window, keyboard_cb);
  glfwSetCharCallback(window, char_cb);
  glfwSetMouseButtonCallback(window, mouse_button_cb);
  glfwSetCursorPosCallback(window, mouse_move_cb);
  glfwSetScrollCallback(window, scroll_cb);
}

extern
void HandleEvents() {
  s_Global.bMouseMove = false;
  s_Global.wheelDelta = 0.0f;
  glfwPollEvents();
}

extern
TEventData const GetEventData() {
  return s_Global;
}

/* -------------------------------------------------------------------------- */


static
void keyboard_cb(GLFWwindow *window, int key, int, int action, int) {
  if (    (key == GLFW_KEY_ESCAPE)
       && (action == GLFW_PRESS)) {
      glfwSetWindowShouldClose(window, 1);
  }

  // When the UI capture keyboard, don't process it.
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureKeyboard) {
    if (action == GLFW_PRESS) {
      io.KeysDown[key] = true;
    }
    if (action == GLFW_RELEASE) {
      io.KeysDown[key] = false;
    }
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
    return;
  }

  if (    (key == GLFW_KEY_SPACE)
       && (action == GLFW_PRESS)) {
      s_Global.bSpacePressed ^= true;
  }

  s_Global.bTranslatePressed = s_Global.bTranslatePressed || ((key == GLFW_KEY_LEFT_CONTROL) && (action == GLFW_PRESS));
  s_Global.bTranslatePressed = s_Global.bTranslatePressed && ((key == GLFW_KEY_LEFT_CONTROL) && (action != GLFW_RELEASE));
}

static
void char_cb(GLFWwindow *, unsigned int c)
{
  ImGuiIO& io = ImGui::GetIO();
  if (c > 0 && c < 0x10000) {
    io.AddInputCharacter((unsigned short)c);
  }
}

static
void mouse_button_cb(GLFWwindow*, int button, int action, int) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }
  bool &bRotatePressed = s_Global.bRotatePressed;
  bRotatePressed = bRotatePressed || (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS);
  bRotatePressed = bRotatePressed && (button == GLFW_MOUSE_BUTTON_RIGHT && action != GLFW_RELEASE);

  bool &bTranslatePressed = s_Global.bTranslatePressed;
  bTranslatePressed = bTranslatePressed || (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS);
  bTranslatePressed = bTranslatePressed && (button == GLFW_MOUSE_BUTTON_MIDDLE && action != GLFW_RELEASE);
}

static
void mouse_move_cb(GLFWwindow*, double xpos, double ypos) {
  if (ImGui::GetIO().WantCaptureMouse) {
    return;
  }
  s_Global.mouseX = static_cast<float>(xpos);
  s_Global.mouseY = static_cast<float>(ypos);
  s_Global.bMouseMove = true;
}


static
void scroll_cb(GLFWwindow*, double x, double y) {
  ImGuiIO& io = ImGui::GetIO();
  if (io.WantCaptureMouse) {
    io.MouseWheelH += (float)x;
    io.MouseWheel += (float)y;
    return;
  }
  s_Global.wheelDelta = 0.5f * static_cast<float>(y);
}

// ----------------------------------------------------------------------------
