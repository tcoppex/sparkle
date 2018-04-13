#include "ui.h"

#include <cstdlib>
#include "imgui.h"
#include "GLFW/glfw3.h"
#include "opengl.h"


void UI::init(GLFWwindow* window) {
  window_ptr_ = window;

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();

  io.Fonts->AddFontDefault();

  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

  // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
  io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
  io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
  io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
  io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
  io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
  io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
  io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
  io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
  io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
  io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
  io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

  //io.SetClipboardTextFn = cb_SetClipboardText;
  //io.GetClipboardTextFn = cb_GetClipboardText;
  io.ClipboardUserData = window_ptr_;

#ifdef _WIN32
  //io.ImeWindowHandle = glfwGetWin32Window(window_ptr_);
#endif

  setup_callbacks();

  // Setup style.
  ImGui::StyleColorsDark();
}

void UI::deinit() {
  ImGui::DestroyContext();
}

void UI::update() {
  if (!device_.fontTexture) {
    create_device_objects();
  }

  ImGuiIO& io = ImGui::GetIO();

  // Setup display size (every frame to accommodate for window resizing)
  int w, h;
  int display_w, display_h;
  glfwGetWindowSize(window_ptr_, &w, &h);
  glfwGetFramebufferSize(window_ptr_, &display_w, &display_h);
  io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
  io.DisplayFramebufferScale = ImVec2((w > 0) ? (static_cast<float>(display_w) / w) : 0.0f,
                                      (h > 0) ? (static_cast<float>(display_h) / h) : 0.0f);

  // Setup time step
  const double current_time = glfwGetTime();
  io.DeltaTime = static_cast<float>((time_ > 0.0) ? (current_time - time_) : (1.0/60.0));
  time_ = current_time;

  // Setup inputs
  // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
  if (glfwGetWindowAttrib(window_ptr_, GLFW_FOCUSED)) {
    // Set OS mouse position if requested (only used when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
    if (io.WantSetMousePos) {
      glfwSetCursorPos(window_ptr_, (double)io.MousePos.x, (double)io.MousePos.y);
    } else {
      double mouse_x, mouse_y;
      glfwGetCursorPos(window_ptr_, &mouse_x, &mouse_y);
      io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    }
  } else {
    io.MousePos = ImVec2(-FLT_MAX,-FLT_MAX);
  }

  for (int i = 0; i < 3; i++) {
      // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
      io.MouseDown[i] = /*g_MouseJustPressed[i] ||*/ glfwGetMouseButton(window_ptr_, i) != 0;
      //g_MouseJustPressed[i] = false;
  }

  // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
  if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0 && glfwGetInputMode(window_ptr_, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
      ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
      if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None) {
        glfwSetInputMode(window_ptr_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
      } else {
        //glfwSetCursor(window_ptr_, g_MouseCursors[cursor] ? g_MouseCursors[cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(window_ptr_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      }
  }

  // Gamepad navigation mapping [BETA]
  memset(io.NavInputs, 0, sizeof(io.NavInputs));
  if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) {
      // Update gamepad inputs
      #define MAP_BUTTON(NAV_NO, BUTTON_NO)       { if (buttons_count > BUTTON_NO && buttons[BUTTON_NO] == GLFW_PRESS) io.NavInputs[NAV_NO] = 1.0f; }
      #define MAP_ANALOG(NAV_NO, AXIS_NO, V0, V1) { float v = (axes_count > AXIS_NO) ? axes[AXIS_NO] : V0; v = (v - V0) / (V1 - V0); if (v > 1.0f) v = 1.0f; if (io.NavInputs[NAV_NO] < v) io.NavInputs[NAV_NO] = v; }
      int axes_count = 0, buttons_count = 0;
      const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axes_count);
      const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttons_count);
      MAP_BUTTON(ImGuiNavInput_Activate,   0);     // Cross / A
      MAP_BUTTON(ImGuiNavInput_Cancel,     1);     // Circle / B
      MAP_BUTTON(ImGuiNavInput_Menu,       2);     // Square / X
      MAP_BUTTON(ImGuiNavInput_Input,      3);     // Triangle / Y
      MAP_BUTTON(ImGuiNavInput_DpadLeft,   13);    // D-Pad Left
      MAP_BUTTON(ImGuiNavInput_DpadRight,  11);    // D-Pad Right
      MAP_BUTTON(ImGuiNavInput_DpadUp,     10);    // D-Pad Up
      MAP_BUTTON(ImGuiNavInput_DpadDown,   12);    // D-Pad Down
      MAP_BUTTON(ImGuiNavInput_FocusPrev,  4);     // L1 / LB
      MAP_BUTTON(ImGuiNavInput_FocusNext,  5);     // R1 / RB
      MAP_BUTTON(ImGuiNavInput_TweakSlow,  4);     // L1 / LB
      MAP_BUTTON(ImGuiNavInput_TweakFast,  5);     // R1 / RB
      MAP_ANALOG(ImGuiNavInput_LStickLeft, 0,  -0.3f,  -0.9f);
      MAP_ANALOG(ImGuiNavInput_LStickRight,0,  +0.3f,  +0.9f);
      MAP_ANALOG(ImGuiNavInput_LStickUp,   1,  +0.3f,  +0.9f);
      MAP_ANALOG(ImGuiNavInput_LStickDown, 1,  -0.3f,  -0.9f);
      #undef MAP_BUTTON
      #undef MAP_ANALOG
      if (axes_count > 0 && buttons_count > 0)
        io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
      else
        io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
  }

  ImGui::NewFrame();
}

void UI::render() {
  render_views();
  ImGui::Render();
  render_frame(ImGui::GetDrawData());
}

void UI::render_views() {
  // 1. Show a simple window.
  {
    static float f = 0.0f;
    static int counter = 0;
    ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    //ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    ImGui::Checkbox("Another Window", &data_.show_another_window);

    if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    double ms = 1000.0f / ImGui::GetIO().Framerate;
    double framerate = ImGui::GetIO().Framerate;
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ms, framerate);
  }

  // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
  if (data_.show_another_window) {
    ImGui::Begin("Another Window", &data_.show_another_window);
      ImGui::Text("Hello from another window!");
      data_.show_another_window = !ImGui::Button("Close Me");
    ImGui::End();
  }
}

void UI::render_frame(ImDrawData* draw_data) {
  ImGuiIO& io = ImGui::GetIO();

  // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
  int fb_width = static_cast<int>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fb_height = static_cast<int>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if (fb_width == 0 || fb_height == 0) {
    return;
  }
  draw_data->ScaleClipRects(io.DisplayFramebufferScale);

  // Backup GL state
  GLenum last_active_texture =  GetUInteger(GL_ACTIVE_TEXTURE);
  glActiveTexture(GL_TEXTURE0);
  GLuint last_program = GetUInteger(GL_CURRENT_PROGRAM);
  GLuint last_texture = GetUInteger(GL_TEXTURE_BINDING_2D);
  GLuint last_sampler = GetUInteger(GL_SAMPLER_BINDING);
  GLuint last_array_buffer = GetUInteger(GL_ARRAY_BUFFER_BINDING);
  GLuint last_element_array_buffer = GetUInteger(GL_ELEMENT_ARRAY_BUFFER_BINDING);
  GLuint last_vertex_array = GetUInteger(GL_VERTEX_ARRAY_BINDING);
  GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
  GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
  GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
  GLenum last_blend_src_rgb = GetUInteger(GL_BLEND_SRC_RGB);
  GLenum last_blend_dst_rgb = GetUInteger(GL_BLEND_DST_RGB);
  GLenum last_blend_src_alpha = GetUInteger(GL_BLEND_SRC_ALPHA);
  GLenum last_blend_dst_alpha = GetUInteger(GL_BLEND_DST_ALPHA);
  GLenum last_blend_equation_rgb = GetUInteger(GL_BLEND_EQUATION_RGB);
  GLenum last_blend_equation_alpha = GetUInteger(GL_BLEND_EQUATION_ALPHA);
  GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
  GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
  GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
  GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

  // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Setup viewport, orthographic projection matrix
  glViewport(0, 0, fb_width, fb_height);
  const float ortho_projection[4][4] =
  {
    { 2.0f/io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
    { 0.0f,                  2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
    { 0.0f,                  0.0f,                  -1.0f, 0.0f },
    {-1.0f,                  1.0f,                   0.0f, 1.0f },
  };
  glUseProgram(device_.shaderHandle);
  glUniform1i(device_.uTex, 0);
  glUniformMatrix4fv(device_.uProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
  glBindSampler(0, 0); // Rely on combined texture/sampler state.

  // Recreate the VAO every time
  // (This is to easily allow multiple GL contexts. VAO are not shared among GL contexts, and we don't track creation/deletion of windows so we don't have an obvious key to use to cache them.)
  GLuint vao_handle = 0;
  glGenVertexArrays(1, &vao_handle);
  glBindVertexArray(vao_handle);
  glBindBuffer(GL_ARRAY_BUFFER, device_.vboHandle);
  glEnableVertexAttribArray(device_.aPosition);
  glEnableVertexAttribArray(device_.aUV);
  glEnableVertexAttribArray(device_.aColor);
  glVertexAttribPointer(device_.aPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(offsetof(ImDrawVert, pos)));
  glVertexAttribPointer(device_.aUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(offsetof(ImDrawVert, uv)));
  glVertexAttribPointer(device_.aColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(offsetof(ImDrawVert, col)));

  // Draw
  for (int n = 0; n < draw_data->CmdListsCount; n++)
  {
    const ImDrawList* cmd_list = draw_data->CmdLists[n];
    const ImDrawIdx* idx_buffer_offset = nullptr;

    glBindBuffer(GL_ARRAY_BUFFER, device_.vboHandle);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                 (const GLvoid*)cmd_list->VtxBuffer.Data,
                 GL_STREAM_DRAW
    );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, device_.elementsHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx)),
                 (const GLvoid*)cmd_list->IdxBuffer.Data,
                 GL_STREAM_DRAW
    );
    for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
    {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmd_list, pcmd);
      }
      else
      {
        GLuint texid = static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId));
        glBindTexture(GL_TEXTURE_2D, texid);
        glScissor(static_cast<GLint>(pcmd->ClipRect.x),
                  static_cast<GLint>(fb_height - pcmd->ClipRect.w),
                  static_cast<GLint>(pcmd->ClipRect.z - pcmd->ClipRect.x),
                  static_cast<GLint>(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
      }
      idx_buffer_offset += pcmd->ElemCount;
    }
  }
  glDeleteVertexArrays(1, &vao_handle);

  // Restore modified GL state
  glUseProgram(last_program);
  glBindTexture(GL_TEXTURE_2D, last_texture);
  glBindSampler(0, last_sampler);
  glActiveTexture(last_active_texture);
  glBindVertexArray(last_vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
  glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
  glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
  if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
  if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
  if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
  if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
  glViewport(last_viewport[0], last_viewport[1], last_viewport[2], last_viewport[3]);
  glScissor(last_scissor_box[0], last_scissor_box[1], last_scissor_box[2], last_scissor_box[3]);
}

void UI::setup_callbacks() {
  // Conflict with events.cc own callbacks.
}

/// --------------------------------------------------------------------------
///
/// What follow is basically ImGui examples reworked.
/// [ TODO : integrate more intuitively with the app ]
///
/// --------------------------------------------------------------------------

void UI::create_device_objects() {
  // Backup GL state
  GLint last_texture, last_array_buffer, last_vertex_array;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

  const GLchar* vertex_shader =
    "uniform mat4 ProjMtx;\n"
    "in vec2 Position;\n"
    "in vec2 UV;\n"
    "in vec4 Color;\n"
    "out vec2 Frag_UV;\n"
    "out vec4 Frag_Color;\n"
    "void main()\n"
    "{\n"
    "	Frag_UV = UV;\n"
    "	Frag_Color = Color;\n"
    "	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
      "}\n";

  const GLchar* fragment_shader =
    "uniform sampler2D Texture;\n"
    "in vec2 Frag_UV;\n"
    "in vec4 Frag_Color;\n"
    "out vec4 Out_Color;\n"
    "void main()\n"
    "{\n"
    "	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
    "}\n";

  const char glslVersion[32] = "#version 150\n";
  const GLchar* vertex_shader_with_version[2] = { glslVersion, vertex_shader };
  const GLchar* fragment_shader_with_version[2] = { glslVersion, fragment_shader };

  device_.shaderHandle = glCreateProgram();
  device_.vertHandle = glCreateShader(GL_VERTEX_SHADER);
  device_.fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(device_.vertHandle, 2, vertex_shader_with_version, nullptr);
  glShaderSource(device_.fragHandle, 2, fragment_shader_with_version, nullptr);
  glCompileShader(device_.vertHandle);
  glCompileShader(device_.fragHandle);
  glAttachShader(device_.shaderHandle, device_.vertHandle);
  glAttachShader(device_.shaderHandle, device_.fragHandle);
  glLinkProgram(device_.shaderHandle);

  if (!CheckProgramStatus(device_.shaderHandle, "ImGui font rendering shader.")) {
    exit(EXIT_FAILURE);
  }

  device_.uTex = GetUniformLocation(device_.shaderHandle, "Texture");
  device_.uProjMtx = GetUniformLocation(device_.shaderHandle, "ProjMtx");
  device_.aPosition = GetAttribLocation(device_.shaderHandle, "Position");
  device_.aUV = GetAttribLocation(device_.shaderHandle, "UV");
  device_.aColor = GetAttribLocation(device_.shaderHandle, "Color");

  glGenBuffers(1, &device_.vboHandle);
  glGenBuffers(1, &device_.elementsHandle);

  create_font_texture();

  // Restore modified GL state
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(last_texture));
  glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(last_array_buffer));
  glBindVertexArray(static_cast<GLuint>(last_vertex_array));

  CHECKGLERROR();
}

void UI::create_font_texture() {
  // Build texture atlas
  ImGuiIO& io = ImGui::GetIO();
  unsigned char* pixels;
  int width, height;

  // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small)
  // because it is more likely to be compatible with user's existing shaders.
  // If your ImTextureId represent a higher-level concept than just a GL texture id,
  // consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  // Upload texture to graphics system
  GLint last_texture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
  glGenTextures(1, &device_.fontTexture);
  glBindTexture(GL_TEXTURE_2D, device_.fontTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t)device_.fontTexture;

  // Restore state
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(last_texture));
}
