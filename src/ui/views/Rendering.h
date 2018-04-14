#ifndef SPARKLE_UI_VIEWS_RENDERING_H_
#define SPARKLE_UI_VIEWS_RENDERING_H_

#include "ui/view.h"
#include "api/gpu_particle.h"

namespace views {

//using TParameters = GPUParticle::RenderingParameters_t;

class Rendering : public ParametrizedUIView<GPUParticle::RenderingParameters_t> {
 public:
  Rendering(GPUParticle::RenderingParameters_t &params) : ParametrizedUIView(params) {}

  void render() override;

private:
  static const char *kRenderModeDescriptions[2];
  static const char *kColorModeDescriptions[2];

  static constexpr float kParticleSizeStep = 0.25f;
  static constexpr float kParticleSizeMin = 0.0f;
  static constexpr float kParticleSizeMax = 100.0f;

  static constexpr float kStretchedFactorStep = 0.5f;
  static constexpr float kStretchedFactorMin = 0.5f;
  static constexpr float kStretchedFactorMax = 500.0f;

  static constexpr float kFadingFactorStep = 0.01f;
  static constexpr float kFadingFactorMin = 0.01f;
  static constexpr float kFadingFactorMax = 1.0f;
};

}  // namespace views

#endif  // SPARKLE_UI_VIEWS_RENDERING_H_
