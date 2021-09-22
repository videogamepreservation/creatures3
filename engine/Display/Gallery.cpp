// --------------------------------------------------------------------------
// Filename:	Gallery.cpp
// Class:		Gallery
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

#include "Gallery.h"
#include "System.h"
#include <stdio.h>
#include "ErrorMessageHandler.h"
#ifdef _WIN32
#include "../../common/RegistryHandler.h"
#endif // _WIN32
#include	"../App.h"
#include	"Bitmap.h"
#include	"../File.h"


const std::string S16 = ".s16";
const std::string C16 = ".c16";

CREATURES_IMPLEMENT_SERIAL_NOT( Gallery )


Gallery::~Gallery()
{
if( myBitmaps) 
{
	delete [] myBitmaps;	
		myBitmaps = NULL;
}
}


Gallery::Gallery( FilePath const &name )
:myName(name),
myFileSpec(0)
{
	InitUndefinableMembers();
}


void Gallery::InitUndefinableMembers()
	{
		myCount= 0;
		myBitmaps=NULL;
		// someone has requested you so someone is using you
		myReferenceCount = 1; 
		// read the pixel format from the registry
		// Has display type been previously determined?
		// this should look alot better when the new registry
		// class comes!!
		uint32 tempReg = 0;//RGB_UNKNOWN
//		theApp.GetRegistry().
//			Get(REG_APP,"Display Type",(int&)tempReg);

		std::string value("Display Type");
#ifdef _WIN32
		theRegistry.GetValue(theRegistry.DefaultKey(),
						value,
						tempReg,	
						HKEY_LOCAL_MACHINE);
#else
		theApp.MachineSettings().Get( value,(int&)tempReg );
#endif // _WIN32
		myPixelFormat = tempReg;
	}

#ifdef THIS_FUNCTION_IS_NOT_USED
// ----------------------------------------------------------------------
// Method:      LoadFromBmp 
// Arguments:   name - name of the bitmap file to load from
//				width - the width of the bitmaps that the whole file will be
//				split into.
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
bool Gallery::LoadFromBmp(char* name,uint32 width,uint32 height)
{
	bool	success=false;

	BITMAP	bitmap;

	if (myBitmaps) delete [] myBitmaps;

 	HBITMAP	handle=(HBITMAP)LoadImage(	NULL,
										name,
										IMAGE_BITMAP,
										0,
										0,
										LR_LOADFROMFILE|LR_CREATEDIBSECTION);
	if (handle)
	{
		GetObject(handle,sizeof bitmap,&bitmap);
		if (bitmap.bmPlanes==1 && bitmap.bmBitsPixel==24)
		{
			// get the height and width of each bitmap the file is split into
			uint32 bitmap_width=bitmap.bmWidth/width;
			uint32 bitmap_height=bitmap.bmHeight/height;

			// i need so many bitmaps to store this file
			myCount=width*height;
			myBitmaps=new Bitmap[myCount];

			uint32 index=0;
			uint8* inverted_bitmap=(uint8*)bitmap.bmBits+
				(3*bitmap.bmWidth*(bitmap.bmHeight-1));

			uint32 i,j;
			for ( j=height;j--;)
			{
				for ( i=width;i--;)
				{
					myBitmaps[index].SetWidth(bitmap_width);
					myBitmaps[index].SetHeight(bitmap_height);
			
					uint8* src=inverted_bitmap;
					uint16* dst=new uint16[bitmap_width*bitmap_height];
					myBitmaps[index].SetData(dst);
					for (int32 pj=bitmap_height;pj--;)
					{
						for (int32 pi=bitmap_width;pi--;)
						{
							uint32 b=*src++;
							uint32 g=*src++;
							uint32 r=*src++;
							*dst++=(uint16)((r<<10)|(g<<5)|(b));
						}
						src-=3*bitmap_width;
						src-=3*bitmap.bmWidth;
					}
					inverted_bitmap+=3*bitmap_width;
					index++;
				}
				inverted_bitmap-=3*bitmap_width*width;
				inverted_bitmap-=3*bitmap.bmWidth*bitmap_height;
			}
			success=true;
		}
	}
	return success;
}
#endif // THIS_FUNCTION_IS_NOT_USED


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
uint32 Gallery::Save(uint8*& data)
{

	uint32 bytesWritten = 0;
//	flag = 0x01 for
	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file
	(*(uint32*)data) = myPixelFormat;

	data+=4;

	// Now the number of images in the file
	*((uint16*)data) = myCount;
	data+=2;

	
	bytesWritten +=6;
		
	int i;
	for( i = 0; i < myCount; i++)
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
uint32 Gallery::Save(MemoryMappedFile& file)
{

	uint32 bytesWritten = 0;
//	flag = 0x01 for
	// First four bytes is a set of flags to say whether this is
	// a 565 or 555 sprite file. I am assuming this is a 565 file
//	(*(uint32*)data) = myPixelFormat;

	file.WriteUINT32(myPixelFormat);
//	data+=4;

	// Now the number of images in the file
//	*((uint16*)data) = myCount;
	file.WriteUINT16(myCount);
//	data+=2;
	bytesWritten +=6;
		
	int i;
	for( i=0; i < myCount; i++)
	{
		bytesWritten += myBitmaps[i].SaveHeader(file);
	}


	//now read in each bit

	for(i = 0; i < myCount; i++)
	{
		bytesWritten += myBitmaps[i].SaveData(file);
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
bool Gallery::Write(CreaturesArchive &archive) const
{
	// Verision info:

	archive << myCount;
	for	(int i=0; i<myCount; i++)
		myBitmaps[i].Write( archive );

	archive << myReferenceCount;
	archive << myName;
	archive << myFileSpec;

	archive << myPixelFormat;
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Gallery::Read(CreaturesArchive &archive)
{
			
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		archive >> myCount;

		CreateBitmaps();

		for	(int i = 0; i < myCount; i++) 
		{
			if(!myBitmaps[i].Read( archive ))
				return false;
		}

		archive >> myReferenceCount;
		archive >> myName;
		archive >> myFileSpec;

		archive >> myPixelFormat;
	}
	else
	{
		_ASSERT(false);
		return false;
	}


	return true;
}

void Gallery::CreateBitmaps()
{
	myBitmaps = new Bitmap[myCount];
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
bool Gallery::IsPixelTransparent(uint32 x,
								uint32 y,
								int32 imageIndex)
{
	
	Bitmap* bitmap = GetBitmap(imageIndex);

	if(bitmap)
	{
	return	bitmap->IsPixelTransparent(Position(x,y));
	}
	return true;
}

	// any gallery can create a normal sprite or a fast sprite
Sprite* Gallery::CreateSprite(EntityImage* owner)
{
	return NULL;
}
Sprite* Gallery::CreateFastSprite(EntityImage* owner)
{
	return NULL;
}


Bitmap* Gallery::GetBitmap(uint32 index)
{
	if (index<myCount)
		return index<myCount ? myBitmaps+index : myBitmaps;
	else
		throw GalleryException(theCatalogue.Get("gallery_error", 0),__LINE__);
}


int32 Gallery::GetBitmapWidth(uint32 index)
	{
		ASSERT(index<myCount);
		return index<myCount ? myBitmaps[index].GetWidth() :  myBitmaps[0].GetWidth();
	}

int32 Gallery::GetBitmapHeight(uint32 index)
	{
		ASSERT(index<myCount);
		return index<myCount ? myBitmaps[index].GetHeight() : myBitmaps[0].GetHeight();
	}

bool Gallery::IsValid() 
{
	return myBitmaps?true:false;
}


bool Gallery::ValidateBitmapSizes(int32 minimumWidth, int32 minimumHeight)
{
	for (int32 i = 0; i< myCount; i++)
	{
		// as soon as you catch just one bitmap which is under 3x3 bail
		if(myBitmaps[i].GetWidth() < minimumWidth ||
			myBitmaps[i].GetHeight() < minimumHeight )
			return false;
	}

	// all bitmaps conform
	return true;
}

void Gallery::Recolour(const uint16 tintTable[65536])
{
	_ASSERT(myBitmaps);

	for(int i = 0; i < myCount; i++)
	{
		myBitmaps[i].Recolour(tintTable);
	}
}
