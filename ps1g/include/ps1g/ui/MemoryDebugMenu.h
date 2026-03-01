#pragma once

namespace ps1g {
	class Debugger;

	class MemoryDebugMenu {
	public:
		bool enabled = false;
		void render(Debugger& debugger) const;
	};
}
