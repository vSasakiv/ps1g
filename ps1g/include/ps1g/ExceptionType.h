#pragma once
#include <stdint.h>

namespace ps1g {
	enum class ExceptionType : uint32_t {
		LoadAddressError = 0x4,
		StoreAddressError = 0x5,
		Syscall = 0x8,
		Break =	0x9,
		ArithmeticOverflow = 0xC,
	};
}
