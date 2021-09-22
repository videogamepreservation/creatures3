// --------------------------------------------------------------------------
// Filename:	ClonedGallery.cpp
// Class:		ClonedGallery
// Purpose:		This class holds a series of bitmaps for an entity.
//				Cloned galleries need to hold all their bitmap data locally
//				so they do not use memory mapped files.  Also we must be
//				provided with the bae image and the number of images that belong
//				to this particular gallery since there may be many sets of
//				related sprites in a sprite file.
//			
//				
//
// Description: A cloned gallery cannot be compressed because its bitmaps
//				are used as a canvas to display other sprites.
//
// History:
// -------  
// 11Nov98	Alima			Created
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ClonedGallery.h"
#include "SharedGallery.h"
#include "System.h"
#include <stdio.h>
#include "ClonedSprite.h"
#include "ErrorMessageHandler.h"
#include "DisplayEngine.h"
#include	"Bitmap.h"
#include	"../File.h"

CREATURES_IMPLEMENT_SERIAL( ClonedGallery )

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//				baseImage - the image in the sprite file
//							to start loading from
//				numImages - the number of images in the file that belong
//							to this entity (starting from the base image)
//				
//
// Description: Cloned Galleries keep their bitmap data locally
//						
// ----------------------------------------------------------------------

ClonedGallery::ClonedGallery(FilePath const &name,
							 uint32 baseimage,
							 uint32 numImages)
:Gallery(name),
myBaseImage(baseimage)
{
	myCount = numImages;
	if(!InitBitmaps())
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)DisplayEngine::sidGalleryNotCreated,
									std::string("ClonedGallery::ClonedGallery"),
									name.GetFullPath().c_str());

		throw(Gallery::GalleryException(string,__LINE__));
	}
}

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//				baseImage - the image in the sprite file
//							to start loading from
//				numImages - the number of images in the file that belong
//							to this entity (starting from the base image)
//				
//
// Description: Cloned Galleries keep their bitmap data locally
//						
// ----------------------------------------------------------------------
ClonedGallery::ClonedGallery(FilePath const &name,
							 uint32 baseimage,
							 uint32 numImages,
							 uint32 defaultBitmapWidth,
							 uint32 defaultBitmapHeight)
:Gallery(name),
myBaseImage(baseimage)
{
	myCount = numImages;
	//create the correct number of bitmaps
	myBitmaps=new Bitmap[myCount];

	if(myBitmaps)
	{
		int offset = myCount * SpriteFileHeaderSize;
		// only read in data specific to you
		int i;
		for( i = 0; i < myCount; i++)
		{
			myBitmaps[i].SetOffset(offset);
			myBitmaps[i].SetWidth(defaultBitmapWidth);
			myBitmaps[i].SetHeight(defaultBitmapHeight);
			myBitmaps[i].CreateBlankCanvas();

			offset+=defaultBitmapWidth * defaultBitmapHeight* 2;
		}
	}
}


ClonedGallery::~ClonedGallery()
{			
	if(myBitmaps) delete [] myBitmaps;
	myBitmaps = NULL;
}

// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool ClonedGallery::Write(CreaturesArchive &archive) const
{
	Gallery::Write( archive );
	archive << myBaseImage;
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool ClonedGallery::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!Gallery::Read( archive ))
			return false;

		archive >> myBaseImage;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	//success
	SharedGallery::theSharedGallery().AddClonedGallery( this );
	
	// Destination format is...
	uint32 dest = DisplayEngine::theRenderer().GetMyPixelFormat();
	uint32 from = RGB_555;
	if( myPixelFormat & C16_FLAG_565 )
		from = RGB_565;
	int i;
	for( i=0; i < myCount; i++)
	{
		myBitmaps[i].Convert(from, dest);
	}

	if( dest == RGB_555 ) // Clear the 565 bit
		myPixelFormat = myPixelFormat & ~1;
	else // Set the 565 bit
		myPixelFormat = myPixelFormat | 1;
	return true;
}

Sprite* ClonedGallery::CreateSprite(EntityImage* owner)
{
	Sprite* sprite=new ClonedSprite(owner);
	ASSERT(sprite);
	return sprite;
}

// ----------------------------------------------------------------------
// Method:      InitBitmaps 
// Arguments:   None
//			
// Returns:     None
//
// Description: Creates all the bitmaps from the given sprite file
//				
//			
//			
// ----------------------------------------------------------------------
bool ClonedGallery::InitBitmaps()
	{
	
	if (myBitmaps) delete [] myBitmaps;

	File file(myName.GetFullPath().data(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE);


	// First four bytes is a set of flags to say whther this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file
	file.Read(&myPixelFormat,4);

	file.SeekToBegin();

	if((myPixelFormat & C16_16BITFLAG))
	{
		return  DecompressC16(file);
	}


	int x = myName.GetFullPath().find_last_of(".");

	if(x == -1)
	{
		return false;
	}

	std::string ext = myName.GetFullPath().substr(x, 3);

	// check for a preferred C16 version
	if(ext[1] == 'C' || ext[1] == 'c')	
	{
		return DecompressC16(file);
	}

	uint32 size = file.GetSize();

	ASSERT(size);
	
	if(size)
	{
		// First four bytes is a set of flags to say whther this is
		// a 565 or 555 sprite file. I am assuming this is a 565 file
		file.Read(&myPixelFormat,4);

		// Now the number of images in the file
		uint16 numImages =0;
		file.Read(&numImages,2);

		// but we really want to use our count as we were 
		// initialised
		if(myCount == 0)
			myCount = numImages;

		//create the correct number of bitmaps
		myBitmaps=new Bitmap[myCount];

		if(myBitmaps)
		{
			int16 bitmap_width;
			int16 bitmap_height;
			uint32 offset = 0;
			uint32 actualBitmap =0;

			// only read in data specific to you
			int i;
			for( i = 0; i < numImages; i++)
			{
			
				// Set the offset
				file.Read(&offset,4);
				// Now read in the width and height as shorts
				file.Read(&bitmap_width,2);
				file.Read(&bitmap_height,2);

				if(i >= myBaseImage && i < myBaseImage + myCount)
				{	
					myBitmaps[actualBitmap].SetOffset(offset);
					myBitmaps[actualBitmap].SetWidth(bitmap_width);
					myBitmaps[actualBitmap].SetHeight(bitmap_height);
					actualBitmap++;
				}
			}

			//now read in each bit
			// actualBitmap =0;
			uint32 dest = DisplayEngine::theRenderer().GetMyPixelFormat();
			uint32 from = RGB_555;
			if( myPixelFormat & C16_FLAG_565 )
				from = RGB_565;
			for(i = 0; i < myCount; i++)
			{
			// look for your data in the file
				file.Seek(myBitmaps[i].GetOffset(),File::Start);
					
				myBitmaps[i].LoadFromS16(file);
// myPixelFormat isn't what Dan expects, so this doesn't work!
// The variable is pantsly and inconsistently initialised
				myBitmaps[i].Convert(from, dest);
				if( dest == RGB_555 ) // Clear the 565 bit
					myPixelFormat = myPixelFormat & ~1;
				else // Set the 565 bit
					myPixelFormat = myPixelFormat | 1;
			}

			file.Close();
			return true;
		}
	}
	

	//!!! if the file didn't exist it wouldn't get
	// this far

	return false;
	}

void ClonedGallery::Recolour(const uint16 tintTable[65536])
{
	int i;
	for( i = 0; i < myCount; i++)
	{
		myBitmaps[i].Recolour(tintTable);
	}
}


bool ClonedGallery::DecompressC16(File& file)
{
	if (myBitmaps) delete [] myBitmaps;

	uint32 size = file.GetSize();
	
	if(size)
	{
		// First four bytes is a set of flags to say whther this is
		// a 565 or 555 sprite file. I am assuming this is a 565 file
		file.Read(&myPixelFormat,4);
		myPixelFormat = myPixelFormat & ~C16_16BITFLAG;
		// Now the number of images in the file

		//create the correct number of bitmaps
		uint16 numImages = 0;
		file.Read(&numImages,2);
		if( myCount == 0 )
			myCount = numImages;

		myBitmaps=new Bitmap[myCount];

		int16 bitmap_width;
		int16 bitmap_height;
		uint32 offset = 0;
		int actualBitmap = 0;
		int i;
		for( i = 0; i < numImages; i++)
		{
			// read the offset

			file.Read(&offset,4);

			// Now read in the width and height as shorts
			file.Read(&bitmap_width,2);
			file.Read(&bitmap_height,2);
			if( i >= myBaseImage && i < myBaseImage + myCount )
			{
				myBitmaps[actualBitmap].SetOffset(offset);
				myBitmaps[actualBitmap].SetWidth(bitmap_width);
				myBitmaps[actualBitmap].SetHeight(bitmap_height);
				++actualBitmap;
			}

		//	bitmap->SetScanLines(file);
		// skip scan lines
				uint32 scanValue = 0;
			// do one less scan line because we have the first offset
			// value in myOffset in the header
			int h;
			for( h = bitmap_height-1; h--;)
			{
				file.Read(&scanValue,sizeof(scanValue));
			}


		}


	//now read in each bit


		uint32 dest = DisplayEngine::theRenderer().GetMyPixelFormat();
		uint32 from = RGB_555;
		if( myPixelFormat & C16_FLAG_565 )
			from = RGB_565;
		for(i = 0; i < myCount; i++)
		{
			file.Seek(myBitmaps[i].GetOffset(),File::Start);
			myBitmaps[i].DecompressC16(file);
			if (dest != from)
				myBitmaps[i].Convert(from, dest);
		}
		if( dest == RGB_555 ) // Clear the 565 bit
			myPixelFormat = myPixelFormat & ~1;
		else // Set the 565 bit
			myPixelFormat = myPixelFormat | 1;

		return true;
	}

	//!!! if the file didn't exist it wouldn't get
	// this far

	return false;
}
