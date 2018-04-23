#include "ui/views/Rendering.h"
#include "imgui.h"

namespace views {

constexpr float Rendering::kParticleSizeStep;
constexpr float Rendering::kParticleSizeMin;
constexpr float Rendering::kParticleSizeMax;
constexpr float Rendering::kStretchedFactorStep;
constexpr float Rendering::kStretchedFactorMin;
constexpr float Rendering::kStretchedFactorMax;
constexpr float Rendering::kFadingFactorStep;
constexpr float Rendering::kFadingFactorMin;
constexpr float Rendering::kFadingFactorMax;

const char *Rendering::kRenderModeDescriptions[] = {
  "Stretched", 
  "Pointsprite"
};

const char *Rendering::kColorModeDescriptions[] = {
  "Default", 
  "Gradient"
};

void Rendering::render() {
  if (!ImGui::CollapsingHeader("Rendering")) {
    return;
  }

  if (ImGui::TreeNode("Shader")) {
    ImGui::Combo("Type", reinterpret_cast<int*>(&params_.rendermode),
      kRenderModeDescriptions, IM_ARRAYSIZE(kRenderModeDescriptions));
    switch (params_.rendermode) {
      case GPUParticle::RENDERMODE_STRETCHED:
        ImGui::DragFloat("Stretch factor", &params_.stretched_factor,
          kStretchedFactorStep, kStretchedFactorMin, kStretchedFactorMax);
        Clamp(params_.stretched_factor, kStretchedFactorMin, kStretchedFactorMax);
        ImGui::Separator();
      break;

      case GPUParticle::RENDERMODE_POINTSPRITE:
      default:
        ImGui::DragFloatRange2("Size", &params_.min_size, &params_.max_size,
          kParticleSizeStep, kParticleSizeMin, kParticleSizeMax, "Min: %.2f", "Max: %.2f");
        Clamp(params_.min_size, kParticleSizeMin, kParticleSizeMax);
        Clamp(params_.max_size, kParticleSizeMin, kParticleSizeMax);
        ImGui::Separator();
      break;
    }

    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Color")) {
    ImGui::Combo("Mode", reinterpret_cast<int*>(&params_.colormode),
      kColorModeDescriptions, IM_ARRAYSIZE(kColorModeDescriptions));

    if (GPUParticle::COLORMODE_GRADIENT == params_.colormode) {
      ImGui::ColorEdit3("Start", params_.birth_gradient);
      ImGui::ColorEdit3("End", params_.death_gradient);
    }

    ImGui::DragFloat("Fading", &params_.fading_factor,
      kFadingFactorStep, kFadingFactorMin, kFadingFactorMax);
    Clamp(params_.fading_factor, kFadingFactorMin, kFadingFactorMax);
    ImGui::TreePop();
  }

}

}  // namespace views
