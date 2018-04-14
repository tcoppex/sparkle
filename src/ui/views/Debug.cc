#include "ui/views/Debug.h"
#include "imgui.h"

namespace views {

void Debug::render() {
  if (!ImGui::CollapsingHeader("Debug")) {
    return;
  }

  ImGui::Checkbox("Show grid", &params_.show_grid);
  ImGui::Checkbox("Show simulation volume", &params_.show_simulation_volume);
  ImGui::Checkbox("Freeze", &params_.freeze);
}

}  // namespace views
