#include "ps1g/ui/CpuDebugMenu.h"
#include "ps1g/Debugger.h"
#include "ps1g/MIPSR3000A.h"
#include "ps1g/utils/disassembler.h"
#include "ps1g/ui/Components.h"

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

		if (ImGui::CollapsingHeader("Execution Control", ImGuiTreeNodeFlags_DefaultOpen)) {
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

			if (!debugger.isResumed()) {
				if (ImGui::ArrowButton("##resume", ImGuiDir_Right)) {
					debugger.resume();
				}
				ImGui::SameLine();
				ImGui::Text("Resume Execution");
			}
			else {
				if (PauseButton("DebuggerPause")) {
					debugger.stop();
				}
				ImGui::SameLine();
				ImGui::Text("Pause Execution");
			}

		}

		if (ImGui::CollapsingHeader("Cpu Overview", ImGuiTreeNodeFlags_DefaultOpen)) {
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
				sprintf(pc_str, "0x%08X", debugger.readPc());
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
				ImGui::Text(disassemble(debugger.currentInstruction(), debugger.readPc()).c_str());

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
				sprintf(pc_next_str, "0x%08X", debugger.nextPc());
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
				ImGui::Text(disassemble(debugger.nextInstruction(), debugger.nextPc()).c_str());

				ImGui::EndTable();
			}
		}

		if (ImGui::CollapsingHeader("Cpu Coprocessor 0 Registers")) {
			ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
			if (ImGui::BeginTable("Coprocessor 0 Registers", 2, tflags)) {
				ImGui::TableSetupColumn("Register", ImGuiTableColumnFlags_WidthFixed);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableHeadersRow();
			}

			// TODO: ADD REST OF REGISTER LATER
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", this->cop0_register_names_[MIPSR3000A::CP0::SR - 12]);
			ImGui::TableSetColumnIndex(1);

			for (int i = 31; i >= 0; i--) {
				bool important = i < 6 || i == 16 || i == 22;
				uint32_t value = debugger.readCp0(MIPSR3000A::CP0::SR);
				DrawInteractiveBit("SRBIT", value, i, important, this->cop0_system_register_description_[i]);
				if (i % 8 == 0 && i > 0) ImGui::SameLine(0, 10.0f);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", this->cop0_register_names_[MIPSR3000A::CP0::Cause - 12]);
			ImGui::TableSetColumnIndex(1);

			for (int i = 31; i >= 0; i--) {
				bool important = (i > 1 && i < 7) || (i > 7 && i < 16) || i == 28 || i == 29 || i == 31;
				uint32_t value = debugger.readCp0(MIPSR3000A::CP0::Cause);
				DrawInteractiveBit("CBIT", value, i, important, this->cop0_cause_register_description_[i]);
				if (i % 8 == 0 && i > 0) ImGui::SameLine(0, 10.0f);
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%s", this->cop0_register_names_[MIPSR3000A::CP0::EPC - 12]);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("0x%08X", debugger.readCp0(MIPSR3000A::CP0::EPC));

			ImGui::EndTable();
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