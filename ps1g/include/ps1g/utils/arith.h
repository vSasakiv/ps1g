#pragma once

#include <stdint.h>

namespace ps1g {
	inline uint32_t sized_wrapping_add(uint32_t a, uint32_t b, uint32_t max_value)
	{
		uint64_t sum = (uint64_t)a + (uint64_t)b;
		return (uint32_t)(sum % (max_value + 1));
	}

	inline bool add_signed_overflow(int32_t a, int32_t b) {
		int32_t res = a + b;
		return ((a ^ res) & (b ^ res)) < 0;
	}
}
