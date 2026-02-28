#pragma once
#include <string>
#include <array>
#include <functional>

struct GLFWwindow;

typedef char nfdchar_t;

namespace ps1g {
	class Bus;
	class MainMenuBar {
	public:
		bool enabled = true;
		void render(GLFWwindow* window, Bus* bus);

		std::function<void()> toggleCpuDebug;

	private:
		nfdchar_t* openFilePicker();
		std::string status_message_;
		std::array<float, 4> status_message_color_ = { 0.4f, 0.8f, 1.0f, 1.0f };
		void menuOpenBios(Bus* bus);
	};
}
