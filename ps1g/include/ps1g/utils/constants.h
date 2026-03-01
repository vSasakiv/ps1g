#pragma once

namespace ps1g {
	inline constexpr uint32_t kBiosSize = 512 * 1024;
	inline constexpr uint32_t kMainRamSize = 2048 * 1024;
	inline constexpr uint32_t kBiosOffset = 0x1FC00000;
	inline constexpr uint32_t kMemoryControlOffset = 0x1F801000;
}
