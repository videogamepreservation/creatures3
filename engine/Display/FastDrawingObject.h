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
// Description: 
//								
//
// History:
// -------  
// Feb98	Alima			Created
//			
// --------------------------------------------------------------------------

#ifndef FAST_DRAWING_OBJECT_H
#define FAST_DRAWING_OBJECT_H

#ifndef C2E_SDL
#pragma		warning(disable:4201)
#include	<ddraw.h>
#pragma		warning(default:4201)
#endif

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif



#include "../../common/C2eTypes.h"
#include	"../../common/BasicException.h"

class EntityImage;
class DrawableObjectHandler;

class FastDrawingObject
{
public:
	FastDrawingObject();
	FastDrawingObject(int32 plane,
						bool fullScreen);

	virtual ~FastDrawingObject();

	virtual bool Update()  = 0;
	virtual void DrawToBackBuffer(DrawableObjectHandler& entityHandler) = 0;
	virtual void MakeOrphan(){;}
	void Destroy();

	void Enable();
	void Disable();

	int32 GetPlane(){return myPlane;}

//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
	class FastDrawingObjectException: public BasicException
	{
	public:
		FastDrawingObjectException(std::string what, uint16 line):
		  BasicException(what.c_str()),
		lineNumber(line){;}

		uint16 LineNumber(){return lineNumber;}
	private:

		uint16 lineNumber;

	};
protected:


	// Copy constructor and assignment operator
	// Declared but not implemented
	FastDrawingObject (const FastDrawingObject&);
	FastDrawingObject& operator= (const FastDrawingObject&);

	bool	myFullScreenFlag;
	int32 myPlane;
};

#endif //FAST_DRAWING_OBJECT_H