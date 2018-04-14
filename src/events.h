#ifndef SPARKLE_EVENTS_H_
#define SPARKLE_EVENTS_H_

// ----------------------------------------------------------------------------

struct GLFWwindow;

// ----------------------------------------------------------------------------

struct TEventData {
  float wheelDelta;
  float mouseX;
  float mouseY;
  bool bMouseMove;
  bool bRotatePressed;
  bool bTranslatePressed;
  bool bSpacePressed;
};

// ----------------------------------------------------------------------------

extern void InitEvents(GLFWwindow* window);

extern void HandleEvents();

extern TEventData const GetEventData();

// ----------------------------------------------------------------------------

#endif // SPARKLE_EVENTS_H_
