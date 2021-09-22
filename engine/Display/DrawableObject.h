// --------------------------------------------------------------------------
// Filename:	DrawableObject.h
// Class:		DrawableObject
// Purpose:		This class is the abstract base class of all drawable
//				objects.
//				All this means is that they know how to draw themselves.
//				
//				
//
// Description: 
//				
//			
//				
//
// History:
// -------  
// 11Nov98	Alima			Created.
// --------------------------------------------------------------------------
#ifndef		DRAWABLEOBJECT_H
#define		DRAWABLEOBJECT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Position.h"

class DrawableObject
{
public:

	DrawableObject();



virtual	~DrawableObject(){;}

// ----------------------------------------------------------------------
// Method:      Draw
// Arguments:   None
// Returns:     None
// Description: Tells the object to draw itself!
//			
// ----------------------------------------------------------------------
	virtual void Draw() =0;
	virtual void DrawMirrored()=0;

// ----------------------------------------------------------------------
// Method:      Visible
// Arguments:   test - rectangle inside which to test whether I am visible.
// Returns:     true if i am visible within that rectangle false otherwise
// Description: Tells caller whether the entity exists inside the bounds
//              of the given rectangle
//			
// ----------------------------------------------------------------------
    virtual bool Visible(RECT& rect)=0;

// ----------------------------------------------------------------------
// Method:      GetWorldPosition
// Arguments:   None
// Returns:     None
// Description: This function is needed to draw the object
//				but each drawable object can have a different way of
//				storing it.
//				
//			
// ----------------------------------------------------------------------
	virtual Position GetWorldPosition() = 0;

	bool AreYouASprite()
	{
		return myAmIASpriteFlag;
	}

	bool AreYouACamera()
	{
		return myAmIACameraFlag;
	}

	bool AreYouALine()
	{
		return myAmIALineFlag;
	}

	int32 GetScreenX(void)
	{
		return myScreenPosition.GetX();
	}

	int32 GetScreenY(void)
	{
		return myScreenPosition.GetY();
	}

	Position& GetScreenPosition(void)
	{
		return myScreenPosition;
	}

	virtual void SetScreenPosition(Position pos) = 0;
	virtual void SetClippedDimensions(int32 w, int32 h){;}


// ----------------------------------------------------------------------
// Method:      Get/SetPlane 
// Arguments:   None/plane - the plane in which you wish the entity
//				to reside
//
// Returns:     The plane that the entity exists on/None
//			
// ----------------------------------------------------------------------
	int32 GetPlane(){return myPlane;}

	void SetPlane(int32 plane){myPlane = plane;}

	void GetBound(RECT& r)
	{
		r.left   = myCurrentBound.left;
		r.top    = myCurrentBound.top ;
		r.right  = myCurrentBound.right;
		r.bottom = myCurrentBound.bottom;
	}

	virtual void SetCurrentBound(RECT* rect= NULL) = 0;

	// new serialization stuff
	virtual bool Write( CreaturesArchive &ar ) const;
	virtual bool Read( CreaturesArchive &ar );

	bool AreYouCameraShy() { return myCameraShyFlag;}
	void YouAreCameraShy(bool really) { myCameraShyFlag = really; }


protected:
	
	Position		myScreenPosition;
	RECT			myCurrentBound;
	bool			myCameraShyFlag;
	int32			myPlane;

	bool			myAmIASpriteFlag;
	bool			myAmIALineFlag;
	bool			myAmIACameraFlag;

private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	DrawableObject (const DrawableObject&);
	DrawableObject& operator= (const DrawableObject&);
};


#endif		// DRAWABLEOBJECT_H