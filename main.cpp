#include <windows.h>

#include "Game.h"

int WINAPI WinMain(HINSTANCE   hInstance,    
	HINSTANCE    hPrevInstance,     
	LPSTR        lpCmdLine,         
	int          nCmdShow) {

	Game game;
	game.run();

	return 1;
}