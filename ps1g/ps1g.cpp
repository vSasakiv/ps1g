// ps1g.cpp: definisce il punto di ingresso dell'applicazione.
//
#include "ps1g.h"
#include "ps1g/Bus.h"
#include <iostream>
#include <fstream>
#include <memory>

#include "include/ps1g/ui/UiManager.h"

int main()
{

	auto ps1 = new ps1g::Bus();
    ps1g::UiManager uiManager;

	uiManager.Initialize();

	while (uiManager.IsRunning()) {
        uiManager.BeginFrame();
        uiManager.RenderUI(ps1);
        uiManager.EndFrame();
    }

	return 0;
}
