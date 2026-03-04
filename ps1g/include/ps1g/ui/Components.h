#pragma once
#include <stdint.h>

namespace ps1g {
	bool PauseButton(const char* str_id);
	void DrawInteractiveBit(const char* label, uint32_t& value, int bit_idx, bool has_underline, const char* tooltip);
}
