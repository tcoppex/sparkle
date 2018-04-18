#ifndef DEMO_GLFW_H_
#define DEMO_GLFW_H_

/* Wrapper around glfw to always load OpenGL extensions before it. */

// Use GLEW if enabled, otherwise load extensions manually.
#if USE_GLEW
#include "GL/glew.h"
#else
#include "ext/_extensions.h"
#endif

#include "GLFW/glfw3.h"

#endif  // DEMO_GLFW_H_