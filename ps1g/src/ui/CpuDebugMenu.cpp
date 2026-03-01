#include "ps1g/ui/CpuDebugMenu.h"
#include "ps1g/Debugger.h"
#include "ps1g/MIPSR3000A.h"
#include "ps1g/utils/disassembler.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <iostream>
#include <format>
#include <array>

namespace ps1g {

	void CpuDebugMenu::render(Debugger& debugger) const {

		if (!enabled) return;

		ImGui::Begin("CPU Debug Menu");
		static float copy_timer;

		if (ImGui::CollapsingHeader("Execution Control")) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.8f, 0, 1));
			if (ImGui::ArrowButton("##play", ImGuiDir_Right)) {
				debugger.step();
			}  ImGui::SameLine();
			ImGui::PopStyleColor();

			ImGui::Text("Execute Instruction"); ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0, 0, 1));
			if (ImGui::ArrowButton("##play10x", ImGuiDir_Right)) {
				for (int i = 0; i < 10; i++)
					debugger.step();
			}  ImGui::SameLine();
			ImGui::PopStyleColor();

			ImGui::Text("Execute 10 Instructions");

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0.8f, 1));
			if (ImGui::ArrowButton("##resume", ImGuiDir_Right)) {
				debugger.resume();
				if (this->setStatusMessage) {
					std::string message = "Breakpoint Reached";
					std::array<float, 4> color = { 0.8f, 0.0f, 0.0f, 1.0f };
					this->setStatusMessage(message, color);
				}
			}  ImGui::SameLine();
			ImGui::PopStyleColor();

			ImGui::Text("Resume Execution");
		}

		if (ImGui::CollapsingHeader("Cpu Overview")) {
			ImGui::SeparatorText("Special Registers");
			ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
			if (ImGui::BeginTable("PC", 2,  tflags)) {
                ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("PC         ");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", debugger.readPc());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("RA         ");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", debugger.readGeneralRegs()[31]);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("HI         ");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", debugger.readHi());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("LO         ");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", debugger.readLo());

				ImGui::EndTable();
			}

			ImGui::SeparatorText("Current Instruction");
			if (ImGui::BeginTable("Current Instruction", 3, tflags)) {
                ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Disassembly", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				char pc_str[11];
				sprintf(pc_str, "0x%08X", debugger.prevPc());
				if (ImGui::Selectable(pc_str, false, ImGuiSelectableFlags_AllowDoubleClick)) {
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						ImGui::SetClipboardText(&pc_str[2]);
						copy_timer = 0.5f;
					}
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Double-click to copy: %s", &pc_str[2]);
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", debugger.currentInstruction()); ImGui::SameLine();

				ImGui::TableSetColumnIndex(2);
				ImGui::Text(disassemble(debugger.currentInstruction(), debugger.prevPc()).c_str());

				ImGui::EndTable();
			}

			ImGui::SeparatorText("Next Instruction");
			if (ImGui::BeginTable("Next Instruction", 3, tflags)) {
                ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Instruction", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Disassembly", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				char pc_next_str[11];
				sprintf(pc_next_str, "0x%08X", debugger.readPc());
				if (ImGui::Selectable(pc_next_str, false, ImGuiSelectableFlags_AllowDoubleClick)) {
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						ImGui::SetClipboardText(&pc_next_str[2]);
						copy_timer = 0.5f;
					}
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Double-click to copy: %s", &pc_next_str[2]);
				}

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", debugger.nextInstruction()); ImGui::SameLine();

				ImGui::TableSetColumnIndex(2);
				ImGui::Text(disassemble(debugger.nextInstruction(), debugger.readPc()).c_str());

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Cpu General Purpose Registers")) {

			ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
			if (ImGui::BeginTable("Registers", 3,  tflags)) {
                ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Delayed Slot", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();
			}

			std::array<MIPSR3000A::LoadDelay*, 32> delays = {nullptr};

			for (MIPSR3000A::LoadDelay& ld : debugger.getLoadDelayQueue()) {
				delays[ld.reg] = &ld;
			}

			for (int i = 0; i < 32; i++) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", this->register_names_[i]);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", debugger.readGeneralRegs()[i]);

				ImGui::TableSetColumnIndex(2);
				if (delays[i] != nullptr) {
					ImGui::Text("<- 0x%08X (%d)", delays[i]->value, delays[i]->remaining_duration);
				}
				else {
					ImGui::Text("--");
				}
			}

			ImGui::EndTable();
		}

		if (copy_timer > 0.0f) {
			ImGui::SetNextWindowPos(ImGui::GetMousePos(), ImGuiCond_Always, ImVec2(0.5f, 1.5f));
			
			ImGui::Begin("##copy_popup", nullptr, 
				ImGuiWindowFlags_Tooltip | 
				ImGuiWindowFlags_NoInputs | 
				ImGuiWindowFlags_NoTitleBar | 
				ImGuiWindowFlags_AlwaysAutoResize);
				
			ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Copied!");
			ImGui::End();

			copy_timer -= ImGui::GetIO().DeltaTime;
		}

		ImGui::End();
	}
}