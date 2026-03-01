// ps1g.cpp: definisce il punto di ingresso dell'applicazione.
//
#include "ps1g.h"
#include "ps1g/Bus.h"
#include "ps1g/Debugger.h"
#include <iostream>
#include <fstream>
#include <memory>

#include "include/ps1g/ui/UiManager.h"

int main()
{
	std::ofstream out("emulator_log.txt");
    std::streambuf* cout_buffer = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

	auto ps1 = new ps1g::Bus();
    auto debugger = new ps1g::Debugger(*ps1);
    ps1g::UiManager uiManager;

	uiManager.Initialize();

	while (uiManager.IsRunning()) {
        uiManager.BeginFrame();
        uiManager.RenderUI(*debugger);
        uiManager.EndFrame();
    }
    
    std::cout.rdbuf(cout_buffer);

	return 0;
}
