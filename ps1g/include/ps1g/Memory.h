#pragma once
#include <stdint.h>
#include <array>
#include <ps1g/utils/arith.h>

namespace ps1g {
	template <size_t Size>
	class Memory {

	public:
		Memory() : memory_{} {}

		uint8_t readU8(uint32_t addr) {
			return memory_[addr];
		}

		uint16_t readU16(uint32_t addr) {
			uint16_t b0 = memory_[addr];
			uint16_t b1 = memory_[sized_wrapping_add(addr, 1, Size)];
			return b0 | (b1 << 8);
		}

		uint32_t readU32(uint32_t addr) {
			uint32_t b0 = memory_[addr];
			uint32_t b1 = memory_[sized_wrapping_add(addr, 1, Size)];
			uint32_t b2 = memory_[sized_wrapping_add(addr, 2, Size)];
			uint32_t b3 = memory_[sized_wrapping_add(addr, 3, Size)];
			return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
		}

		void writeU8(uint32_t addr, uint8_t data) {
			memory_[addr] = data;
		}

		void writeU16(uint32_t addr, uint16_t data) {
			memory_[addr] = data & 0xFF;
			memory_[sized_wrapping_add(addr, 1, Size)] = (data >> 8) & 0xFF;
		}

		void writeU32(uint32_t addr, uint32_t data) {
			memory_[addr] = data & 0xFF;
			memory_[sized_wrapping_add(addr, 1, Size)] = (data >> 8) & 0xFF;
			memory_[sized_wrapping_add(addr, 2, Size)] = (data >> 16) & 0xFF;
			memory_[sized_wrapping_add(addr, 3, Size)] = (data >> 24) & 0xFF;
		}

		std::array<uint8_t, Size>* getMemoryArray() {
			return &this->memory_;
		}

	private:
		std::array<uint8_t, Size> memory_;
	};
}
