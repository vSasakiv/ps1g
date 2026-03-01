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
	};
}
