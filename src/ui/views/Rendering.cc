#include "ui/views/Rendering.h"
#include "imgui.h"

namespace views {

const char *Rendering::kRenderModeDescriptions[2] = {
  "Stretched", 
  "Pointsprite"
};

const char *Rendering::kColorModeDescriptions[2] = {
  "Default", 
  "Gradient"
};

void Rendering::render() {
  if (!ImGui::CollapsingHeader("Rendering")) {
    return;
  }
  
  ImGui::Combo("Mode", reinterpret_cast<int*>(&params_.rendermode),
    kRenderModeDescriptions, IM_ARRAYSIZE(kRenderModeDescriptions));
  switch (params_.rendermode) {
    case GPUParticle::STRETCHED:
      ImGui::DragFloat("Stretch factor", &params_.stretched_factor,
        kStretchedFactorStep, kStretchedFactorMin, kStretchedFactorMax);
      ImGui::Separator();
    break;

    case GPUParticle::POINTSPRITE:
    default:
      ImGui::DragFloatRange2("Particle size", &params_.min_size, &params_.max_size,
        kParticleSizeStep, kParticleSizeMin, kParticleSizeMax, "Min: %.2f", "Max: %.2f");
      ImGui::Separator();
    break;
  }
  
  ImGui::Combo("Color", reinterpret_cast<int*>(&params_.colormode),
    kColorModeDescriptions, IM_ARRAYSIZE(kColorModeDescriptions));
  if (GPUParticle::GRADIENT == params_.colormode) {

    ImGui::Spacing();
    ImGui::ColorPicker4("Birth gradient", params_.birth_gradient);
    ImGui::Spacing();
    ImGui::ColorPicker4("Death gradient", params_.death_gradient);
  }
  ImGui::Separator();

  ImGui::DragFloat("Fade coefficient", &params_.fading_factor,
    kFadingFactorStep, kFadingFactorMin, kFadingFactorMax);
}

}  // namespace views
