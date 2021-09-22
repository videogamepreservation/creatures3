#ifndef _WIN32
#include "unix/MemoryMappedFile.h"
#else
// rest of file is win32




#ifndef		MEMORY_MAPPED_FILE_H
#define		MEMORY_MAPPED_FILE_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include	"../File.h"



class MemoryMappedFile 
{
public:
	// constructors
	MemoryMappedFile();

	MemoryMappedFile(std::string filename,
					uint32 desiredAccessflags =GENERIC_READ|GENERIC_WRITE,
					uint32 sharemodeFlags = FILE_SHARE_READ|FILE_SHARE_WRITE);
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
	MemoryMappedFile(HANDLE memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap,
				  uint32 accessRights = FILE_MAP_WRITE);

	virtual ~MemoryMappedFile();

	void Open(std::string& name,
		uint32 desiredAccessflags =GENERIC_READ|GENERIC_WRITE,
		uint32 sharemodeFlags = FILE_SHARE_READ|FILE_SHARE_WRITE,
		uint32 fileSize = 0);
	void Close();

	inline void ReadUINT8Ptr(uint8*& dataBufferPtr);
	inline void ReadUINT32Ptr(uint32*& dataBufferPtr);
	inline void ReadUINT16Ptr(uint16*& dataBufferPtr);



	inline uint32* GetUINT32Ptr();

	inline uint8 ReadUINT8();
	inline uint16 ReadUINT16();
	inline uint32 ReadUINT32();

	inline void WriteUINT8(uint8 value);
	inline void WriteUINT16(uint16 value);
	inline void WriteUINT32(uint32 value);

	inline void Blast(uint32 countInBytes, uint8* source);
	inline void BlockCopy(uint32 countInBytes,uint8* dest);

//	void Seek(uint32 numBytes,){myPosition += numBytes;}

	int32 Seek(int32 moveBy, File::FilePosition from);

	unsigned char* GetFileStart(){return myFileData;}
	HANDLE GetFileMapping(){return myMemoryFile;}

	uint32 GetPosition(){return myPosition;}
	void Reset(){myPosition = 0;}

	bool Valid(){return (myMemoryFile !=NULL);}

//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
class MemoryMappedFileException: public BasicException
{
public:
	MemoryMappedFileException(std::string what, uint16 line):
	  BasicException(what.c_str()),
	lineNumber(line){;}

	uint16 LineNumber(){return lineNumber;}
private:

	uint16 lineNumber;

};
	
protected:

	// keep track of the file you use
	// so you can tidy up when it's all over
	HANDLE	myMemoryFile;
//	HANDLE	myDiskFile;

	// the start of the data in my memory mapped file
	unsigned char* myFileData;
	unsigned char* myConstantPtrToViewOfFile;
private:
	// Copy constructor and assignment operator
	// Declared but not implemented
	MemoryMappedFile (const MemoryMappedFile&);
	MemoryMappedFile& operator= (const MemoryMappedFile&);

	// underlying file
	File myFile;
	uint32	myLength;
	uint32	myPosition;


};


// inline stuff

// get a pointer to some part of the file without advancing
// the file pointer
uint32* MemoryMappedFile::GetUINT32Ptr()
{
	_ASSERT(myPosition <= myLength);
	_ASSERT(myFileData);

	uint32* dataBufferPtr = (uint32*)(myFileData + myPosition);
	_ASSERT(dataBufferPtr);
	return dataBufferPtr;
}

void MemoryMappedFile::ReadUINT16Ptr(uint16*& dataBufferPtr)
{
	_ASSERT(myPosition < myLength);
	_ASSERT(myFileData);

	dataBufferPtr = (uint16*)(myFileData + myPosition);
	_ASSERT(dataBufferPtr);
	myPosition += sizeof(uint16);
}


void MemoryMappedFile::ReadUINT32Ptr(uint32*& dataBufferPtr)
{
	_ASSERT(myPosition <= myLength);
	_ASSERT(myFileData);

	dataBufferPtr = (uint32*)(myFileData + myPosition);
	_ASSERT(dataBufferPtr);
	myPosition += sizeof(uint32);
}


void MemoryMappedFile::ReadUINT8Ptr(uint8*& dataBufferPtr)
{
	_ASSERT(myPosition <= myLength);
	_ASSERT(myFileData);

	dataBufferPtr = (uint8*)(myFileData + myPosition);
	_ASSERT(dataBufferPtr);
	myPosition += sizeof(uint8);
}


uint8 MemoryMappedFile::ReadUINT8()
{
	_ASSERT(myPosition <= myLength);
	_ASSERT(myFileData);
	uint8 value = *(myFileData + myPosition);
	myPosition+= sizeof(uint8);
	return value;
}
	
uint16 MemoryMappedFile::ReadUINT16()
{
	_ASSERT(myPosition <= myLength);
	_ASSERT(myFileData);

	uint16 value =  GetUINT16At(myFileData + myPosition);
	myPosition += sizeof(uint16);
	return value;
}
	
uint32 MemoryMappedFile::ReadUINT32()
{
	_ASSERT(myPosition <= myLength);
	_ASSERT(myFileData);

	uint32 value =  GetUINT32At(myFileData + myPosition);
	myPosition += sizeof(uint32);
	return value;
}


void MemoryMappedFile::WriteUINT8(uint8 value)
{
	_ASSERT(myPosition <= myLength);
		_ASSERT(myFileData);

	uint8* data = myFileData + myPosition;
	*data = value;
	myPosition+= sizeof(uint8);
}
	
void MemoryMappedFile::WriteUINT16(uint16 value)
{
	_ASSERT(myPosition <= myLength);
	uint16* data = (uint16*)(myFileData + myPosition);
	*data = value;
	myPosition+= sizeof(uint16);
}

void MemoryMappedFile::WriteUINT32(uint32 value)
{
	_ASSERT(myPosition <= myLength);
	uint32* data = (uint32*)(myFileData + myPosition);
	*data = value;
	myPosition+= sizeof(uint32);
}

void MemoryMappedFile::Blast(uint32 countInBytes, uint8* source)
{
	_ASSERT(myPosition <= myLength);
	uint8* data = (uint8*)(myFileData + myPosition);
	memcpy(data,source,countInBytes);
	myPosition+= countInBytes;
}

void MemoryMappedFile::BlockCopy(uint32 countInBytes,uint8* dest)
{
	_ASSERT(myPosition <= myLength);
	uint8* data = (uint8*)(myFileData + myPosition);
	memcpy(dest,data,countInBytes);
	dest+=countInBytes;
	myPosition+= countInBytes;
}

#endif //MEMORY_MAPPED_FILE
#endif // _WIN32

