#ifndef SPARKLE_UI_VIEWS_MAIN_H_
#define SPARKLE_UI_VIEWS_MAIN_H_

#include <vector>
#include "ui/view.h"

namespace views {

class Main : public UIView {
 public:
  void render() override;
  
  inline void push_view(UIView* view) {
    views_.push_back(view);
  }

private:
  std::vector<UIView*> views_;
};

}  // namespace views

#endif  // SPARKLE_UI_VIEWS_MAIN_H_
