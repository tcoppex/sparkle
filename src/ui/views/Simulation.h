#ifndef SPARKLE_UI_VIEWS_SIMULATION_H_
#define SPARKLE_UI_VIEWS_SIMULATION_H_

#include "ui/view.h"
#include "api/gpu_particle.h"

namespace views {

//using TParameters = GPUParticle::SimulationParameters_t;

class Simulation : public ParametrizedUIView<GPUParticle::SimulationParameters_t> {
 public:
  Simulation(GPUParticle::SimulationParameters_t &params) : ParametrizedUIView(params) {}

  void render() override;

 private:
  static const char *kSimulationVolumeDescriptions[3];

  static constexpr float kTimestepFactorStep = 0.1f;
  static constexpr float kTimestepFactorMin = 0.0f;
  static constexpr float kTimestepFactorMax = 10.0f;
  static constexpr float kAgeRangeStep = 0.01f;
  static constexpr float kAgeRangeMin = 0.01f;
  static constexpr float kAgeRangeMax = 10.0f;
};

}  // namespace views

#endif  // SPARKLE_UI_VIEWS_SIMULATION_H_
