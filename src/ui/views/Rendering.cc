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
  if (GPUParticle::STRETCHED == params_.rendermode) {
    ImGui::DragFloat("Stretch factor", &params_.stretched_factor,
      kStretchedFactorStep, kStretchedFactorMin, kStretchedFactorMax);
  }
  
  ImGui::Combo("Color", reinterpret_cast<int*>(&params_.colormode),
    kColorModeDescriptions, IM_ARRAYSIZE(kColorModeDescriptions));
  if (GPUParticle::GRADIENT == params_.colormode) {
    ImGui::ColorPicker4("Birth gradient", params_.birth_gradient);
    ImGui::ColorPicker4("Death gradient", params_.death_gradient);
  }
}

}  // namespace views
