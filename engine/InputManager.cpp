// -------------------------------------------------------------------------
// Filename:	InputManager.cpp
// Class:		InputManager
// Purpose:		Provide an independant mechanism for handling input devices.
// Description:
//
// Coming soon to a header file near you!
//
//
// Usage:
//
//
//
// History:
// 26Jan99		Creation	BenC
// -------------------------------------------------------------------------


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "InputManager.h"
#include "C2eServices.h"
#include "App.h"
#include "CreaturesArchive.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>	// for GetAsyncKeyState()
#endif

// how many ticks we store mouse position for
const int InputManager::ourRecentPositions = 3;
// how many ticks ago velocity is measured from
const int InputManager::ourVelocityAgo = 3; 

InputManager::InputManager()
{
	myEventPendingMask = 0;
	myMouseX = 0;
	myMouseY = 0;

	ASSERT(ourVelocityAgo <= ourRecentPositions);
	myRecentPos = 0;
	myRecentMouseY.resize(ourRecentPositions);
	myRecentMouseX.resize(ourRecentPositions);
	for (int i = 0; i < ourRecentPositions; ++i)
	{
		myRecentMouseX[i] = 0;
		myRecentMouseY[i] = 0;
	}

	myTranslatedCharTarget = 0;
}


int InputManager::GetEventCount()
{
	return myEventBuffer.size();
}

int InputManager::GetPendingMask()
{
	return myEventPendingMask;
}


const InputEvent& InputManager::GetEvent( int i )
{
	return myEventBuffer[i];
}


// system-framework access:
void InputManager::SysFlushEventBuffer()
{
	// store mouse move information for velocity
	myRecentPos++;
	if (myRecentPos >= ourRecentPositions)
		myRecentPos = 0;
	myRecentMouseX[myRecentPos] = myMouseX;
	myRecentMouseY[myRecentPos] = myMouseY;

	myEventBuffer.clear();
	myEventPendingMask = 0;
}


void InputManager::SysAddKeyDownEvent( int keycode )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventKeyDown;
	ev.KeyData.keycode = keycode;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventKeyDown;
}

void InputManager::SysAddKeyUpEvent( int keycode )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventKeyUp;
	ev.KeyData.keycode = keycode;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventKeyUp;
}

void InputManager::SysAddTranslatedCharEvent( int keycode )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventTranslatedChar;
	ev.KeyData.keycode = keycode;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventTranslatedChar;
}

void InputManager::SysAddMouseDownEvent( int mx, int my, int button )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseDown;
	ev.MouseButtonData.button = button;
	ev.MouseButtonData.mx = mx;
	ev.MouseButtonData.my = my;
	myEventBuffer.push_back( ev );

	myMouseX = mx;
	myMouseY = my;

	myEventPendingMask |= InputEvent::eventMouseDown;
}

void InputManager::SysAddMouseUpEvent( int mx, int my, int button )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseUp;
	ev.MouseButtonData.button = button;
	ev.MouseButtonData.mx = mx;
	ev.MouseButtonData.my = my;
	myEventBuffer.push_back( ev );

	myMouseX = mx;
	myMouseY = my;
	myEventPendingMask |= InputEvent::eventMouseUp;
}

void InputManager::SysAddMouseMoveEvent( int mx, int my )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseMove;
	ev.MouseMoveData.mx = mx;
	ev.MouseMoveData.my = my;
	myEventBuffer.push_back( ev );

	myMouseX = mx;
	myMouseY = my;
	myEventPendingMask |= InputEvent::eventMouseMove;
}


void InputManager::SysAddMouseWheelEvent( int mx, int my, int delta )
{
	InputEvent ev;
	ev.EventCode = InputEvent::eventMouseWheel;
	ev.MouseWheelData.delta = delta;
	ev.MouseWheelData.mx = mx;
	ev.MouseWheelData.my = my;
	myEventBuffer.push_back( ev );

	myEventPendingMask |= InputEvent::eventMouseWheel;
}

bool InputManager::IsKeyDown( int keycode )
{
#ifdef WIN32
	// keys are only down if we have the focus
	// no window has the mouse captured (which happens
	// if we are resizing/moving with the keyboard)
	if (::GetForegroundWindow() == theMainWindow &&
		::GetCapture() == NULL)
		return ( GetAsyncKeyState( keycode ) < 0 );
	else
		return false;
#else
	#warning keyboard polling not implemented
	return false;
#endif // WIN32
}

void InputManager::SetMousePosition(int newX, int newY)
{
#ifdef _WIN32
	POINT pt;
	pt.x = newX;
	pt.y = newY;
	ClientToScreen(theMainWindow, &pt);
	SetCursorPos(pt.x, pt.y);
#else
	#warning "TODO: implement SetMousePosition() if possible"
#endif
	myMouseX = newX;
	myMouseY = newY;
}

float InputManager::GetMouseVX()
{
	int velo_pos = (myRecentPos + 1 - ourVelocityAgo + ourRecentPositions) % ourRecentPositions;
	return (float)(myMouseX - myRecentMouseX[velo_pos]) / (float)ourVelocityAgo;
}

float InputManager::GetMouseVY()
{
	int velo_pos = (myRecentPos + 1 - ourVelocityAgo + ourRecentPositions) % ourRecentPositions;
	return (float)(myMouseY - myRecentMouseY[velo_pos]) / (float)ourVelocityAgo;
}

TranslatedCharTarget *InputManager::GetTranslatedCharTarget() const
{
	return myTranslatedCharTarget;
}

void InputManager::SetTranslatedCharTarget( TranslatedCharTarget *target, bool tellLoser )
{
	TranslatedCharTarget *oldTarget = myTranslatedCharTarget;
	myTranslatedCharTarget = target;
	if( tellLoser && oldTarget )
		oldTarget->LoseFocus();
	if (myTranslatedCharTarget)
		myTranslatedCharTarget->GainFocus();
}

TranslatedCharTarget::~TranslatedCharTarget()
{
	InputManager& inputManager = theApp.GetInputManager();
	if( inputManager.GetTranslatedCharTarget() == this )
		inputManager.SetTranslatedCharTarget( NULL, false );
}

void TranslatedCharTarget::SaveFocusState( CreaturesArchive &archive ) const
{
	archive << bool( theApp.GetInputManager().GetTranslatedCharTarget() == this );
}

void TranslatedCharTarget::RestoreFocusState( CreaturesArchive &archive )
{
	bool flag;
	archive >> flag;
	if( flag ) theApp.GetInputManager().SetTranslatedCharTarget( this, true );
}

