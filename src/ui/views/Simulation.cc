#include "ui/views/Simulation.h"
#include "imgui.h"

namespace views {

const char *Simulation::kSimulationVolumeDescriptions[3] = {
  "Sphere", 
  "Box", 
  "None"
};

void Simulation::render() {
  if (!ImGui::CollapsingHeader("Simulation")) {
    return;
  }

  ImGui::DragFloat("Timestep factor", &params_.time_step_factor,
    kTimestepFactorStep, kTimestepFactorMin, kTimestepFactorMax);
  ImGui::Separator();

  ImGui::DragFloatRange2("Age range", &params_.min_age, &params_.max_age,
    kAgeRangeStep, kAgeRangeMin, kAgeRangeMax, "Min: %.2f", "Max: %.2f");
  ImGui::Separator();

  ImGui::Combo("Simulation volume", reinterpret_cast<int*>(&params_.bounding_volume),
    kSimulationVolumeDescriptions, IM_ARRAYSIZE(kSimulationVolumeDescriptions));
  ImGui::DragFloat("Simulation size", &params_.bounding_volume_size,
    0.5f, 16.0f, 2048.0f);
}

}  // namespace views
