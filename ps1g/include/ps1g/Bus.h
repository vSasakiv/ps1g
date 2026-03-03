#pragma once
#include <stdint.h>
#include <memory>
#include <ps1g/Memory.h>
#include <ps1g/utils/constants.h>


namespace ps1g {

	class MIPSR3000A;

	class Bus {

	// Memory Regions:
		// KUSEG: 0x00000000 - 0x7FFFFFFF
		// KUSEG0: 0x80000000 - 0x9FFFFFFF
		// KUSEG1: 0xA0000000 - 0xBFFFFFFF
		// KUSEG2: 0xC0000000 - 0xFFFFFFFF
	enum class MemoryRegion {
		KSEG,
		KSEG0,
		KSEG1,
		KSEG2
	};

	public:
		Bus();
		~Bus();
		void step();
		void reset();
		void writeU8(uint32_t addr, uint8_t data);
		void writeU16(uint32_t addr, uint16_t data);
		void writeU32(uint32_t addr, uint32_t data);
		uint8_t readU8(uint32_t addr) const;
		uint16_t readU16(uint32_t addr) const;
		uint32_t readU32(uint32_t addr) const;
		void loadBiosfromFile(const char* filename);
		std::shared_ptr<MIPSR3000A> cpu() const { return this->cpu_; };
		Memory<kBiosSize> const& bios() { return this->bios_; }
		Memory<kMainRamSize> const& ram() { return this->main_ram_; }
		static bool isRamAddress(uint32_t effective_addr) { return effective_addr < 0x00200000; }
		static bool isBiosAddress(uint32_t effective_addr) { return effective_addr >= 0x1FC00000 && effective_addr < 0x1FC80000; }

		static bool isMemControlAddress(uint32_t effective_addr) { return effective_addr >= 0x1F801000 && effective_addr < 0x1F801024; }
		static bool isSPUAddress(uint32_t effective_addr) { return effective_addr >= 0x1F801D80 && effective_addr < 0x1F802000; }
		static bool isExpansion1Address(uint32_t effective_addr) { return effective_addr >= 0x1F000000 && effective_addr < 0x1F080000;  }
		static bool isRamSizeAddress(uint32_t effective_addr) { return effective_addr >= 0x1F801060 && effective_addr < 0x1F801064;  }
		static bool isExpansion2Address(uint32_t effective_addr) { return effective_addr >= 0x1F802000 && effective_addr < 0x1F802068;  }
		static bool isCacheControlAddress(uint32_t effective_addr) { return effective_addr >= 0xFFFE0130 && effective_addr < 0xFFFE0134;  }
		static bool isInterruptControlAddress(uint32_t effective_addr) { return effective_addr >= 0x1F801070 && effective_addr < 0x1F801078;  }
		static bool isTimerRegisterAddress(uint32_t effective_addr) { return effective_addr >= 0x1F801100 && effective_addr < 0x1F801130;  }

		uint32_t getEffectiveAddress(uint32_t addr, MemoryRegion mem_region) const;
		MemoryRegion getMemoryRegion(uint32_t addr) const;

	private:

		std::shared_ptr<MIPSR3000A> cpu_;

		// main ram size 2048KB
		Memory<kMainRamSize> main_ram_;

		// bios size 512KB
		Memory<kBiosSize> bios_;

		// Memory Control Registers
		Memory<36> mem_control_;
	};

}
