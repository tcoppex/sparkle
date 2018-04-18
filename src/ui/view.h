#ifndef SPARKLE_UI_UI_VIEW_H_
#define SPARKLE_UI_UI_VIEW_H_

/**
 * @brief Interface used to render User Interface View.
 */
class UIView {
public:
  virtual ~UIView() {}
  virtual void render() = 0;
};

/**
 * @brief Abstract class to a UIView with parameters.
 */
template<typename T>
class ParametrizedUIView : public UIView {
  public:
    using TParameters = T;

    ParametrizedUIView(T &params) : 
      params_(params)
    {}

  protected:
    T& params_;
};

#endif // SPARKLE_UI_UI_VIEW_H_
