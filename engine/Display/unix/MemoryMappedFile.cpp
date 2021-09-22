// --------------------------------------------------------------------------
// Filename:	MemoryMappedFile.cpp
// Class:		MemoryMappedFile
// Purpose:		Allows client to map to a file on disk or map to part
//				of an already existing memory mapped file.
//
// Description:
//				note myFileData can be moved to the relevant part of the 
//				memory mapped view e.g. in a composite file like creature
//				gallery I like to skip the offset and moniker key.
//				However myConstantPtrToViewOfFile will _always_ point to
//				the start of the file mapping.  Always use this pointer to
//				unmap the file.
//
// History:
// --------------------------------------------------------------------------


#include "MemoryMappedFile.h"
//#include "ErrorMessageHandler.h"


#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


MemoryMappedFile::MemoryMappedFile() :
	myFile(0),myLength(0),myBasePtr(NULL),myPosition(0)
{
}

MemoryMappedFile::MemoryMappedFile( const std::string& filename,
	uint32 desiredAccessFlags, uint32 sharemodeFlags ) :
	myFile(0),myLength(0),myBasePtr(NULL),myPosition(0)
{
	Open( filename, desiredAccessFlags, sharemodeFlags );
}



void MemoryMappedFile::Open( const std::string& filename,
	uint32 desiredAccessFlags,	// =GENERIC_READ|GENERIC_WRITE
	uint32 sharemodeFlags,		// =FILE_SHARE_READ|FILE_SHARE_WRITE
	uint32 fileSize )			// =0
{
	// open the file

	ASSERT( myFile == 0 );		// don't allow reopening.

	int oflags = 0;

	if( ( desiredAccessFlags & GENERIC_READ) &&
		( desiredAccessFlags & GENERIC_WRITE ) )
	{
		oflags |= O_RDWR;
	}
	else if( desiredAccessFlags & GENERIC_READ )
        oflags |= O_RDONLY;
    else if( desiredAccessFlags & GENERIC_WRITE )
        oflags |= O_WRONLY;

	myFile = open( filename.c_str(), oflags, S_IREAD|S_IWRITE );
	if( !myFile )
	{
		throw MemoryMappedFileException(
			"MemoryMappedFile::Open() - open failed", __LINE__ );
	}

	if( fileSize == 0 )
	{
		// get size of entire file
		struct stat inf;
		if( fstat( myFile, &inf ) != 0 )
		{
			close( myFile );
			throw MemoryMappedFileException(
				"MemoryMappedFile::Open() - fstat failed", __LINE__ );
		}
		myLength = (uint32)inf.st_size;
	}
	else
		myLength = fileSize;

	// map it into memory

	int flags = MAP_SHARED;	// or MAP_PRIVATE?
	int prot = 0; 
	if( desiredAccessFlags & GENERIC_WRITE )
		prot |= PROT_WRITE;
	if( desiredAccessFlags & GENERIC_READ )
		prot |= PROT_READ;

	myBasePtr = (byte*)mmap( 0, fileSize, prot, flags, myFile, 0 );

	if( !myBasePtr )
	{
		close( myFile );
		throw MemoryMappedFileException(
			"MemoryMappedFile::Open() - mmap failed", __LINE__ );
	}
}





MemoryMappedFile::~MemoryMappedFile()
{
	Close();
}



void MemoryMappedFile::Close()
{
	if( myBasePtr && myLength > 0 );
	{
		munmap( myBasePtr, myLength );
		myBasePtr = NULL;
		myLength = 0;
	}

	if( myFile )
	{
		close( myFile );
		myFile = 0;
	}

}



void MemoryMappedFile::Seek(int32 moveBy, File::FilePosition from)
{

	uint32 newpos;
	switch(from)
	{
	case(File::Start): 
		{
			newpos = moveBy;
			break;
		}

	case(File::Current):
		{
			newpos += moveBy;
			break;
		}
	case(File::End):
		{
			newpos = myLength - moveBy;
			break;
		}
	}

	// allowed to point to last+1
	if( newpos > myLength )
	{
		throw MemoryMappedFileException(
			"MemoryMappedFile::Seek() - out of range", __LINE__ );
	}
		
	myPosition = newpos;
}


