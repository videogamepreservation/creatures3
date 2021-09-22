#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "EntityImageClone.h"
#include "ClonedSprite.h"
#include "MainCamera.h"
#include "ErrorMessageHandler.h"
#include "../resource.h"
#include "SharedGallery.h"
#include "ClonedGallery.h"
#include "DisplayEngine.h"

//make sure that this is serializable
CREATURES_IMPLEMENT_SERIAL( ClonedEntityImage)

ClonedEntityImage::ClonedEntityImage()
{
	myReadArchivedGalleryFlag = false;
}

ClonedEntityImage::ClonedEntityImage(FilePath const& galleryName,
									 int32 plane,//plane
									 int32 worldx, // world x
									 int32 worldy, // world y
									 uint32 baseimage, // base image
									 uint32 numImages,
									 uint32 currentimage )
{
	myPlane = plane;

	myGallery = NULL;
	myReadArchivedGalleryFlag = false;
	SetGallery(galleryName,baseimage,numImages);


	// check that you now have a gallery
	if(!myGallery)
		{
		
		// "The display engine may shut down because the following gallery was not created %1."
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)DisplayEngine::sidNoGalleryCreated,
									std::string("ClonedEntityImage::ClonedEntityImage"),
									galleryName.GetFullPath().c_str());

		throw EntityImageException(string,__LINE__);
		}

	// the base index and current index should always be zero
	// since their galleries contain their bitmaps exclusively
	// and they are not shared by other entity images

	Init(plane,//plane
		 worldx, // world x
		 worldy, // world y
		 0,	// base image
		 0,
		 numImages); // current image
}

ClonedEntityImage::ClonedEntityImage(
	int32 plane,//plane
	 int32 worldx, // world x
	 int32 worldy, // world y
	 int32 width,
	 int32 height
	 )
{
	myReadArchivedGalleryFlag = false;
}

// ----------------------------------------------------------------------
// Method:      SetGallery 
// Arguments:   galleryName - file name containg the sprites for this entity
//				baseImage - the image in the sprite file that this
//							 particular entity will start from 
//				numImages - must know exactly how many images to copy from the
//							file
//							
//				
// Returns:     None
//
// Description: Create a gallery for this entity  and set up a
//				sprite to hold your current image data.
//						
// ----------------------------------------------------------------------
bool ClonedEntityImage::SetGallery(FilePath const& galleryName,
								   uint32 baseImage,
								   uint32 numImages/*= 0*/,
								   bool reset/* = true*/)
{
	// the base index and current index should always be zero
	// since their galleries contain their bitmaps exclusively
	// and they are not shared by other entity images
	myAbsoluteBaseImage = 0;
	myCurrentImageIndex = 0;
	myCurrentBaseImageIndex = 0;


	// if you already have a gallery get rid of it
	if(myGallery)
	{
		SharedGallery::theSharedGallery().
									RemoveGallery(myGallery);
	}

	myGallery = SharedGallery::theSharedGallery().
							CreateClonedGallery(galleryName,
												baseImage,
												numImages);

	// now we have a gallery we can create our sprite

	if(myGallery)
	{
		Sprite* sprite = myGallery->CreateSprite(this);
		SetSprite(sprite);
		SetCurrentBitmap();
		return true;
	}

	

	return false;
}

void ClonedEntityImage::DrawString(std::string& word,
				bool centred,
				uint16 textColour,
				uint16 backgroundColour)
{
	ClonedSprite* clone = (ClonedSprite*)mySprite;
	clone->DrawString(word, centred, textColour, backgroundColour);
}

int ClonedEntityImage::SelectFont( const std::string &fontName )
{
	ClonedSprite* clone = (ClonedSprite*)mySprite;
	return clone->SelectFont(fontName);
}

void ClonedEntityImage::DrawString( int32 x, int32 y, const std::string& text,
								   int fontIndex )
{
	ClonedSprite* clone = (ClonedSprite*)mySprite;
	clone->DrawString(x, y, text, fontIndex);
}

void ClonedEntityImage::MeasureString( const std::string &text, int fontIndex,
									  int32 &width, int32 &height )
{
	ClonedSprite* clone = (ClonedSprite*)mySprite;
	clone->MeasureString( text, fontIndex, width, height );
}


void ClonedEntityImage::DrawLine(int32 x1, int32 y1, int32 x2, int32 y2,
							 uint8 lineColourRed /*= 0*/,
							 uint8 lineColourGreen/*= 0*/,
							 uint8 lineColourBlue /*= 0*/)  
{

	ClonedSprite* clone = (ClonedSprite*)mySprite;
	clone->DrawLine(x1,
					y1,
					x2,
					y2,
					lineColourRed,
					lineColourGreen,
					lineColourBlue);  
}


void ClonedEntityImage::DrawSprite(Position& whereToDraw,
							Bitmap* const bitmapToDraw)
{
	ClonedSprite* clone = (ClonedSprite*)mySprite;
	clone->DrawOnMe(whereToDraw,bitmapToDraw);

}

void ClonedEntityImage::DrawSprite(Position& whereToDraw,
					FilePath const& galleryName,
					int32 spriteIndex,
					int32 baseimage,
					int32 numimages)
{
	ClonedGallery* gallery = new ClonedGallery(galleryName,
												baseimage,
												numimages);

	Bitmap* bitmap = gallery->GetBitmap(spriteIndex);

	if(bitmap)
	{
	ClonedEntityImage::DrawSprite(whereToDraw,
							bitmap);

	}
}

void ClonedEntityImage::Clear()
{
	ClonedSprite* clone = (ClonedSprite*)mySprite;

	clone->Clear();
}

bool ClonedEntityImage::Write(CreaturesArchive &archive) const
{
	EntityImage::Write( archive );
	archive << myGallery;
	return true;
}

bool ClonedEntityImage::Read(CreaturesArchive &archive)
{
		
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{


		if(!EntityImage::Read( archive ))
			return false;

		archive >> myGallery;

		if(myGallery)
		{
			Sprite* sprite = myGallery->CreateSprite(this);
			SetSprite(sprite);
		}
		Link( true );
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}
