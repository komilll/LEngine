#include "engineController.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	EngineController* engine;

	// Create the system object.
	engine = new EngineController;
	if (!engine)
	{
		return 0;
	}

	// Initialize and run the system object.
	if (!engine->Initialize())
		return 0;

	engine->Run();

	// Shutdown and release the system object.
	engine->Shutdown();
	delete engine;
	engine = 0;

	return 0;
}