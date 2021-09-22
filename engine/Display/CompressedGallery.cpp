// --------------------------------------------------------------------------
// Filename:	CompressedGallery.cpp
// Class:		CompressedGallery
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

#include "CompressedGallery.h"
#include "CompressedSprite.h"
#include "ErrorMessageHandler.h"
#include "../resource.h"
#include "System.h"
#include <stdio.h>
#include "DisplayEngine.h"

CREATURES_IMPLEMENT_SERIAL( CompressedGallery )

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
CompressedGallery::CompressedGallery()
:myCompressedBitmaps(NULL)	
{
}

CompressedGallery::CompressedGallery(FilePath const &galleryName)
:Gallery(galleryName),
myMemoryMappedFile(galleryName.GetFullPath() ),
myCompressedBitmaps(NULL)

{
	
	if(!InitBitmaps())
	{
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)DisplayEngine::sidNoGalleryCreated,
									std::string("CompressedGallery::CompressedGallery"),
									galleryName.GetFullPath().c_str());

		throw(GalleryException(string,__LINE__));
	}
}

bool CompressedGallery::Reload(FilePath const &name)
{
	if(myName.GetFullPath() == name.GetFullPath())
		return true;

	myName = name;
	myCount = 0;
	myPixelFormat =0;
	myMemoryMappedFile.Open(name.GetFullPath());
	return InitBitmaps();
}

// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   memoryFile - handle to a memory file to map to
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
CompressedGallery::CompressedGallery(FilePath const &filename,
					HANDLE memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap)
				 :Gallery(filename),
					myMemoryMappedFile(memoryFile, 
							highOffset,
							lowOffset,
							numBytesToMap),
							myCompressedBitmaps(0)
{
	
	if(!InitBitmaps())
	{
		// "The following sprite gallery was not created %1"
		std::string string = ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)DisplayEngine::sidNoGalleryCreated,
									std::string("CompressedGallery::CompressedGallery"),
									filename.GetFullPath().c_str());


		throw(GalleryException(string,__LINE__));
	}
}
#endif



CompressedGallery::~CompressedGallery()
{
	Trash();
}

void CompressedGallery::Trash()
{
	delete [] myCompressedBitmaps;
	myCompressedBitmaps = NULL;
	myName.SetExtension(".bollocksed"); // This is __NECESSARY__ - Dan
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
bool CompressedGallery::InitBitmaps()
{

	// get my 16 bit pixel format flag 565 or 555
	myPixelFormat = myMemoryMappedFile.ReadUINT32();

	// for new 16 bit tag check the tag!  because this could be an old c16
	// using the 8 bit tag.
	ASSERT((myPixelFormat & C16_16BITFLAG)==0);

	myCount = myMemoryMappedFile.ReadUINT16();


	delete [] myCompressedBitmaps;

	//create the correct number of bitmaps
	myCompressedBitmaps= new CompressedBitmap[myCount];
//	CompressedBitmap* bitmap = myCompressedBitmaps;
	uint32 pos =0;
	for(uint32 i = 0; i < myCount; i++)
	{
#ifdef _DEBUG
		// set the name and offset
		myCompressedBitmaps[i].SetName(myName.GetFullPath());
#endif

		myCompressedBitmaps[i].InitHeader(myMemoryMappedFile);
		pos = myMemoryMappedFile.GetPosition();

		myCompressedBitmaps[i].SetData(myMemoryMappedFile);	
		myMemoryMappedFile.Seek(pos,File::Start);
	
	}

	return true;

}


// ----------------------------------------------------------------------
// Method:      LoadFromC16 
// Arguments:   name - name of the bitmap file to load from
//			
// Returns:     true if the file loads OK false otherwise
//
// Description: Splits the file into the given amount of bitmaps.  Copies
//				the bits for each bitmap.
//			
//			
// ----------------------------------------------------------------------
bool CompressedGallery::LoadFromC16(FilePath& fileName)
{
	if(myName.GetFullPath() == fileName.GetFullPath())
		return true;

	myName = fileName;
	myCount = 0;
	myPixelFormat =0;
	myMemoryMappedFile.Open(fileName.GetFullPath() );

	if (myCompressedBitmaps)
		delete [] myCompressedBitmaps;

	// get my 16 bit pixel format flag 565 or 555
	myPixelFormat = myMemoryMappedFile.ReadUINT32();

	// for new 16 bit tag check the tag!  because this could be an old c16
	// using the 8 bit tag.
	ASSERT( (myPixelFormat & C16_16BITFLAG) == 0 );

	myCount = myMemoryMappedFile.ReadUINT16();

	ASSERT(myCount != 0);

	if(myCount == 0)
	{

		ErrorMessageHandler::Show("gallery_error",
									1,
									std::string("CompressedGallery::CompressedGallery"),
									myName.GetFullPath().c_str());
	}

	//create the correct number of bitmaps
	myCompressedBitmaps= new CompressedBitmap[myCount];
	uint32 pos =0;
	for(uint32 i = 0; i < myCount; i++)
	{
#ifdef _DEBUG
		// set the name and offset
		myCompressedBitmaps[i].SetName(myName.GetFullPath());
#endif

		myCompressedBitmaps[i].InitHeader(myMemoryMappedFile);
		pos = myMemoryMappedFile.GetPosition();

		myCompressedBitmaps[i].LoadFromC16(myMemoryMappedFile);	
		myMemoryMappedFile.Seek(pos,File::Start);	
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
//			
// ----------------------------------------------------------------------
uint32 CompressedGallery::Save(uint8*& data)
{

	uint32 bytesWritten = 0;
	uint32 flag = 0x01;

	CompressedBitmap* bitmap = myCompressedBitmaps;
	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file
	(*(uint32*)data) = flag;
	data+=4;

	// Now the number of images in the file
	*((uint16*)data) = myCount;
	data+=2;

	
	bytesWritten +=6;
		

	int i;
	for( i = 0; i < myCount; i++,bitmap++)
	{
		bytesWritten += bitmap->SaveHeader(data);
	}

	// now save out each bitmaps offset information 
	bitmap = myCompressedBitmaps;
	for(i = 0; i < myCount; i++,bitmap++)
	{
		bytesWritten +=  bitmap->SaveOffsetData(data);
	}

	// now save out each bit
	bitmap = myCompressedBitmaps;
	for(i = 0; i < myCount; i++,bitmap++)
	{
		bytesWritten +=  bitmap->SaveData(data);
	}

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   dataPtr - point in file to write to
//			
// Returns:     true if the file writes OK false otherwise
//
// Description: Saves the bits and headers for each bitmap.
//			
// ----------------------------------------------------------------------
uint32 CompressedGallery::Save(MemoryMappedFile& file)
{

	uint32 bytesWritten = 0;
	uint32 flag = 0x01;

	// *********Changes
//	CompressedBitmap* bitmap = myCompressedBitmaps;
	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file

	file.WriteUINT32(myPixelFormat);

	// Now the number of images in the file

	file.WriteUINT16(myCount);

	
	bytesWritten +=6;
		
	int i;
	for( i = 0; i < myCount; i++)
	{
		bytesWritten += myCompressedBitmaps[i].SaveHeaderAndOffsetData(file);
	//	bytesWritten += myCompressedBitmaps[i].SaveOffsetData(file);
	}

	// now save out each bitmaps offset information 


	// now save out each bit
//	bitmap = myCompressedBitmaps;
	for(i = 0; i < myCount; i++)
	{
		bytesWritten +=  myCompressedBitmaps[i].SaveData(file);
	}

	return bytesWritten;
}



// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   dataPtr - point in file to write to
//			
// Returns:     true if the file writes OK false otherwise
//
// Description: Saves the bits and headers for each bitmap.
//			
// ----------------------------------------------------------------------
uint32 CompressedGallery::Save()
{
	myMemoryMappedFile.Seek(0,File::Start);

	uint32 bytesWritten = 0;
	uint32 flag = 0x01;

	CompressedBitmap* bitmap = myCompressedBitmaps;
	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file

	myMemoryMappedFile.WriteUINT32(flag);

	// Now the number of images in the file

	myMemoryMappedFile.WriteUINT16(myCount);

	
	bytesWritten +=6;
		
	int i;
	for( i = 0; i < myCount; i++,bitmap++)
	{
		bytesWritten += bitmap->SaveHeader(myMemoryMappedFile);
		bytesWritten +=  bitmap->SaveOffsetData(myMemoryMappedFile);
	}

	// now save out each bitmaps offset information 

	// now save out each bit
	bitmap = myCompressedBitmaps;
	for(i = 0; i < myCount; i++,bitmap++)
	{
		bytesWritten +=  bitmap->SaveData(myMemoryMappedFile);
	}

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool CompressedGallery::Write(CreaturesArchive &archive) const
	{

	archive << myCount;
//	for	(int i=0; i<myCount; i++) myCompressedBitmaps[i].Write(archive);
	return true;
	}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool CompressedGallery::Read(CreaturesArchive &archive)
{
	// Check version info
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		archive >> myCount;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return false;

}

Sprite* CompressedGallery::CreateSprite(EntityImage* owner)
{

	Sprite* sprite=new CompressedSprite(owner);

	return sprite;
}

// virtual because of the call to save()
bool CompressedGallery::ConvertTo(uint32 to)
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

	CompressedBitmap* bitmap = myCompressedBitmaps;

	for(uint32 i = 0; i < myCount; i++,bitmap++)
	{
		bitmap->Convert(from,to);
	}

	myPixelFormat ^= C16_FLAG_565;

	// now save your data out
	// as only you would
	myMemoryMappedFile.Seek(0,File::Start);
	Save(myMemoryMappedFile);

	return true;

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
bool CompressedGallery::IsPixelTransparent(uint32 x,
								uint32 y,
								int32 imageIndex)
{
	
	if(imageIndex < myCount)
	{
		CompressedBitmap* bitmap = myCompressedBitmaps+imageIndex;

		if(bitmap)
		{
			return bitmap->IsPixelTransparent(Position(x,y));
		}
	}
	return true;
}

Bitmap* CompressedGallery::GetBitmap(uint32 index)
{
//	CompressedBitmap* bitmap = myCompressedBitmaps;
	return index<myCount ? myCompressedBitmaps+index : myCompressedBitmaps;
}

bool CompressedGallery::IsValid()
	
{
	return myCompressedBitmaps ? true:false;
}


bool CompressedGallery::ValidateBitmapSizes(int32 minimumWidth, int32 minimumHeight)
{
	for (int32 i = 0; i< myCount; i++)
	{
		// as soon as you catch just one bitmap which is under 3x3 bail
		if(myCompressedBitmaps[i].GetWidth() < minimumWidth || 
			myCompressedBitmaps[i].GetHeight() < minimumHeight )
			return false;
	}

	// all bitmaps conform
	return true;
}


int32 CompressedGallery::GetBitmapWidth(uint32 index)
{
	ASSERT(index<myCount);
	return index<myCount ? myCompressedBitmaps[index].GetWidth() :  myCompressedBitmaps[0].GetWidth();
}

int32 CompressedGallery::GetBitmapHeight(uint32 index)
{
	ASSERT(index<myCount);
	return index<myCount ? myCompressedBitmaps[index].GetHeight() : myCompressedBitmaps[0].GetHeight();
}

void CompressedGallery::Recolour(const uint16 tintTable[65536])
{
	for(int i = 0; i < myCount; i++)
		{
		myCompressedBitmaps[i].Recolour(tintTable);
		}
}
