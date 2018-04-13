#ifndef DEMO_UI_H_
#define DEMO_UI_H_

struct GLFWwindow;
struct ImDrawData;

class UI {
 public:
  UI() :
    window_ptr_(nullptr),
    time_(0.0)
  {}

  void init(GLFWwindow* window);
  void deinit();

  void update();
  void render();

 private:
  void setup_callbacks();
  void create_device_objects();
  void create_font_texture();

  void render_views();
  void render_frame(ImDrawData* draw_data);

  GLFWwindow *window_ptr_;
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

  struct TData {
    bool show_another_window = false;
  } data_;
};

#endif // DEMO_UI_H_
