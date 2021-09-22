// -------------------------------------------------------------------------
// Filename:    Window.h
// Class:       None
// Purpose:     Main window and message pump stuff
// Description:
//				Window.cpp and Window.h provide the basic framework
//				services used by the game:
//				- The Main Window
//				- Timing
//				- Mouse and keyboard input
//
//				The single App object is probably about the only class
//				which should interact with Window.h and Window.cpp...
//
// Usage:
//
//
// History:
// -------------------------------------------------------------------------
#ifndef OURWINDOW_H
#define OURWINDOW_H

#ifdef C2E_SDL
	// redirect to appropriate include file.
	#include "SDL/SDL_Main.h"
#else
// rest of file for normal windows version


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <windows.h>
#include "../../common/c2etypes.h"
#include "../Agents/AgentHandle.h"
#include "../TimeFuncs.h"

extern HWND theMainWindow;
void ResetWindowTitle();

bool SendTickMessage();
bool SetMultimediaTimer(bool bActive);
bool GetMultimediaTimer();

// Moved time functions out into TimeFuncs.h  - 29Nov99 BenC 
//int GetTimeStamp(); // accurate to milliseconds
//int64 GetHighPerformanceTimeStamp(); // as accurate as possible!
//int64 GetHighPerformanceTimeStampFrequency(); // in counts per second
// seconds since midnight (00:00:00), January 1, 1970 UTC
//uint32 GetRealWorldTime();

void SignalTerminateApplication();

// provides a means to single step through _every_ line of code executed
// by an agent, even if several are run in one tick
void SetSingleStepAgent(AgentHandle& agent);
AgentHandle GetSingleStepAgent();
void WaitForSingleStepCommand();

extern AgentHandle waitingForAgent;
inline bool CheckSingleStepAgent(AgentHandle& agent)
{
	return agent == waitingForAgent && agent.IsValid();
}

#endif // end of C2E_SDL guard

#endif // OURWINDOW_H
