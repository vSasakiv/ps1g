#include "ps1g/Debugger.h"
#include "ps1g/Bus.h"
#include "ps1g/MIPSR3000A.h"
#include "ps1g/Memory.h"
#include "ps1g/utils/loglevel.h"

namespace ps1g {

	bool Debugger::step() {
		try {
			this->bus_.step();
			uint32_t curr_effective_addr = this->bus_.getEffectiveAddress(this->readPc(), this->bus_.getMemoryRegion(this->readPc()));
			uint32_t offset;

			if (this->bus_.isRamAddress(curr_effective_addr)) {
				offset = curr_effective_addr;
				if (this->ramBreakpoints_.find(offset) != this->ramBreakpoints_.end()) {
					this->stop();
					return false;
				}
			}
			else if (this->bus_.isBiosAddress(curr_effective_addr)) {
				offset = curr_effective_addr - kBiosOffset;
				if (this->biosBreakpoints_.find(offset) != this->biosBreakpoints_.end()) {
					this->stop();
					return false;
				}
			}
			else {
				std::cout << "not yet implemented" << std::endl;
				this->stop();
				return false;
			}
		}
		catch (const std::runtime_error& e) {
			this->last_message_ = e.what();
			this->last_message_level_ = LogLevel::Error;
			this->stop();
			this->bus_.cpu()->rollBack();
			return false;
		}
		return true;
	}

	void Debugger::reset() {
		this->bus_.reset();
	}

	void Debugger::execute() {
		if (this->is_resumed_)
			for (int i = 0; i < 10000; i++)
				if (!this->step())
					break;
	}

	void Debugger::resume() {
		this->is_resumed_ = true;
	}

	void Debugger::stop() {
		this->is_resumed_ = false;
	}

	bool Debugger::isResumed() {
		return this->is_resumed_;
	}

	void Debugger::loadBiosFromPath(const char* filepath) {
		this->bus_.loadBiosfromFile(filepath);
		this->last_message_ = "Bios Loaded Successfully";
		this->last_message_level_ = LogLevel::Success;
	}

	uint32_t Debugger::readPc() {
		return this->bus_.cpu()->pc();
	}

	uint32_t Debugger::nextPc() {
		return this->bus_.cpu()->next_pc();
	}

	uint32_t Debugger::currentInstruction() {
		return this->bus_.readU32(this->readPc());
	}

	uint32_t Debugger::nextInstruction() {
		return this->bus_.readU32(this->nextPc());
	}

	uint32_t Debugger::readHi() {
		return this->bus_.cpu()->hi();
	}

	uint32_t Debugger::readLo() {
		return this->bus_.cpu()->lo();
	}

	uint32_t Debugger::readCp0(uint32_t reg) const {
		return this->bus_.cpu()->readCP0(reg);
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

	std::string_view Debugger::getLastMessage() const { 
		return this->last_message_; 
	}

	LogLevel Debugger::getLastMessageLevel() const { 
		return this->last_message_level_; 
	}

}