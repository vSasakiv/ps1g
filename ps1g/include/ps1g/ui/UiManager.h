#pragma once
#include "ps1g/ui/MainMenuBar.h"
#include "ps1g/ui/CpuDebugMenu.h"
#include "ps1g/ui/MemoryDebugMenu.h"


struct GLFWwindow;

namespace ps1g {

	class UiManager {

	public:
		UiManager() = default;
		~UiManager();

		bool Initialize();
		bool IsRunning();
		void BeginFrame();
		void RenderUI(Bus* bus);
		void EndFrame();

		GLFWwindow* window() const { return window_; }


	private:
		void initCallbacks();
		GLFWwindow* window_;
		MainMenuBar main_menu_bar_;
		CpuDebugMenu cpu_debug_menu_;
		MemoryDebugMenu mem_debug_menu_;
	};
}

