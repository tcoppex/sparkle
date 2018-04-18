#include "ui/views/Simulation.h"
#include "imgui.h"

namespace views {

const char *Simulation::kEmitterTypeDescriptions[] = {
  "Point",
  "Disk",
  "Sphere",
  "Ball"
};

const char *Simulation::kSimulationVolumeDescriptions[] = {
  "Sphere", 
  "Box", 
  "None"
};

void Simulation::render() {
  if (!ImGui::CollapsingHeader("Simulation")) {
    return;
  }

  ImGui::DragFloat("Timestep", &params_.time_step_factor,
    kTimestepFactorStep, kTimestepFactorMin, kTimestepFactorMax);

  if (ImGui::TreeNode("Emitter")) {
    ImGui::Combo("Type", reinterpret_cast<int*>(&params_.emitter_type),
      kEmitterTypeDescriptions, IM_ARRAYSIZE(kEmitterTypeDescriptions));
    ImGui::DragFloatRange2("Age range", &params_.min_age, &params_.max_age,
      kAgeRangeStep, kAgeRangeMin, kAgeRangeMax, "Min: %.2f", "Max: %.2f");
    ImGui::DragFloat3("Position", params_.emitter_position, 0.25f);
    ImGui::DragFloat3("Velocity", params_.emitter_direction, 0.01f);
    switch(params_.emitter_type) {
      case GPUParticle::EMITTER_DISK:
      case GPUParticle::EMITTER_SPHERE:
      case GPUParticle::EMITTER_BALL:
        ImGui::DragFloat("Radius", &params_.emitter_radius,
          kEmitterRadiusStep, kEmitterRadiusMin, kEmitterRadiusMax);
      break;

      default:
      break;
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Bounding Volume")) {
    ImGui::Combo("Type", reinterpret_cast<int*>(&params_.bounding_volume),
      kSimulationVolumeDescriptions, IM_ARRAYSIZE(kSimulationVolumeDescriptions));
    ImGui::DragFloat("Size", &params_.bounding_volume_size,
      kSimulationSizeStep, kSimulationSizeMin, kSimulationSizeMax);
    ImGui::TreePop();
  }

}

}  // namespace views
