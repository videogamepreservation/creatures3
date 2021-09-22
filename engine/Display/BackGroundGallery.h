// --------------------------------------------------------------------------
// Filename:	BackgroundGallery.h
// Class:		BackgroundGallery
// Purpose:		This class holds a series of bitmaps for an entity.
//				The gallery is read on demand from a memory mapped file
//			
//				BACKGROUND FILES HAVE FORMAT *.blk
//
// Description: 
//				Sprite galleries create a bitmap shell for each sprite
//				which contains its width and height
//				Background galleries rely on the fact that all tiles have the
//				same width and height.  Only one bitmap shell is created.
//
//				When the gallery is accessed the bitmap is updated to the
//				start of its data in the memory mapped file.
//			
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Now reads galleries from memory mapped files.
// --------------------------------------------------------------------------
#ifndef		BACKGROUND_GALLERY_H
#define		BACKGROUND_GALLERY_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"Bitmap.h"
//#include	"../File.h"
#include	"MemoryMappedFile.h"
#include	<string>
#include	"../PersistentObject.h"
#include	"Gallery.h"



// this will need to be a persistent
class BackgroundGallery: public Gallery
{

public:

	// This is the threshold number of entities above which we will do a complete
	// screen redraw
	enum {MAX_DIRTY_RECTS = 500};
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
	BackgroundGallery(FilePath const &name);

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   memoryFile - handle to a memeory file to map to
//				highOffset - 
//				lowOffset - these make the point in the file to start 
//							mapping from
//				numBytesToMap - how many bytes to map.
//
// Description: This memory maps the supplied file.  Then it Initialises
//				the bitmaps.  This can be used for the Creature gallery
//				which a composite file of galleries hanlded elsewhere.
//				The gallery only needs to map to it's part of the file.
//	
//				Note needs some exception handling
//					
// ----------------------------------------------------------------------

#ifdef _WIN32
BackgroundGallery(HANDLE& memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap);
#endif // _WIN32

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
	BackgroundGallery(char* name,uint32 width,uint32 height);


// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   none
//
// Description: Deletes the created bitmaps and unmaps and closes any
//				memory mapped files
//						
// ----------------------------------------------------------------------
	virtual ~BackgroundGallery();


////////////////////////////////////////////////////////////////////////////
// Get and Set Methods...
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:      GetTile 
// Arguments:   tile - index of the tile we want
//				
//			
// Returns:     The bitmap pointing to the correct data in the file
//				for the requested tile
//
// Description: Relies on all tiles having same width and height
//				calculates the correct data point in the memory mapped
//				file on the fly
//			
//			
// ----------------------------------------------------------------------
	Bitmap* GetTile(uint32 index)
	{
		
		if(myDrawnTilesMap.find(index) == myDrawnTilesMap.end())
		{
	//		myBitmaps->SetData((uint16*)(myPixelData + 
			//	(index * GetBitmapWidth(0) * GetBitmapWidth(0) << 1)) );
			
			myDrawnTilesMap[index] = true;

			return index<myCount ? myBitmaps + index: NULL;
		}

		return NULL;
	}



	uint32 GetTileHeight()
	{
		return myTileHeight;
	}

	uint32 GetTileWidth()
	{
		return myTileWidth;
	}

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
// Description: Creates one bitmap and initialises it with it's tile width
//				and height.  The default position for the bitmaps data is 
//				set to the start of the pixel data in the file.  
//				Every time a bitmap is summoned this is recalculated.
//				
//			
//			
// ----------------------------------------------------------------------
virtual bool InitBitmaps();

virtual bool ValidateBitmapSizes(int32 minimumWidth, int32 minimumHeight){return true;}


virtual uint32 Save(MemoryMappedFile& file);
virtual bool ConvertTo(uint32 format);

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

void StartTileCount(){ClearTileArray();}
void EndTileCount(){ClearTileArray();}

void ClearTileArray(){myDrawnTilesMap.clear();}


private:

	// Copy constructor and assignment operator
	// Declared but not implemented
	BackgroundGallery (const BackgroundGallery&);
	BackgroundGallery& operator= (const BackgroundGallery&);

	// the background must keep a pointer  to the start of its pixel
	// data
//	uint8* myPixelData;

	// what is the height and width in tiles for a background file?
	uint32 myTileWidth;
	uint32 myTileHeight;
	MemoryMappedFile myMemoryMappedFile;
	std::map<uint32, bool> myDrawnTilesMap;
};

#endif		// BACKGROUND_GALLERY_H
