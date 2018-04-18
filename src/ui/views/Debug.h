#ifndef SPARKLE_UI_VIEWS_DEBUG_H_
#define SPARKLE_UI_VIEWS_DEBUG_H_

#include "ui/view.h"
#include "scene.h"

namespace views {

class Debug : public ParametrizedUIView<Scene::DebugParameters_t> {
 public:
  Debug(TParameters &params) : ParametrizedUIView(params) {}

  void render() override;
};

}  // namespace views

#endif  // SPARKLE_UI_VIEWS_DEBUG_H_
