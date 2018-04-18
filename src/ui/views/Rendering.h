#ifndef SPARKLE_UI_VIEWS_RENDERING_H_
#define SPARKLE_UI_VIEWS_RENDERING_H_

#include "ui/view.h"
#include "api/gpu_particle.h"

namespace views {

class Rendering : public ParametrizedUIView<GPUParticle::RenderingParameters_t> {
 public:
  Rendering(TParameters &params) : ParametrizedUIView(params) {}

  void render() override;

private:
  static const char *kRenderModeDescriptions[GPUParticle::kNumRenderMode];
  static const char *kColorModeDescriptions[GPUParticle::kNumColorMode];

  static constexpr float kParticleSizeStep = 0.25f;
  static constexpr float kParticleSizeMin = 0.0f;
  static constexpr float kParticleSizeMax = 75.0f;

  static constexpr float kStretchedFactorStep = 0.05f;
  static constexpr float kStretchedFactorMin = 0.05f;
  static constexpr float kStretchedFactorMax = 100.0f;

  static constexpr float kFadingFactorStep = 0.005f;
  static constexpr float kFadingFactorMin = 0.005f;
  static constexpr float kFadingFactorMax = 1.0f;
};

}  // namespace views

#endif  // SPARKLE_UI_VIEWS_RENDERING_H_
