#ifndef DEMO_EVENTS_H_
#define DEMO_EVENTS_H_

// ----------------------------------------------------------------------------

struct GLFWwindow;

// ----------------------------------------------------------------------------

typedef struct {
  float wheelDelta;
  float mouseX;
  float mouseY;
  bool bMouseMove;
  bool bRotatePressed;
  bool bTranslatePressed;
  bool bSpacePressed;
} TEventData;

// ----------------------------------------------------------------------------

extern void InitEvents(GLFWwindow* window);

extern void HandleEvents();

extern TEventData const GetEventData();

// ----------------------------------------------------------------------------

#endif // DEMO_EVENTS_H_
