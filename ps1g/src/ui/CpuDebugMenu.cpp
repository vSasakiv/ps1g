#include "ps1g/ui/CpuDebugMenu.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <iostream>

namespace ps1g {

	void CpuDebugMenu::render() {

		if (!enabled) return;

		ImGui::Begin("CPU Control");
		ImGui::Text("Status: Running");
		
		if (ImGui::Button("Step Instruction")) {
			std::cout << "stepped" << std::endl;
		}
		ImGui::End();
	}
}