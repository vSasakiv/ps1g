#include "ps1g/ui/Components.h"
#include <imgui.h>
#include "imgui_internal.h"

namespace ps1g {
	bool PauseButton(const char* str_id) {
		ImGuiContext& g = *GImGui;
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		float sz = ImGui::GetFrameHeight();
		const ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + sz, window->DC.CursorPos.y + sz));
		ImGui::ItemSize(bb, g.Style.FramePadding.y);
		if (!ImGui::ItemAdd(bb, window->GetID(str_id))) return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, window->GetID(str_id), &hovered, &held);

		const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderNavHighlight(bb, window->GetID(str_id));
		ImGui::RenderFrame(bb.Min, bb.Max, col, true, g.Style.FrameRounding);

		float padding = sz * 0.30f;
		float bar_width = sz * 0.15f;
		float bar_h = sz - padding * 2.0f;
		
		ImDrawList* draw_list = window->DrawList;
		ImU32 icon_col = ImGui::GetColorU32(ImGuiCol_Text);

		draw_list->AddRectFilled(
			ImVec2(bb.Min.x + padding, bb.Min.y + padding), 
			ImVec2(bb.Min.x + padding + bar_width, bb.Min.y + padding + bar_h), 
			icon_col);
		
		draw_list->AddRectFilled(
			ImVec2(bb.Max.x - padding - bar_width, bb.Min.y + padding), 
			ImVec2(bb.Max.x - padding, bb.Min.y + padding + bar_h), 
			icon_col);

		return pressed;
	}

	void DrawInteractiveBit(const char* label, uint32_t& value, int bit_idx, bool has_underline, const char* tooltip) {
		bool is_set = (value >> bit_idx) & 1;
		
		ImGui::PushStyleColor(ImGuiCol_Text, is_set ? ImVec4(1, 1, 1, 1) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImGui::Text(is_set ? "1" : "0");
		
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("Bit %d: %s", bit_idx, label);
			if (tooltip) ImGui::TextDisabled("%s", tooltip);
			ImGui::EndTooltip();
		}

		if (has_underline) {
			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			float width = ImGui::GetItemRectSize().x;
			draw_list->AddLine(
				ImVec2(pos.x, pos.y + ImGui::GetFontSize()), 
				ImVec2(pos.x + width, pos.y + ImGui::GetFontSize()), 
				ImGui::GetColorU32(ImGuiCol_Text), 1.0f);
		}

		ImGui::PopStyleColor();
		
		if (bit_idx > 0) ImGui::SameLine(0, 2.0f); 
	}
}