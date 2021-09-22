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
#ifndef		CLONED_GALLERY_H
#define		CLONED_GALLERY_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

//#include	"Bitmap.h"
//#include	"../File.h"
#include	<string>
#include	"../PersistentObject.h"
#include	"Gallery.h"

class Bitmap;
class File;

// this will need to be a persistent
class ClonedGallery: public Gallery
{
	CREATURES_DECLARE_SERIAL( ClonedGallery )

public:

////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//				background - create a background file
//
// Description: This memory maps the supplied file
//						
// ----------------------------------------------------------------------
	ClonedGallery(){;}

	ClonedGallery(FilePath const &name,
					uint32 baseimage,
					uint32 numImages);

	ClonedGallery(FilePath const &name,
							 uint32 baseimage,
							 uint32 numImages,
							 uint32 defaultBitmapWidth,
							 uint32 defaultBitmapHeight);

#ifdef THIS_FUNCTION_IS_NOT_USED
// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//				width - of each tile
//				height - of each tile
//
// Description: We could load the background straight from a bit map file
//				on disk. (No memory mapping)
//				In this case we need to know how many tiles we wish to
//				split the background into using width (in tiles) and height
//				in tiles
//						
// ----------------------------------------------------------------------
	ClonedGallery(char* name,uint32 width,uint32 height)
	{
	//	InitAll();
		LoadFromBmp(name,width,height);
	}
#endif // THIS_FUNCTION_IS_NOT_USED

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//
// Description: We could load the background straight from a sprite file 
//				on disk in which case the gallery comes
//				in tiles which know their individual sizes.
//						
// ----------------------------------------------------------------------
//	ClonedGallery(char* name,
//				uint32 baseimage,
//				uint32 numImages);

// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   none
//
// Description: Deletes the created bitmaps and unmaps and closes any
//				memory mapped files
//						
// ----------------------------------------------------------------------
	virtual ~ClonedGallery();

	void Recolour(const uint16* tintTable);


////////////////////////////////////////////////////////////////////////////
// Get and Set Methods...
////////////////////////////////////////////////////////////////////////////

	uint32  GetCount()
	{
		return myCount;
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
	virtual bool InitBitmaps();
	bool DecompressC16(File& file);

	virtual bool ConvertTo(uint32 format){return false;}

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

	Sprite* CreateSprite(EntityImage* owner);

//	virtual void CreateBitmaps();

private:

	// Copy constructor and assignment operator
	// Declared but not implemented
	ClonedGallery (const ClonedGallery&);
	ClonedGallery& operator= (const ClonedGallery&);

	// cloned galleries should only keep their own data
	uint32 myBaseImage;

};

#endif		// CLONED_GALLERY_H
