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

	enum class LogLevel;

	class Debugger {
	public:
		Debugger(Bus& bus) : bus_(bus), is_resumed_(false), last_message_(""), last_message_level_() {};
		bool step();
		void reset();
		void resume();
		void stop();
		void execute();
		bool isResumed();

		void loadBiosFromPath(const char* filepath);

		uint32_t readPc();
		uint32_t nextPc();

		uint32_t currentInstruction();
		uint32_t nextInstruction();

		uint32_t readHi();
		uint32_t readLo();

		uint32_t readCp0(uint32_t reg) const;

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

		std::string_view getLastMessage() const;
		LogLevel getLastMessageLevel() const;

	private:
		Bus& bus_;
		bool is_resumed_;

		std::string last_message_;
		LogLevel last_message_level_;

		std::set<uint32_t> biosBreakpoints_;
		std::set<uint32_t> ramBreakpoints_;
	};
}
