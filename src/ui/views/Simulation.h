#ifndef SPARKLE_UI_VIEWS_SIMULATION_H_
#define SPARKLE_UI_VIEWS_SIMULATION_H_

#include "ui/view.h"
#include "api/gpu_particle.h"

namespace views {

class Simulation : public ParametrizedUIView<GPUParticle::SimulationParameters_t> {
 public:
  Simulation(TParameters &params) : ParametrizedUIView(params) {}

  void render() override;

 private:
  static const char *kEmitterTypeDescriptions[GPUParticle::kNumEmitterType];
  static const char *kSimulationVolumeDescriptions[GPUParticle::kNumSimulationVolume];

  static constexpr float kTimestepFactorStep = 0.0125f;
  static constexpr float kTimestepFactorMin = -10.0f;
  static constexpr float kTimestepFactorMax = 10.0f;
  static constexpr float kAgeRangeStep = 0.005f;
  static constexpr float kAgeRangeMin = 0.01f;
  static constexpr float kAgeRangeMax = 1.0f;
  static constexpr float kEmitterRadiusStep = 0.5f;
  static constexpr float kEmitterRadiusMin = 1.0f;
  static constexpr float kEmitterRadiusMax = 1024.0f;
  static constexpr float kSimulationSizeStep = 0.5f;
  static constexpr float kSimulationSizeMin = 16.0f;
  static constexpr float kSimulationSizeMax = 1024.0f;
};

}  // namespace views

#endif  // SPARKLE_UI_VIEWS_SIMULATION_H_
