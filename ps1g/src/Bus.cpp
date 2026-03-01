#include "ps1g/Bus.h"
#include <iostream>
#include <ps1g/MIPSR3000A.h>
#include <ps1g/utils/mem.h>

namespace ps1g {

	Bus::Bus() {
		this->cpu_ = new MIPSR3000A();
	}

	Bus::~Bus() {
		delete this->cpu_;
	}

	void Bus::reset() {
		this->cpu_->reset();
	}

	void Bus::step() {
		this->cpu_->step(*this);
	}

	MIPSR3000A* Bus::cpu() {
		return this->cpu_;
	}

	void Bus::writeU8(uint32_t addr, uint8_t data) {
		MemoryRegion region = this->getMemoryRegion(addr);
		uint32_t effective_addr = this->getEffectiveAddress(addr, region);

		if (Bus::isRamAddress(effective_addr)) {
			this->main_ram_.writeU8(effective_addr, data);
		}
		else if (Bus::isExpansion2Address(effective_addr)) {
			std::cout << "Write to Expansion 2 not yet implemented" << std::endl;
		}
		else {
			std::cout << "Unmapped address: " << std::hex << addr << std::dec << std::endl;
			throw std::runtime_error("Write to unmapped address");
		}

	}

	void Bus::writeU16(uint32_t addr, uint16_t data) {
		if (addr % 2 != 0) {
			std::cout << "Unaligned store half-word access at address " << std::hex << addr << std::dec << std::endl;
		}
		MemoryRegion region = this->getMemoryRegion(addr);
		uint32_t effective_addr = this->getEffectiveAddress(addr, region);

		if (Bus::isRamAddress(effective_addr)) {
			this->main_ram_.writeU16(effective_addr, data);
		}
		else if (Bus::isSPUAddress(effective_addr)) {
			std::cout << "Write to SPU Registers not yet implemented" << std::endl;
		}
		else {
			std::cout << "Unmapped address: " << std::hex << addr << std::dec << std::endl;
			throw std::runtime_error("Write to unmapped address");
		}

	}

	void Bus::writeU32(uint32_t addr, uint32_t data) {
		if (addr % 4 != 0) {
			std::cout << "Unaligned store word access at address " << std::hex << addr << std::dec << std::endl;
		}
		MemoryRegion region = this->getMemoryRegion(addr);
		uint32_t effective_addr = this->getEffectiveAddress(addr, region);

		if (Bus::isRamAddress(effective_addr)) {
			this->main_ram_.writeU32(effective_addr, data);
		}
		else if (Bus::isMemControlAddress(effective_addr)) {
			std::cout << "Write to Memory Control 1 not yet implemented" << std::endl;
		}
		else if (Bus::isRamSizeAddress(effective_addr)) {
			std::cout << "Write to Ram Size not yet implemented" << std::endl;
		}
		else if (Bus::isBiosAddress(effective_addr)) {
			std::cout << "Tried writing to bios at address: " << std::hex << addr << std::dec << std::endl;
		}
		else if (Bus::isCacheControlAddress(effective_addr)) {
			std::cout << "Write to cache control not yet implemented" << std::endl;
		}
		else {
			std::cout << "Unmapped address: " << std::hex << addr << std::dec << std::endl;
			throw std::runtime_error("Write to unmapped address");
		}

	}

	uint8_t Bus::readU8(uint32_t addr) {
		MemoryRegion region = this->getMemoryRegion(addr);
		uint32_t effective_addr = this->getEffectiveAddress(addr, region);

		if (Bus::isRamAddress(effective_addr)) {
			return this->main_ram_.readU8(effective_addr);
		}
		else if (Bus::isBiosAddress(effective_addr)) {
			return this->bios_.readU8(effective_addr - kBiosOffset);
		}
		else if (Bus::isExpansion1Address(effective_addr)) {
			std::cout << "Tried reading from Expansion 1 address, return default 0xFF: " << std::hex << addr << std::dec << std::endl;
			return 0xFF;
		}
		else {
			std::cout << "Tried reading from unmapped address: " << std::hex << addr << std::dec << std::endl;
			throw std::runtime_error("Read from unmapped address");
		}
	}

	uint16_t Bus::readU16(uint32_t addr) {
		if (addr % 2 != 0) {
			std::cout << "Unaligned load half-word access at address " << std::hex << addr << std::dec << std::endl;
		}

		MemoryRegion region = this->getMemoryRegion(addr);
		uint32_t effective_addr = this->getEffectiveAddress(addr, region);

		if (Bus::isRamAddress(effective_addr)) {
			return this->main_ram_.readU16(effective_addr);
		}
		else {
			std::cout << "Tried reading from unmapped address: " << std::hex << addr << std::dec << std::endl;
			throw std::runtime_error("Read from unmapped address");
		}
	}

	uint32_t Bus::readU32(uint32_t addr) {
		if (addr % 4 != 0) {
			std::cout << "Unaligned load word access at address " << std::hex << addr << std::dec << std::endl;
		}
		MemoryRegion region = this->getMemoryRegion(addr);
		uint32_t effective_addr = this->getEffectiveAddress(addr, region);

		if (Bus::isRamAddress(effective_addr)) {
			return this->main_ram_.readU32(effective_addr);
		}
		else if (Bus::isMemControlAddress(effective_addr)) {
			std::cout << "Read of Mem Control not yet implemented" << std::endl;
			// return this->mem_control_.readU32(effective_addr - kMemoryControlOffset);
		}
		else if (Bus::isRamSizeAddress(effective_addr)) {
			std::cout << "Read of Ram Size not yet implemented" << std::endl;
		}
		else if (Bus::isBiosAddress(effective_addr)) {
			return this->bios_.readU32(effective_addr - kBiosOffset);
		}
		else if (Bus::isCacheControlAddress(effective_addr)) {
			std::cout << "Read of cache control not yet implemented" << std::endl;
		}
		else {
			std::cout << "Tried reading from unmapped address: " << std::hex << addr << std::dec << std::endl;
			throw std::runtime_error("Read from unmapped address");
		}
	}

	void Bus::loadBiosfromFile(const char* filename) {
		setMemoryFromFile<512 * 1024>(this->bios_, filename);
	}

	Bus::MemoryRegion Bus::getMemoryRegion(uint32_t addr) {
		if (addr < 0x80000000) {
			return MemoryRegion::KSEG;
		} else if (addr < 0xA0000000) {
			return MemoryRegion::KSEG0;
		} else if (addr < 0xC0000000) {
			return MemoryRegion::KSEG1;
		} else {
			return MemoryRegion::KSEG2;
		}
	}

	uint32_t Bus::getEffectiveAddress(uint32_t addr, MemoryRegion mem_region) {
		switch (mem_region) {
			case MemoryRegion::KSEG:
				return addr;
			case MemoryRegion::KSEG0:
				return addr - 0x80000000;
			case MemoryRegion::KSEG1:
				return addr - 0xA0000000;
			case MemoryRegion::KSEG2:
				return addr;
			default:
				std::cout << "Memory Region not recognized" << std::endl;
				return addr;
		}
	}

}