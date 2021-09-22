// --------------------------------------------------------------------------
// Filename:	BackgroundGallery.cpp
// Class:		BackgroundGallery
// Purpose:		This class holds a series of bitmaps for an entity.
//				The gallery is read on demand from a memory mapped file
//			
//				
//
// Description: 
//				Sprite galleries create a bitmap shell for each sprite
//				which contains its width and height
//				Background galleries rely on the fact that all tiles have the
//				same width and height.  Only one bitmap shell  is created.
//
//				When the gallery is accessed the bitmap is updated to the
//				start of its data in the memory mapped file.
//
//				NOTE ALL SPRITE FILES ARE CONSIDERED UNCOMPRESSED FOR NOW
//			
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Now reads galleries from memory mapped files.
//							background files now contain their tile width and
//							height.  The extension has changed to blk.
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BackGroundGallery.h"
#include "System.h"
#include <stdio.h>
#include "DisplayEngine.h"
#include	"Bitmap.h"
#include	"../File.h"


// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   galleryName - the gallery file to read from
//				background - create a background file
//
// Description: This memory maps the supplied file.  Then it calls the
//				right initialisation fucntion depending on whether this is
//				a background file or a sprite file
//	
//				Note needs some exception handling
//					
// ----------------------------------------------------------------------
BackgroundGallery::BackgroundGallery(FilePath const &galleryName)
:Gallery(galleryName),
myMemoryMappedFile(galleryName.GetFullPath(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_WRITE|FILE_SHARE_READ)
//myPixelData(0)
{
	InitBitmaps();
}


#ifdef THIS_FUNCTION_IS_NOT_USED
BackgroundGallery::BackgroundGallery(char* name,uint32 width,uint32 height)
{
	LoadFromBmp(name,width,height);
}
#endif THIS_FUNCTION_IS_NOT_USED


BackgroundGallery::~BackgroundGallery()
{
	if (myBitmaps) 
	{
		delete [] myBitmaps;

	myBitmaps = NULL;
	}

	myDrawnTilesMap.clear();
}



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
bool BackgroundGallery::InitBitmaps()
{

	// take advantage of the fact that background tiles
	// all have the same width and height

//*************** Changes

	// skip to the number of images in the file
	// get my 16 bit pixel format flag 565 or 555
	myPixelFormat = myMemoryMappedFile.ReadUINT32();


	// put here the tilewidth and tileheight of the background
	// offset from the world position
	myTileWidth =  myMemoryMappedFile.ReadUINT16();
	ASSERT(myTileWidth);


	myTileHeight =  myMemoryMappedFile.ReadUINT16();
	ASSERT(myTileHeight);


	myCount = myMemoryMappedFile.ReadUINT16();
	ASSERT(myCount);

	//create the correct number of bitmaps
	myBitmaps=new Bitmap[myCount];

	uint32 i;	
	for(i = 0; i < myCount; i++)
	{
#ifdef _DEBUG
		myBitmaps[i].SetName(myName.GetFullPath());
#endif

	/*	myBitmaps[i].SetOffset(myMemoryMappedFile.ReadUINT32());


		myBitmaps[i].SetWidth(myMemoryMappedFile.ReadUINT16());
		myBitmaps[i].SetHeight(myMemoryMappedFile.ReadUINT16());
		myBitmaps[i].SetName(myName.GetFullPath());*/
		myBitmaps[i].InitHeader(myMemoryMappedFile);


	}

	//now point to your data in the sprite file
	for(i = 0; i < myCount; i++)
	{

		myBitmaps[i].SetData(myMemoryMappedFile);
	}
	return true;

}

// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool BackgroundGallery::Write(CreaturesArchive &archive) const
	{

	archive << myCount;
//	for	(int i=0; i<myCount; i++) myBitmaps[i].Write(archive);
	return true;
	}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool BackgroundGallery::Read(CreaturesArchive &archive)
	{

	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		archive >> myCount;

		myBitmaps = new Bitmap[myCount];
	
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;

	}

// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   dataPtr - point in file to write to
//			
// Returns:     true if the file writes OK false otherwise
//
// Description: Saves the bits and headers for each bitmap.
//				This routine will always save the file out as a
//				656 file
//			
//			
// ----------------------------------------------------------------------
uint32 BackgroundGallery::Save(MemoryMappedFile& file)
{

	uint32 bytesWritten = 0;

	file.WriteUINT32(myPixelFormat);

	file.WriteUINT16(myTileWidth);
	file.WriteUINT16(myTileHeight);
	file.WriteUINT16(myCount);

	bytesWritten +=8;
		
// ******** changes from gettile(i)
	uint32 i;
	for(i = 0; i < myCount; i++)
	{
		bytesWritten +=myBitmaps[i].SaveHeader(file);
	}


	//now read in each bit

	for(i = 0; i < myCount; i++)
	{
		bytesWritten += myBitmaps[i].SaveData(file);
	}

	return bytesWritten;
}

// virtual because of the call to save()
bool BackgroundGallery::ConvertTo(uint32 to)
{
	uint32 from;
	switch(to)
	{
	case RGB_555:
		{
		if(!(myPixelFormat & C16_FLAG_565))
			return false;
		from = RGB_565;
		break;
		}
	case RGB_565:
		{
		if(myPixelFormat & C16_FLAG_565)
			return false;
		from = RGB_555;
		break;
		}
	default:
		return false;
	}

	// bail if we are already in the right format
	
	for(uint32 i = 0; i < myCount; i++)
	{
		//GetTile(i)->Convert(from,to);
		myBitmaps[i].Convert(from,to);
	}

	myPixelFormat ^= C16_FLAG_565;


	// now save your data out
	// as only you would
	myMemoryMappedFile.Seek(0,File::Start);
	Save(myMemoryMappedFile);

	

	return true;
}

