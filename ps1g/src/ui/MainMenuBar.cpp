#include "ps1g/ui/MainMenuBar.h"
#include "ps1g/Bus.h"

#include <nfd.hpp>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace ps1g {
	void MainMenuBar::render(GLFWwindow* window, Bus* bus) {
		if (!enabled) return;

		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Open BIOS...", "Ctrl+B")) { this->menuOpenBios(bus); }
				if (ImGui::MenuItem("Open ROM...", "Ctrl+O")) { }
				if (ImGui::MenuItem("Exit", "Alt+F4")) { glfwSetWindowShouldClose(window, true); }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Emulation")) {
				if (ImGui::MenuItem("Run")) { /* cpu.run(); */ }
				if (ImGui::MenuItem("Pause")) { /* cpu.pause(); */ }
				if (ImGui::MenuItem("Reset", "Ctrl+R")) { bus->reset();  }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Debug Tools")) {
				if (ImGui::MenuItem("Cpu Debugger", NULL, this->enabledCpuDebug ? this->enabledCpuDebug() : false)) { 
					if (this->toggleCpuDebug) 
						this->toggleCpuDebug(); 
				}
				if (ImGui::MenuItem("Memory Viewer", NULL, this->enabledMemDebug ? this->enabledMemDebug() : false)) {
					if (this->toggleMemDebug) 
						this->toggleMemDebug(); 
				}

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		float statusBarHeight = ImGui::GetFrameHeight() + 10.0f;
		if (ImGui::BeginViewportSideBar("##MainStatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, statusBarHeight, ImGuiWindowFlags_NoDecoration)) 
		{
			ImGui::AlignTextToFramePadding();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX());
			ImGui::TextColored(
				ImVec4(
					this->status_message_color_[0],
					this->status_message_color_[1],
					this->status_message_color_[2],
					this->status_message_color_[3]), "%s", status_message_.c_str());
			ImGui::End();
		}

	}

	nfdchar_t* MainMenuBar::openFilePicker() {
		NFD_Init();
		nfdchar_t *out_path = nullptr;
		nfdfilteritem_t filters[1] = {{ "Binary Files", "BIN,bin" }};
		nfdopendialogu8args_t args = {0};
		args.filterList = filters;
		args.filterCount = 1;

		nfdresult_t result = NFD_OpenDialogU8_With(&out_path, &args);

		if (result == NFD_OKAY) {
			std::cout << "Success! Path: " << out_path << std::endl;
		}
		else if (result == NFD_CANCEL) {
			std::cout << "User pressed cancel." << std::endl;
		}
		
		NFD_Quit();
		return out_path;
	}

	void MainMenuBar::menuOpenBios(Bus* bus) {
		nfdchar_t* path = this->openFilePicker();
		if (path == nullptr) return;
		bus->loadBiosfromFile(path);
		this->status_message_ = "BIOS Loaded Successfully";
		this->status_message_color_ = { 0.1f, 0.8f, 0.1f, 1.0f };
		NFD_FreePath(path);
	}

}
