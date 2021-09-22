// --------------------------------------------------------------------------
// Filename:	Camera.cpp
// Class:		Camera
// Purpose:		This class provides a view of the world to clients.
//				The camera has a position in world coordinates.
//				The interface will give the client the usual camera
//				operations e.g. panning, tracking. 
//
//				Each camera must keep track of the of its world coordinates
//				and the agents.
//				
//				
//
// Description: All entities are stored in myEntities.  
//			
//				
//
// History:
// -------  
// 11Nov98	Alima			Created.
// 28Feb98					Updated to take a vector collection of background
//							names instead of just one.  Should I rely on the
//							 map always being there and just take a reference?
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Camera.h"
#include "DisplayEngine.h"
#include "EntityImage.h"
#include "ErrorMessageHandler.h"
#include "../resource.h"
#include "../InputManager.h"
#include "../App.h"
#include "Line.h"
#include <algorithm>
#include "../World.h"
#include "../Map/Map.h"
#include "../Creature/Creature.h"
#include "../Agents/Agent.h"
#include "../mfchack.h"

#include <fstream>

// serialize out as many members as you need to recreate yourself
CREATURES_IMPLEMENT_SERIAL( Camera )

Camera::Camera(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
				int32 width, // width of view
				int32 height, // height of view
				std::string& defaultBackground,
				uint32 topLeftXofBackground, // top left world co-ordinates 
				uint32 topLeftYofBackground // of background as a meta room
			)
:myWorldPosition(viewx,viewy)
{
	myAmIACameraFlag = true;
	InitialiseCameraVariables();	
	if(!Create(viewx, // world x co-ordinate of view
			viewy, // world y co-ordinate of view
			width, // width of camera
			height, // height of camera
			defaultBackground,// background file to use
			topLeftXofBackground, // top left world co-ordinates 
			topLeftYofBackground))
	{
		// "The display engine is shutting down because the following background file was not created %1."
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
										(int)DisplayEngine::sidNoBackgroundCreated,
										std::string("Camera::Camera"),
										defaultBackground.c_str());


		throw(CameraException(string,__LINE__));
	}
	myDisabledFlag = false;
	myEntityHandler.SetInterestLevel(false);
}

Camera::Camera(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
				int32 width, // width of camera
				int32 height, // height of camera
				std::string& defaultBackground,
				RECT& bounds // of background as a meta room
				)
:myWorldPosition(viewx,viewy)
{
	myAmIACameraFlag = true;
	InitialiseCameraVariables();

	if(!Create(viewx, // world x co-ordinate of view
			viewy, // world y co-ordinate of view
			width, // width of camera
			height, // height of camera
			defaultBackground,// background file to use
			bounds // top left world co-ordinates 
			))
	{

		// "The display engine is shutting down because the following background file was not created %1."
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
										(int)DisplayEngine::sidNoBackgroundCreated,
										std::string("Camera::Camera"),
										defaultBackground.c_str());

		throw(CameraException(string,__LINE__));
	}
	myDisabledFlag = false;
	myEntityHandler.SetInterestLevel(false);
}


Camera::Camera():
myWorldPosition(0,0)
{
	myAmIACameraFlag = true;
	InitialiseCameraVariables();
	myEntityHandler.SetInterestLevel(false);
}

void Camera::InitialiseCameraVariables()
{
		myObjectToKeepUpWith = NULL;
		myTrackObject = NULLHANDLE;
		myTrackPercentX = 75;
		myTrackPercentY = 75;
		myTrackStyle = 0;
		myTrackInRectangleLastTick = false;
		myTrackPositionLastTick = Position(0, 0);
		myTrackMetaRoomLastTick = -1;
		myTrackTransition = 0;
		myMiddleDrag = false;
		myViewWidth = -1;
		myViewHeight = -1;
		myAcceleration =4;
		myMinimumSpeed =4;
		myChangingRoomsFlag = 0;
		myKeepUpSpeedX = 0;
		myKeepUpSpeedY =0;
		myKeepUpCountX = 0;
		myKeepUpCountY = 0;
		myKeepUpPanX = 0;
		myKeepUpPanY = 0;
		myCompleteRedraw = true;
#ifndef C2E_SDL
		mySurface = NULL;
#endif
		myLoadingFlag= false;
		myDisabledFlag = true;
}

Camera::~Camera()
{
	// everyone should have unregistered themselves from floating by now?
	ASSERT(myFloatingThings.empty());
}

//TO DO: REFINE THESE OVERLOADED METHODS SO THAT THEY USE THE SAME
// CALL INSTEAD OF DUPLICATING CODE 
// ----------------------------------------------------------------------
// Method:      Create
// Arguments:  viewx - world x co-ordinate of view
//				viewy - world y co-ordinate of view
//				width -  width of camera
//				height -  height of camera
//				backgroundName - background file to use
//				topLeftXofBackground - top left world co-ordinates 
//				topLeftYofBackground) - of background as a meta room
// Returns:     true if the camera was created OK false otherwise
// Description: Used in two stage creation of camera it's possible to
//				run the world with the camera disabled
//			
// ----------------------------------------------------------------------
bool Camera::Create(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground,
			int32 topLeftXofBackground, // top left world co-ordinates 
			int32 topLeftYofBackground // of background as a meta room
			)
{
	myCurrentBound.top = 0;
	myCurrentBound.left = 0;
	myCurrentBound.bottom = height;
	myCurrentBound.right = width;

	FilePath back1;
	if(!defaultBackground.empty())
	{
		back1 = MakeBackgroundPath(defaultBackground);
	}

	// set default values should be atltered by later
	// commands such as change meta room
	myViewHeight= height;
	myViewWidth = width;



	if(!myBackground0.Create(back1,
							Position(topLeftXofBackground,
							topLeftYofBackground),this))
	{
//		OutputDebugString("Background::Create Failed\n");
//	return false;
	}

	myBackground1.SetOwner(this);

	// incoming
	myBackgrounds.push_back(&myBackground0);
	// outgoing blank for now
	myBackgrounds.push_back(&myBackground1);

	myBackground0.SetDisplayPosition(myWorldPosition);
	
//	myMonitor.Create("Data\\Charset9x15.bmp");
	myEntityHandler.SetWorldPosition(myWorldPosition);
	myEntityHandler.SetOwner(this);

	// find out if the background is smaller than the display width do something
	SetViewArea();
	return true;
}


bool Camera::Create(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground,
		RECT& bounds // of background as a meta room
	)
{
	FilePath back1;
	back1 = MakeBackgroundPath(defaultBackground);

	myViewHeight= height;
	myViewWidth = width;


	if(!myBackground0.Create(back1,
							bounds,this))
	{
	return false;
	}

	myBackground1.SetOwner(this);

	// incoming
	myBackgrounds.push_back(&myBackground0);
	// outgoing blank for now
	myBackgrounds.push_back(&myBackground1);

	myBackground0.SetDisplayPosition(myWorldPosition);
	
//	myMonitor.Create("Data\\Charset9x15.bmp");
	myEntityHandler.SetWorldPosition(myWorldPosition);	
	myEntityHandler.SetOwner(this);

	// find out if the background is smaller than the display width do something
	SetViewArea();
	return true;
}

FilePath Camera::MakeBackgroundPath( std::string const &backgroundName)
{
	return FilePath( backgroundName + ".blk", BACKGROUNDS_DIR );
}


void Camera::DoChangeMetaRoom(std::string& backgroundName,
			RECT& bounds,
			int32 newviewx,
			int32 newviewy,
			bool useTopLeftOnly,
			bool bCentre)
{

	myChangingRoomsFlag= true;

	FilePath backgroundPath;
	backgroundPath = MakeBackgroundPath(backgroundName);

	// switch the backgrounds so that the current one is now the
	// outgoing background
	Background* background0 = myBackgrounds[0];
	Background* background1 = myBackgrounds[1];
	myBackgrounds[0] = background1;
	myBackgrounds[1] = background0;

	bool ok = false;
	if(useTopLeftOnly)
	{
	ok = myBackgrounds[0]->Create(backgroundPath,
									Position(bounds.top,
								bounds.left),this);
	}
	else
	{
	ok = myBackgrounds[0]->Create(backgroundPath,
								bounds,this);
	}

	if(!ok)
	{
		Background* background0 = myBackgrounds[0];
		Background* background1 = myBackgrounds[1];
		myBackgrounds[0] = background1;
		myBackgrounds[1] = background0;
		return;
	}


	// if no view set set it to the top left
	if(newviewx == 0 && newviewy == 0)
	{
		myWorldPosition.SetX(bounds.top);
		myWorldPosition.SetY(bounds.left);
	}
	else
	{
		if (bCentre)
			CentriseCoordinates(newviewx, newviewy);
		myWorldPosition.SetX(newviewx);
		myWorldPosition.SetY(newviewy);
	}

	NormaliseWorldPosition();

	myBackgrounds[0]->SetDisplayPosition(myWorldPosition);

}

void Camera::Flip(int32 flip)
{
	// if this is not the main camera then don't do any screen transitions
		switch(flip)
		{
		case NOFLIP:
			{
				myChangingRoomsFlag= false;
				Update(true,myLoadingFlag);
				break;
			}
			case FLIP:
			{

				Update(true,true); // full screen, just the back buffer
				DisplayEngine::theRenderer().FlipScreenHorizontally();
				myChangingRoomsFlag= false;
				break;
			}
			case BURST:
				{
					Update(true,true); // full screen, just the back buffer
					DisplayEngine::theRenderer().Burst();
					myChangingRoomsFlag= false;
					break;
				}
			default:
			{
				myChangingRoomsFlag= false;
				Update(true,myLoadingFlag);
				break;
			}
		}
}

void Camera::SetBackground(std::string& backgroundName,
						   int32 transitionMethod)
{
	RECT bounds;
	bounds.top = myBackgrounds[0]->GetTop();
	bounds.left = myBackgrounds[0]->GetLeft();
	bounds.right = bounds.left + myBackgrounds[0]->GetPixelWidth();
	bounds.bottom = bounds.top + myBackgrounds[0]->GetPixelHeight();

	DoChangeMetaRoom(backgroundName,
			bounds,
			myWorldPosition.GetX(),
			myWorldPosition.GetY(),
			false,// use top left only
			false);

	Flip(transitionMethod);

}


// ----------------------------------------------------------------------
// Method:      ChangeMetaRoom
// Arguments:   backgroundname - new backgrounds name
//			
// Returns:     None
// Description: Switch the backgrounds so that the current one is now the
//				outgoing background.  Create the incoming background
//			
// ----------------------------------------------------------------------
void Camera::ChangeMetaRoom(std::string& backgroundName	,
		RECT& bounds,
			int32 newviewx,
			int32 newviewy,
			int32 flip,
			bool bCentre)
{

	DoChangeMetaRoom(backgroundName,bounds,newviewx,newviewy,false,bCentre);
	myChangingRoomsFlag= false;

	Update(true,true);


}

// ----------------------------------------------------------------------
// Method:      Add
// Arguments:   newEntity - pointer to new entity to add to the 
//				list
// Returns:     None
// Description: This adds the given entityImage to the current update
//				list.
//			
// ----------------------------------------------------------------------
void Camera::Add(EntityImage* const newEntity)
{
	if(newEntity)
	{
		myEntityHandler.Add(newEntity);
	}
}



// ----------------------------------------------------------------------
// Method:      Add
// Arguments:   newEntity - pointer to new entity to add to the 
//				list
// Returns:     None
// Description: This adds the given entityImage to the current update
//				list.
//			
// ----------------------------------------------------------------------
void Camera::Add(Line* const newEntity)
{
	if(newEntity)
	{
		myEntityHandler.Add(newEntity);
	}
}

// ----------------------------------------------------------------------
// Method:      Remove
// Arguments:   newEntity - pointer to new entity to chop from the
//				list
// Returns:     true if the entity was found and removed false otherwise
// Description: This removes the given entityImage from the current update
//				list.
//			
// ----------------------------------------------------------------------
bool Camera::Remove(EntityImage* const chop,bool stopTracking/*=true*/)
{
	// make sure that this isn't the
	// trackobject if so stop tracking
	if ((myTrackObject.IsValid()) && myTrackObject.GetAgentReference().GetEntityImage() == chop && stopTracking)
	{
		myTrackObject = NULLHANDLE;
	}

	return myEntityHandler.Remove(chop);
}


// ----------------------------------------------------------------------
// Method:      Remove
// Arguments:   newEntity - pointer to new entity to chop from the
//				list
// Returns:     true if the entity was found and removed false otherwise
// Description: This removes the given entityImage from the current update
//				list.
//			
// ----------------------------------------------------------------------
bool Camera::Remove(Line* const chop)
{
	// make sure that this isn't the
	// trackobject if so stop tracking

	return myEntityHandler.Remove(chop);
}

// ----------------------------------------------------------------------
// Method:      DoUpdateAfterAdjustments
// Arguments:   None
// Returns:     None
// Description: Does common camera update needed when the camera has been
//				adjusted.
//			
// ----------------------------------------------------------------------
void Camera::DoUpdateAfterAdjustments()
{
	// Remember we need to do a complete redraw this time
	myCompleteRedraw = true;
}

// ----------------------------------------------------------------------
// Method:      MoveTo
// Arguments:   x - xpositon to move to
//				y - yposition to move to
// Returns:     None
// Description: Resets the camera postion to the given co-ordinates
//	
//			
// ----------------------------------------------------------------------
void Camera::MoveTo(int32 x, int32 y, int pan)
{
	if (pan)
	{
		PanTo(x,y);
	}
	else
	{
	//	Position pos(x,y);

		Position pos = myWorldPosition;
		myWorldPosition.SetX(x);
		myWorldPosition.SetY(y);

		NormaliseWorldPosition();

		if(myWorldPosition != pos)
			DoUpdateAfterAdjustments(); 
	}
}

// ----------------------------------------------------------------------
// Method:      MoveBy
// Arguments:   x - xpositon to move by
//				y - yposition to move by
// Returns:     None
// Description: Adjusts the camera postion to the given co-ordinates
//	
//			
// ----------------------------------------------------------------------
void Camera::MoveBy(int32 x,int32 y)
{
	Position testPos = myWorldPosition;

	myWorldPosition.AdjustX(x);
	myWorldPosition.AdjustY(y);
	NormaliseWorldPosition();

	if(testPos != myWorldPosition)
	{
	DoUpdateAfterAdjustments(); 
	}
}

// ----------------------------------------------------------------------
// Method:      PanTo
// Arguments:   x - xpositon to pan to
//				y - yposition to pan to
// Returns:     None
// Description: Sets up a pan cycle where the camera will pan each update
//				until it has reached the gievn x,y position
//	
//			
// ----------------------------------------------------------------------
void Camera::PanTo(int32 x, int32 y)
{   	
	// How far is this from where the camera is now?

	int32 xd,yd;
		
	// must send even numbers or else
	// pan won't work properly

	NormaliseWorldCoordinates(x,y);

	xd=x-myWorldPosition.GetX();
	yd=y-myWorldPosition.GetY();

	yd -= yd%4;
	xd -= xd%4;

	myPanVector.SetX(xd);
	myPanVector.SetY(yd);

}

bool Camera::Pan()
{
	//	char buf[200];
	//	wsprintf(buf,"pan %d %d\n",myPanVector.GetX(),myPanVector.GetY());
	//	OutputDebugString(buf);

	// only pan if we have somewhere we are heading to
	int32 acceleration = 8;
	int32 minimumSpeed =4;
	
	if(myPanVector.GetX() == 0 &&
		myPanVector.GetY() == 0)
	{
		myPanVelocity.Reset();
		myDistancePanned.Reset();
		return false;
	}

	// taken from creatures2 probably could be tidied up
	// will look later

	if (myPanVector.GetX()<0)	// Moving left
	{
		if (myPanVector.GetX()>=-acceleration)
		{
			// Finish last step
			myPanVelocity.SetX(myPanVector.GetX());
		}
		else
		{
			// Should we still accelerate
			// (if we accelarate, will we still be able to
			// slow down in time)
			if ( (myDistancePanned.GetX()+myPanVelocity.GetX()-acceleration) 
												> myPanVector.GetX())
			{
				myPanVelocity.AdjustX(-acceleration);
			}
			else
			{
				// Slow down until we reach minimum speed
				if (myPanVelocity.GetX()+acceleration<=-minimumSpeed)
				{
					myPanVelocity.AdjustX(acceleration);
				}
				else
				{
					myPanVelocity.SetX(-minimumSpeed);
				}
			}
		}
	}
	else
	{
		if (myPanVector.GetX()>0)	// Moving right
		{
			if (myPanVector.GetX()<=acceleration)
			{
				// Finish last step
				myPanVelocity.SetX(myPanVector.GetX());
			}
			else
			{
				// Should we still accelarate
				// (if we accelarate, will we still be able to
				// slow down in time)
				if ( (myDistancePanned.GetX()+myPanVelocity.GetX()+acceleration) 
											< myPanVector.GetX())
				{
					myPanVelocity.AdjustX(acceleration);
				}
				else
				{
					// Slow down until we reach minimum speed
					if (myPanVelocity.GetX()-acceleration>=minimumSpeed)
					{
						myPanVelocity.AdjustX(-acceleration);
					}
					else
					{
						myPanVelocity.SetX(minimumSpeed);
					}
				}
			}
		}
		else
		{
			// Stop Panning horizontally
			myPanVelocity.SetX(0);
		}
	}

	if (myPanVector.GetY()<0)	// Moving up
	{
		if (myPanVector.GetY()>=-acceleration)
		{
			// Finish last step
			myPanVelocity.SetY(myPanVector.GetY());
		}
		else
		{
			// Should we still accelerate
			// (if we accelarate, will we still be able to
			// slow down in time)
			if ( (myDistancePanned.GetY()+myPanVelocity.GetY()-acceleration)
				> myPanVector.GetY())
			{
				myPanVelocity.AdjustY(-acceleration);
			}
			else
			{
				// Slow down until we reach minimum speed
				if (myPanVelocity.GetY()+acceleration<=-minimumSpeed)
				{
					myPanVelocity.AdjustY(acceleration);
				}
				else
				{
					myPanVelocity.SetY(-minimumSpeed);
				}
			}
		}
	}
	else
	{
		if (myPanVector.GetY()>0)	// Moving down
		{
			if (myPanVector.GetY()<=acceleration)
			{
				// Finish last step
				myPanVelocity.AdjustY(myPanVector.GetY());
			}
			else
			{
				// Should we still accelarate
				// (if we accelerate, will we still be able to
				// slow down in time)
				if ( (myDistancePanned.GetY()+myPanVelocity.GetY()+acceleration)
					< myPanVector.GetY())
				{
					myPanVelocity.AdjustY(acceleration);
				}
				else
				{
					// Slow down until we reach minimum speed
					if (myPanVelocity.GetY()-acceleration>=minimumSpeed)
					{
						myPanVelocity.AdjustY(-acceleration);
					}
					else
					{
						myPanVelocity.SetY(minimumSpeed);
					}
				}
			}
		}
		else
		{
			// Stop Panning vertically
			myPanVelocity.SetY(0);
		}
	}

	MoveBy(myPanVelocity.GetX(),myPanVelocity.GetY());

	myDistancePanned.AdjustX(myPanVelocity.GetX());
	myDistancePanned.AdjustY(myPanVelocity.GetY());
	myPanVector.AdjustX(-myPanVelocity.GetX());
	myPanVector.AdjustY(-myPanVelocity.GetY());

	return true;
}

void Camera::Update(bool completeRedraw,
					bool justBackBuffer)
{
	//	char buf[200];
	//	wsprintf(buf,"changing rooms? %d\n",myChangingRoomsFlag);
	//		OutputDebugString(buf);

	if((myChangingRoomsFlag && !justBackBuffer) || myDisabledFlag)
			return;

	NormaliseWorldPosition();

	myBackgrounds[0]->SetDisplayPosition(myWorldPosition);

	// find out if the background is smaller than the display width do something
	SetViewArea();

	int n = myFloatingThings.size();
	for (int i = 0; i < n; ++i)
	{
		if( myFloatingThings[i].IsValid() )
			myFloatingThings[i].GetAgentReference().CameraPositionNotify();
	}

	myEntityHandler.Update(myWorldPosition);
#ifndef C2E_SDL
	// the individual cameras should only draw to the back buffer
	DisplayEngine::theRenderer().Update(myBackgrounds[0],&myEntityHandler,
       completeRedraw,justBackBuffer,mySurface);
#endif
}




void Camera::NormaliseWorldPosition()
{
	int32 x = myWorldPosition.GetX();
	int32 y = myWorldPosition.GetY();

	NormaliseWorldCoordinates(x,y);
	myWorldPosition.SetX(x);
	myWorldPosition.SetY(y);

}

void Camera::CalculateDisplayDimensions(int32& displayWidth, int32& displayHeight)
{
	myBackgrounds[0]->GetConsideredDisplayArea(displayWidth,displayHeight);

	if(myViewWidth != -1 && myViewHeight != -1)
	{
		myBackgrounds[0]->GetConsideredDisplayArea(displayWidth,displayHeight);

		if(myViewWidth < displayWidth || displayWidth == 0)
		{
			displayWidth = myViewWidth;
		}
		else
		{
		myViewWidth = displayWidth;
		}

		if(myViewHeight < displayHeight || displayHeight == 0)
		{
			displayHeight = myViewHeight;
		}
		else
		{
			myViewHeight = displayHeight;
		}
	}
}

void Camera::NormaliseWorldCoordinates(int32& x, int32& y)
{
	int32 displayWidth;
	int32 displayHeight;

	CalculateDisplayDimensions(displayWidth,displayHeight);

	// make sure that we never are asked to do draw more background
	// than is possible.  We'll have no world wrapping here

	// absolute x plus the display width is too large for
	// my bitmap
	if(x-myBackgrounds[0]->GetLeft() + displayWidth> 
		myBackgrounds[0]->GetPixelWidth())
	{
		x = myBackgrounds[0]->GetPixelWidth() - displayWidth +
			myBackgrounds[0]->GetLeft();
	}

	if(y-myBackgrounds[0]->GetTop() + displayHeight>  
		myBackgrounds[0]->GetPixelHeight())
	{
		y = myBackgrounds[0]->GetPixelHeight();
		y = myBackgrounds[0]->GetPixelHeight() - displayHeight
			+myBackgrounds[0]->GetTop() ;
	}

	// don't go off the left edge or the top
	if(x-myBackgrounds[0]->GetLeft() < 0 )
	{
		x=myBackgrounds[0]->GetLeft();
	}

	if(y -myBackgrounds[0]->GetTop()< 0)
	{
		y=myBackgrounds[0]->GetTop();
	}
}

void Camera::Track(AgentHandle& agent, int xpercent, int ypercent, int style, int transition)
{
	// do any unfinished business first
	StopTracking();

	myTrackObject = agent;
	myTrackPercentX = xpercent;
	myTrackPercentY = ypercent;
	myTrackStyle = style;
	myTrackInRectangleLastTick = false;
	myTrackPositionLastTick = myWorldPosition;
	myTrackTransition = transition;
	
	// if this is a creature then tell it to 
	// remember that it is being tracked when it grows up for example
	if(myTrackObject.IsCreature())
	{
		myTrackObject.GetCreatureReference().RememberThatYouAreBeingTracked(true);
	}

	SetTrackMetaRoom();
}

// this is for when a creature has been tracked but has grown up
// in between.  The growing up process destroys all current entity
// images and in doing they get unlinked from the camera.
// So the camera needs to know that it should start tracking again
void Camera::YourTrackObjectHasGrownUp(AgentHandle& agent)
{
	// presumably then we can use the previous tracking settings
	myTrackObject = agent;
}

void Camera::SetTrackMetaRoom()
{
	ASSERT(myTrackObject.IsValid());

	float objx = myTrackObject.GetAgentReference().GetCentre().x;
	float objy = myTrackObject.GetAgentReference().GetCentre().y;

	theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
		(Vector2D(objx, objy), myTrackMetaRoomLastTick);
}






// ----------------------------------------------------------------------
// Method:      ChangeDisplayMode 
// Arguments:   width - width to change to
//				height - height to change to
//			
// Returns:     true if you choose a legitimate display mode and we
//				switched to it,  false otherwise.
//
// Description: Finds out whether the requested display mode is viable and 
//				switches to it.  For smoother results change the size of
//				window too. Note that the display modes are enumrated at
//				start up of the engine
//			
// ----------------------------------------------------------------------
bool Camera::ChangeDisplayMode(uint32 width, uint32 height)
{
	bool ok = DisplayEngine::theRenderer().ChangeDisplayMode(width,height);

	Update(true); // complete redraw
	return ok;
}



void Camera::FadeScreen()
{
	DisplayEngine::theRenderer().FadeScreen();
}

void Camera::Refresh()
{
	TrackObject();
	Update(true,myLoadingFlag);
}

AgentHandle Camera::GetTrackAgent()
{
	return myTrackObject;
}

bool Camera::TrackObject()
{
	if (myTrackObject.IsValid())
	{
		// if flexible style, don't track unless we were in the
		// rectangle last tick.
		if ((myTrackStyle == 1) && !myTrackInRectangleLastTick)
			return false;

		// again, if flexible style, don't track is someone else moved 
		// the camera (for example, gone to favourite place)
		if ((myTrackStyle == 1) && (myTrackPositionLastTick != myWorldPosition))
			return false;

		int offbyx, offbyy;
		if (!InTrackingRectangle(offbyx, offbyy))
		{
			float objx = myTrackObject.GetAgentReference().GetCentre().x;
			float objy = myTrackObject.GetAgentReference().GetCentre().y;

			int metaRoomID;
			bool ok = theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
				(Vector2D(objx, objy),metaRoomID);
			if (!ok)
				return false;

			if (theApp.GetWorld().GetMap().GetCurrentMetaRoom() != metaRoomID)
			{
				// if we're not hard tracking, and it's the camera that's
				// changed meta room, not us, then don't change back
				if ((myTrackStyle != 2) && (myTrackMetaRoomLastTick == metaRoomID))
					return false;
	
				// let the pointer decide whether it should carry objects from
				// meta room to meta room
				thePointer.GetPointerAgentReference().DecideIfCarriedAgentShouldBeDropped();
				theApp.GetWorld().SetMetaRoom(metaRoomID, myTrackTransition, objx, objy, true);
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

bool Camera::InTrackingRectangle()
{
	int offbyx, offbyy;
	return InTrackingRectangle(offbyx, offbyy);
}

bool Camera::InTrackingRectangle(int& offbyx, int& offbyy)
{
	ASSERT(myTrackObject.IsValid());

	offbyx = 0;
	offbyy = 0;

	int x = (int)myTrackObject.GetAgentReference().GetCentre().x;
	int y = (int)myTrackObject.GetAgentReference().GetCentre().y;

	// get view rectangle
	RECT rect;

	int width = myViewArea.right - myViewArea.left;
	int height = myViewArea.bottom - myViewArea.top;

	// update to current camera coordinates
	rect.left = myWorldPosition.GetX();
	rect.right = rect.left + width;
	rect.top = myWorldPosition.GetY();
	rect.bottom = rect.top + height;

	{
		int allow = (width * myTrackPercentX) / 100;
		int disallow = width - allow;
		rect.left += disallow / 2;
		rect.right -= disallow / 2;
		if (rect.right < rect.left)
			rect.right = rect.left + 1;

		if (x < rect.left)
			offbyx = rect.left - x;
		else if (x > rect.right)
			offbyx = rect.right - x;
	}

	{
		int allow = (height * myTrackPercentY) / 100;
		int disallow = height - allow;
		rect.top += disallow / 2;
		rect.bottom -= disallow / 2;
		if (rect.bottom < rect.top)
			rect.bottom = rect.top + 1;

		if (y < rect.top)
			offbyy = rect.top - y;
		else if (y > rect.bottom)
			offbyy = rect.bottom- y;
	}

	return (offbyx == 0) && (offbyy == 0);
}

// explicit call from outside to stop tracking
void Camera::StopTracking()
{
	// if this is a creature then tell it to 
	// forget that it is being tracked
	if (myTrackObject.IsCreature())
	{
		myTrackObject.GetCreatureReference().RememberThatYouAreBeingTracked(false);
	}

	myTrackObject = NULLHANDLE;
}

void Camera::CentriseCoordinates(int32& x, int32& y)
{

	x = x - (myDisplayRect.right/2);
	y = y - (myDisplayRect.bottom/2);
}

void Camera::CentreOn(int32 x, int32 y, int pan)
{
	if (pan == 2)
		pan = IsPointOnScreen(x, y);

	CentriseCoordinates(x, y);

	if (pan)
	{
		PanTo(x,y);
	}
	else
	{
		MoveTo(x,y);
	}
}

// entity to keep on screen
// distance to stay away from it
void Camera::KeepUpWithMouse(EntityImage* me)
{
	myObjectToKeepUpWith = me;
}

bool Camera::KeepUpThenWithMouse()
{
	if (myObjectToKeepUpWith != NULL)
	{	
		int32 x = myObjectToKeepUpWith->GetPosition().GetX();
		int32 y = myObjectToKeepUpWith->GetPosition().GetY();

		int32 panx = 0, pany = 0;

		if (DisplayEngine::theRenderer().IsFullScreen() && ((theApp.myScrollingMask & 1) == 1))
		{
			const int32 mouseActiveBorderSize = 2;

			// mouse
			if (x <= myWorldPosition.GetX() + mouseActiveBorderSize)
				panx = -1;
			else if (x >= myWorldPosition.GetX() +  (myDisplayRect.right - myDisplayRect.left) - mouseActiveBorderSize)
				panx = 1;
			if (y <= myWorldPosition.GetY() + mouseActiveBorderSize)
				pany = -1;
			else if (y >= myWorldPosition.GetY() + (myDisplayRect.bottom - myDisplayRect.top) - mouseActiveBorderSize)
				pany = 1;
		}

#ifdef _WIN32
		// hmmm. need some non-win32 scancode defs...

		if (!theApp.GetInputManager().IsKeyDown(VK_SHIFT)
			&& !theApp.GetInputManager().IsKeyDown(VK_CONTROL)
			&& !theApp.GetInputManager().IsKeyDown(VK_MENU))
		{
			if ((theApp.myScrollingMask & 2) == 2)
			{
				// keyboard
				if (theApp.GetInputManager().IsKeyDown(VK_LEFT))
					panx = -1;
				else if (theApp.GetInputManager().IsKeyDown(VK_RIGHT))
					panx = 1;
				
				if (theApp.GetInputManager().IsKeyDown(VK_UP))
					pany = -1;
				else if (theApp.GetInputManager().IsKeyDown(VK_DOWN))
					pany = 1;
			}
		}
#endif // _WIN32

		// process them to decide how much to move camera by:
		if (panx != 0)
		{	
			myKeepUpCountX++;
			if (myKeepUpCountX > theApp.myScrollingSpeedRangeUp.size())
				myKeepUpCountX = theApp.myScrollingSpeedRangeUp.size();
			myKeepUpSpeedX = theApp.myScrollingSpeedRangeUp[myKeepUpCountX - 1];
			myKeepUpPanX = panx;
		}
		else if (myKeepUpCountX != 0)
		{
			if (myKeepUpCountX > theApp.myScrollingSpeedRangeDown.size())
				myKeepUpCountX = theApp.myScrollingSpeedRangeDown.size();

			ASSERT(myKeepUpPanX != 0);
			myKeepUpSpeedX = theApp.myScrollingSpeedRangeDown[myKeepUpCountX - 1];
			panx = myKeepUpPanX;

			myKeepUpCountX--;
		}
		else
		{
			// ASSERT(myKeepUpSpeedX == 0);
			// ASSERT(myKeepUpCountX == 0);
			// OutputDebugString("ResetX\n");
		}

		if (pany != 0)
		{	
			myKeepUpCountY++;
			if (myKeepUpCountY > theApp.myScrollingSpeedRangeUp.size())
				myKeepUpCountY = theApp.myScrollingSpeedRangeUp.size();
			myKeepUpSpeedY = theApp.myScrollingSpeedRangeUp[myKeepUpCountY - 1];
			myKeepUpPanY = pany;
		}
		else if (myKeepUpCountY != 0)
		{
			if (myKeepUpCountY > theApp.myScrollingSpeedRangeDown.size())
				myKeepUpCountY = theApp.myScrollingSpeedRangeDown.size();

			ASSERT(myKeepUpPanY != 0);
			myKeepUpSpeedY = theApp.myScrollingSpeedRangeDown[myKeepUpCountY - 1];
			pany = myKeepUpPanY;

			myKeepUpCountY--;
		}
		else
		{
			// ASSERT(myKeepUpSpeedY == 0);
			// ASSERT(myKeepUpCountY == 0);
			// OutputDebugString("ResetY\n");
		}

//std::ostringstream zub;
//zub << "s:" <<  << " " << '\0';
//OutputFormattedDebugString("s: %d %d", myKeepUpSpeedX, myKeepUpSpeedY);

		panx *= myKeepUpSpeedX;
		pany *= myKeepUpSpeedY;

		if (panx != 0 || pany != 0)
		{
			MoveBy(panx, pany);
			return true;
		}
	}
	return false;
}

bool Camera::MiddleMouseDragMouseWheel()
{
	InputManager& im = theApp.GetInputManager();
	int panx = 0;
	int pany = 0;

	// Scan the pending inputevents:
	for(int i = 0; i < im.GetEventCount(); ++i)
	{
		const InputEvent* ev = &im.GetEvent(i);
 
		// Middle button down
		if( ev->EventCode == InputEvent::eventMouseDown &&
			ev->MouseButtonData.button == InputEvent::mbMiddle &&
			((theApp.myScrollingMask & 4) == 4) )
		{
			myMiddleDrag = true;
			myMiddleDragMX = ev->MouseButtonData.mx;
			myMiddleDragMY = ev->MouseButtonData.my;
#ifndef C2E_SDL
			::SetCapture(theApp.GetMainHWND());
#endif
		}
		else if( ev->EventCode == InputEvent::eventMouseUp &&
			ev->MouseButtonData.button == InputEvent::mbMiddle)
		{
			myMiddleDrag = false;
#ifndef C2E_SDL
			::ReleaseCapture();
#endif
		}
		else if ( myMiddleDrag &&
				ev->EventCode == InputEvent::eventMouseMove)
		{
			panx += (myMiddleDragMX - ev->MouseButtonData.mx);
			pany += (myMiddleDragMY - ev->MouseButtonData.my);
			myMiddleDragMX = ev->MouseButtonData.mx;
			myMiddleDragMY = ev->MouseButtonData.my;
		}

		if ((ev->EventCode == InputEvent::eventMouseWheel)
			&& ((theApp.myScrollingMask & 8) == 8) )
		{
			pany -= ev->MouseWheelData.delta;
		}

	}
	if (panx != 0 || pany != 0)
	{
		MoveBy(panx, pany);
		return true;
	}

	return myMiddleDrag;
}


// ----------------------------------------------------------------------
// Method:      AddDirtyRect 
// Arguments:   rect - new dirty rectangle for a fast drawn object			
//
// Returns:     None
//
// Description: 
//						
// ----------------------------------------------------------------------
void Camera::AddDirtyRect(RECT rect)
{
	myEntityHandler.AddDirtyRect(rect);
}

bool Camera::IsPointOnScreen(int32 x, int32 y)
{
	POINT point;
	point.x = x;
	point.y = y;

	return PointInRectangle(&myViewArea, point) ? true : false;
}

void Camera::AddFloatingThing(AgentHandle& a)
{
	ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) == myFloatingThings.end());
	myFloatingThings.push_back(a);
	ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) != myFloatingThings.end());
}

void Camera::RemoveFloatingThing(AgentHandle& a)
{
	ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) != myFloatingThings.end());
	myFloatingThings.erase(std::find(myFloatingThings.begin(), myFloatingThings.end(), a));
	ASSERT(std::find(myFloatingThings.begin(), myFloatingThings.end(), a) == myFloatingThings.end());
}

Position Camera::GetWorldPosition(void)
	{
		return myWorldPosition;
	}


bool Camera::Visible(RECT& rect)
{
	return myEntityHandler.IsRectOnScreen(rect,myViewArea);
}

bool Camera::Write(CreaturesArchive &ar) const
{
	DrawableObject::Write(ar);

	myEntityHandler.Write(ar);


	ar << myPanVelocity;
	ar << myPanVector;
	ar << myDistancePanned;
	ar << myAcceleration;
	ar << myMinimumSpeed;


	ar << myViewWidth;
	ar << myViewHeight;
	ar << myViewArea;

	ar << myWorldPosition;
	
	ar << myTrackObject;
	ar << myTrackPercentX << myTrackPercentY;
	ar << myTrackStyle;
	ar << myTrackInRectangleLastTick;
	ar << myTrackPositionLastTick;
	ar << myTrackMetaRoomLastTick;
	ar << myTrackTransition;
	ar << myMiddleDrag;
	ar << myMiddleDragMX << myMiddleDragMY;

	// if you want you can alway make sure that the 
	// pointer stays a certain
	ar << myObjectToKeepUpWith;
	ar << myKeepUpSpeedX << myKeepUpSpeedY;
	ar << myKeepUpCountX << myKeepUpCountY;
	ar << myKeepUpPanX << myKeepUpPanY;
 
	ar << myChangingRoomsFlag;

	ar << myDisabledFlag;

	myBackground0.Write(ar);

	myBackground1.Write(ar);
	bool swap( myBackgrounds[0] == &myBackground1 );
	ar << swap;

	return true;
}

bool Camera::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		if(!DrawableObject::Read(ar))
			return false;

		myEntityHandler.Read(ar);
		myEntityHandler.SetOwner(this);

		ar >> myPanVelocity;
		ar >> myPanVector;
		ar >> myDistancePanned;
		ar >> myAcceleration;
		ar >> myMinimumSpeed;


		ar >> myViewWidth;
		ar >> myViewHeight;
		ar >> myViewArea;


		ar >> myWorldPosition;

		ar >> myTrackObject;
		
		ar >> myTrackPercentX >> myTrackPercentY;
		ar >> myTrackStyle;
		ar >> myTrackInRectangleLastTick;
		ar >> myTrackPositionLastTick;
		ar >> myTrackMetaRoomLastTick;
		ar >> myTrackTransition;
		ar >> myMiddleDrag;
		ar >> myMiddleDragMX >> myMiddleDragMY;

		// if you want you can alway make sure that the 
		// pointer stays a certain
		ar >> myObjectToKeepUpWith;
		ar >> myKeepUpSpeedX >> myKeepUpSpeedY;
		ar >> myKeepUpCountX >> myKeepUpCountY;
		ar >> myKeepUpPanX >> myKeepUpPanY;


		ar >> myChangingRoomsFlag;

		ar >> myDisabledFlag;

		if(!myBackground0.Read(ar))
			return false;

		myBackground0.SetOwner(this);

		if(!myBackground1.Read(ar))
			return false;

		myBackground1.SetOwner(this);

		bool swap;
		ar >> swap;
		if( swap )
		{
			myBackgrounds.push_back(&myBackground1);
			myBackgrounds.push_back(&myBackground0);
		}
		else
		{
			myBackgrounds.push_back(&myBackground0);
			myBackgrounds.push_back(&myBackground1);
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


// ----------------------------------------------------------------------
// Method:      ScreenToWorld 
// Arguments:   x - screen x position
//				y - screen y position
//
// Returns:     None
// Description: Converts screen coordinates to world coordinates
//				
// ----------------------------------------------------------------------
void Camera::ScreenToWorld(int& x, int& y)
	{
		x = x +  myViewArea.left;
		y = y +  myViewArea.top;
	}

void Camera::ScreenToWorld(Vector2D& vec)
	{
		vec.x = vec.x + (float)myViewArea.left;
		vec.y = vec.y + (float)myViewArea.top;
	}


// ----------------------------------------------------------------------
// Method:      WorldtoScreen 
// Arguments:   x - screen x position
//				y - screen y position
//
// Returns:     None
// Description: Converts world coordinates to screen coordinates
//				
// ----------------------------------------------------------------------
bool Camera::WorldToScreen(int& x, int& y)
{
	x = x -  myViewArea.left;
	y = y -  myViewArea.top;

	POINT point;
	point.x = x;
	point.y = y;

//	RECT displayRect;
//	DisplayEngine::theRenderer().GetDisplayArea(displayRect);
	return PointInRectangle(&myDisplayRect,point) ? true : false;
}

bool Camera::WorldToScreen(Vector2D& vec)
{
	vec.x = vec.x -  (float)myViewArea.left;
	vec.y = vec.y -  (float)myViewArea.top;

	POINT point;
	point.x = Map::FastFloatToInteger(vec.x);
	point.y = Map::FastFloatToInteger(vec.y);

	
	return PointInRectangle(&myDisplayRect,point) ? true : false;
}


// ----------------------------------------------------------------------
// Method:      ConvertToWorldCoordinates 
// Arguments:   rect - rectangle to convert from world co-ordinates to
//						the screen co-ordinates			
//
// Returns:     None
//
// Description: Converts teh bounds of the given rectangle into 
//				world coordinates
//						
// ----------------------------------------------------------------------
void Camera::ConvertToWorldCoordinates(RECT& rect)
{
	int32 width = rect.right - rect.left;
	int32 height = rect.bottom - rect.top;

	rect.top = rect.top + myViewArea.top;
	rect.bottom = rect.top + height;
	rect.left = rect.left + myViewArea.left;
	rect.right = rect.left + width;

}


// ----------------------------------------------------------------------
// Method:      ConvertToDisplayCoordinates 
// Arguments:   rect - rectangle to convert from world co-ordinates to
//						the screen co-ordinates			
//
// Returns:     None
//
// Description: Converts the bounds of the given rectangle into 
//				screen coordinates
//						
// ----------------------------------------------------------------------
void Camera::ConvertToDisplayCoordinates(RECT& rect)
{
	int32 width = rect.right - rect.left;
	int32 height = rect.bottom - rect.top;

	rect.top = rect.top - myViewArea.top;
	rect.bottom = rect.top + height;
	rect.left = rect.left - myViewArea.left;
	rect.right = rect.left + width;

}



void Camera::GetViewCentre(int32& centrex,int32& centrey)
{
	centrex = myViewArea.left + (myDisplayRect.right/2);

	centrey = myViewArea.top + (myDisplayRect.bottom/2);
}

// ----------------------------------------------------------------------
// Method:      SetViewArea 
// Arguments:   pos - camera x,y position
//				
//
// Returns:     None
// Description: Updates the view area to where the camera is positioned
//				
// ----------------------------------------------------------------------
void Camera::SetViewArea()
 {
	int32 displayWidth;
	int32 displayHeight;

	
	CalculateDisplayDimensions(displayWidth,displayHeight);


	 myViewArea.top = myWorldPosition.GetY();
	 myViewArea.bottom = myWorldPosition.GetY() + displayHeight;
	 myViewArea.left = myWorldPosition.GetX();
	 myViewArea.right =myWorldPosition.GetX() + displayWidth;


	 // only draw in this area - this takes the size of the
	 // background into consideration
	 myDisplayRect.top = 0;
	 myDisplayRect.bottom = displayHeight;
	 myDisplayRect.left = 0;
	 myDisplayRect.right = displayWidth;

 }

void Camera::GetViewArea(RECT& rect)
 {
    rect.top = myViewArea.top;
    rect.bottom = myViewArea.bottom;
    rect.left = myViewArea.left;
    rect.right = myViewArea.right;
 }


bool Camera::IsSameBackGround(const std::string& background)
{
	bool same = false;
	
	if(background == myBackgrounds[0]->GetBackgroundName())
		same = true;

	return same;
}

std::string Camera::GetBackgroundName()const
{
	return myBackgrounds[0]->GetBackgroundName();
}

// ----------------------------------------------------------------------
// Method:      UpdatePlane
// Arguments:   entityImage - the image to be updated
//		
//
// Returns:     None
//
// Description: an image has changed its plane order
//				remove it and add it so that it gets put in the correct
//				z order
//				 
//						
// ----------------------------------------------------------------------
void Camera::UpdatePlane(EntityImage* entityImage)
{
	Remove(entityImage,false);//don't stop tracking if you were being tracked
	Add(entityImage);		// as I am adding you in right now
}



void Camera::GetDisplayArea(RECT& rect)
{
    rect.top = myDisplayRect.top;
    rect.bottom = myDisplayRect.bottom;
    rect.left = myDisplayRect.left;
    rect.right = myDisplayRect.right;
}


void Camera::SetLoading(bool flag)
{
	myLoadingFlag = flag;
}
 

void Camera::DoTrackingChecks()
{
	if (myTrackObject.IsValid())
	{
		int offbyx, offbyy;
		if (!InTrackingRectangle(offbyx, offbyy))
		{
			// stop tracking if brittle and out of rectangle
			if (myTrackStyle == 0)
				StopTracking();
			// force stay in rectangle if hard tracking
			else if (myTrackStyle == 2)
				MoveBy(-offbyx, -offbyy);
			
			myTrackInRectangleLastTick = false;
		}
		else
			myTrackInRectangleLastTick = true;

		myTrackPositionLastTick = myWorldPosition;
	}
}



void Camera::SetPosition(Position& position)
{
	myWorldPosition = position;
}

bool Camera::SaveAsSpriteFile( std::string const &filename ) const
{
#ifndef C2E_SDL

	DDSURFACEDESC2	surfaceDescription;
	surfaceDescription.dwSize = sizeof( DDSURFACEDESC2 );

	if( mySurface->Lock( NULL, &surfaceDescription, DDLOCK_WAIT, NULL ) == DD_OK )
	{
		uint16* sourcePtr = (uint16*)surfaceDescription.lpSurface;
		int32 sourceStep = surfaceDescription.lPitch >> 1;
		uint16 width = surfaceDescription.dwWidth;
		uint16 height = surfaceDescription.dwHeight;
		int32 byteWidth = width << 1;

		std::ofstream stream( filename.c_str(), std::ios::binary | std::ios::out );

		uint32 pixelFormat, pixelFlag = 0;
		DisplayEngine::theRenderer().GetPixelFormat( pixelFormat );
		if( pixelFormat == RGB_565 ) pixelFlag = 1;
		uint16 count = 1;
		uint32 offset = 4 + 4 + 2 + 2 + 2;
		stream.write( (const char *) &pixelFlag, sizeof( uint32 ) );
		stream.write( (const char *) &count, sizeof( uint16 ) );
		stream.write( (const char *) &offset, sizeof( uint32 ) );
		stream.write( (const char *) &width, sizeof( uint16 ) );
		stream.write( (const char *) &height, sizeof( uint16 ) );
		while( height-- )
		{
			stream.write( (const char *)sourcePtr, byteWidth );
			sourcePtr += sourceStep;
		}
		mySurface->Unlock( NULL );
		return true;
	}
	else
		return false;

#else
	// non-directx
	return false;
#endif

}


Position Camera::GetBackgroundTopLeft()
{
	return myBackgrounds[0]->GetTopLeft();
}
