#pragma once
#include <array>
#include <string>
#include <functional>

namespace ps1g {
	class Debugger;
	class CpuDebugMenu {
	public:
		bool enabled = false;
		void render(Debugger& debugger) const;

		std::function<void(std::string&, std::array<float, 4>&)> setStatusMessage;

	private:
		static constexpr std::array<const char*, 32> register_names_ = {
			"zero (r0 )",
			"at   (r1 )",
			"v0   (r2 )",
			"v1   (r3 )",
			"a0   (r4 )",
			"a1   (r5 )",
			"a2   (r6 )",
			"a3   (r7 )",
			"t0   (r8 )",
			"t1   (r9 )",
			"t2   (r10)",
			"t3   (r11)",
			"t4   (r12)",
			"t5   (r13)",
			"t6   (r14)",
			"t7   (r15)",
			"s0   (r16)",
			"s1   (r17)",
			"s2   (r18)",
			"s3   (r19)",
			"s4   (r20)",
			"s5   (r21)",
			"s6   (r22)",
			"s7   (r23)",
			"t8   (r24)",
			"t9   (r25)",
			"k0   (r26)",
			"k1   (r27)",
			"gp   (r28)",
			"sp   (r29)",
			"fp   (r30)",
			"ra   (r31)"
		};

		static constexpr std::array<const char*, 3> cop0_register_names_ = {
			"System Status (r12)",
			"Cause (r13)",
			"Exception PC (r14)",
		};

		static constexpr std::array<const char*, 32> cop0_system_register_description_ = {
			"Current Interrupt Enable",
			"Current Kernel/User Mode",
			"Previous Interrupt Disable",
			"Previous Kernel/User Mode",
			"Old Interrupt Disable",
			"Old Kernel/User Mode",
			"Not Used",
			"Not Used",
			"8bit Interrupt mask field",
			"8bit Interrupt mask field",
			"8bit Interrupt mask field",
			"8bit Interrupt mask field",
			"8bit Interrupt mask field",
			"8bit Interrupt mask field",
			"8bit Interrupt mask field",
			"8bit Interrupt mask field",
			"Isolate Cache",
			"Swapped Cache Mode",
			"Cache Parity as 0",
			"Result of last Load with D-cache Isolated",
			"Cache parity Error",
			"TLB Shutdown",
			"Boot exception vector",
			"Not Used",
			"Not Used",
			"Reverse Endianess",
			"Not Used",
			"Not Used",
			"COP0 enable",
			"COP1 enable",
			"COP2 enable",
			"COP3 enable",
		};

		static constexpr std::array<const char*, 32> cop0_cause_register_description_ = {
			"Not Used",
			"Not Used",
			"Exception Occoured",
			"Exception Occoured",
			"Exception Occoured",
			"Exception Occoured",
			"Exception Occoured",
			"Not Used",
			"Interrupt Pending Field",
			"Interrupt Pending Field",
			"Interrupt Pending Field",
			"Interrupt Pending Field",
			"Interrupt Pending Field",
			"Interrupt Pending Field",
			"Interrupt Pending Field",
			"Interrupt Pending Field",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"Not Used",
			"COP Opcode",
			"COP Opcode",
			"Not Used",
			"Branch Delay"
		};



	};
}
