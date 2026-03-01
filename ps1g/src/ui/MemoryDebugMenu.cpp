#include "ps1g/ui/MemoryDebugMenu.h"
#include "ps1g/Bus.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"

namespace ps1g {
	void MemoryDebugMenu::render(Bus* bus) {
		if (!enabled) return;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		float windowWidth = viewport->WorkSize.x * (3.0f / 4.0f);
		float windowHeight = viewport->WorkSize.y;
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

		ImGui::Begin("Memory Debug Menu");


		ImGui::End();

	}
}
