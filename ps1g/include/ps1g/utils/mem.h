#pragma once

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <ps1g/Memory.h>

namespace ps1g {

	template <size_t Size>
	void setMemoryFromFile(Memory<Size>& memory, const char* filename) {

		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		if (!file) {
			std::cerr << "Error opening file: " << filename << std::endl;
			return;
		}

		std::streamsize size = file.tellg();
		if (size != Size) {
			std::cerr << "File size does not match Memory Size: " << filename << std::endl;
			return;
		}

		file.seekg(0, std::ios::beg);

		if (file.read(reinterpret_cast<char*>(memory.getMemoryArray()->data()), size)) {
			std::cout << "Successfully loaded file into memory: " << filename << std::endl;
		} else {
			std::cerr << "Error reading file: " << filename << std::endl;
		}
	};
}
