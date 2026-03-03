#pragma once
#include <stdint.h>
#include <array>
#include <ps1g/utils/constants.h>
#include <ps1g/MIPSR3000A.h>
#include <set>

namespace ps1g {
	class Bus;

	template <size_t Size>
	class Memory;

	class Debugger {
	public:
		Debugger(Bus& bus) : bus_(bus) {};
		void step();
		void reset();
		void resume();

		void loadBiosFromPath(const char* filepath);

		uint32_t readPc();
		uint32_t nextPc();

		uint32_t currentInstruction();
		uint32_t nextInstruction();

		uint32_t readHi();
		uint32_t readLo();

		std::array<uint32_t, 32>const& readGeneralRegs();
		std::vector<MIPSR3000A::LoadDelay>& getLoadDelayQueue();

		Memory<kBiosSize> const& getBios();
		Memory<kMainRamSize> const& getMainRam();

		bool isBiosBreakpoint(uint32_t addr);
		bool isRamBreakpoint(uint32_t addr);

		void addBiosBreakpoint(uint32_t addr);
		void addRamBreakpoint(uint32_t addr);
		void removeBiosBreakpoint(uint32_t addr);
		void removeRamBreakpoint(uint32_t addr);

		void removeAllBreakpoints();

		std::set<uint32_t>& getBiosBreakpoints();
		std::set<uint32_t>& getRamBreakpoints();


	private:
		Bus& bus_;
		std::set<uint32_t> biosBreakpoints_;
		std::set<uint32_t> ramBreakpoints_;
	};
}
