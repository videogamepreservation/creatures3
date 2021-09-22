// --------------------------------------------------------------------------
// Filename:	FastEntityImage.h
// Class:		FastEntityImage
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


#ifdef C2E_SDL
// diversion!
#include "SDL/SDL_FastEntityImage.h"
#else
// we now return to our scheduled program


#ifndef FAST_ENTITY_IMAGE_H
#define FAST_ENTITY_IMAGE_H



#pragma		warning(disable:4201)
#include	<ddraw.h>
#pragma		warning(default:4201)
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "../../common/C2eTypes.h"
#include	"../../common/BasicException.h"
#include "FastDrawingObject.h"

class EntityImage;
class DrawableObjectHandler;

class FastEntityImage :public FastDrawingObject
{
public:
	FastEntityImage();
	FastEntityImage(EntityImage& entity,
						IDirectDrawSurface4* image,
						IDirectDrawSurface4* background,
						int32 plane,
						bool fullScreen);

	virtual ~FastEntityImage();

	virtual bool Update();
	virtual void DrawToBackBuffer(DrawableObjectHandler& entityHandler);
	RECT GetBound();
	RECT GetLastBound();
	virtual void MakeOrphan();

//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
	class FastEntityImageException: public BasicException
	{
	public:
		FastEntityImageException(std::string what, uint16 line):
		  BasicException(what.c_str()),
		lineNumber(line){;}

		uint16 LineNumber(){return lineNumber;}
	private:

		uint16 lineNumber;

	};
private:
	bool MoveTo(int32 x, int32 y);
	bool WorkOutClippedArea(RECT& clip,
							int32& x,
							int32& y);

	// Copy constructor and assignment operator
	// Declared but not implemented
	FastEntityImage (const FastEntityImage&);
	FastEntityImage& operator= (const FastEntityImage&);

	int32	mylastx;
	int32	mylasty;
	uint8 myLastPose;
	EntityImage* myEntityImage;
	bool	myDirtyIsRectValid;
	RECT 	myDirtyRect;
	RECT 	myNewRect;
	RECT 	myImageBound;
	RECT	myBackDirtyRect;
	RECT	myDirtyClip;
	int32	mybitmapHeight;
	int32	mybitmapWidth;
		// store your back ground
	LPDIRECTDRAWSURFACE4 	myBackground;
	
	// the current sprite image
	LPDIRECTDRAWSURFACE4		myImage;

};
#endif //FAST_ENTITY_IMAGE_H

#endif	// end of directx version
