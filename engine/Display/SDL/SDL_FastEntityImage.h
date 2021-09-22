// --------------------------------------------------------------------------
// Filename:	SDL_FastEntityImage.h
// Class:		FastEntityImage
// Purpose:		This class upgrades an entity image so that it can own
//				its own direct draw surfaces and can be drawn straight to
//				the display engine when updated.
//
// Description: 
//								
//
// History:
// --------------------------------------------------------------------------

#ifndef SDL_FASTENTITYIMAGE_H
#define SDL_FASTENTITYIMAGE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../../common/C2eTypes.h"
#include "../../../common/BasicException.h"
#include "../FastDrawingObject.h"

#include "SDL/SDL.h"

class EntityImage;
class DrawableObjectHandler;

class FastEntityImage :public FastDrawingObject
{
public:
	FastEntityImage();
	FastEntityImage(EntityImage& entity,
						SDL_Surface* image,
						SDL_Surface* background,
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
	SDL_Surface* 	myBackground;

	// the current sprite image
	SDL_Surface*		myImage;
};

#endif // SDL_FASTENTITYIMAGE_H
