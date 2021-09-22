// --------------------------------------------------------------------------
// Filename:	CompressedBitmap.h
// Class:		CompressedBitmap
// Purpose:		Bitmaps hold their width height and data
//				
//				
//				
//
// Description: CompressedBitmap have transparency encoding of their
//				 pixel data. They also have pointers to the first pixel in
//				 every line to aid rendering.  
//
//				CompressedBitmap point to 
//				their data in a memory mapped file.
//
//				The format is very similar to the s16 format except that
//				after the normal sprite header come the scanline offsets.
//				We have a scanline value for each line except the top one
//				which is covered by the original S16 format Header member:
//				myOffset
//
//			
//
// History:
// -------  Alima	Created
// 11Nov98	Alima			Added LoadFromS16
//  Jan99  Alima			Added compressed format
//////////////////////////////////////////////////////////////////////////////
#ifndef		COMPRESSEDBITMAP_H
#define		COMPRESSEDBITMAP_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Bitmap.h"
#include "MemoryMappedFile.h"

class MemoryMappedFile;

class CompressedBitmap : public Bitmap
{
public:
	CompressedBitmap();
	virtual ~CompressedBitmap();

	virtual void SetData(uint8*& dataPtr);
	virtual void SetData(MemoryMappedFile& file);
	
	inline void SetScanLines(MemoryMappedFile& file);

	void SetScanLines(File& file);

	virtual uint8* GetScanLine(uint32 index);


	virtual void Save(File& file);

	virtual uint32 SaveData(uint8*& data);
	virtual uint32 SaveData(MemoryMappedFile& file);

	virtual void Recolour(const uint16*&tintTable);

	inline uint32 SaveHeaderAndOffsetData(MemoryMappedFile& file);

	uint32 SaveOffsetData(uint8*& data);
	uint32 SaveOffsetData(MemoryMappedFile& file);

	void LoadFromC16(MemoryMappedFile& file);

	void StoreLineLengths();

	void UpdateOffsetValues(uint32 firstOffset);
	virtual void Convert(uint32 from, uint32 to);
	virtual bool IsPixelTransparent(Position pixelPos);
	virtual void InitHeader(MemoryMappedFile& file);

protected:

	std::vector<uint8*> myScanLines;
	std::vector<uint32> myOffsetValues;

private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	CompressedBitmap (const CompressedBitmap&);
	CompressedBitmap& operator= (const CompressedBitmap&);

#ifdef _DEBUG
	bool myAlreadyRecolouredFlag;
#endif

	void GetMyLineLengths(std::vector<uint32>& lineLengths);
	std::vector<uint32> myLineLengths;
};

uint32 CompressedBitmap::SaveHeaderAndOffsetData(MemoryMappedFile& file)
{
	uint32 bytesWritten = 0;
	
	file.WriteUINT32(myOffset);
	bytesWritten += sizeof(uint32);

	file.WriteUINT16(myWidth);
	bytesWritten += sizeof(uint16);

	file.WriteUINT16(myHeight);
	bytesWritten += sizeof(uint16);

	std::vector<uint32>::iterator it;

	for(it = myOffsetValues.begin(); it !=myOffsetValues.end();it++ )
	{	
		file.WriteUINT32(*it);
		bytesWritten+=sizeof(uint32);
	}

	return bytesWritten;
}

void CompressedBitmap::SetScanLines(MemoryMappedFile& file)
	{
		// this is the pointer we will store when it points to a
		// scan line.
		uint32 scanPosition = file.GetPosition(); 


		uint8* data = NULL;
		uint32 offset =0;
		// we find the scan lines by calculating where the next
		// scanline starts, using the previous offset.
		for(int i = 0 ; i< myHeight; i++)
		{
			
		//	data =  fileStart;
			// for the first offsetvalue use myOffset
			if(i == 0)
			{
		
			file.Seek(myOffset,File::Start);
			file.ReadUINT8Ptr(data);
			myScanLines.push_back(data);
			}
			else
			{
			// find the actual start of this line of data
			// and store a pointer to it

			// go to where the offset of the first
			// line of data is stored
			file.Seek(scanPosition,File::Start);

			// read the offset and point the pixel data there
			offset = file.ReadUINT32();
	
			// store the next scan position
			scanPosition = file.GetPosition();

			// go to where the pixel data for this line
			// is stored
			file.Seek(offset,File::Start);
			file.ReadUINT8Ptr(data);

			myScanLines.push_back(data);
			myOffsetValues.push_back(offset);

			}
		}
		file.Seek(scanPosition,File::Start);
	}

#endif		// COMPRESSEDBITMAP_H