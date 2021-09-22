// --------------------------------------------------------------------------
// Filename:	EntityImage.cpp
// Class:		EntityImage
// Purpose:		This class is the interface to the rest of C2e.  This class 
//				is closely based on the C2 entity and will evolve as required.
//				Some member may appear now to allow integration with the current
//				version of the C2e engine but be removed later.
//				
//				
//
// Description: 
///* Notes:
//
//	An entity is a 'thing' that exists in the game world (unlike a sprite
//	for example, which only exists in the computer). Entities are contained
//	within the various types of Object (creatures, machines, etc.); some
//	objects may contain several Entities.
//
//	An entity is the most primitive thing that exists in the game world.
//	On its own it represents simple, one-pose, unintelligent objs but it
//	is inherited by other classes for creating more complex objs
//
//	Entities are the things that get redrawn when a rectangle is invalidated.
//	A library of pointers to all extant entities is stored in EntityLib[]
//	for this purpose.
//
//	Entities are ALWAYS encapsulated in Objects of one kind or another. Thus,
//	although each entity contains its own X&Y members, these get set as
//	appropriate by the Object which owns them, and don't need to be
//	initialised when creating the entity itself. Thus, entities can be
//	created dynamically by the objects that contain them.
//
//
//	Entities:
//
//
//	Entity				Simplest form of entity, instantiated for use
//	|					within SimpleObjects and CompoundObjects.
//	|					Also serves as base class for articulated entities,
//	| 					for encapsulation within Creatures.
//	| 					Whereas a Sprite uniquely represents a given bitmap,
//	| 					several Entities can refer to the same Sprite
//	| 					- an Entity represents the simplest individual
//	| 					object in the world; several Entities may LOOK
//	| 					the same, but ARE different.
//	|
//	|
//	BodyPart
//	| |
//	| Body				Parts of articulated objects. A Limb is connected to
//	| 					one or no daughter Limbs (eg. a thigh Limb connects
//	| 					to a shin Limb); A Body is similar, but connects
//	Limb                to several Limb chains. The BodyPart class contains
//						methods & data that are common to both types
//						Body & Limb classes contain pointers to BodyData
//						or LimbData classes respectively.
//
//						The BodyPart Entities contain pointers to LimbData
//						or BodyData structs, which contain the XY
//						information that is common to all BodyParts with
//						the same appearance.
//
//
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added comments. 
//							Changed Class and file names from Object to Entity.
//							Introducing Methods from the C2 Entity class.
//
// 27Nov98	Alima			Made constructor and destructor public so that the
//							C2e engine can create EntityImages.
//							Adding galleries to EntityImages to replace current 
//							single bit map method.  Eventually I will remove
//							old method.
// 01Feb98 Alima			removed all references to physicals
// --------------------------------------------------------------------------

#ifndef		ENTITYIMAGE_H
#define		ENTITYIMAGE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include	"Gallery.h"
#include	"../PersistentObject.h"
#include	"../../common/BasicException.h"
#include	"Position.h"



// forward declarations
//class EntityHandler;
class Sprite;
class FastEntityImage;
class Line;

const uint32 MaxAnimLength = 100;
const uint32 AnimWrapValue = 255;

class EntityImage: public PersistentObject
{
	CREATURES_DECLARE_SERIAL( EntityImage )

public:

	enum {PLANE_NEAREST_THE_FRONT = 0x7FFFFFFF};

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   None
//				
// Returns:     None
//
// Description: Creates an entity	
//						
// ----------------------------------------------------------------------
	EntityImage();


	EntityImage(FilePath const& galleryName, 
			int32 numimages,
		int32 plane,//plane
		 int32 worldx, // world x
		 int32 worldy, // world y
		 uint32 baseimage,		// base image
		 uint32 currentimage );	// current image

	void Init(int32 plane,//plane
		 int32 worldx, // world x
		 int32 worldy, // world y
		 uint32 baseimage,		// base image
		 uint32 currentimage,
		 uint32 numImages);

// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   None 			
// Returns:     None
//
// Description: 
//						
// ----------------------------------------------------------------------
	virtual ~EntityImage();


void Link(bool forceLink = false);

void Unlink();

void MakeFastImage();

   Vector2D CentrePoint() const
    {   
        return Vector2D(
		    GetWorldX()+ 
				(GetWidth() / 2.0f), 
			GetWorldY() +
				(GetHeight() / 2.0f));
    }

   void CentreMe();

// ----------------------------------------------------------------------
// Method:      Get/SetPosition 
// Arguments:   None/position - the position in which you wish the entity
//				to reside
//
// Returns:     The position that the entity exists in/None
//			
// ----------------------------------------------------------------------
	Position& GetPosition(void)
	{
		return myWorldPosition;
	}


	virtual void SetPosition(int32 x, int32 y);



	virtual void AdjustXY(int32 x, int32 y)
	{
		myWorldPosition.AdjustX(x);
		myWorldPosition.AdjustY(y);
	}

	void DrawMirrored(bool mirror);

	bool IsMirrored();


// ----------------------------------------------------------------------
// Method:      Get/SetWorldX/WorldY
// Arguments:   None/x,y - the x or y position in which you wish 
//				the entity to reside
//
// Returns:     The x or y position that the entity exists in/None
//			
// ----------------------------------------------------------------------
int32 GetWorldX() const
{
	return myWorldPosition.GetX();
}

int32 GetWorldY() const
{
	return myWorldPosition.GetY();
}

virtual void SetWorldXY(int32 x, int32 y)
{
	myWorldPosition.SetX(x);
	myWorldPosition.SetY(y);

}


// ----------------------------------------------------------------------
// Method:      SetSprite 
// Arguments:   sprite - set the first sprite in my list
//
// Returns:     None
//			
// ----------------------------------------------------------------------
	void SetSprite(Sprite* sprite);

	Sprite& GetSprite() const
	{
		return *mySprite;
	}

	Sprite* GetSpritePtr() const
	{
		return mySprite;
	}

// ----------------------------------------------------------------------
// Method:      Get/SetPlane 
// Arguments:   None/plane - the plane in which you wish the entity
//				to reside
//
// Returns:     The plane that the entity exists on/None
//			
// ----------------------------------------------------------------------
	int32 GetPlane() const
	{
		return myPlane;
	}

	virtual void SetPlane(int plane);

// ----------------------------------------------------------------------
// Method:      Get/SetCurrentIndex 
// Arguments:   None/image - the index of the current sprite showing
//
// Returns:     The index of the current sprite showing/None
//			
// ----------------------------------------------------------------------
	uint32 GetCurrentIndex() const
	{
		return myCurrentImageIndex;
	}

	void SetCurrentIndex(uint32 image) ;

	void HideCurrentBitmap();

	void ShowCurrentBitmap();


	void SetCurrentIndexFromBase(uint32 image);

	bool SetOverlayIndex(int32 image,int32 layer);

	void CreateGalleryUponSerialisation(bool flag){myReadArchivedGalleryFlag = flag;}

// ----------------------------------------------------------------------
// Method:      Get/SetBaseIndex 
// Arguments:   None/baseimage - the index of the first image
//
// Returns:     The index of first sprite Image/None
//			
// ----------------------------------------------------------------------

	bool SetBaseAndCurrentIndex(uint32 baseimage);

	uint32 GetBaseAndCurrentIndex()
	{
		return myCurrentBaseImageIndex - myAbsoluteBaseImage;
	}

	bool ValidatePose(int pose)
	{
		if (pose < 0 || myCurrentBaseImageIndex < 0)
			return false;

		if ((myCurrentBaseImageIndex + pose) >= myAbsoluteBaseImage + myNumberOfImages)
			return false;

		if ((myCurrentBaseImageIndex + pose) >= myGallery->GetCount())
			return false;
		
		return true;
	}

// ----------------------------------------------------------------------
// Method:      GetWidth/Height
// Arguments:   index - image to return the width or height of.
//
// Returns:     The width of the image at the specified index
//			
// ----------------------------------------------------------------------
	inline int32 GetWidth() const{return myCurrentWidth;}

	inline int32 GetHeight() const{return myCurrentHeight;}

	inline void SetCurrentWidth(int32 width){myCurrentWidth = width;}
	inline void SetCurrentHeight(int32 height){myCurrentHeight = height;}


	void CreateTest();


	// ----------------------------------------------------------------------
	// Method:		SetAnim
	// Arguments:   newanim - array of indexes (unsigned bytes)
	//				length - size of newanim array
	// Returns:		None
	// Description:
	// Starts an animation. Each frame in the anim is taken as an offset
	// from the current base index (see Get/SetBaseIndex()).
	// If the last frame in the animation is 255 (EntityImage::animWrapValue)
	// the animation loops. Otherwise, the animation stops at the last frame.
	// SetAnim(NULL) is a valid call, and will stop a currently running anim.
	// The maximum length of an anim, including loop byte, is set by
	// EntityImage::maxAnimLength.
	// ----------------------------------------------------------------------
	void SetAnim( const uint8* newanim=NULL, int length=0 );
	bool ValidateAnim( const uint8* newanim, int length=0 );

	// ----------------------------------------------------------------------
	// Method:		SetFrameRate
	// Arguments:	rate - the frame rate
	// Returns:		None
	// Description:
	//  Sets the ticks per animation frame for the entity.
	// ----------------------------------------------------------------------
	void SetFrameRate(const uint8 rate = 1);

	// ----------------------------------------------------------------------
	// Method:		AnimOver
	// Arguments:   None
	// Returns:		true if anim has finshed
	// Description: Tells if an anim has finished playing.
	// ----------------------------------------------------------------------
	bool AnimOver();

	// ----------------------------------------------------------------------
	// Method:		Animate
	// Arguments:   None
	// Returns:		None
	// Description:
	// Updates the currently-running animation (if any). Should be called
	// every tick.
	// ----------------------------------------------------------------------
	void Animate();




// ----------------------------------------------------------------------
// Method:      GetBound
// Arguments:   rect - a rect structure to receive the bounds 
//
// Returns:     None
//			
// ----------------------------------------------------------------------
	void GetBound(RECT& r) const
	{
		r.left   = myWorldPosition.GetX();
		r.top    = myWorldPosition.GetY();
		r.right  = myWorldPosition.GetX()+GetWidth();
		r.bottom = myWorldPosition.GetY()+GetHeight();
	}

	void GetLargestBound(RECT& r);

// ----------------------------------------------------------------------
// Method:      Get/SetPose
// Arguments:   None/the pose you want to change to 
//
// Returns:     return the actual pose number/None
//			
// ----------------------------------------------------------------------
	int32 GetPose()
	{
		return (myCurrentImageIndex - myCurrentBaseImageIndex);
	}

	void SetPose(int32 pose);

	// Get pose relative to the absolute base
	int32 GetAbsolutePose()
	{
		return (myCurrentImageIndex - myAbsoluteBaseImage);
	}

	int32 GetAbsolutePoseMax()
	{
		return myNumberOfImages;
	}


	int32 GetAbsoluteBaseImage()
	{
		return myAbsoluteBaseImage;
	}


	void AdoptMe(FastEntityImage* boyAmIFast);
	void LosingFastImage();

	void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen = 0,
					uint8 lineColourBlue = 0,
						 uint8 stippleon =0,
							 uint8 stippleoff= 0,
							 uint32 stipplestartAt =0) ;

	void ResetLines();



// ----------------------------------------------------------------------
// Method:      SetGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//				baseimage - the first image in the gallery relating to this
//				entity.
// Returns:     None
//
// Description: Create a gallery for this entity 	
//						
// ----------------------------------------------------------------------
	virtual bool SetGallery(FilePath const& galleryName,
							uint32 baseimage,
							uint32 numImages = 0,
							bool reset = true);


	virtual void ReloadGallery(FilePath const& galleryName, 
							 uint32 baseimage,
							 uint32 numImages = 0);

	// Sometimes as for Creatures the removal of theCreature gallery
	// is done in one fell swoop so as to same time rather
	// than having each limb decrement the reference count.
	// when this happens the entity image should be prepared for it.
	void PrepareForReloadingGallery();

	void YouAreCameraShy(bool really);
	bool AreYouCameraShy();

	void SetOverlayGallery(FilePath const& galleryName);

	Gallery* GetGallery()  const
	{ 
		return myGallery;
	}

	bool RemoveGallery();

// ----------------------------------------------------------------------
// Method:      GetBitmap 
// Arguments:   index - index into sprite gallery
//				
// Returns:     pointer to the selected bitmap
//
// Description: returns a bitmap specified by index	
//						
// ----------------------------------------------------------------------
	Bitmap* GetBitmap(int32 index);

	Bitmap* GetCurrentBitmap();

	bool SetCurrentBitmap();


	bool Visible(const RECT& test);
	bool Visible(int32 scope);

	bool IsPixelTransparent(uint32 x,
							uint32 y,
							int32 imageIndex = -1);

	uint16* GetImageData() const ;

	// set the gallery back to the default it was created with
	// if the name is empty
	bool ResetMainGallery(FilePath & name);

	void DestroyGallery();

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

//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
class EntityImageException: public BasicException
{
public:
	EntityImageException(std::string what, uint32 line):
	  BasicException(what.c_str()),
	lineNumber(line){;}

	uint32 LineNumber(){return lineNumber;}
private:


	uint32 lineNumber;

};



protected:
	void Init();

	// Copy constructor and assignment operator
	// Declared but not implemented
	EntityImage (const EntityImage&);
	EntityImage& operator= (const EntityImage&);

	// Plane is used to determine plot order (0=back)
	int32 myPlane;

	// x,y Location in the world
	Position	myWorldPosition;

	//list of sprites for this entity
	Sprite*		mySprite;
	FastEntityImage* myFastEntityImage;

	// Currently displaying image (index into list of sprites) 
	uint32 myCurrentImageIndex;
	// Base image for entity
	uint32 myCurrentBaseImageIndex;

	uint32 myAbsoluteBaseImage;
	bool myReadArchivedGalleryFlag;

	// a gallery of sprites
	Gallery*	myGallery;
	FilePath myDefaultGalleryName;
	uint32 myDefaultBaseImage;
	uint32 mySavedImageIndexState;
	uint32		myNumberOfImages;


	// a gallery of possible overlays
	Gallery*	myOverlayGallery;
	int32 myCurrentOverlayIndex;


	// Animation variables
	uint32		myAnim[ MaxAnimLength ]; // Expands the uint8's to 32's for efficience
	int32			myAnimLength;		// 0 if no anim playing
	int32			myAnimNext;			// next frame to show
	uint32		myFrameRate;			// Frame Rate
	uint32		myFrameCount;			// Frames expired

	bool		myVisibleFlag;

	std::vector<Line*> myLines;
	
	uint32 myCurrentWidth;
	uint32 myCurrentHeight;

	bool mySpriteIsCameraShy;
};

#endif		// ENTITYIMAGE_H
