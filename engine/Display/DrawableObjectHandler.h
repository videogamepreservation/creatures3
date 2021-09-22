// --------------------------------------------------------------------------
// Filename:	DrawableObjectHandler.h
// Class:		DrawableObjectHandler
// Purpose:		This class manages all drawable objects. 
//				
//				
//
// Description: This handler receives objects and sorts them into plane order  
//				ready for drawing.  The entitie are put in myRenderObjects
//
//
//				There are two lists of rectangles:
//				myOldRects holds the previous frame's dirty rectangles. 
//				myNewRects holds the current frame's dirty rectangles.
//				Before doing a dirty rect draw the new list is merged into 
//				the old list.
//				When the entities have been drawn
//				myOldRects are discarded and MyNewRects are moved into myOldRects.			
//				
//
//				Note: Eventually there will be methods to add and remove
//					  entities from the handler rather than refreshing
//					  the whole list each update.
// History:
// ------- 
// 11Nov98	Alima			Created
//							Hooks are in to draw cameras
// 16Dec98  Alima			Added dirty Rect scheme
// 20Aug99  Daniel			Made it much better indeedy
// --------------------------------------------------------------------------

#ifndef		DRAWABLE_OBJECT_HANDLER_H
#define		DRAWABLE_OBJECT_HANDLER_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../mfchack.h"

#include	"DrawableObject.h"
#include	<list>
#include	<vector>
#include    <map>

////////////////////////////////////////////////////////////////////////////
// typedefs
////////////////////////////////////////////////////////////////////////////
typedef  std::list<DrawableObject*> DrawableObjectList;
typedef  std::vector<RECT>::iterator RECT_ITERATOR;

typedef std::pair<int,int> IntegerPair;
typedef std::list<IntegerPair> IntegerPairList;

typedef std::map<uint32,DrawableObjectList> DrawableObjectListMap;
typedef std::map<DrawableObject*,std::list<DrawableObject*>::iterator> DrawableObjectRenderMappings;

////////////////////////////////////////////////////////////////////////////
// forward declaration
////////////////////////////////////////////////////////////////////////////
class EntityImage;
class Camera;
class FastEntityImage;
class Line;

class DrawableObjectHandler
{
public:

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   maximum amount of entities this engine can handle 
//				
// Returns:     None
//
// Description: Creates the required amount of entities and puts them into
//				a list of free objects.
//						
// ----------------------------------------------------------------------	
	DrawableObjectHandler():myOwnerCamera(0),myShutDownFlag(0){;}

	DrawableObjectHandler(Camera* camera):myOwnerCamera(camera),myShutDownFlag(0){;}

	// ----------------------------------------------------------------------
	// Method:      Destructor 
	// Arguments:   None 			
	// Returns:     None
	//
	// Description: Deletes all objects.
	//						
	// ----------------------------------------------------------------------
	~DrawableObjectHandler();

	// ----------------------------------------------------------------------
	// Method:      Add 
	// Arguments:   camera or line
	//
	// Returns:     true - no fail conditions yet!
	//
	// Description: Cameras are just drawable enties which must be drawn in
	//				plane order
	//						
	// ----------------------------------------------------------------------
	bool Add(Camera* camera);

	bool Add(Line* const object);


	// ----------------------------------------------------------------------
	// Method:      Draw 
	// Arguments:   completeRedraw - whether to draw the whole screen or
	//								 just dirty rects.
	// Returns:     None
	//
	// Description: myRenderObjects has been presorted into plane order and
	//				non visible entities have been weeded out so just draw
	//				the list.
	//			
	// ----------------------------------------------------------------------
	void Draw(bool completeRedraw);

	// ----------------------------------------------------------------------
	// Method:      Update 
	// Arguments:   pos - new view position in world coords			
	//
	// Returns:     true - if update was OK false otherwise
	//
	// Description: This updates the items in the renderList and the 
	//				dirty rectangles and resets to the new view position
	//						
	// ----------------------------------------------------------------------
	void Update(Position pos);

	// ----------------------------------------------------------------------
	// Method:      InsertObject 
	// Arguments:   object - object to draw 
	// Returns:     None
	//
	// Description: The objects are put in a map of lists according to
	//				their positions on the screen.  Starting from left top to
	//				right bottom.
	//			
	// ----------------------------------------------------------------------
	bool InsertObject(DrawableObject*  const object, RECT* rect = NULL);


	void SetOwner(Camera* camera){myOwnerCamera = camera;
									myShutDownFlag = false;}

	////////////////////////////////////////////////////////////////////////////
	// Get and Set Methods...
	////////////////////////////////////////////////////////////////////////////


	bool Remove(EntityImage* const chop);
	bool Remove(Camera* const chop);

	bool Remove(Line* const chop);


	bool Add(EntityImage* const newEntity);

    std::vector<RECT>& GetUpdateList() {return myOldRects;}
	IntegerPairList& GetDirtyTileList(){return myOldDirtyTiles;}

	void GetScreenRect(RECT& rect)
	{
		 rect.top -= myWorldPosition.GetY();
		 rect.bottom -= myWorldPosition.GetY();
		 rect.right -= myWorldPosition.GetX();
		 rect.left -= myWorldPosition.GetX();
	}

	void SetWorldPosition(Position pos);

	void Update();
	void ShutDown();

	void AddFastRect(FastEntityImage* fastObject);

	void AddDirtyRect(RECT rect);

	void SetInterestLevel(bool interested);

	// ----------------------------------------------------------------------
	// Method:		Write
	// Arguments:	archive - archive being written to
	// Returns:		true if successful
	// Description:	Overridable function - writes details to archive,
	//				taking serialisation into account
	// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;


	// ----------------------------------------------------------------------
	// Method:		Read
	// Arguments:	archive - archive being read from
	// Returns:		true if successful
	// Description:	Overridable function - reads detail of class from archive
	// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);

	inline bool IsRectOnScreen(RECT& rect,RECT& displayRect);

	void MakeSetCurrentBoundsGetCalled()
	{
		ourAlreadyDoneSetCurrentBoundsThisTick = false;
	}

protected:
	bool amIInterestedInCameraShyObjects;
private:

	static bool ourAlreadyDoneSetCurrentBoundsThisTick;

	// Copy constructor and assignment operator
	// Declared but not implemented
	DrawableObjectHandler (const DrawableObjectHandler&);
	DrawableObjectHandler& operator= (const DrawableObjectHandler&);

	// objects that need updating

	static DrawableObjectListMap myRenderObjects;
	static DrawableObjectRenderMappings myRenderMappings;

	// rectangles dirtied in the last draw
	std::vector<RECT> myOldRects;

	// current dirty rectangles
	std::vector<RECT> myNewRects;

	// current dirty rectangles of fast objects
	std::vector<RECT> myFastRects;

	IntegerPairList myNewDirtyTiles;
	IntegerPairList myOldDirtyTiles;

	// where are we looking in the world?
	Position myWorldPosition;

	Camera* myOwnerCamera;
	bool myShutDownFlag;

};

bool DrawableObjectHandler::IsRectOnScreen(RECT& rect,RECT& displayRect)
{
	// return if rect and displayRect intersect at all (Intersection is not important here)
	return ((rect.right >= displayRect.left) &&
			(rect.left <= displayRect.right) &&
			(rect.top <= displayRect.bottom) &&
			(rect.bottom >= displayRect.top));
}

/*
{
	POINT topLeft;
	POINT bottomRight;
	POINT bottomLeft;
	POINT topRight;

   //weed out rects that are just not on screen
    topLeft.x = rect.left;
    topLeft.y = rect.top;
    bottomRight.x = rect.right;
    bottomRight.y = rect.bottom;
	bottomLeft.x = rect.left;
    bottomLeft.y = rect.bottom;
	topRight.x = rect.right;
	topRight.y = rect.top;


	// check that the object is not totally off screen!
	// and check whether the object is so large that its top and bottom are off screen!
	// or so wide that the left and right edges are off screen

	return ((PointInRectangle(&displayRect,topLeft) || PointInRectangle(&displayRect,bottomRight)
		   ||PointInRectangle(&displayRect,bottomLeft) || PointInRectangle(&displayRect,topRight))
		   || rect.top <= displayRect.top && rect.bottom >= displayRect.bottom 
		   || rect.left <= displayRect.left && rect.right >= displayRect.right);

}
*/
#endif		// DRAWABLE_OBJECT_HANDLER_H