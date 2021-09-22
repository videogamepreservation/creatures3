#ifndef		UNIX_MEMORY_MAPPED_FILE_H
#define		UNIX_MEMORY_MAPPED_FILE_H

#include <string.h>			// for memcpy()
#include "../../File.h"		// for flag defs
#include "../../../common/C2eTypes.h"


class MemoryMappedFile 
{
public:
	// constructors
	MemoryMappedFile();

	MemoryMappedFile(const std::string& filename,
					uint32 desiredAccessFlags =GENERIC_READ|GENERIC_WRITE,
					uint32 sharemodeFlags = FILE_SHARE_READ|FILE_SHARE_WRITE);

	virtual ~MemoryMappedFile();

	void Open(const std::string& name,
		uint32 desiredAccessFlags =GENERIC_READ|GENERIC_WRITE,
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

	void Seek( int32 moveBy, File::FilePosition from );

	unsigned char* GetFileStart();
//	HANDLE GetFileMapping(){return myMemoryFile;}

	uint32 GetPosition();
	void Reset();

	bool Valid();

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
	
	// Copy constructor and assignment operator
	// Declared but not implemented
	MemoryMappedFile (const MemoryMappedFile&);
	MemoryMappedFile& operator= (const MemoryMappedFile&);


private:
	int myFile;
	uint8*  myBasePtr;
	uint32 myLength;
	uint32 myPosition;
};


inline uint32* MemoryMappedFile::GetUINT32Ptr()
{
	return (uint32*)(myBasePtr + myPosition);
}

inline void MemoryMappedFile::ReadUINT8Ptr(uint8*& dataBufferPtr)
{
	dataBufferPtr = (uint8*)(myBasePtr + myPosition);
	myPosition += sizeof(uint8);
}


inline void MemoryMappedFile::ReadUINT32Ptr(uint32*& dataBufferPtr)
{
	dataBufferPtr = (uint32*)(myBasePtr + myPosition);
	myPosition += sizeof( uint32 );
}

inline void MemoryMappedFile::ReadUINT16Ptr(uint16*& dataBufferPtr)
{
	dataBufferPtr = (uint16*)(myBasePtr + myPosition);
	myPosition += sizeof( uint16 );
}


inline uint8 MemoryMappedFile::ReadUINT8()
{
	uint8* p = (uint8*)(myBasePtr + myPosition);
	myPosition += sizeof( uint8 );
	return *p;
}

inline uint16 MemoryMappedFile::ReadUINT16()
{
	uint16* p = (uint16*)(myBasePtr + myPosition);
	myPosition += sizeof( uint16 );
	return *p;
}


inline uint32 MemoryMappedFile::ReadUINT32()
{
	uint32* p = (uint32*)(myBasePtr + myPosition);
	myPosition += sizeof( uint32 );
	return *p;
}


inline void MemoryMappedFile::WriteUINT8(uint8 value)
{
	uint8* p = (uint8*)(myBasePtr+myPosition);
	*p = value;
	myPosition += sizeof( uint8 );
}

inline void MemoryMappedFile::WriteUINT16(uint16 value)
{
	uint16* p = (uint16*)(myBasePtr+myPosition);
	*p = value;
	myPosition += sizeof( uint16 );
}

inline void MemoryMappedFile::WriteUINT32(uint32 value)
{
	uint32* p = (uint32*)(myBasePtr+myPosition);
	*p = value;
	myPosition += sizeof( uint32 );
}


inline void MemoryMappedFile::Blast( uint32 countInBytes, uint8* source )
{
	memcpy( (uint8*)(myBasePtr + myPosition), source, countInBytes );
	myPosition += countInBytes;
}

inline void MemoryMappedFile::BlockCopy( uint32 countInBytes, uint8* dest )
{
	memcpy( dest, (uint8*)(myBasePtr + myPosition), countInBytes );
	myPosition += countInBytes;
}



#endif //UNIX_MEMORY_MAPPED_FILE

