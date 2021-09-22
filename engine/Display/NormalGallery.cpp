// --------------------------------------------------------------------------
// Filename:	NormalGallery.cpp
// Class:		NormalGallery
// Purpose:		This class holds a series of bitmaps for an entity.
//				The gallery is read on demand from a memory mapped file
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

#include "NormalGallery.h"
#include "Sprite.h"
#include "System.h"
#include <stdio.h>
#include "DisplayEngine.h"

const std::string S16 = ".s16";
const std::string C16 = ".c16";

CREATURES_IMPLEMENT_SERIAL( NormalGallery )

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
NormalGallery::NormalGallery(FilePath const &galleryName)
:Gallery(galleryName),
myMemoryMappedFile(galleryName.GetFullPath())
{
	InitBitmaps();
}

NormalGallery::~NormalGallery()
{ 
	delete [] myBitmaps;
	myBitmaps = NULL;
}

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

#ifdef _WIN32
// Is this used at all?

NormalGallery::NormalGallery(HANDLE memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap)
				 :myMemoryMappedFile( memoryFile, 
							highOffset,
							lowOffset,
							numBytesToMap)
{
	InitBitmaps();
}

#endif


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
bool NormalGallery::InitBitmaps()
{

//	uint8* data = myMemoryMappedFile.GetFileStart();
	uint8* data = NULL;

		// skip to the number of images in the file
	//	data+=4;
//	myMemoryMappedFile.Seek(sizeof(uint32),File::Start);
	myPixelFormat = myMemoryMappedFile.ReadUINT32();

	
	if((myPixelFormat & C16_16BITFLAG))
	{
		ErrorMessageHandler::Show("gallery_error",
										2,
										std::string("NormalGallery::InitBitmaps"),
										GetName().GetFullPath().c_str());
		return false;
	}

	//	myCount = GetUINT16At(data);
	//	data+=2;
	myCount = myMemoryMappedFile.ReadUINT16();

	//create the correct number of bitmaps
	myBitmaps=new Bitmap[myCount];

	int i;	
	for( i = 0; i < myCount; i++)
	{
		myBitmaps[i].SetName(myName.GetFullPath());
	//	myMemoryMappedFile.ReadUINT8Ptr(data);
	//	myBitmaps[i].SetOffset(data);
		myBitmaps[i].SetOffset(myMemoryMappedFile.ReadUINT32());

	//	myMemoryMappedFile.ReadUINT8Ptr(data);
		//point to your width and height
	//	myBitmaps[i].SetWidthHeight(data);
		myBitmaps[i].SetWidth(myMemoryMappedFile.ReadUINT16());
		myBitmaps[i].SetHeight(myMemoryMappedFile.ReadUINT16());

	}

	//now point to your data in the sprite file
	for(i = 0; i < myCount; i++)
	{
//		myMemoryMappedFile.ReadUINT8Ptr(data);
		//myBitmaps[i].SetData(data);
		myBitmaps[i].SetData(myMemoryMappedFile);
	}
	return true;

}





/*
// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   dataPtr - point in file to write to
//			
// Returns:     true if the file writes OK false otherwise
//
// Description: Saves the bits and headers for each bitmap.
//			
//			
// ----------------------------------------------------------------------
uint32 NormalGallery::Save(uint8*& data)
{

	uint32 bytesWritten = 0;
	uint32 flag = 0x01;
		// First four bytes is a set of flags to say whether this is
		// a 565 or 555 sprite file. I am assuming this is a 565 file
		(*(uint32*)data) = flag;
		data+=4;

		// Now the number of images in the file
		*((uint16*)data) = myCount;
		data+=2;

	
	bytesWritten +=6;
		

	for(uint32 i = 0; i < myCount; i++)
		{
			bytesWritten += myBitmaps[i].SaveHeader(data);
		}


	//now read in each bit

	for(i = 0; i < myCount; i++)
		{
		bytesWritten += myBitmaps[i].SaveData(data);
		}

	return bytesWritten;
}
*/
// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool NormalGallery::Write(CreaturesArchive &archive) const
{

	archive << myCount;

	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool NormalGallery::Read(CreaturesArchive &archive)
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

Sprite* NormalGallery::CreateSprite(EntityImage* owner)
{
	Sprite* sprite=new Sprite(owner);
	return sprite;
}


// virtual because of the call to save()
bool NormalGallery::ConvertTo(uint32 to)
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

	int i;	
	for( i = 0; i < myCount; i++)
	{
		myBitmaps[i].Convert(from,to);
	}

	myPixelFormat ^= C16_FLAG_565;

	// now save your data out
	// as only you would
	myMemoryMappedFile.Seek(0,File::Start);
	Save(myMemoryMappedFile);

	return true;
}
