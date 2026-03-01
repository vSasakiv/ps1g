#include "ps1g/Debugger.h"
#include "ps1g/Bus.h"
#include "ps1g/MIPSR3000A.h"
#include "ps1g/Memory.h"

namespace ps1g {

	void Debugger::step() {
		this->bus_.step();
	}

	void Debugger::reset() {
		this->bus_.reset();
	}

	void Debugger::resume() {
		for (;;) {
			this->bus_.step();
			uint32_t curr_effective_addr = this->bus_.getEffectiveAddress(this->readPc(), this->bus_.getMemoryRegion(this->readPc()));
			uint32_t offset;

			if (this->bus_.isRamAddress(curr_effective_addr)) {
				offset = curr_effective_addr;
				if (this->ramBreakpoints_.find(offset) != this->ramBreakpoints_.end()) {
					break;
				}
			} 
			else if (this->bus_.isBiosAddress(curr_effective_addr)) {
				offset = curr_effective_addr - kBiosOffset;
				if (this->biosBreakpoints_.find(offset) != this->biosBreakpoints_.end()) {
					break;
				}
			}
			else {
				std::cout << "not yet implemented" << std::endl;
				continue;
			}
		}
	}

	void Debugger::loadBiosFromPath(const char* filepath) {
		this->bus_.loadBiosfromFile(filepath);
	}

	uint32_t Debugger::readPc() {
		return this->bus_.cpu()->pc();
	}

	uint32_t Debugger::prevPc() {
		return this->bus_.cpu()->prev_pc();
	}

	uint32_t Debugger::currentInstruction() {
		return this->bus_.cpu()->fetched_next();
	}

	uint32_t Debugger::nextInstruction() {
		return this->bus_.readU32(this->readPc());
	}

	uint32_t Debugger::readHi() {
		return this->bus_.cpu()->hi();
	}

	uint32_t Debugger::readLo() {
		return this->bus_.cpu()->lo();
	}

	std::array<uint32_t, 32>const& Debugger::readGeneralRegs() {
		return this->bus_.cpu()->registers();
	}

	std::vector<MIPSR3000A::LoadDelay>& Debugger::getLoadDelayQueue() {
		return this->bus_.cpu()->load_delay_queue();
	}

	Memory<kBiosSize> const& Debugger::getBios() {
		return this->bus_.bios();
	};

	Memory<kMainRamSize> const& Debugger::getMainRam() {
		return this->bus_.ram();
	};

	bool Debugger::isBiosBreakpoint(uint32_t addr) {
		return (this->biosBreakpoints_.find(addr) != this->biosBreakpoints_.end());
	}
	bool Debugger::isRamBreakpoint(uint32_t addr) {
		return (this->ramBreakpoints_.find(addr) != this->ramBreakpoints_.end());
	}

	void Debugger::addBiosBreakpoint(uint32_t addr) {
		this->biosBreakpoints_.insert(addr);
	}

	void Debugger::addRamBreakpoint(uint32_t addr) {
		this->ramBreakpoints_.insert(addr);
	}

	void Debugger::removeBiosBreakpoint(uint32_t addr) {
		this->biosBreakpoints_.erase(addr);
	}

	void Debugger::removeRamBreakpoint(uint32_t addr) {
		this->ramBreakpoints_.erase(addr);
	}

	void Debugger::removeAllBreakpoints() {
		this->biosBreakpoints_.clear();
		this->ramBreakpoints_.clear();
	}

	std::set<uint32_t>& Debugger::getBiosBreakpoints() {
		return this->biosBreakpoints_;
	}

	std::set<uint32_t>& Debugger::getRamBreakpoints() {
		return this->ramBreakpoints_;
	}
}