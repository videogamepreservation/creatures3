// --------------------------------------------------------------------------
// Filename:	EntityImage.cpp
// Class:		EntityImage
// Purpose:		This class is the interface to the rest of C2e.  This class 
//				is closely based on the C2 entity and will evolve as required.
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
//
//				
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added comments. 
//							Changed Class and file names from Object to EntityImage.
//							Introducing Methods from the C2 Entity class.
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"EntityImage.h"
#include	"Sprite.h"
#include	"CompressedSprite.h"
#include	"SharedGallery.h"
#include	"../AgentManager.h"
#include	"MainCamera.h"
#include	"ErrorMessageHandler.h"
#include	"../resource.h"
#include	"FastEntityImage.h"
#include "DisplayEngine.h"
#include	"Line.h"
#include "../App.h"

//make sure that this is serializable
CREATURES_IMPLEMENT_SERIAL( EntityImage)

EntityImage::EntityImage():
	mySprite(NULL),
	myGallery(NULL),
	myFastEntityImage(0),
	myOverlayGallery(0),
	myCurrentOverlayIndex(-1),
	myReadArchivedGalleryFlag(1),
	myCurrentBaseImageIndex(0),
myAbsoluteBaseImage(0),
myCurrentImageIndex (0),
mySavedImageIndexState(0),
myFrameRate(1), 
myFrameCount(0),
myCurrentWidth(0),
myCurrentHeight(0),
myVisibleFlag(0),
mySpriteIsCameraShy(false)
{
	;
}


EntityImage::EntityImage(FilePath const& galleryName,
						int32 numimages,
		int32 plane,
		 int32 worldx, 
		 int32 worldy, 
		 uint32 baseimage,	
		 uint32 currentimage )			
:myNumberOfImages(numimages),
myOverlayGallery(0),
myCurrentOverlayIndex(-1),
myFastEntityImage(0),
mySprite(0),
myReadArchivedGalleryFlag(1),
myFrameRate(1), myFrameCount(0),
myCurrentWidth(0),
myCurrentHeight(0),
myVisibleFlag(0),
mySpriteIsCameraShy(false)
{


	myPlane = plane;

	myGallery = NULL;

	// keep these values for when the gallery gets reset
	myDefaultGalleryName = galleryName;
	myDefaultBaseImage = baseimage;

	myCurrentImageIndex = baseimage;
	myCurrentBaseImageIndex = baseimage;

	// allow for entity images with blank galleries now
	if(!galleryName.empty())
	{
	SetGallery(galleryName,  baseimage);
	// check that you now have a gallery
	if(!myGallery)
		{
		char buf[_MAX_PATH] = "\0";

		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)DisplayEngine::sidNoGalleryCreated,
									std::string("EntityImage::EntityImage"),
									galleryName.GetFullPath().c_str());

	
		throw EntityImageException(string,__LINE__);
		}
	

	Init(plane,//plane
		 worldx, // world x
		 worldy, // world y
		 baseimage,	// base image
		 currentimage,
		 numimages);
	}
}

void EntityImage::YouAreCameraShy(bool really)
{
	mySpriteIsCameraShy = really;
	mySprite->YouAreCameraShy(really);
}

bool EntityImage::AreYouCameraShy()
{
	return mySprite->AreYouCameraShy();
}

void EntityImage::Init(int32 plane,//plane
		 int32 worldx, // world x
		 int32 worldy, // world y
		 uint32 baseimage,					// base image
		 uint32 currentimage,
		 uint32 numimages)
{
	// myNumberOfImages is relative to myAbsoluteBaseImage (baseimage)
	if(numimages)
		myNumberOfImages = numimages;
	else 
		myNumberOfImages = myGallery->GetCount() - baseimage;

	myAnimLength = 0;

	myPlane = plane;
	myWorldPosition.SetX(worldx);
	myWorldPosition.SetY(worldy);
	myCurrentBaseImageIndex = baseimage;
	myAbsoluteBaseImage = baseimage;
	myCurrentImageIndex = baseimage;
	mySavedImageIndexState = currentimage;
	SetCurrentBitmap();

	Link();
}

EntityImage::~EntityImage()
{
	Unlink();

	if(myGallery)
	{
		SharedGallery::RemoveGallery(myGallery);
		myGallery = NULL;
	}

	ResetLines();

	if(mySprite)
	{
		delete mySprite;
		mySprite = NULL;
	}
	if(myFastEntityImage)
	{
		myFastEntityImage->Destroy();
	}

}

// when reloading the skeleton we don't want to register
// the entity image with the display until the very last minute
void EntityImage::Link(bool forceLink /*=false*/)
{
	if(myReadArchivedGalleryFlag || forceLink)
	{
		theMainView.Add(this);
		 myVisibleFlag = true;
	}
}

void EntityImage::Unlink()
{

	theMainView.Remove(this);
	myVisibleFlag = false;

}

void EntityImage::MakeFastImage()
{
	theMainView.MakeFastImage(*this);
	// the fast object should now do what it
	// has to do to get drawn wash your hands of it
	Unlink();
}


void EntityImage::AdoptMe(FastEntityImage* boyAmIFast)
{
	myFastEntityImage = boyAmIFast;
}

void EntityImage::LosingFastImage()
{
	myFastEntityImage = NULL;
}

// ----------------------------------------------------------------------
// Method:      SetGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//				baseImage - the image in the sprite file that this
//							 particular entity will start from 
//				numImages - the number of images for this sprite
//							
//				
// Returns:     None
//
// Description: Create a gallery for this entity  and set up a
//				sprite to hold your current image data	in this base class
//				the number of images is largely ignored, override if you 
//				want to utilise it
//						
// ----------------------------------------------------------------------
bool EntityImage::SetGallery(FilePath const& galleryName, 
							 uint32 baseimage,
							 uint32 numImages /*= 0*/,
							 bool reset /*= true*/)
{
	if(galleryName.GetFullPath().empty())
		return false;
	
	myAbsoluteBaseImage = baseimage;

	// if you already have a gallery get rid of it
	if(myGallery && 
		myGallery->GetName() != galleryName)
	{
	
		SharedGallery::theSharedGallery().
									RemoveGallery(myGallery);
		myGallery = 0;
	}


		if (!myGallery)
	{
		// entity images can use one of several galleries
		// let the gallery decide which type of sprite to create 
		myGallery = SharedGallery::theSharedGallery().CreateGallery(galleryName);

		// now we have a gallery we can create our sprite
		if(myGallery)
		{
			if(mySprite)
			{
				delete mySprite;
				mySprite = NULL;
			}

			Sprite* sprite = myGallery->CreateSprite(this);	
		

			ASSERT(sprite);
			SetSprite(sprite);
		
			// gallery could be reset during the game
			// so we need to update the current width and height of
			// any new bitmap
			if( reset ) myCurrentImageIndex = baseimage;
	
			SetCurrentBitmap();

			return true;
		}
	}
		


	return false;

}





// ----------------------------------------------------------------------
// Method:      SetOverlayGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//		
//				
// Returns:     None
//
// Description: Create an overlay gallery for this entity.  Certain
//				types of entity (body parts) will want to replace their
//				normal image by one from an overlay gallery.  The overlay
//				gallery is optional and must be created explicitly if
//				it is needed.
//						
// ----------------------------------------------------------------------
void EntityImage::SetOverlayGallery(FilePath const& galleryName)
{
	if(galleryName.GetFullPath().empty())
		return;

	// If you have a gallery, and it is not the one you want...
	if(myOverlayGallery && myOverlayGallery->GetName() != galleryName)
	{
		SharedGallery::theSharedGallery().
									RemoveGallery(myOverlayGallery);
		myOverlayGallery = NULL;
	}

	// If no gallery yet...
	if (!myOverlayGallery)
	{
		// entity images can use one of several galleries
		// let the gallery decide which type of sprite to create 
		myOverlayGallery = SharedGallery::theSharedGallery().CreateGallery(galleryName);
	}

	//char db[256];
//	sprintf(db, "SetOverlayGallery %s\n", galleryName.c_str());
//	OutputDebugString(db);
}


bool EntityImage::SetOverlayIndex(int32 image, int32 layer)
{
	myCurrentOverlayIndex = image;
	if(myOverlayGallery )
	{
		if(myCurrentOverlayIndex != -1)
		{
			mySprite->ShowBitmap(layer);
			return mySprite->SetBitmap(myOverlayGallery->GetBitmap(image),layer);
		}
		else
		{
			mySprite->ShowBitmap(0);
		}

	}
	return false;
}

// ----------------------------------------------------------------------
// Method:      GetBitmap 
// Arguments:   index - index into sprite gallery
//				
// Returns:     pointer to the selected bitmap
//
// Description: returns a bitmap specified by index	
//						
// ----------------------------------------------------------------------
Bitmap* EntityImage::GetBitmap(int32 index)
{
	_ASSERT(myGallery);
	return myGallery->GetBitmap(index);
}

void EntityImage::DestroyGallery()
{
	_ASSERT(myGallery);
	SharedGallery::theSharedGallery().RemoveGallery(myGallery);
}





// ----------------------------------------------------------------------
// Method:      GetCurrentBitmap 
// Arguments:   None
//				
// Returns:     pointer to the currently showing bitmap
//
// Description: returns a bitmap sopecified by the current Image Index	
//						
// ----------------------------------------------------------------------
Bitmap* EntityImage::GetCurrentBitmap()
	{
	_ASSERT(myGallery);
	return myGallery->GetBitmap(myCurrentImageIndex);
	}

bool EntityImage::SetCurrentBitmap()
{
	// bounds checking here?  the gallery checks for
	// overall out of bounds we have no means of checking
	// whether the agent engineer has exceeded individual
	// sprite bounds
	if(mySprite)
		return mySprite->SetBitmap(GetBitmap(myCurrentImageIndex),0);

	return false;
}
	
void EntityImage::SetPose(int32 pose)
{    
	// kill any running anim
	myAnimLength = 0;

	// set image index into gallery*/
    myCurrentImageIndex = myCurrentBaseImageIndex + pose;	

	SetCurrentBitmap();

}

// ----------------------------------------------------------------------
// Method:      SetCurrentIndex 
// Arguments:   image - the absolute index of the image in the gallery
//				file
//				
// Returns:     None
//
// Description: updates the current image index and bitmap	
//						
// ----------------------------------------------------------------------
void EntityImage::SetCurrentIndex(uint32 image) 
{
	myCurrentImageIndex = image;
	SetCurrentBitmap();
}

void EntityImage::HideCurrentBitmap()
{
	mySprite->HideBitmap(0);
}

void EntityImage::ShowCurrentBitmap()
{
	mySprite->ShowBitmap(0);
}

// ----------------------------------------------------------------------
// Method:      SetCurrentIndexFromBase 
// Arguments:   image - the index of the image in the gallery
//				file as an offset from the base.
//				
// Returns:     None
//
// Description: updates the current image index and bitmap	
//						
// ----------------------------------------------------------------------
void EntityImage::SetCurrentIndexFromBase(uint32 image)
{
	myCurrentImageIndex = myCurrentBaseImageIndex + image;
	SetCurrentBitmap();
}

/*
int32 EntityImage::GetWidth() const
	{
		ASSERT(mySprite);
		return mySprite->GetWidth();
	}

int32 EntityImage::GetHeight() const
	{
		ASSERT(mySprite);
		return mySprite->GetHeight();
	}
*/

// ----------------------------------------------------------------------
// Method:      GetLsrgestBound
// Arguments:   rect - a rect structure to receive the bounds 
//
// Returns:     Cycle through bitmaps in your gallery and get the largest
//				bound
//			
// ----------------------------------------------------------------------
void EntityImage::GetLargestBound(RECT& rect) 
{
	for (uint32 i = myAbsoluteBaseImage; i< myAbsoluteBaseImage + myNumberOfImages; i++)
	{
		GetBitmap(i)->UpdateBoundIfLarger(rect);
	}
}


// ----------------------------------------------------------------------
// Method:      SetSprite 
// Arguments:   sprite - set the first sprite in my list
//
// Returns:     None
//			
// ----------------------------------------------------------------------
void EntityImage::SetSprite(Sprite* sprite)
	{
		_ASSERT(sprite);
		mySprite=sprite;
		mySprite->SetPlane(myPlane);
		YouAreCameraShy(mySpriteIsCameraShy);
	}

// ----------------------------------------------------------------------
// Method:      Visible 
// Arguments:   test - rectangle to test my position against
//
// Returns:     true if this entity is visible within the bounds of the
//				rectangle supplied. false otherwise
//			
// ----------------------------------------------------------------------
bool EntityImage::Visible(const RECT& test)
{
		// compute dl & dr as the left and right bounds of
	// the object, relative to the View rectangle
	
   // if (myPlane&0x80000000) return FALSE;

	int dl = GetWorldX() - test.left;


    int dr = dl+GetWidth();

	return (((dl > (test.right - test.left) )
		||(test.bottom < GetWorldY())
		||(dr<0)
		||(test.top > (GetWorldY() + GetHeight())))? 0:1) ;
		
}

// ----------------------------------------------------------------------
// Method:      IsPixelTransparent 
// Arguments:   imageIndex - the sprite image to test
//								if this is -1 then use the current image
//				x - x coordinate of the pixel to test
//				y - y coordinate of the pixel to test
//
// Returns:     true if the pixel at the given coordinates is transparent
//				false otherwise
//			
// ----------------------------------------------------------------------
bool EntityImage::IsPixelTransparent(uint32 x,
									 uint32 y,
									 int32 imageIndex/* =-1*/)
{
	if(imageIndex == -1)
		imageIndex = myCurrentImageIndex;

	// make the world coordinates local to the bitmap
	x  -= myWorldPosition.GetX();
	y  -= myWorldPosition.GetY();

	return myGallery->IsPixelTransparent(x,
									 y,
									 imageIndex);
}

void EntityImage::CentreMe()
{
	int32 x;
	int32 y;

	ASSERT(mySprite);
	mySprite->CentreMe(x,y);
	myWorldPosition.SetX(x);
	myWorldPosition.SetY(y);
}


bool EntityImage::Write(CreaturesArchive &archive) const
{
	archive << myPlane << myWorldPosition;
	archive << mySpriteIsCameraShy;

	archive << myGallery->GetName();
	archive << myAbsoluteBaseImage;

	archive << myCurrentImageIndex;
	archive << myCurrentBaseImageIndex;
	archive << myNumberOfImages;
	archive << myDefaultBaseImage;
	archive << mySavedImageIndexState;

	archive << myReadArchivedGalleryFlag;

	// a gallery of possible overlays
	if( myOverlayGallery )
		archive << myOverlayGallery->GetName();
	else
		archive << FilePath();

	archive	<< myCurrentOverlayIndex;


	for(int32 i= 0; i < MaxAnimLength;i++)
	{
		archive << myAnim[i];
	}

	archive <<myAnimLength;		// 0 if no anim playing
	archive << myAnimNext;			// next frame to show
	archive << myFrameRate;
	archive << myFrameCount;
	archive << myCurrentWidth << myCurrentHeight;
	archive << myVisibleFlag;

	return true;
}

bool EntityImage::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{

		archive >> myPlane;
		archive >> myWorldPosition;
		archive >> mySpriteIsCameraShy;

		FilePath galleryName;
		archive >> galleryName;

		if(galleryName.empty())
		{
			std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)DisplayEngine::sidGalleryNotSpecified,
									std::string("EntityImage::Read"));

			throw EntityImageException(string, __LINE__);
		}
		archive >> myAbsoluteBaseImage;
		archive >> myCurrentImageIndex;
		archive >> myCurrentBaseImageIndex;
		archive >> myNumberOfImages;
		archive >> myDefaultBaseImage;
		archive >> mySavedImageIndexState;

		archive >> myReadArchivedGalleryFlag;

		if(myReadArchivedGalleryFlag)
		{

			SetGallery(galleryName,myAbsoluteBaseImage, 0, false);
		}

		archive >> galleryName;

		if(myReadArchivedGalleryFlag)
		{
			// a gallery of possible overlays
			if(!galleryName.empty())
			{
				SetOverlayGallery(galleryName);
			}
		}

		archive >> myCurrentOverlayIndex;


		for(int32 i= 0; i < MaxAnimLength;i++)
		{
			archive >> myAnim[i];
		}

		archive >>myAnimLength;		// 0 if no anim playing
		archive >> myAnimNext;			// next frame to show
		archive >> myFrameRate;
		archive >> myFrameCount;
		archive >> myCurrentWidth >> myCurrentHeight;
		archive >> myVisibleFlag;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success
	SetCurrentBitmap();
	
	if(myVisibleFlag)
		Link();

	return true;
}


void EntityImage::SetFrameRate(const uint8 rate)
{
	if (rate < 1)
		return;
	myFrameRate = rate;
	myFrameCount = 0;
}


void EntityImage::SetAnim( const uint8* newanim, int length )
{
	ASSERT( length <= MaxAnimLength );

	// allow SetAnim(NULL)
	if( newanim == NULL || length == 0 || length > MaxAnimLength )
	{
		myAnimLength = 0;
		return;
	}

	// check for malicious agent engineers
	if( newanim[0] == AnimWrapValue )
	{
		myAnimLength = 0;
		return;
	}

	//memcpy( myAnim, newanim, length );
	for(int i=0; i < length; i++)
		myAnim[i] = newanim[i];
	myAnimLength = length;
	myAnimNext = 0;
}

bool EntityImage::ValidateAnim( const uint8* newanim, int length )
{
	ASSERT( length <= MaxAnimLength );

	// allow SetAnim(NULL)
	if( newanim == NULL || length == 0 || length > MaxAnimLength )
		return true;

	// check for malicious agent engineers
	if( newanim[0] == AnimWrapValue )
		return true;

	// loop through animation string, looking for errors
	for (int i = 0; i < length; ++i)
	{
		int pose = *(newanim + i);
		// stop at anim wrap value
		if (pose == AnimWrapValue) 
		{
			// check wrap point is OK if there is one
			if (i < length - 1)
			{
				int wrap = *(newanim + i + 1);
				if (wrap < 0 || wrap >= i)
					return false;
			}
			break;
		}
		// check pose OK
		if (!ValidatePose(pose))
			return false;
	}

	return true;
}

bool EntityImage::SetBaseAndCurrentIndex(uint32 baseimage)
{
	// check new POSE is in range
	if(baseimage + myAbsoluteBaseImage < myGallery->GetCount() && 
	   baseimage < myNumberOfImages)
	{
		// change the value
		uint32 old_value = myCurrentBaseImageIndex;
		myCurrentBaseImageIndex = myAbsoluteBaseImage + baseimage;

		// check any animation string in progress is valid
		// Darn, we have to check if our animation can cope :(
		uint8 testAnim[MaxAnimLength];
		for(int i=0; i < MaxAnimLength; i++)
			testAnim[i] = (uint8)(myAnim[i]);
		if (!ValidateAnim(testAnim, myAnimLength))
		{
			// if invalid anim, go back to old base
			myCurrentBaseImageIndex = old_value;
			return false;
		}
		return true;
	}
	return false;
}


bool EntityImage::AnimOver()
{
	return (myAnimLength == 0);
}

void EntityImage::Animate()
{
	// anim running?
	if( !myAnimLength )
		return;

	//Return if not time for an animation tick :)
	if (++myFrameCount % myFrameRate)
		return;

	uint32 frame = myAnim[ myAnimNext ];
	if( frame == AnimWrapValue )
	{
		// if there is a number after AnimWrapValue, loop back to that frame
		if( myAnimNext < myAnimLength - 1 )
			myAnimNext = myAnim[ myAnimNext + 1];
		else
			myAnimNext = 0;
		frame = myAnim[ myAnimNext ];
	}

	// add some runtime bounds checking here?
	myCurrentImageIndex = myCurrentBaseImageIndex + frame;

	if(myCurrentImageIndex >= myAbsoluteBaseImage + myNumberOfImages)
	{
		myCurrentImageIndex = myAbsoluteBaseImage;
	}

	ASSERT( myCurrentImageIndex < myGallery->GetCount() );
	SetCurrentBitmap();

	// frame advance, anim over?
	if( ++myAnimNext >= myAnimLength )
		myAnimLength = 0;	// kill kill kill!
}

uint16* EntityImage::GetImageData() const
{
	return mySprite->GetImageData();
}

void EntityImage::SetPlane(int plane)
{
	myPlane = plane; 
	mySprite->SetPlane(plane);

	theMainView.UpdatePlane(this);

	// Now for each Line, update its plane...

	if (myLines.empty())
		return;

	std::vector<Line*>::iterator it;

	for(it = myLines.begin(); it != myLines.end(); it++)
	{
		Line* l = (*it);
		l->SetPlane(myPlane);
		theMainView.Remove(l);
		theMainView.Add(l);
	}

	
}

void EntityImage::ResetLines()
{
	if (myLines.empty())
		return;

	std::vector<Line*>::iterator it;

	for(it = myLines.begin(); it != myLines.end(); it++)
	{
		delete (*it);
	}
	myLines.clear();
}

void EntityImage::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stipplestartAt/* = 0*/) 
{
	myLines.push_back(new Line(x1,y1,x2,y2,lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stipplestartAt,theApp.GetLinePlane()));
}


bool EntityImage::ResetMainGallery(FilePath &name)
{
	// if there is no name then we are swapping in the
	// normal default gallery
	if(name.empty())
	{
		name = myDefaultGalleryName;
		myCurrentImageIndex = mySavedImageIndexState;
	}
	else
	{
		// we are replacing the normal gallery
		// save the last index
		mySavedImageIndexState = myCurrentImageIndex;
	}
	Unlink();
	myCurrentImageIndex = myDefaultBaseImage;
	myCurrentBaseImageIndex = myDefaultBaseImage;
	bool ok= 	SetGallery(name,myDefaultBaseImage);
	Link();
	return myGallery ? true : false;
}

bool EntityImage::RemoveGallery()
{
	bool done = false;	
	Unlink();
	done = SharedGallery::theSharedGallery().RemoveCreatureGallery(myGallery);
	myGallery =NULL;
	return done;

}

void EntityImage::ReloadGallery(FilePath const& galleryName, 
							 uint32 baseimage,
							 uint32 numImages /*= 0*/)
{
	myCurrentBaseImageIndex = baseimage;
	SetGallery(galleryName,baseimage,numImages);
}


void EntityImage::SetPosition(int32 x, int32 y)
{
	myWorldPosition.SetX(x);
	myWorldPosition.SetY(y);
}

bool EntityImage::Visible(int32 scope)
{
	RECT r;
	GetBound(r);
	return theMainView.Visible(r,scope);
}

void EntityImage::PrepareForReloadingGallery()
{
	Unlink();
									
	myGallery = NULL;
}

bool EntityImage::IsMirrored()
{
	_ASSERT(mySprite);
	return mySprite->IsMirrored();
}

void EntityImage::DrawMirrored(bool mirror)
{
	mySprite->DrawMirrored(mirror);
}
