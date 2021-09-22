// --------------------------------------------------------------------------
// Filename:	FastDrawingObject.h
// Class:		FastDrawingObject
// Purpose:		This class upgrades an entity image so that it can own
//				its own direct draw surfaces and can be drawn straight to
//				the display engine when updated.
//				
//				
//				
//
// Description: Based on Small Furry Creatures FastMouse
//								
//
// History:
// -------  
// Feb98	Alima			Created
//			
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "FastDrawingObject.h"
#include "EntityImage.h"
#include "DisplayEngine.h"
#include "../App.h"
#include "../World.h"
#include "DrawableObjectHandler.h"
#include "ErrorMessageHandler.h"

FastDrawingObject::FastDrawingObject()
:myPlane(0),
myFullScreenFlag(0)
{

}

FastDrawingObject::FastDrawingObject(int32 plane,
									 bool fullScreen)
:myPlane(plane),
myFullScreenFlag(fullScreen)
{
}

FastDrawingObject::~FastDrawingObject()
{

}

void FastDrawingObject::Destroy()
{
	DisplayEngine::theRenderer().FastObjectSigningOff(this);
}

void FastDrawingObject::Enable()
{
	DisplayEngine::theRenderer().TakeMeOffHold(this);
}


void FastDrawingObject::Disable()
{
	DisplayEngine::theRenderer().PutMeOnHold(this);
}

