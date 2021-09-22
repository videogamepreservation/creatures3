#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "RemoteCamera.h"
#include "DisplayEngine.h"
#include "ErrorMessageHandler.h"
#include "../Agents/Agent.h"
#include "../App.h"
#include "../World.h"
#include "MainCamera.h"

// serialize out as many members as you need to recreate yourself
CREATURES_IMPLEMENT_SERIAL( RemoteCamera )

RemoteCamera::RemoteCamera()
:myCameraWidth(0),
myCameraHeight(0)
{
myScreenBound.top =0;	
myScreenBound.bottom =0;
myScreenBound.left =0;
myScreenBound.right =0;
myEntityHandler.SetInterestLevel(true);

}

RemoteCamera::~RemoteCamera()
{
	if(mySurface)
	 DisplayEngine::theRenderer().ReleaseSurface(mySurface);
}

RemoteCamera::RemoteCamera(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground,
			uint32 topLeftXofBackground, // top left world co-ordinates 
			uint32 topLeftYofBackground, // of background as a meta room
			int32 plane,
			RECT& bound)
:Camera(viewx, // world x co-ordinate of view
					viewy, // world y co-ordinate of view
					width, // width of camera view
					height, // height of camera view
					defaultBackground,
					topLeftXofBackground, // top left world co-ordinates 
					topLeftYofBackground)
{


	myPlane = plane;
	SetCurrentBound(&bound);
	myOriginalViewToPhysicalWidthRatio = float((float(bound.right - bound.left) )/float(width));
	myOriginalViewToPhysicalHeightRatio = float((float(bound.bottom - bound.top) )/float(height));

	if(!SetUpDrawingSurface())
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
										(int)DisplayEngine::sidNoSecondaryCameraCreated,
										std::string("RemoteCamera::RemoteCamera"));


		throw(CameraException(string,__LINE__));
	}

	// set up a default meta room to look at
	int x, y, w, h;
	RECT metaRoomMBR;
	int metaRoomID=0;
	theApp.GetWorld().GetMap().GetMetaRoomIDForPoint( Vector2D( viewx, viewy ), metaRoomID );
	if( theApp.GetWorld().GetMap().GetMetaRoomLocation
		(metaRoomID, x, y, w, h) )
	{
		std::string background;
		theApp.GetWorld().GetMap().GetCurrentBackground(metaRoomID, background);
		
		if(!background.empty())
		{
			metaRoomMBR.left = x;
			metaRoomMBR.right = x+w-1;
			metaRoomMBR.top = y;
			metaRoomMBR.bottom = y+h-1;
		

			ChangeMetaRoom(background, metaRoomMBR,
				-1, -1, 0, 0);
		}
		SetPosition( Position( viewx, viewy ) );
	}
	myEntityHandler.SetInterestLevel(true);
}

void RemoteCamera::SetScreenPosition(Position pos)
{
	myScreenBound.top = pos.GetY();
	myScreenBound.left = pos.GetX();
	myScreenBound.bottom = myScreenBound.top + myCameraHeight;
	myScreenBound.right = myScreenBound.left + myCameraWidth;
}

void RemoteCamera::SetCurrentBound(RECT* rect /*= NULL*/)
{
	if(rect)
	{
		myCurrentBound.bottom = rect->bottom;
		myCurrentBound.top = rect->top;
		myCurrentBound.left = rect->left;
		myCurrentBound.right = rect->right;

		myCameraHeight= myCurrentBound.bottom - myCurrentBound.top;
		myCameraWidth = myCurrentBound.right - myCurrentBound.left;

	}

}

void RemoteCamera::PhysicallyMoveToWorldPosition(int32 x, int32 y)
{
	RECT rect;
	rect.top = y;
	rect.bottom = rect.top + myCameraHeight;
	rect.left = x;
	rect.right = rect.left + myCameraWidth;

	SetCurrentBound(&rect);
}

void RemoteCamera::ZoomBy(int32 pixels,int32 x, int32 y )
{

	if(x != -1 && y != -1)
		CentreOn(x,y);
	else
		GetViewCentre(x,y);

	// take the set amount of pixels of the veiw width and height
	// do it at both ends ie take from the view area and the view width/height


	// now make sure that whatever adjustments that were made are
	// symmetrical
	int testWidth = myViewWidth+pixels;
	int testHeight = myViewHeight + pixels;

	if( (testWidth > 0 && testWidth < DisplayEngine::theRenderer().GetSurfaceWidth()) &&
		( testHeight > 0 && testHeight < DisplayEngine::theRenderer().GetSurfaceHeight()))
	{
		myViewWidth+= pixels;
		myViewHeight+=pixels;
	}

	myOriginalViewToPhysicalWidthRatio = float((float(myCameraWidth) )/float(myViewWidth));
	myOriginalViewToPhysicalHeightRatio = float((float(myCameraHeight) )/float(myViewHeight));

	SetUpDrawingSurface();

	CentreOn(x,y);	
	
}


bool RemoteCamera::SetUpDrawingSurface()
{
		// find out if the background is smaller than the display width do something
		SetViewArea();
		
		// make it the size of our surface area
		int width = myViewArea.right - myViewArea.left;
		int height = myViewArea.bottom - myViewArea.top;



		RECT drawFrom;
		drawFrom.top =0;
		drawFrom.left =0;
		drawFrom.right = width;
		drawFrom.bottom =height;

		if(mySurface)
		{
			DisplayEngine::theRenderer().ReleaseSurface(mySurface);
		}

		// get the display engine to create a surface the size of my view rect
		// for me
		mySurface = DisplayEngine::theRenderer().
												CreateSurface(drawFrom.right,
												drawFrom.bottom);
	
	return (mySurface == NULL ? false : true);
}



void RemoteCamera::Draw()
{

	if(!Pan())
	{
		// if we are tracking an object then pan to set it in the centre
		TrackObject();
		{
		}	
	}

	DoTrackingChecks();

	if(!mySurface)
		return;

	Update(true,
		   true);
	RECT tempView;
	tempView.bottom = myViewArea.bottom;
	tempView.top = myViewArea.top;
	tempView.left = myViewArea.left;
	tempView.right = myViewArea.right;






	
	bool transparencyAware = false;

	// clip the screen bound against the displayArea
	RECT displayrect;
	int temp =0;

	theMainView.GetDisplayArea(displayrect);

	// clip the camera's physical bound
	if(myScreenBound.top < displayrect.top)
	{
		myScreenBound.top = displayrect.top;
		temp =  (myScreenBound.bottom - myScreenBound.top)/myOriginalViewToPhysicalHeightRatio;
		tempView.top = myViewArea.bottom - temp;
	}

	if(myScreenBound.left < displayrect.left)
	{
		myScreenBound.left = displayrect.left;
		temp = (myScreenBound.right - myScreenBound.left)/myOriginalViewToPhysicalWidthRatio;
		tempView.left = myViewArea.right - temp;
	}

	if(myScreenBound.right > displayrect.right)
	{
		myScreenBound.right = displayrect.right;
		temp = (myScreenBound.right - myScreenBound.left)/myOriginalViewToPhysicalWidthRatio;
		tempView.right = myViewArea.left+ temp;

	}

	if(myScreenBound.bottom > displayrect.bottom)
	{

	
		myScreenBound.bottom = displayrect.bottom;
			
		temp =  (myScreenBound.bottom - myScreenBound.top)/myOriginalViewToPhysicalHeightRatio;
		tempView.bottom = myViewArea.top+ temp;
	}

//	theMainView.GetViewArea(displayrect);
/*	// clip the view port
	if(tempView.top < displayrect.top)
		tempView.top = displayrect.top;

	if(tempView.left < displayrect.left)
		tempView.left = displayrect.left;

	if(tempView.right > displayrect.right)
		tempView.right = displayrect.right;

	if(tempView.bottom > displayrect.bottom)
		tempView.bottom = displayrect.bottom;*/

	RECT drawFrom;
	drawFrom.top =tempView.top - myViewArea.top;
	drawFrom.left =tempView.left - myViewArea.left;
	drawFrom.right =tempView.right-myViewArea.left;
	drawFrom.bottom =tempView.bottom - myViewArea.top;

	DisplayEngine::theRenderer().BlitToBackBuffer(myScreenBound, 
									  mySurface,
									  drawFrom,
									  transparencyAware);

}

void RemoteCamera::DrawMirrored()
{
	Update(true,true);
}

void RemoteCamera::Refresh()
{
	Update(true,true);
}



bool RemoteCamera::Write(CreaturesArchive &ar) const
{
	Camera::Write(ar);

	ar << myCameraWidth;
	ar << myCameraHeight;
	ar << myScreenBound;
	ar << myPlane;
	ar << myOriginalViewToPhysicalWidthRatio;
	ar << myOriginalViewToPhysicalHeightRatio;
	return true;
}

bool RemoteCamera::Read(CreaturesArchive &ar)
{
			
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		if(!Camera::Read(ar))
			return false;

		ar >> myCameraWidth;
		ar >> myCameraHeight;
		ar >> myScreenBound;
		ar >> myPlane;
		ar >> myOriginalViewToPhysicalWidthRatio;
		ar >> myOriginalViewToPhysicalHeightRatio;

	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success
	if(!SetUpDrawingSurface())
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
										(int)DisplayEngine::sidNoSecondaryCameraCreated,
										std::string("RemoteCamera::RemoteCamera"));


		throw(CameraException(string,__LINE__));
	}
	return true;
}

void RemoteCamera::Track(AgentHandle& agent, int xpercent, int ypercent,
											 int style, int transition)
{
	// don't allow hard tracking
//	if(style != 2)
		myTrackStyle = style;

	Camera::Track(agent,xpercent,ypercent,myTrackStyle,0);
}

bool RemoteCamera::TrackObject()
{
	if (myTrackObject.IsValid())
	{
		// if flexible style, don't track unless we were in the
		// rectangle last tick.
		if ((myTrackStyle == 1) && !myTrackInRectangleLastTick)
			return false;

		int offbyx, offbyy;
		if (!InTrackingRectangle(offbyx, offbyy))
		{
			int32 objx = myTrackObject.GetAgentReference().GetCentre().x;
			int32 objy = myTrackObject.GetAgentReference().GetCentre().y;

			int metaRoomID;
			bool ok = theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
				(Vector2D(objx, objy),metaRoomID);
			if (!ok)
				return false;

			
			int oldmetaRoomID;
			theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
				(Vector2D(myWorldPosition.GetX(), myWorldPosition.GetY()),oldmetaRoomID);

			if(!ok)
				return false;

			if (oldmetaRoomID != metaRoomID)
			{
				// if we're not hard tracking, and it's the camera that's
				// changed meta room, not us, then don't change back
				if ((myTrackStyle != 2) && (myTrackMetaRoomLastTick == metaRoomID))
					return false;
				// get the new background
				std::string background;

				int x, y, w, h;
				RECT metaRoomMBR;

				if( !theApp.GetWorld().GetMap().GetMetaRoomLocation(metaRoomID, x, y, w, h) )	
				{
					return false;
				}


				metaRoomMBR.left = x;
				metaRoomMBR.right = x+w-1;
				metaRoomMBR.top = y;
				metaRoomMBR.bottom = y+h-1;


			
				theApp.GetWorld().GetMap().GetCurrentBackground(metaRoomID, background);
				ChangeMetaRoom(background,metaRoomMBR,objx,objy,false,0);
			}
			else
			{
				MoveBy(-offbyx, -offbyy);
			}

			return true;
		}
	}
	return false;
}