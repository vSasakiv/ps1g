#pragma once
#include <array>
#include <string>

namespace ps1g {
	class Debugger;

	class MemoryDebugMenu {
	public:
		bool enabled = false;
		void render(Debugger& debugger) const;

	private:
		static int selected_memory_type_;
		static uint32_t base_offset_[];
		static uint32_t goto_address_[];
		static bool jump_table_[];
		static constexpr const char* const memory_type_names_[] = {
			"BIOS",
			"RAM"
		};

	};
}
