#include "ps1g/ui/MemoryDebugMenu.h"
#include "ps1g/Bus.h"
#include "ps1g/Memory.h"
#include "ps1g/Debugger.h"
#include "ps1g/utils/disassembler.h"
#include "ps1g/utils/constants.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"

#include <iostream>

namespace ps1g {
	void MemoryDebugMenu::render(Debugger& debugger) const {
		if (!enabled) return;

		const float kVerticalPadding = 20.0f;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		float window_width = viewport->WorkSize.x * (3.0f / 4.0f);
		float window_height = viewport->WorkSize.y;
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(ImVec2(window_width, window_height));

		ImGui::Begin("Memory Debug Menu");

		// Memory selector 
		const char* memory_types[] = {"BIOS", "RAM"};
		static int selected = 0;

		ImGui::Text("Memory Type:");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(300.0f);
		if (ImGui::Combo("##Memory", &selected, memory_types, IM_ARRAYSIZE(memory_types))) {}

		// Base offset input
		static uint32_t base_offset[] = { 0x0, 0x0 };
		ImGui::Text("Base Offset:");
		ImGui::SameLine();

		ImGui::PushItemWidth(300.0f);
		ImGui::InputScalar("##BaseOffset", ImGuiDataType_U32, &(base_offset[selected]), nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::SameLine();

		ImGui::Dummy(ImVec2(10.0f, 0.0f));
		ImGui::SameLine();

		// goto input
		static uint32_t goto_address[] = { 0x0, 0x0 };
		ImGui::Text("Go to:");
		ImGui::SameLine();

		static bool jump_table[] = { false, false };
		ImGui::PushItemWidth(300.0f);
		ImGui::InputScalar("##GoToAddress", ImGuiDataType_U32, &(goto_address[selected]), nullptr, nullptr, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
		ImGui::SameLine();
		if (ImGui::ArrowButton("##goto", ImGuiDir_Right)) {
			jump_table[selected] = true;
		};

		ImGui::Dummy(ImVec2(0.0f, kVerticalPadding/2));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, kVerticalPadding/2));

		float full_width = ImGui::GetContentRegionAvail().x;
		float half_width = (full_width - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

		ImGui::BeginChild("LeftMemoryTable", ImVec2(half_width, 0), ImGuiChildFlags_None);
		// Memory table
		ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
			ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_ScrollY;


		ImGui::SeparatorText("Memory Table");
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, kVerticalPadding));
		if (ImGui::BeginTable("Memory Table", 4, tflags, ImVec2(0.0, -1.0f))) {
			ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Raw Bytes", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Raw", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Disassembly", ImGuiTableColumnFlags_WidthStretch);

			// BIOS
			if (selected == 0) {
				Memory<kBiosSize>const& mem = debugger.getBios();
				ImGuiListClipper clipper;
				float row_height = ImGui::GetTextLineHeight() + kVerticalPadding * 2;

				if (jump_table[selected]) {
					ImGui::SetScrollY(((goto_address[selected] - base_offset[selected]) / 4) * row_height);
					jump_table[selected] = false;
				}

				clipper.Begin(kBiosSize / 4);

				while (clipper.Step()) {
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
						uint32_t addr = i * 4;
						uint32_t contents = mem.readU32(addr);
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isBreakpoint = debugger.isBiosBreakpoint(addr);

						char id[32]; sprintf(id, "##bp_%08X", addr);
						if (ImGui::Selectable(id, false, ImGuiSelectableFlags_None)) {
							if (isBreakpoint) 
								debugger.removeBiosBreakpoint(addr);
							else              
								debugger.addBiosBreakpoint(addr);
						}

						if (isBreakpoint) {
							ImVec2 pos = ImGui::GetItemRectMin();
							ImVec2 size = ImGui::GetItemRectSize();
							ImVec2 circle_pos = ImVec2(pos.x + 25.0f, pos.y + size.y * 0.5f);
							
							ImGui::GetWindowDrawList()->AddCircleFilled(circle_pos, 10.0f, IM_COL32(200, 30, 20, 255));
						}
						ImGui::SameLine(25.0f);
						ImGui::Text("  0x%08X  ", addr + base_offset[selected]);

						ImGui::TableSetColumnIndex(1);
						ImGui::Text(" %02X %02X %02X %02X ", mem.readU8(addr), mem.readU8(addr+1), mem.readU8(addr+2), mem.readU8(addr+3));

						ImGui::TableSetColumnIndex(2);
						ImGui::Text("  0x%08X  ", contents);

						ImGui::TableSetColumnIndex(3);
						ImGui::Text(disassemble(contents, addr + base_offset[selected]).c_str());
					}
				}

			}
			else if (selected == 1) {
				Memory<kMainRamSize>const& mem = debugger.getMainRam();
				ImGuiListClipper clipper;
				float row_height = ImGui::GetTextLineHeight() + kVerticalPadding * 2;

				if (jump_table[selected]) {
					ImGui::SetScrollY(((goto_address[selected] - base_offset[selected]) / 4) * row_height);
					jump_table[selected] = false;
				}

				clipper.Begin(kMainRamSize / 4);

				while (clipper.Step()) {
					for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
						uint32_t addr = i * 4;
						uint32_t contents = mem.readU32(addr);
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isBreakpoint = debugger.isRamBreakpoint(addr);

						char id[32]; sprintf(id, "##bp_%08X", addr);
						if (ImGui::Selectable(id, false, ImGuiSelectableFlags_None)) {
							if (isBreakpoint) 
								debugger.removeRamBreakpoint(addr);
							else              
								debugger.addRamBreakpoint(addr);
						}

						if (isBreakpoint) {
							ImVec2 pos = ImGui::GetItemRectMin();
							ImVec2 size = ImGui::GetItemRectSize();
							ImVec2 circle_pos = ImVec2(pos.x + 25.0f, pos.y + size.y * 0.5f);
							
							ImGui::GetWindowDrawList()->AddCircleFilled(circle_pos, 10.0f, IM_COL32(200, 30, 20, 255));
						}

						ImGui::SameLine(25.0f);
						ImGui::Text("  0x%08X  ", addr + base_offset[selected]);

						ImGui::TableSetColumnIndex(1);
						ImGui::Text(" %02X %02X %02X %02X ", mem.readU8(addr), mem.readU8(addr+1), mem.readU8(addr+2), mem.readU8(addr+3));

						ImGui::TableSetColumnIndex(2);
						ImGui::Text("  0x%08X  ", contents);

						ImGui::TableSetColumnIndex(3);
						ImGui::Text(disassemble(contents, addr + base_offset[selected]).c_str());
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::BeginChild("RightMemory", ImVec2(half_width, 0), ImGuiChildFlags_None);

		if (ImGui::CollapsingHeader("Breakpoints")) {

			if (ImGui::Button("Remove All")) {
				debugger.removeAllBreakpoints();
			}

			ImGuiTableFlags tflags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.0f, kVerticalPadding));
			if (ImGui::BeginTable("BreakPointTable", 3,  tflags)) {
                ImGui::TableSetupColumn("Memory Type", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Disassembly", ImGuiTableColumnFlags_WidthStretch);

				Memory<kBiosSize>const& bios = debugger.getBios();
				uint32_t addr_to_remove;
				bool remove = false;
				for (const uint32_t& addr : debugger.getBiosBreakpoints()) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					char id[32]; sprintf(id, "##bios_%08X", addr);
					if (ImGui::Selectable(id, false, ImGuiSelectableFlags_SpanAllColumns)) {
						selected = 0;
						goto_address[0] = addr + base_offset[0];
						jump_table[0] = true;
					}
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
						remove = true;
						addr_to_remove = addr;
					}

					ImGui::SameLine();
					ImGui::Text("BIOS");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("  0x%08X  ", addr + base_offset[0]);
					ImGui::TableSetColumnIndex(2);
					ImGui::Text(disassemble(bios.readU32(addr), addr + base_offset[0]).c_str());
				}
				if (remove) {
					debugger.removeBiosBreakpoint(addr_to_remove);
				}

				remove = false;
				Memory<kMainRamSize>const& ram = debugger.getMainRam();
				for (const uint32_t& addr : debugger.getRamBreakpoints()) {
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);

					char id[32]; sprintf(id, "##ram_%08X", addr);
					if (ImGui::Selectable(id, false, ImGuiSelectableFlags_SpanAllColumns)) {
						selected = 1;
						goto_address[1] = addr + base_offset[1];
						jump_table[1] = true;
					}
					if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
						remove = true;
						addr_to_remove = addr;
					}

					ImGui::SameLine();
					ImGui::Text("RAM");
					ImGui::TableSetColumnIndex(1);
					ImGui::Text("  0x%08X  ", addr + base_offset[1]);
					ImGui::TableSetColumnIndex(2);
					ImGui::Text(disassemble(ram.readU32(addr), addr + base_offset[1]).c_str());
				}
				if (remove) {
					debugger.removeRamBreakpoint(addr_to_remove);
				}

				ImGui::EndTable();
			}
			ImGui::PopStyleVar();
		}

		ImGui::EndChild();

		ImGui::End();
	}
}
