// allow an entity image to draw a line on the screen	

#ifndef	LINE_FILE_H
#define	LINE_FILE_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "DrawableObject.h"

class Line: public DrawableObject
{
public:
	Line();
	~Line();

	Line( int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed = 0,
							 uint8 lineColourGreen = 0,
							 uint8 lineColourBlue = 0,
							 uint8 stippleOnValue =0,
							 uint8 stippleOffValue =0,
							 uint32 stipplestartAt =0,
							 uint32 _plane = 9998);

	bool SetParameters( int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed = 0,
							 uint8 lineColourGreen = 0,
							 uint8 lineColourBlue = 0 ,
							 uint8 stippleOnValue =0,
							 uint8 stippleOffValue =0,
							 uint32 stipplestartAt =0,
							 uint32 _plane = 9998);

//	RECT GetBound(){return myBound;}

	void EraseLine(){IamValid = false;}

// ----------------------------------------------------------------------
// Method:      Draw
// Arguments:   None
// Returns:     None
// Description: Tells the object to draw itself!
//			
// ----------------------------------------------------------------------
	virtual void Draw();
	virtual void DrawMirrored();

	virtual void SetCurrentBound(RECT* rect= NULL);

	virtual bool Visible( RECT& rect);
		 	
	virtual Position GetWorldPosition();

	virtual void SetScreenPosition(Position pos);

private:
	// screen values
	int32 myX1;
	int32 myX2;
	int32 myY1;
	int32 myY2;

	// world values
	int32 myWorldx1;
	int32 myWorldx2;
	int32 myWorldy1;
	int32 myWorldy2;

	uint8 myLineColourRed;
	uint8 myLineColourGreen;
	uint8 myLineColourBlue;

	uint8 myStippleOnValue;
	uint8 myStippleOffValue;

	uint32 myStippleStartAt;

//	RECT myBound;
	bool IamValid;
};

#endif // LINE_H