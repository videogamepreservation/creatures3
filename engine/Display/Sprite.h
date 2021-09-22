// --------------------------------------------------------------------------
// Filename:	Sprite.h
// Class:		Sprite
// Purpose:		Each sprite has an undelying bitmap for its associated object.
//				A sprite keeps track of its Screen position and knows how to
//				draw itself taking into account transparent pixels.
//
// Description: I went for the has-a bitmap relationship rather is-a because
//				I could use the drawable object as a base class for all things
//				to be displayed over the background.
//			
//				
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Added comments.
//							Restructed adding drawable object base class
// --------------------------------------------------------------------------
#ifndef		ENGINESPRITE_H
#define		ENGINESPRITE_H

//#include	"EntityImage.h"
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"DrawableObject.h"


class EntityImage;
class Bitmap;

// the possible number of bitmaps that could be drawn
// one on top of the other
// bitmap zero is the default sprite
const int NUM_BITMAPS = 4;

class Sprite : public DrawableObject
{

public:
	
	Sprite();

	Sprite(EntityImage* entity);

	virtual ~Sprite();

	virtual Bitmap* GetBitmap(void);

	virtual bool SetBitmap(Bitmap* bitmap, int32 layer);

	void HideBitmap(int32 layer);
	virtual void ShowBitmap(int32 layer);

	virtual Position GetWorldPosition(void);
	
	virtual int32 GetPlane(void) const;


	int32 GetWorldX(void);

	int32 GetWorldY(void);


	virtual int32 GetWidth(void);

	virtual int32 GetHeight(void);

	virtual void SetScreenPosition(Position pos);


	virtual uint16* GetImageData();

	virtual void Draw();

	virtual void SetClippedDimensions(int32 w, int32 h);

// ----------------------------------------------------------------------
// Method:      Visible
// Arguments:   test - rectangle inside which to test whether I am visible.
// Returns:     true if i am visible within that rectangle false otherwise
// Description: Tells caller whether the entity exists inside the bounds
//              of the given rectangle
//
//              dummy function returns true until I have sussed 
//              cameras out
//			
// ----------------------------------------------------------------------
    virtual bool Visible( RECT& test);

	// this is so that the dirty rects can look after themselves
	virtual void SetCurrentBound(RECT* rect= NULL);

	virtual void DrawMirrored();
	bool IsMirrored(){return myDrawMirroredFlag;}

	void DrawMirrored(bool mirror){myDrawMirroredFlag = mirror;}

	void CentreMe(int32& x, int32& y);

	

protected:
	EntityImage*	myEntity;
	Bitmap* myBitmaps[NUM_BITMAPS];
	//we can hide some layers if we want
	bool myBitmapsToDraw[NUM_BITMAPS];
	bool myDrawMirroredFlag;

private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	Sprite (const Sprite&);
	Sprite& operator= (const Sprite&);
};


#endif		// ENGINESPRITE_H