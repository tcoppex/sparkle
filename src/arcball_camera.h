#ifndef ARCBALL_CAMERA_H_
#define ARCBALL_CAMERA_H_

// ----------------------------------------------------------------------------

#include <cmath>

// ----------------------------------------------------------------------------

class ArcBallCamera {
 public:
  ArcBallCamera()
    : last_mouse_x_(0.0f),
      last_mouse_y_(0.0f),
      pitch_(0.0f),
      yaw_(0.0f),
      dolly_(0.0f),
      tx_(0.0f),
      ty_(0.0f)
  {}

  inline
  void event(bool const bMoving,
             bool const btnTranslate,
             bool const btnRotate,
             float const mouseX,
             float const mouseY,
             float const wheelDelta) {
    /** @warning minor teleporting bug when translating / rotating might appears. */
    if (bMoving) {
      eventMouseMoved(btnTranslate, btnRotate, mouseX, mouseY);
    } else {
      eventMousePressed(btnTranslate, btnRotate, mouseX, mouseY);
    }

    eventWheel(wheelDelta);
  }

  inline
  void eventMousePressed(bool const btnTranslate,
                         bool const btnRotate,
                         float const mouseX,
                         float const mouseY) {
    if (btnTranslate || btnRotate) {
      last_mouse_x_ = mouseX;
      last_mouse_y_ = mouseY;
    }
  }

  inline
  void eventMouseMoved(bool const btnTranslate,
                       bool const btnRotate,
                       float const mouseX,
                       float const mouseY) {
    float const dv_x = mouseX - last_mouse_x_;
    float const dv_y = mouseY - last_mouse_y_;

    if (btnRotate) {
      pitch_ += dv_x * kMouseRAcceleration;
      yaw_   += dv_y * kMouseRAcceleration;
      pitch_ = fmodf(pitch_, kAngleModulo);
      yaw_   = fmodf(yaw_, kAngleModulo);
    }

    if (btnTranslate) {
      const float acc = kMouseTAcceleration * dolly_;
      tx_ += dv_x * acc;
      ty_ -= dv_y * acc;
    }

    eventMousePressed(btnTranslate, btnRotate, mouseX, mouseY);
  }

  inline
  void eventWheel(float const delta) {
    float const sgn = (delta != 0.0f) ? ((delta > 0.0f) ? -1.0f : 1.0f) : 0.0f;
    dolly_ = (1.0f + sgn * kMouseWAcceleration) * dolly_;
  }

  inline float pitch() const { return pitch_; }
  inline float yaw() const { return yaw_; }
  inline float dolly() const { return dolly_; }
  inline float translate_x() const { return tx_; }
  inline float translate_y() const { return ty_; }

  inline void pitch(float const value) { pitch_ = value; }
  inline void yaw(float const value) { yaw_ = value; }
  inline void dolly(float const value) { dolly_ = value; }
  inline void translate_x(float const value) { tx_ = value; }
  inline void translate_y(float const value) { ty_ = value; }

#if 0
  void get_view_matrix(mat4 &view) const {
      view.setToIdentity();
      view.lookAt(vec3(0.0f, 0.0f, dolly_),
                  vec3(0.0f, 0.0f, 0.0f),
                  vec3(0.0f, 1.0f, 0.0f));
      view.translate(tx_, ty_, 0.0f);
      view.rotate(camera_.yaw,    1.0f, 0.0f, 0.0f);
      view.rotate(camera_.pitch,  0.0f, 1.0f, 0.0f);
  }
#endif

 private:
  /* Arbitrary damping parameters (Rotation, Translation, and Wheel / Dolly) */
  static float constexpr kMouseRAcceleration = 0.005f;
  static float constexpr kMouseTAcceleration = 0.0015f;
  static float constexpr kMouseWAcceleration = 0.25f;

  /* Modulo value for angles to avoid overflow [should be TwoPi cyclic] */
  static float constexpr kAngleModulo = 2.0f * static_cast<float>(M_PI);

  float last_mouse_x_;
  float last_mouse_y_;
  float pitch_;
  float yaw_;
  float dolly_;
  float tx_;
  float ty_;
};

// ----------------------------------------------------------------------------

#endif // ARCBALL_CAMERA_H_
