// --------------------------------------------------------------------------
// Filename:	Bitmap.cpp
// Class:		Bitmap
// Purpose:		Bitmaps hold their width height and data
//				
//				
//				
//
// Description: This class represents a compressed bitmap.
//				Compressed bitmaps have pointers to the first pixel
//				in every line.  
//
//				They are not writable they always have 
//				their data in a memory mapped file and should not
//				be used with a cloned gallery.
//
// History:
// -------  
//		   Alima			Created
//  Jan99  Alima			Added compressed format
//////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"CompressedBitmap.h"
#include	"DisplayEngine.h"
#include	"MemoryMappedFile.h"

CompressedBitmap::CompressedBitmap()
{
#ifdef _DEBUG
	myAlreadyRecolouredFlag = false;
#endif
}

CompressedBitmap::~CompressedBitmap()
{
	myScanLines.clear();
	myOffsetValues.clear();
	myLineLengths.clear();
}
// ----------------------------------------------------------------------
// Method:      Save 
// Arguments:   file - file to save my data to
//			
// Returns:     None
//
// Description: Saves pixel data to the given file
//			
// ----------------------------------------------------------------------
void CompressedBitmap::Save(File& file)
{
	ASSERT(FALSE);
}


// ----------------------------------------------------------------------
// Method:      SetData 
// Arguments:   dataPtr - pointer to the start of the bitmaps data in the
//				file.
//			
// Returns:     None
//
// Description: Marks the point in the sprite file where this bitmaps
//				data begins.
//			
// ----------------------------------------------------------------------
	void CompressedBitmap::SetData(uint8*& dataPtr)
	{
		myDataFlag = false;

		myData = (uint16*)dataPtr;

		// divide by two because pointer is 16bit
		myData+=myOffset/2;
#ifdef _DEBUG
		myAlreadyRecolouredFlag = false;
#endif
		
	}

	void CompressedBitmap::SetData(MemoryMappedFile& file)
	{
		file.Seek(myOffset,File::Start);

		myDataFlag = false;

		uint8* data;

		file.ReadUINT8Ptr(data);

		myData = (uint16*)data;

#ifdef _DEBUG
		myAlreadyRecolouredFlag = false;
#endif
		
	}

// ----------------------------------------------------------------------
// Method:      SaveData 
// Arguments:   data - pointer to the start of a file containing
//				a compressed gallery
//				scanLineStartPtr - pointer to the place in the gallery
//				file where the scan line offset values are stored.
//			
// Returns:     the number of bytes we have written
//
// Description: Stores a pointer to each line in the compressed bitmap.
//				The first line's offset value is stored in myOffset
//				which is the original S16 header member.  The other
//				scanline offset values are stored in a block directly after
//				the formal header.
//
//			
// ----------------------------------------------------------------------
uint32 CompressedBitmap::SaveData(uint8*& data)
{
	// if we use this again we need to update to 16bit tag format
	// but it may be obsolete now
	ASSERT(FALSE);

	uint32 bytesWritten = 0;
	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveData 
// Arguments:   file - memory mapped to save my data to
//			
// Returns:     None
//
// Description: Saves C16 pixel data to the given file
//			
// ----------------------------------------------------------------------
uint32 CompressedBitmap::SaveData(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;
	uint16* compressedData = myData;
	uint32 count =0;
	uint32 bytes =0;
	uint16 tag =0;

	for(int32 i = 0; i< myHeight; i++)
	{
		tag = *compressedData++;
		

		while( tag != 0)	
		{	
			// write the tag
			file.WriteUINT16(tag);
			bytesWritten+=sizeof(uint16);
			// find the number of colours to plot
			count = tag >>1;
				
			// check whether the run is transparent or colour
			if(tag & 0x1)
			{
				bytes = count << 1;
				bytesWritten+=bytes;
				file.Blast(bytes,(uint8*)compressedData);
				compressedData+=count;
			}
			else
			{
			//do nothing
			}

			tag = *compressedData++;
		}

		//write the zero at the end of the line 
			bytesWritten+=2;
		file.WriteUINT16(tag);
	}
	
	// now write the zero byte
	file.WriteUINT16(*compressedData);
	bytesWritten+=2;
	
	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveOffsetData 
// Arguments:   data - pointer to the start of a file containing
//				a compressed gallery
//			
// Returns:     the number of bytes we have written
//
// Description: Stores each pixel data line offset value in the given file
//
//			
// ----------------------------------------------------------------------
uint32 CompressedBitmap::SaveOffsetData(uint8*& data)
{
	// if we use this again we need to update to 16bit tag format
	// but it may be obsolete now
	ASSERT(FALSE);

	uint32 bytesWritten = 0;
	return bytesWritten;
}

// ----------------------------------------------------------------------
// Method:      SaveOffsetData 
// Arguments:   data - pointer to the start of a file containing
//				a compressed gallery
//			
// Returns:     the number of bytes we have written
//
// Description: Stores each pixel data line offset value in the given file
//
//			
// ----------------------------------------------------------------------
uint32 CompressedBitmap::SaveOffsetData(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;
	std::vector<uint32>::iterator it;

	for(it = myOffsetValues.begin(); it !=myOffsetValues.end();it++ )
	{	
		file.WriteUINT32(*it);
		bytesWritten+=sizeof(uint32);
	}

	return bytesWritten;
}

void CompressedBitmap::LoadFromC16(MemoryMappedFile& file)
{
#ifdef _DEBUG
		myAlreadyRecolouredFlag = false;
#endif

	file.Seek(myOffset,File::Start);
	
	myDataFlag = true;

	const int bitsPerPixel = 2;
		
	delete[] myData;

	// this will be far too much space but at least it is
	// guaranteed to cover the whole set of data
	myData = new uint16[myWidth*myHeight];
	// Alima, maybe we had better allocate the right space later?
	// Done below

	// big oops! sorry folks

	if (myData)
	{
		uint32 posAtStart = file.GetPosition();

		// read int all the
		uint16* pixels = myData;
		uint16 tag = file.ReadUINT16();

		for (uint32 i = 0; i < myHeight; i++)
		{
			while(tag)
			{				
				*pixels++ = tag;

				if (tag & 0x01)
				{
					uint32 count = tag >> 1;
					uint8* data = (uint8*)pixels;
					file.BlockCopy(count * sizeof(uint16),data);
					pixels += count;
				}

				tag = file.ReadUINT16();
			}
			// add the zero that we just found
			*pixels++ = tag;
			tag = file.ReadUINT16();
		}

		*pixels++ = tag;

		// Move the data to free lots of memory
		uint32 posAtEnd = file.GetPosition();
		uint32 actualSize = posAtEnd - posAtStart;
		uint32 actualSizeInWords = pixels - myData;
		uint32 actualSize2 = actualSizeInWords * sizeof(uint16);
		ASSERT(actualSize == actualSize2);
		uint16* myMovingData = new uint16[actualSizeInWords];
		memcpy(myMovingData, myData, actualSize);
		delete[] myData;
		myData = myMovingData;
		myMovingData = NULL;
	}
}

// ----------------------------------------------------------------------
// Method:      GetMyLineLengths 
// Arguments:   lineLengths - place to put all line lengths
//			
// Returns:     None
//
// Description: Helper method for LoadfromC16 
//				store the actual lengths of lines of data so that we
//				can store scanlines in heap memory.
//
//			
// ----------------------------------------------------------------------
void CompressedBitmap::GetMyLineLengths(std::vector<uint32>& lineLengths)
{

	int size = myOffsetValues.size();

	for(uint32 i = 1; i<myHeight-1; i++)
	{
	
	lineLengths.push_back(myOffsetValues[i] - myOffsetValues[i-1]);

	}
}

void CompressedBitmap::StoreLineLengths()
{

	int size = myOffsetValues.size();

	for(uint32 i = 1; i<myHeight-1; i++)
	{
	
	myLineLengths.push_back(myOffsetValues[i] - myOffsetValues[i-1]);

	}
}
// ----------------------------------------------------------------------
// Method:      SetScanLines 
// Arguments:   file - file to read from.  MUST be atposition containing
//						compressed bitmaps line offsets
//			
// Returns:     None
//
// Description: Stores a pointer to each line in the compressed bitmap.
//				The first line's offset value is stored in myOffset
//				which is the original S16 header member.  The other
//				scanline offset values are stored in a block directly after
//				the formal header.
//
//			
// ----------------------------------------------------------------------
void CompressedBitmap::SetScanLines(File& file)
{
	uint32 scanValue = 0;
	// do one less scan line because we have the first offset
	// value in myOffset in the header
	for(int h = myHeight-1; h--;)
	{
		file.Read(&scanValue,sizeof(scanValue));
		myOffsetValues.push_back(scanValue);
		// We will need the line pointers
		// in the heap memory created by the bitmap
		// this will be done as we get the pixel data
	}
}

// needs re writing for compressed format
void CompressedBitmap::Recolour(const uint16*&tintTable)
{
#ifdef _DEBUG
	_ASSERT(!myAlreadyRecolouredFlag);
	myAlreadyRecolouredFlag = true;
#endif

	// Replace each pixel with its tinted equivalent
	uint16* ptr = (uint16*)myData;
	uint16 tag = 0;
	uint32 count =0;
	uint16 colour = 0;
	uint16* pixels = 0;

	//for each line
	for (uint32 numLines =myHeight;numLines--;)
	{
		tag = *ptr++;
		while(tag)
		{
			// if a colour recolour the pixels
			if(tag & 0x01)
			{
				count = tag >> 1;

				pixels = ptr;

				for(uint32 i =0; i < count; i++)
				{
					colour = tintTable[ GetUINT16At(pixels) ];
					*pixels= colour;
					pixels++;
				}
				ptr+=count;
			}
		
		tag = *ptr++;
		}
		
	}

}


void CompressedBitmap::UpdateOffsetValues(uint32 firstDataLine)
{
	int32 oldOffset = myOffset;
	myOffset = firstDataLine;

	std::vector<uint32>::iterator it;
	for(it = myOffsetValues.begin(); it !=myOffsetValues.end();it++ )
	{	
	 (*it) = (*it) - oldOffset + firstDataLine;
	}

}

void CompressedBitmap::Convert(uint32 from, uint32 to)
{
	uint16* pixels = myData;
	uint16* compressedData =myData;
	uint16 	s = 0;
	uint16 r=0;
	uint16 g=0;
	uint16 b=0;
	uint32 count =0;

	uint16 result = 0;

	switch(from)
	{
		case RGB_555:
		{
			switch(to)
			{
				case RGB_565:
				{
					for(int32 i = 0; i< myHeight; i++)
					{	
						uint16 tag = *compressedData++;
						while( tag != 0)	
						{	
				
							// find the number of colours to plot
							count = tag >>1;
						
			
							// check whether the run is transparent or colour
							if(tag & 0x01)
							{
								// move on to the pixel data
								pixels = (uint16*)compressedData;

								//convert the colours
								for(uint32 i = 0; i<count; i++)
								{

									P555_TO_RGB(*pixels, r, g, b);
									RGB_TO_565(r, g, b, result);

									*pixels = result;

									// Preserve non-transparency.
									if (*pixels == 0x0000 && 
										(r != 0x00 || g != 0x00 || b != 0x00))
									{
										*pixels = 0x0821; // Dark grey.
									}

									pixels++;
								}// end for
								compressedData+=count; // 16 bit format
							}// end if	
							tag = *compressedData++;
						}// end while
					
					}// end for
					break;
				}// end case
				default:
				break;
			}// end switch
		}// end case
		case RGB_565:
		{
			switch(to)
			{

				case RGB_555:
				{
					for(int32 i = 0; i< myHeight; i++)
					{
						uint16 tag = *compressedData++;
						while( tag != 0)
						{	
							// find the number of colours to plot
							count = tag >>1;
					
							// check whether the run is transparent or colour
							if(tag & 0x01)
							{
								//convert the colours
								// move on to the pixel data
								pixels = compressedData;


								for(uint32 i = 0; i<count; i++)
								{

									P565_TO_RGB(*pixels, r, g, b);
									RGB_TO_555(r, g, b, result);
									*pixels = result;

									// Preserve non-transparency.
									if (*pixels == 0x0000 && 
										(r != 0x00 || g != 0x00 || b != 0x00))
										*pixels = 0x0421; // Dark grey.

									pixels++;
								}
									compressedData+=count; // 16 bit format
							}
							tag = *compressedData++;
						}//end while
					}// end for
				}//end case
			default:
				break;
			}//end switch
		}//end case
		default:
			break;
	}// end switch
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
bool CompressedBitmap::IsPixelTransparent(Position pixelPos)
{
	// get the line that the pixel is in from the y coordinate
	if( pixelPos.GetY() < 0 || pixelPos.GetX() < 0 ||
		pixelPos.GetY() >= myHeight || pixelPos.GetX() >= myWidth )
		return true;

	if(pixelPos.GetY() < myScanLines.size())
	{
		uint16* compressedData = (uint16*)GetScanLine(pixelPos.GetY());

		if(!compressedData)
			return true;

		// search for the pixel based on the x position
		uint16 tag = *compressedData++;
		uint32 count;

		// take account of zero index
		uint32 stopAt = pixelPos.GetX();

		while( tag != 0)	
		{	
			// find the number of colours to plot
			count = tag >>1;

			// if we are under or stopAt number then keep looking
			if(count< stopAt)
			{
				// check whether the run is transparent or colour
				if(tag & 0x01)
				{
					// move the data on
					compressedData+=count;
				}
				stopAt-=count;
			}
			else
			{
				
				if(tag & 0x01)
				{
					return false;
					
				}
				else
				{
					return true;
				}
			}

			tag = *compressedData++;
		}
	}
	return true;
}

void CompressedBitmap::InitHeader(MemoryMappedFile& file)
{
	myOffset = file.ReadUINT32();
	myWidth = file.ReadUINT16();
	myHeight = file.ReadUINT16();

	SetScanLines(file);
}

uint8* CompressedBitmap::GetScanLine(uint32 index)
{
	if (index >= myScanLines.size())
		return NULL;
	return myScanLines[index];
}