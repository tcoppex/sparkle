#ifndef SPARKLE_UI_UI_CONTROLLER_H_
#define SPARKLE_UI_UI_CONTROLLER_H_

struct GLFWwindow;
struct ImDrawData;
class UIView;

class UIController {
 public:
  UIController() :
    window_ptr_(nullptr),
    mainview_ptr_(nullptr),
    time_(0.0)
  {}

  void init(GLFWwindow* window);
  void deinit();
  void update();
  void render();

  inline void set_mainview(UIView *view) {
    mainview_ptr_ = view;
  }

 private:
  void create_device_objects();
  void destroy_device_objects();
  void create_font_texture();
  void render_frame(ImDrawData* draw_data);

  // TODO : use smart pointers.
  GLFWwindow *window_ptr_;
  UIView *mainview_ptr_;
  double time_;

  struct TDeviceObjects {
    unsigned int fontTexture = 0;
    unsigned int shaderHandle = 0;
    unsigned int vertHandle = 0;
    unsigned int fragHandle = 0;
    int uTex = 0;
    int uProjMtx = 0;
    unsigned int aPosition = 0;
    unsigned int aUV = 0;
    unsigned int aColor = 0;
    unsigned int vboHandle = 0;
    unsigned int elementsHandle = 0;
  } device_;
};

#endif // SPARKLE_UI_UI_CONTROLLER_H_
