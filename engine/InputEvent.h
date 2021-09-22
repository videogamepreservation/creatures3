// -------------------------------------------------------------------------
// Filename:    InputEvent.h
// Class:       InputEvent struct (no cpp file)
// Purpose:     struct for storing input events
// Description:
// Should probably be turned into a proper class at some point, but I think
// easier to understand as a struct at the moment...
//
// Usage:
//
//
// History:
// 12Feb99	  BenC	Initial version
// -------------------------------------------------------------------------


#ifndef INPUTEVENT_H
#define INPUTEVENT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


struct InputEvent
{
	// event types
	enum{ eventKeyDown=1, eventKeyUp=2, eventMouseMove=4,
		eventMouseDown=8, eventMouseUp=16, eventMouseWheel=32,
		eventTranslatedChar=64};

	// mouse buttons
	enum{ mbLeft=1, mbRight=2, mbMiddle=4 };

	// type of event
	int EventCode;

	// mouse position at time of event (framework coordinates)

	// mouse velocity at time of event.
	// What units? Pixels per millisecond?
	// not yet implemented.
//	int MouseVX;
//	int MountVY;

	// event type-specific data
	union
	{
		struct { int mx; int my; } MouseMoveData;
		struct { int mx; int my; int button;  } MouseButtonData;	// which mouse button
		struct { int mx; int my; int delta; } MouseWheelData;		// amount moved
		struct { int keycode; } KeyData;							// which key
	};

	// timestamp?
};

#endif // INPUTEVENT_H

