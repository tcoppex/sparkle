#include "ui/views/Main.h"
#include "imgui.h"

namespace views {

void Main::render() {
  ImGui::SetNextWindowPos(ImVec2(8,8));
  ImGui::SetNextWindowSize(ImVec2(316, 850));

  if (!ImGui::Begin("Parameters", NULL, ImGuiWindowFlags_NoMove)) {
    ImGui::End();
    return;
  }

  ImGui::Spacing();
  ImGui::TextWrapped("Welcome to spärkle, a particle simulation playground.");
  ImGui::Spacing();

  ImGui::Text("Here some basic inputs :");
  ImGui::BulletText("Right-click + mouse to rotate.");
  ImGui::BulletText("Scroll to dolly.");
  ImGui::BulletText("Escape to quit. Oh no !");

  ImGui::Spacing();
  ImGui::Spacing();
  ImGui::Spacing();
  ImGui::Spacing();

  ImGui::PushItemWidth(170);

  for (auto view : views_) {
    view->render();
    ImGui::Spacing();
  }

  // Framerate info.
  {
    double ms = 1000.0f / ImGui::GetIO().Framerate;
    double framerate = ImGui::GetIO().Framerate;
    ImGui::Text("%.3f ms/frame (%.1f FPS)", ms, framerate);
  }

  ImGui::End();
}

}  // namespace views
