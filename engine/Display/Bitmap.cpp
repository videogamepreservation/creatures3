// --------------------------------------------------------------------------
// Filename:	Bitmap.cpp
// Class:		Bitmap
// Purpose:		Bitmaps hold their width height and data
//				
//				
//				
//
// Description: Bitmaps can be either compressed or normal.  If they are
//				compressed then they have pointers to the first pixel
//				in every line.  
//
//				Bitmaps either create their own data or point to 
//				their data in a memory mapped file.
//
// History:
// -------  Chris Wylie		Created
// 11Nov98	Alima			Added LoadFromS16
//  Jan99  Alima			Added compressed format
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"Bitmap.h"
#include	"DisplayEngine.h"
#include	"MemoryMappedFile.h"
#include	"../File.h"
#include    "DisplayEnginePlotFunctions.h"
#include    "../CreaturesArchive.h"



CREATURES_IMPLEMENT_SERIAL( Bitmap )

Bitmap::Bitmap()
:myWidth(0),
myHeight(0),
myDataFlag(0),
myOffset(0),
myData(0),
myXOffset(0),
myYOffset(0),
myPosition(0,0),
myClippedWidth(0),
myClippedHeight(0)
{
}

Bitmap::~Bitmap()
	{
		// only delete the bits if you created them
		if(myDataFlag)
		{
		if (myData) delete [] myData;
		myData = NULL;
		}
	}


#ifdef THIS_FUNCTION_IS_NOT_USED
// ----------------------------------------------------------------------
// Method:      LoadFromBmp 
// Arguments:   name - name (or id) of the file to load the bitmap from.
// Returns:     true if bitmap was loaded OK false otherwise
//
// Description: Loads bitmap from given file and copies the bits
//				
// ----------------------------------------------------------------------
bool Bitmap::LoadFromBmp(char* name)
{
	bool	success=false;
	BITMAP	bitmap;
 	HBITMAP	handle;
	
	handle=(HBITMAP)LoadImage(NULL,name,IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_CREATEDIBSECTION);

	if (handle)
	{
		GetObject(handle,sizeof bitmap,&bitmap);


		// we only deal with 1 plane and 3 bytes per pixel
		if (bitmap.bmPlanes==1 && bitmap.bmBitsPixel==24)
		{
			//update the width and height
			myWidth=bitmap.bmWidth;
			myHeight=bitmap.bmHeight;


			//get the bits
			uint8* src=(uint8*)bitmap.bmBits+(3*bitmap.bmWidth*(bitmap.bmHeight-1));
			uint16* dst=new uint16[myWidth*myHeight];
			myData=dst;

			//change the bitmap format
			for (int32 j=myHeight;j--;)
			{
				for (int32 i=myWidth;i--;)
				{
					uint32 b=*src++;
					uint32 g=*src++;
					uint32 r=*src++;
					*dst++=(uint16)((r<<10)|(g<<5)|(b));
				}
				src-=2*3*bitmap.bmWidth;
			}
			success=true;
		}
	}
	return success;
}
#endif // THIS_FUNCTION_IS_NOT_USED


// ----------------------------------------------------------------------
// Method:      LoadFromS16 
// Arguments:   file - file to read the data from.  File must be in the
//					   correct position!!!
// Returns:     None
//
// Description: Loads bitmap from given file and copies the bits
//				
// ----------------------------------------------------------------------
void Bitmap::LoadFromS16(File& file)
{
	myDataFlag = true;

	const int bitsPerPixel = 2;
	myData=new uint16[myWidth*myHeight];

	ASSERT(myData);
	if(myData)
	{

	file.Read(myData,myWidth * myHeight * bitsPerPixel);
	}
	// for testing purposes
	myXOffset=0;//myWidth>>1;
	myYOffset=0;//myHeight;
}
/*
void Bitmap::CreateTest(int32 width,int32 height,int32 index)
{
	uint16 colour0=gColourTable[index&(COLOUR_TABLE_SIZE-1)];
	uint16 colour1=gColourTable[(index/COLOUR_TABLE_SIZE)&(COLOUR_TABLE_SIZE-1)];
	myWidth=width;
	myHeight=height;
	uint16* ptr=new uint16[width*height];
	myData=ptr;
	for (int32 j=height;j--;)
	{
		for (int32 i=width>>1;i--;)
		{
			*ptr++=colour0;
			*ptr++=colour1;
		}
		uint16 temp=colour0;
		colour0=colour1;
		colour1=temp;
	}
}*/

// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   file - file to save my data to
//			
// Returns:     None
//
// Description: Saves pixel data to the given file
//			
// ----------------------------------------------------------------------
void Bitmap::Save(File& file)
{
	file.Write(myData,myWidth*myHeight);
}

// ----------------------------------------------------------------------
// Method:      SaveHeader 
// Arguments:   data - raw memory to write to
//			
// Returns:     None
//
// Description: Saves S16 header data to the given memory location
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveHeader(uint8*& data)
{
	uint32 bytesWritten = 0;

	// write the offset
	*((uint32*)data) = myOffset;
	data+=4;
	bytesWritten+=4;

	// Now save in the width and height as shorts
	*((uint16*)data) = myWidth;
	data+=2;
	bytesWritten+=2;

	*((uint16*)data) = myHeight;
	data+=2;
	bytesWritten+=2;

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveHeader 
// Arguments:   file - memory mapped to save my data to
//			
// Returns:     None
//
// Description: Saves S16 header data to the given file
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveHeader(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;
	
	file.WriteUINT32(myOffset);
	bytesWritten += sizeof(uint32);

	file.WriteUINT16(myWidth);
	bytesWritten += sizeof(uint16);

	file.WriteUINT16(myHeight);
	bytesWritten += sizeof(uint16);

	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveData
// Arguments:   data - memory to save my data to
//			
// Returns:     None
//
// Description: Saves pixel data to the given memory location
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveData(uint8*& data)
{
	uint32 bytesWritten = 0;
	uint16* pixels = myData;
	static int count = 1;

	for(uint32 i = 0; i < myWidth*myHeight;i++)
	{

		*((uint16*)data) = *pixels++;

		data+=2;
		bytesWritten+=2;
	}
	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveData 
// Arguments:   file - memory mapped to save my data to
//			
// Returns:     None
//
// Description: Saves S16 pixel data to the given file
//			
// ----------------------------------------------------------------------
uint32 Bitmap::SaveData(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;
	uint16* pixels = myData;

	for(uint32 i = 0; i < myWidth*myHeight;i++)
	{
		file.WriteUINT16(*pixels);
		bytesWritten+=sizeof(uint16);
		pixels++;
	}
	
	return bytesWritten;
}

bool Bitmap::UpdateBoundIfLarger(RECT& rect)
{
	uint32 rectWidth = rect.right - rect.left;
	uint32 rectHeight = rect.bottom - rect.top;

	bool updated = false;
	if(myWidth > rectWidth)
	{
		rect.right = myWidth;
		updated = true;
	}

	if(myHeight > rectHeight)
	{
		rect.bottom = myHeight;
		updated = true;
	}
	return updated;
}

void Bitmap::Draw()
{
	DisplayEngine::theRenderer().DrawBitmap(myPosition,*this);
}

void Bitmap::DrawWholeBitmapRegardless()
{
	DisplayEngine::theRenderer().DrawWholeBitmapRegardless(myPosition,*this);
}


// world position to start drawing on the screen
void Bitmap::OffsetDraw(Position& pos ,RECT& clip)
{
	DisplayEngine::theRenderer().OffsetDrawTile(pos,*this,
											   clip);
}

void Bitmap::Recolour(const uint16*& tintTable)
{
	// Replace each pixel with its tinted equivalent
	uint16* ptr = myData;
	for (int i=myWidth*myHeight;i--;)
	{
		*ptr ++ = tintTable[ *ptr ];
	}
}


// ----------------------------------------------------------------------
// Method:      IsPixelTransparent 
// Arguments:   imageIndex - the sprite image to test
//								if this is -1 then use the current image
//				x - x coordinate of the pixel to test
//				y - y coordinate of the pixel to test
//
// Returns:     true if the pixel at the given coordinates is transparent
//				false otherwise. Note no checks for out of bounds
//			
// ----------------------------------------------------------------------
bool Bitmap::IsPixelTransparent(Position pixelPos)
{
	if( pixelPos.GetY() < 0 || pixelPos.GetX() < 0 ||
		pixelPos.GetY() >= myHeight || pixelPos.GetX() >= myWidth )
		return true;

	uint32 actualPixel = (myWidth*pixelPos.GetY()) + pixelPos.GetX(); 

	ASSERT(actualPixel < myWidth*myHeight);
	
	return (myData[actualPixel] == 0 ? true : false );
}

void Bitmap::SetData(MemoryMappedFile& file)
{
	myDataFlag = false;

	// point to your dfata in the gievn file
	uint8* data;
	file.ReadUINT8Ptr(data);
	myData = (uint16*)data;
	file.Seek((myWidth * myHeight * BitsPerPixel)-1,File::Current);
}


void Bitmap::Convert(uint32 from, uint32 to)
{
	uint16* pixels = myData;
	uint16 	s = 0;
	uint16 r=0;
	uint16 g=0;
	uint16 b=0;

	switch(from)
	{
		case RGB_555:
		{
			switch(to)
			{
				case RGB_565:
				{
					for(uint32 i = 0; i<myWidth*myHeight;i++)
					{
						P555_TO_RGB(*pixels, r, g, b);
						RGB_TO_565(r, g, b, *pixels);

						// Preserve non-transparency.
						if (*pixels == 0x0000 && (r != 0x00 || g != 0x00 || b != 0x00))
							*pixels = 0x0821; // Dark grey.

						pixels++;
					}
					break;
				}
		
				default:
				break;
			}
	
			break;
		}
		case RGB_565:
		{
			switch(to)
			{

				case RGB_555:
				{
					for(uint32 i = 0; i<myWidth*myHeight;i++)
					{
						P565_TO_RGB(*pixels, r, g, b);
						RGB_TO_555(r, g, b, *pixels);

						// Preserve non-transparency.
						if (*pixels == 0x0000 && (r != 0x00 || g != 0x00 || b != 0x00))
							*pixels = 0x0421; // Dark grey.

						pixels++;
					}
				}
				default:
					break;
			}
		}
		default:
			break;
	}
}


void Bitmap::CreateBlankCanvas()
{
	if(myData)
		delete [] myData;

	myData = new uint16[myWidth*myHeight];

	ZeroMemory(myData,myWidth*myHeight*2);
}

void Bitmap::ResetCanvas()
{
	if(myData)
		ZeroMemory(myData,myWidth*myHeight*2);
}

void Bitmap::SetData(uint8*& dataPtr)
{

	myDataFlag = false;

	// just point to your data in the file
	myData = (uint16*)dataPtr;

	//move the dataPtr on to the next bitmaps details
	dataPtr += myWidth * myHeight * BitsPerPixel;
}


void Bitmap::InitHeader(MemoryMappedFile& file)
{
	myOffset = file.ReadUINT32();

	myWidth = file.ReadUINT16();

	myHeight = file.ReadUINT16();

}

bool Bitmap::Write(CreaturesArchive &archive) const
{
	archive << myGalleryName;
	archive << myWidth;
	archive << myHeight;
	archive << myDataFlag;

	archive << myOffset;
	archive << myXOffset;
	archive << myYOffset;

	archive << myPosition;
	archive.Write( (void *)myData, myWidth * myHeight * 2 );
	return true;
}

bool Bitmap::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		archive >> myGalleryName;
		archive >> myWidth;
		archive >> myHeight;
		archive >> myDataFlag;

		archive >> myOffset;
		archive >> myXOffset;
		archive >> myYOffset;

		archive >> myPosition;
		myData = new uint16[myWidth * myHeight];
		archive.Read( (void *)myData, myWidth * myHeight * 2 );
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

void Bitmap::DecompressC16(File& file)
{
#ifdef _DEBUG
	// see whether we are dealing with a compressed file by looking
			// at the file extension
	std::string galleryName(file.GetName());
	int x = galleryName.find_last_of(".");
	
	std::string ext = galleryName.substr(x, 3);

	ASSERT(ext[1] == 'C' || ext[1] == 'c');	
#endif

	myDataFlag = true;
	uint16 tag =0;

	const int bitsPerPixel = 2;

	delete myData;

	// this will be far too much space but at least it is
	// guaranteed to cover the whole set of data
	myData = new uint16[myWidth*myHeight];

	if(myData)
	{
		// read int all the
		uint16* pixels = (uint16*)myData;

		uint16 data = 255;
		uint32 count=0;
		file.Read(&tag,2);
		count = tag >> 1;

		for(uint32 i = 0; i < myHeight; i++)
		{
			while(tag)
			{
		
				if(tag & 0x01)
				{
					count = tag >> 1;
					for(int i = 0; i < count ; i++)
					{
						file.Read(&tag,2);
						*pixels++ = tag;
					}
				}
				else
				{
					count = tag >> 1;
					for(int i = 0; i < count ; i++)
					{
						*pixels++ = 0;
					}
				}

			file.Read(&tag,2);
			}
			// read the last zero for end of file
			file.Read(&tag,2);

		}

		
		
	}
}