// main for sdl version (derived from window.h/window.cpp)

#ifndef SDL_MAIN_H
#define SDL_MAIN_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

//#include <windows.h>
#include "../../../common/C2eTypes.h"


#include "../../Agents/AgentHandle.h"

//extern HWND theMainWindow;

// tell the game to shut down
void SignalTerminateApplication();


void SetSingleStepAgent( AgentHandle& agent );
AgentHandle GetSingleStepAgent();
void WaitForSingleStepCommand();

#endif // OURWINDOW_H
