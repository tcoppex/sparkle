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

constexpr float Simulation::kTimestepFactorStep;
constexpr float Simulation::kTimestepFactorMin;
constexpr float Simulation::kTimestepFactorMax;
constexpr float Simulation::kForceFactorStep;
constexpr float Simulation::kForceFactorMin;
constexpr float Simulation::kForceFactorMax;

void Simulation::render() {
  if (!ImGui::CollapsingHeader("Simulation")) {
    return;
  }

  ImGui::DragFloat("Timestep", &params_.time_step_factor,
    kTimestepFactorStep, kTimestepFactorMin, kTimestepFactorMax);
  Clamp(params_.time_step_factor, kTimestepFactorMin, kTimestepFactorMax);

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

  if (ImGui::TreeNode("Forces")) {
    ImGui::Checkbox("Scattering", &params_.enable_scattering);
    if (params_.enable_scattering) {
      ImGui::DragFloat("scattering factor", &params_.scattering_factor,
        kForceFactorStep, kForceFactorMin, kForceFactorMax);
    }

    ImGui::Checkbox("Vector field", &params_.enable_vectorfield);
    if (params_.enable_vectorfield) {
      ImGui::DragFloat("vectorfield factor", &params_.vectorfield_factor,
        kForceFactorStep, kForceFactorMin, kForceFactorMax);
    }

    ImGui::Checkbox("Curl Noise", &params_.enable_curlnoise);
    if (params_.enable_curlnoise) {
      ImGui::DragFloat("curlnoise factor", &params_.curlnoise_factor,
        kForceFactorStep, kForceFactorMin, kForceFactorMax);
      ImGui::DragFloat("scale", &params_.curlnoise_scale,
        kCurlnoiseScaleStep, kCurlnoiseScaleMin, kCurlnoiseScaleMax);
    }

    ImGui::Checkbox("Velocity Control", &params_.enable_velocity_control);
    if (params_.enable_velocity_control) {
      ImGui::DragFloat("velocity factor", &params_.velocity_factor,
        kForceFactorStep, kForceFactorMin, kForceFactorMax);
    }

    ImGui::TreePop();
  }
}

}  // namespace views
