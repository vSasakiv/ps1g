#include "ps1g/ui/UiManager.h"
#include "ps1g/Bus.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <iostream>

namespace ps1g {

	UiManager::~UiManager() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwDestroyWindow(this->window_);
		glfwTerminate();
	}

	bool UiManager::Initialize() {
		if (!glfwInit()) return false;

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

		this->window_ = glfwCreateWindow(mode->width, mode->height, "PS1g", nullptr, nullptr);

		if (!this->window_) { glfwTerminate(); return false; }
		glfwMakeContextCurrent(this->window_);
		glfwSwapInterval(1);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		float xscale, yscale;
		glfwGetWindowContentScale(this->window_, &xscale, &yscale);
		io.FontGlobalScale = xscale;

		ImGui_ImplGlfw_InitForOpenGL(this->window_, true);
		ImGui_ImplOpenGL3_Init("#version 130");

		this->initCallbacks();

		return true;
	}

	bool UiManager::IsRunning() {
		return !glfwWindowShouldClose(this->window_);
	}

	void UiManager::BeginFrame() {
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void UiManager::RenderUI(Bus* bus) {
		this->main_menu_bar_.render(this->window_, bus);
		this->cpu_debug_menu_.render();
	}

	void UiManager::EndFrame() {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(this->window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(this->window_);
	}

	void UiManager::initCallbacks() {
		this->main_menu_bar_.toggleCpuDebug = [this]() {
			this->cpu_debug_menu_.enabled = !this->cpu_debug_menu_.enabled;
		};
	}
}