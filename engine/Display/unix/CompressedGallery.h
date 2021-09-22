// --------------------------------------------------------------------------
// Filename:	CompressedGallery.h
// Class:		CompressedGallery
// Purpose:		This class holds a series of bitmaps for an entity.
//				The gallery is read on demand from a memory mapped file.
//				The bitmap data is compressed
//			
//				
//
// Description: On creation the gallery works in one of two ways:
//				Sprite galleries create a bitmap shell for each sprite
//				which contains its width and height
//				Background galleries rely on the fact that all tiles have the
//				same width and height.  Only one bitmap shell  is created.
//
//				When the gallery is accessed the bitmap is updated to the
//				start of its data in the memory mapped file.
//			
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Now reads galleries from memory mapped files.
// --------------------------------------------------------------------------
#ifndef		COMPRESSED_GALLERY_H
#define		COMPRESSED_GALLERY_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"CompressedBitmap.h"
#include	"Gallery.h"
#include	"../File.h"
#include	"MemoryMappedFile.h"
#include	<string>
#include	"../PersistentObject.h"



// this will need to be a persistent
class CompressedGallery: public Gallery
{
	CREATURES_DECLARE_SERIAL( CompressedGallery )

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
	CompressedGallery();

	CompressedGallery(FilePath const &name);

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   memroyFile - handle to a memeory file to map to
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
CompressedGallery::CompressedGallery(FilePath const &name,
				HANDLE memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap);



// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   name - the gallery file to read from
//
// Description: We could load the background straight from a sprite file 
//				on disk in which case the gallery comes
//				in tiles which know their individual sizes.
//						
// ----------------------------------------------------------------------
/*
	CompressedGallery(FilePath const &name)
	{
	//	InitAll();
		LoadFromC16(name.GetFullPath());
	}
*/
	bool Reload(FilePath const &name);
// ----------------------------------------------------------------------
// Method:      Destructor 
// Arguments:   none
//
// Description: Deletes the created bitmaps and unmaps and closes any
//				memory mapped files
//						
// ----------------------------------------------------------------------
	virtual ~CompressedGallery();

	void Trash();

	virtual Sprite* CreateSprite(EntityImage* owner);

	virtual Bitmap* GetBitmap(uint32 index);

	virtual int32 GetBitmapWidth(uint32 index);

	virtual int32 GetBitmapHeight(uint32 index);

	virtual bool IsValid();

	virtual bool ValidateBitmapSizes(int32 minimumWidth, int32 minimumHeight);


////////////////////////////////////////////////////////////////////////////
// Get and Set Methods...
////////////////////////////////////////////////////////////////////////////




// ----------------------------------------------------------------------
// Method:      LoadFromC16 
// Arguments:   name - name of the bitmap file to load from
//				width - the width of the bitmaps that the whole file will
//				be split into.
//				height - the height of the individual bitmaps that the 
//				whole file will be split into.
//			
// Returns:     true if the bitmap loads OK false otherwise
//
// Description: Splits the file into the given amount of bitmaps.  Copies
//				the bits for each bitmap.
//			
//			
// ----------------------------------------------------------------------
	virtual bool LoadFromC16(FilePath& fileName);

// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   dataPtr - point in file to write to
//			
// Returns:     the number of bytes written
//
// Description: Saves the bits and headers for each bitmap.
//			
//			
// ----------------------------------------------------------------------
	virtual uint32 Save(uint8*& data);
	virtual uint32 Save(MemoryMappedFile& file);
	uint32 Save();
	virtual bool ConvertTo(uint32 format);

// ----------------------------------------------------------------------
// Method:      InitBitmaps 
// Arguments:   None
//			
// Returns:     None
//
// Description: Creates a set of bitmaps depending on how many are in the 
//				file and initialises them with their tile widths
//				and heights.  Each bitmaps data is set to the start of the
//				the bitmaps data in the file. 
//			
//			
// ----------------------------------------------------------------------
virtual bool InitBitmaps();

virtual bool IsPixelTransparent(uint32 x,
							uint32 y,
							int32 imageIndex);

virtual void Recolour(const uint16 tintTable[65536]);



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

private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	CompressedGallery (const CompressedGallery&);
	CompressedGallery& operator= (const CompressedGallery&);

	MemoryMappedFile myMemoryMappedFile;
	CompressedBitmap* myCompressedBitmaps;
};

#endif		// COMPRESSED_GALLERY_H
