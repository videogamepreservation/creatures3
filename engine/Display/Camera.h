// --------------------------------------------------------------------------
// Filename:	Camera.h
// Class:		Camera
// Purpose:		This class provides a view of the world to clients.
//				The camera has a position in world coordinates.
//				The interface will give te client the usual camera
//				operations e.g. panning, tracking. 
//
//				Each camera must have access to a display engine.  This is
//				generally passed by the parent window.
//				
//				
//
// Description: All entities are stored in myEntities.  At the start all objects  
//				are on the free list.  From there they can be put on the 
//				update list.
//			
//				

//
// History:
// -------  
// 11Nov98	Alima			Created.
// --------------------------------------------------------------------------
#ifndef		CAMERA_H
#define		CAMERA_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Position.h"
#include "DrawableObjectHandler.h"
#include "Background.h"
#include "DrawableObject.h"
#include	"../../common/BasicException.h"

#ifndef C2E_SDL
#include <ddraw.h>
#endif



////////////////////////////////////////////////////////////////////////////
// typedefs
////////////////////////////////////////////////////////////////////////////
typedef std::vector<std::string>::iterator BACKGROUND_NAME_ITERATOR;

enum TransitionModes
{
	NOFLIP = 0,
	FLIP,
	BURST,
	SHRINK,
	NUM_TRANSITION_MODES,
};

class DrawableObjectHandler;
class Background;



#include "../Agents/AgentHandle.h"

class Camera : public PersistentObject, public DrawableObject
{
	CREATURES_DECLARE_SERIAL( Camera )
public:

	Camera();

Camera(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground,
			uint32 topLeftXofBackground, // top left world co-ordinates 
			uint32 topLeftYofBackground // of background as a meta room
			); 

Camera::Camera(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
				int32 width, // width of camera
				int32 height, // height of camera
				std::string& defaultBackground,
				RECT& bounds // of background as a meta room
			); 

	void InitialiseCameraVariables();

FilePath MakeBackgroundPath( std::string const &backgroundName);
//void MakeBackgroundPath(std::string& back1, std::string backgroundName);

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
bool Create(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground1,
			int32 topLeftXofBackground, // top left world co-ordinates 
			int32 topLeftYofBackground// of background as a meta room
			); 


bool Create(int32 viewx, // world x co-ordinate of view
			   int32 viewy, // world y co-ordinate of view
			int32 width, // width of camera
			int32 height, // height of camera
			std::string& defaultBackground1,
			RECT& bounds// of background as a meta room
			); 

virtual void ChangeMetaRoom(std::string& backgroundName,	
		RECT& rect,
			int32 newviewx,
			int32 newviewy,
			int32 flip,
			bool bCentre);

void SetBackground(std::string& backgroundName,
						   int32 transitionMethod);

void AddDirtyRect(RECT rect);

void DoChangeMetaRoom(std::string& backgroundName,
			RECT& bounds,
			int32 newviewx,
			int32 newviewy,
			bool useTopLeftOnly,
			bool bCentre);

	virtual	~Camera();

	void SetPosition(Position& position);

	virtual Position GetWorldPosition(void);

// ----------------------------------------------------------------------
// Method:      Update
// Arguments:   completeRedraw - whether to do a complete redraw or just 
//              dirty rectangles.
// Returns:     None
// Description: Tells the display engine to update camera
//			
// ----------------------------------------------------------------------
	void Update(bool completeRedraw,
				bool justBackBuffer = false);

	void NormaliseWorldPosition();
	void NormaliseWorldCoordinates(int32& x, int32& y);

    // attempts to draw this camera are foiled
	virtual void Draw(){;}
	virtual void DrawMirrored(){;}
	virtual void SetScreenPosition(Position pos){;}


	void CentreMe(Position& pos);
	void CentreMe(RECT& rect);

	void GetDisplayArea(RECT& rect);

// ----------------------------------------------------------------------
// Method:      Get/SetPlane 
// Arguments:   None/plane - the plane in which you wish the entity
//				to reside
//
// Returns:     The plane that the entity exists on/None
//			
// ----------------------------------------------------------------------
//	virtual int32 GetPlane() const
//	{
	// this is purely a copy of what entity used to do
//	return -1;
//	}

//	virtual void SetPlane(int plane){;}



// ----------------------------------------------------------------------
// Method:      Visible
// Arguments:   test - rectangle inside which to test whether I am visible.
// Returns:     true if i am visible within that rectangle false otherwise
// Description: Tells caller whether the entity exists inside the bounds
//              of the given rectangle
//
//              dummy function returns true until I have sussed 
//              cameras out - needs to be here because it is a property
//				of a drawable object of course
//			
// ----------------------------------------------------------------------
    virtual bool Visible(RECT& rect);

// ----------------------------------------------------------------------
// Method:      SetEntityImageList
// Arguments:   array - list of entityImages to deal with next
//				count - the number of images we are dealing with
// Returns:     None
// Description: This allows the caller to tell us which entities
//				we are working with next.  The array will change
//				to an STL mechanism when i know a bit more about
//				the room model
//			
// ----------------------------------------------------------------------
	void MoveTo(int32 x, int32 y, int pan = 0);

//	bool WorldToScreen(int& x, int& y);
//	void ScreenToWorld(int& x, int& y);
//	bool WorldToScreen(Vector2D& vec);
//	void ScreenToWorld(Vector2D& vec);

	bool Pan();
	void PanTo(int32 x, int32 y);

	// you cannot zoom the main camera!
	virtual void ZoomBy(int32 pixels,int32 x, int32 y){;}


	void MoveBy(int32 x,int32 y);

	void DoUpdateAfterAdjustments();

	virtual void CalculateDisplayDimensions(int32& displayWidth, int32& displayHeight);



	virtual void SetCurrentBound(RECT* rect = NULL){;}


// ----------------------------------------------------------------------
// Method:      Remove
// Arguments:   newEntity - pointer to new entity to chop from the
//				list
// Returns:     true if the entity was found and removed false otherwise
// Description: This removes the given entityImage from the current update
//				list.
//			
// ----------------------------------------------------------------------
virtual bool Remove(EntityImage* const chop,bool stopTracking = true);
virtual bool Remove(Line* const chop);

// ----------------------------------------------------------------------
// Method:      Add
// Arguments:   newEntity - pointer to new entity to add to the 
//				list
// Returns:     None
// Description: This adds the given entityImage to the current update
//				list.
//			
// ----------------------------------------------------------------------
virtual void Add(EntityImage* const newEntity);

virtual void Add(Line* const newEntity);

virtual void Track(AgentHandle& agent, int xpercent, int ypercent, int style, int transition);

void SetTrackMetaRoom();

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
virtual void UpdatePlane(EntityImage* entityImage);

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
bool ChangeDisplayMode(uint32 width, uint32 height);

Position GetBackgroundTopLeft();



void FadeScreen();


bool IsPointOnScreen(int32 x, int32 y);

virtual void Refresh();

virtual bool TrackObject();
void StopTracking();
bool InTrackingRectangle();
bool InTrackingRectangle(int& offbyx, int& offbyy);
AgentHandle GetTrackAgent();

void YourTrackObjectHasGrownUp(AgentHandle& agent);

void DoTrackingChecks();

void CentriseCoordinates(int32& x, int32& y);
void CentreOn(int32 x, int32 y, int pan = 0);

void KeepUpWithMouse(EntityImage* me);
bool KeepUpThenWithMouse();
bool MiddleMouseDragMouseWheel();

void AddFloatingThing(AgentHandle& a);
void RemoveFloatingThing(AgentHandle& a);

void ScreenToWorld(int& x, int& y);
void ScreenToWorld(Vector2D& vec);
bool WorldToScreen(int& x, int& y);
bool WorldToScreen(Vector2D& vec);

void ConvertToWorldCoordinates(RECT& rect);
void ConvertToDisplayCoordinates(RECT& rect);
void GetViewCentre(int32& centrex,int32& centrey);
void GetViewArea(RECT& rect);
void SetViewArea();

int32 GetViewWidth(void){return myViewArea.right-myViewArea.left;}
int32 GetViewHeight(void){return myViewArea.bottom- myViewArea.top;}
int32 LeftOfView(){return myViewArea.left;}
int32 TopOfView(){return myViewArea.top;}

bool IsSameBackGround(const std::string& background);


std::string GetBackgroundName()const;

void SetLoading(bool flag);

void Flip(int32 flip);

int	 GetTrackTransition(){return	myTrackTransition;}
void SetTrackTransition(int trans){myTrackTransition = trans;}

virtual bool IsRemote(){return false;}


// new serialization stuff
virtual bool Write( CreaturesArchive &ar ) const;
virtual bool Read( CreaturesArchive &ar );
bool SaveAsSpriteFile( std::string const &filename ) const;


 //////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
class CameraException: public BasicException
{
public:
	CameraException(std::string what, uint32 line):
	  BasicException(what.c_str()),
	lineNumber(line){;}

	uint32 LineNumber(){return lineNumber;}
private:

	uint32 lineNumber;

};

	
protected:
	
	// Copy constructor and assignment operator
	// Declared but not implemented
	Camera (const Camera&);
	Camera& operator= (const Camera&);

	// camera direction
	Position					myPanVelocity;
	Position					myPanVector;
	Position					myDistancePanned;
	int32						myAcceleration;
	int32						myMinimumSpeed;
		
	// the actual size of the area we are displaying
	// based on the size of the current background
	RECT					myDisplayRect;


	// the size of my display in pixels
	// starting from the top left world coordinate
	// position
	RECT						myViewArea;
	// easier to store these explicitly
	int32						myViewWidth;
	int32						myViewHeight;


	Position					myWorldPosition; // this is the view
	AgentHandle					myTrackObject;
	int32						myTrackPercentX, myTrackPercentY;
	int32						myTrackStyle;
	bool						myTrackInRectangleLastTick;
	Position					myTrackPositionLastTick;
	int							myTrackMetaRoomLastTick;
	int							myTrackTransition;
	bool						myMiddleDrag;
	int							myMiddleDragMX, myMiddleDragMY;
	

	// if you want you can alway make sure that the 
	// pointer stays a certain
	EntityImage*				myObjectToKeepUpWith;
	int32						myKeepUpSpeedX, myKeepUpSpeedY;
	int32						myKeepUpCountX, myKeepUpCountY;
	int32						myKeepUpPanX, myKeepUpPanY;

//	int32						myEntityImageCount;

	DrawableObjectHandler		myEntityHandler;

	bool						myChangingRoomsFlag;

	bool						myDisabledFlag;

	// empty shells of 2 backgrounds
	// to take the incoming and outgoing backgrounds
	Background					myBackground0;
	Background					myBackground1;

//	RECT myScreenBound;

	// so that i can index between the ingoing and outgoing
	// backgrounds and flip them about of necessary
	// the first is the incoming background
	// the second is the outgoing background
	std::vector<Background*>	myBackgrounds;

	// things that need to move with the camera
	std::vector<AgentHandle>			myFloatingThings;

	// whether to do complete redraw next frame
	bool						myCompleteRedraw;
	
#ifndef C2E_SDL
	IDirectDrawSurface4* mySurface;
#endif

	bool myLoadingFlag;


};

#endif		// CAMERA_H