#pragma once
#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <algorithm>

namespace ps1g {

	class Bus;

	class MIPSR3000A {

	public:
		struct Instruction {
			uint32_t instruction_raw;
			Instruction(uint32_t instruction_raw) : instruction_raw(instruction_raw) {};
			uint32_t getPrimaryOpcode() const { return (this->instruction_raw >> 26) & 0x3F; };
			uint32_t getSecondaryOpcode() const { return this->instruction_raw & 0x3F; };
			uint32_t getRs() const { return(this->instruction_raw >> 21) & 0x1F; };
			uint32_t getRt() const { return(this->instruction_raw >> 16) & 0x1F; };
			uint32_t getRd() const { return(this->instruction_raw >> 11) & 0x1F; };
			uint32_t getImm5() const { return(this->instruction_raw >> 6) & 0x1F; };
			uint32_t getImm16() const { return this->instruction_raw & 0xFFFF; };
			uint32_t getImm16Signed() const { return (int16_t)(this->instruction_raw & 0xFFFF); };
			uint32_t getImm26() const { return this->instruction_raw & 0x3FFFFFF; };
		};

		struct LoadDelay {
			uint32_t reg;
			uint32_t value;
			size_t remaining_duration;
			LoadDelay(uint32_t reg, uint32_t value, size_t remaining_duration) : 
				reg(reg), value(value), remaining_duration(remaining_duration) {};
		};

		struct CP0 {
			uint32_t system_status;
			uint32_t cause;
			CP0() : system_status(0), cause(0) {};
			void reset() { this->system_status = 0; this->cause = 0; };
		};

	public:
		MIPSR3000A();
		~MIPSR3000A(); 
		void reset();
		void step(Bus& bus);
		std::array<uint32_t, 32>const& registers() const { return this->registers_;}
		std::vector<LoadDelay>& load_delay_queue() { return this->load_delay_queue_;  }
		uint32_t pc() const { return this->pc_;  }
		uint32_t next_pc() const { return this->next_pc_;  }
		uint32_t hi() const { return this->hi_;  }
		uint32_t lo() const { return this->lo_;  }

	private:

		uint32_t pc_, next_pc_, hi_, lo_;
		CP0 cop0;
		std::array<uint32_t, 32> registers_;
		std::vector<LoadDelay> load_delay_queue_;
		void writeReg(size_t reg, uint32_t value);
		uint32_t readReg(size_t reg);

		void writeLo(uint32_t value);
		uint32_t readLo();
		void writeHi(uint32_t value);
		uint32_t readHi();

		void cop0Execute(Instruction& instruction);
		void evaluateLoadDelays();
	};
}
