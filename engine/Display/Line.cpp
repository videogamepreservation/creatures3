#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Line.h"
#include "MainCamera.h"
#include "DisplayEngine.h"
#include "EntityImage.h"

Line::Line():
	myX1(0),
		myX2(0),
		myY1(0),
		myY2(0),
		myWorldx1(0),
	myWorldx2(0),
	myWorldy1(0),
	myWorldy2(0),
		IamValid(0)
	{
		myCurrentBound.top =0;
		myCurrentBound.left=0;
		myCurrentBound.right=0;
		myCurrentBound.bottom=0;
		myPlane = 9998;
		theMainView.Add(this);
		myAmIALineFlag = true;
	
	}


Line::Line(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/,
							 uint8 stippleOnValue /*=0*/,
							 uint8 stippleOffValue /*=0*/,
							 uint32 stipplestartAt/*=0*/,
							 uint32 _plane/*=9998*/):

	IamValid(1)
{	
	myAmIALineFlag = true;
	myPlane = _plane;
	myCurrentBound.left = 0;
	myCurrentBound.right = x2-x1;
	myCurrentBound.top = 0;
	myCurrentBound.bottom = y2-y1;

	if(!SetParameters(x1,y1,x2,y2,lineColourRed,lineColourGreen,
		lineColourBlue,stippleOnValue,stippleOffValue,stipplestartAt,_plane))
	{
	IamValid =false;
	}
 }


bool Line::SetParameters(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/,
							 uint8 stippleOnValue /*=0*/,
							 uint8 stippleOffValue /*=0*/,
							 uint32 stipplestartAt/*=0*/,
							 uint32 _plane/*=9998*/)
 {

	RECT rect ;
	theMainView.GetViewArea(rect);


	int tempx1 = myWorldx1=x1;
	int tempx2 = myWorldx2=x2;
	int tempy1 = myWorldy1=y1;
	int tempy2 = myWorldy2=y2;
	myPlane = _plane;


	if(!DisplayEngine::ClipLine(&rect, tempx1, tempy1, tempx2, tempy2, 1))
	{
		return false;
	}

	theMainView.WorldToScreen(tempx1,tempy1);
	

	theMainView.WorldToScreen(tempx2,tempy2);


	myX1 = tempx1;
	myY1= tempy1;
	myX2 = tempx2;
	myY2 = tempy2;

	myStippleOffValue = stippleOffValue;
	myStippleOnValue = stippleOnValue;

	myStippleStartAt = stipplestartAt;

	
	myLineColourRed=lineColourRed;
	myLineColourGreen=lineColourGreen;
	myLineColourBlue=lineColourBlue;

	IamValid = true;
	theMainView.Remove(this);
	theMainView.Add(this);
	return true;
 }

void Line::Draw()
{

	if(IamValid)
	{
		// recalculate your screen position
		RECT rect ;
		theMainView.GetViewArea(rect);

	

		int tempx1 = myWorldx1;
		int tempx2 = myWorldx2;
		int tempy1 = myWorldy1;
		int tempy2 = myWorldy2;

		if(!DisplayEngine::ClipLine(&rect, tempx1, tempy1, tempx2, tempy2, 1))
		{
			return ;
		}


		theMainView.WorldToScreen(tempx1,tempy1);
		

		theMainView.WorldToScreen(tempx2,tempy2);


		myX1 = tempx1;
		myY1= tempy1;
		myX2 = tempx2;
		myY2 = tempy2;

		DisplayEngine::theRenderer().
		DrawLine(myX1,myY1,myX2,myY2,
		myLineColourRed,myLineColourGreen,myLineColourBlue,
		myStippleOnValue,
		myStippleOffValue,
		myStippleStartAt);
	}
}


void Line::DrawMirrored()
{
}

void Line::SetCurrentBound(RECT* rect/*= NULL*/)
{
	int tempx1=myX1,tempx2=myX2;
	int tempy1=myY1,tempy2=myY2;

	theMainView.ScreenToWorld(tempx1,tempy1);
	

	theMainView.ScreenToWorld(tempx2,tempy2);

	if(myX2 > myX1)
	{
		myCurrentBound.left =tempx1;
		myCurrentBound.right=tempx2+1;

	}
	else
	{
		myCurrentBound.left =tempx2;
		myCurrentBound.right=tempx1+1;
	}

	if(myY2 > myY1)
	{
		myCurrentBound.top = tempy1;
		myCurrentBound.bottom = tempy2+1;
	}
	else
	{
		myCurrentBound.top = tempy2;
		myCurrentBound.bottom = tempy1+1;
	}
}

// ----------------------------------------------------------------------
// Method:      Visible
// Arguments:   test - rectangle inside which to test whether I am visible.
// Returns:     true if i am visible within that rectangle false otherwise
// Description: Tells caller whether the entity exists inside the bounds
//              of the given rectangle
//			
// ----------------------------------------------------------------------
bool Line::Visible( RECT& rect)
{
	theMainView.Visible(rect,0);
	return true;
}

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
Position Line::GetWorldPosition()
{
	return Position(myX1,myY1);
}


Line::~Line()
{
	theMainView.Remove(this);
}

void Line::SetScreenPosition(Position pos)
	{
		myScreenPosition = pos;
	}