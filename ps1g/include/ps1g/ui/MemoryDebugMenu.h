#pragma once

namespace ps1g {
	class Bus;

	class MemoryDebugMenu {
	public:
		bool enabled = false;
		void render(Bus* bus);
	};
}
