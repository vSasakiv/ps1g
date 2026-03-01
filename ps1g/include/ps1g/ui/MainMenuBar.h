#pragma once
#include <string>
#include <array>
#include <functional>

struct GLFWwindow;

typedef char nfdchar_t;

namespace ps1g {
	class Debugger;
	class MainMenuBar {
	public:
		bool enabled = true;
		void render(GLFWwindow* window, Debugger& debugger);

		std::function<void()> toggleCpuDebug;
		std::function<bool()> enabledCpuDebug;

		std::function<void()> toggleMemDebug;
		std::function<bool()> enabledMemDebug;

		void setStatusMessage(std::string& message) { this->status_message_ = message; };
		void setStatusColor(std::array<float, 4>& color) { this->status_message_color_ = color; };

	private:
		nfdchar_t* openFilePicker();
		std::string status_message_;
		std::array<float, 4> status_message_color_ = { 0.4f, 0.8f, 1.0f, 1.0f };
		void menuOpenBios(Debugger& debugger);
	};
}
