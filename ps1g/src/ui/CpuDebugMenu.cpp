#include "ps1g/ui/CpuDebugMenu.h"
#include "ps1g/Bus.h"
#include "ps1g/MIPSR3000A.h"
#include "ps1g/utils/disassembler.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <iostream>
#include <format>
#include <array>

namespace ps1g {

	void CpuDebugMenu::render(Bus* bus) {

		if (!enabled) return;

		ImGui::Begin("CPU Debug Menu");

		if (ImGui::CollapsingHeader("Execution Control")) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.8f, 0, 1));
			if (ImGui::ArrowButton("##play", ImGuiDir_Right)) {
				bus->step();
			}  ImGui::SameLine();
			ImGui::PopStyleColor();

			ImGui::Text("Execute Instruction"); ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0, 0, 1));
			if (ImGui::ArrowButton("##play10x", ImGuiDir_Right)) {
				for (int i = 0; i < 10; i++)
					bus->step();
			}  ImGui::SameLine();
			ImGui::PopStyleColor();

			ImGui::Text("Execute 10 Instructions");
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
				ImGui::Text("0x%08X", bus->cpu()->pc());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("RA         ");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", bus->cpu()->registers()[31]);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("HI         ");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", bus->cpu()->hi());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("LO         ");

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", bus->cpu()->lo());

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
				ImGui::Text("0x%08X", bus->cpu()->prev_pc()); ImGui::SameLine();

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", bus->cpu()->fetched_next()); ImGui::SameLine();

				ImGui::TableSetColumnIndex(2);
				ImGui::Text(disassemble(bus->cpu()->fetched_next(), bus->cpu()->prev_pc()).c_str());

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
				ImGui::Text("0x%08X", bus->cpu()->pc()); ImGui::SameLine();

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", bus->readU32(bus->cpu()->pc())); ImGui::SameLine();

				ImGui::TableSetColumnIndex(2);
				ImGui::Text(disassemble(bus->readU32(bus->cpu()->pc()), bus->cpu()->pc()).c_str());

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

			for (MIPSR3000A::LoadDelay& ld : bus->cpu()->load_delay_queue()) {
				delays[ld.reg] = &ld;
			}

			for (int i = 0; i < 32; i++) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", this->register_names_[i]);

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("0x%08X", bus->cpu()->registers()[i]);

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
		ImGui::End();
	}
}