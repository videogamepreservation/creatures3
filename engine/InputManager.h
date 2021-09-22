// -------------------------------------------------------------------------
// Filename:	InputManager.h
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

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H


#include "InputEvent.h"

#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include <vector>

class CreaturesArchive;

class TranslatedCharTarget
{
public:
	virtual bool SendChar( int keyCode ) {return false;}
	virtual bool LoseFocus() {return false;}
	virtual void GainFocus() {}
	virtual ~TranslatedCharTarget();
	void SaveFocusState( CreaturesArchive &archive ) const;
	void RestoreFocusState( CreaturesArchive &archive );
};


class InputManager
{
public:
	InputManager();

	// immediate-query stuff stuff
	bool IsKeyDown( int keycode );
	int GetMouseX();
	int GetMouseY();
	// mouse velocity estimates
	float GetMouseVX();
	float GetMouseVY();

	// move the mouse
	void SetMousePosition(int newX, int newY);

	// event reading...
	int GetPendingMask();
	int GetEventCount();
	const InputEvent& GetEvent( int i );

	// system-framework access:
	void SysFlushEventBuffer();
	void SysAddKeyDownEvent( int keycode );
	void SysAddKeyUpEvent( int keycode );
	void SysAddTranslatedCharEvent( int keycode );
	void SysAddMouseMoveEvent( int mx, int my );
	void SysAddMouseDownEvent( int mx, int my, int button );
	void SysAddMouseUpEvent( int mx, int my, int button );
	void SysAddMouseWheelEvent( int mx, int my, int delta );

	TranslatedCharTarget *GetTranslatedCharTarget() const;
	void SetTranslatedCharTarget( TranslatedCharTarget *target, bool tellLoser = true );
private:
	std::vector< InputEvent > myEventBuffer;
	int myEventPendingMask;

	// latest position (i.e. at end of input event processing
	// for this tick, which means at the start)
	int myMouseX;
	int myMouseY;

	// mouse position from the previous tick
	static const int ourRecentPositions;
	static const int ourVelocityAgo;
	int myRecentPos; // index of position at end of last tick
	std::vector< int > myRecentMouseX;
	std::vector< int > myRecentMouseY;

	TranslatedCharTarget *myTranslatedCharTarget;
};

inline int InputManager::GetMouseX()
	{ return myMouseX; }

inline int InputManager::GetMouseY()
	{ return myMouseY; }





#endif // INPUTMANAGER_H
