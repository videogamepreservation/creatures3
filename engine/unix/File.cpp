// --------------------------------------------------------------------------
// Filename:	File.cpp
// Class:		File
// Purpose:		
//
// Description: Posix version
//
// History:
//
// 09Aug99  BenC  Initial version
// --------------------------------------------------------------------------

#include "../File.h"	// platform independent header.
//#include "Display/ErrorMessageHandler.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


File::File(const std::string& name,
		   uint32 desiredAccessFlags/* = GENERIC_READ|GENERIC_WRITE*/,
		   uint32 shareModeFlag /*= FILE_SHARE_READ*/ )
	:myDiskFileHandle(INVALID_HANDLE_VALUE),
	myName(name)
{
	Open(name,desiredAccessFlags,shareModeFlag);
}



// Description: Opens the file only if it exists
void File::Open( const std::string& name,
	uint32 desiredAccessFlags/* = GENERIC_READ|GENERIC_WRITE*/,
	uint32 shareModeFlag /*= FILE_SHARE_READ)*/)
{

	if(myDiskFileHandle != INVALID_HANDLE_VALUE)
		Close();

	int oflags = 0;
	if( (desiredAccessFlags & GENERIC_READ) &&
		(desiredAccessFlags & GENERIC_WRITE) )
	{
		oflags |= O_RDWR;
	}
	else if( (desiredAccessFlags & GENERIC_READ) )
		oflags |= O_RDONLY;
	else if( (desiredAccessFlags & GENERIC_WRITE) )
		oflags |= O_WRONLY;

	myDiskFileHandle = open( name.c_str(), oflags, S_IREAD|S_IWRITE );
	if (myDiskFileHandle==INVALID_HANDLE_VALUE)
		throw FileException( "File::Open", __LINE__);
}



// Description: Creates and opens the file
void File::Create( const std::string& name,   
			uint32 desiredAccessFlags/* = GENERIC_READ|GENERIC_WRITE*/,
		   uint32 shareModeFlag /*= FILE_SHARE_READ)*/)
{
	if(myDiskFileHandle != INVALID_HANDLE_VALUE)
		Close();

	int oflags = O_CREAT;
	if( (desiredAccessFlags & GENERIC_READ) &&
		(desiredAccessFlags & GENERIC_WRITE) )
	{
		oflags |= O_RDWR;
	}
	else if( (desiredAccessFlags & GENERIC_READ) )
		oflags |= O_RDONLY;
	else if( (desiredAccessFlags & GENERIC_WRITE) )
		oflags |= O_WRONLY;


	myDiskFileHandle = open( name.c_str(), oflags, S_IREAD|S_IWRITE );

	if( myDiskFileHandle==INVALID_HANDLE_VALUE)
		throw FileException( "File::Open", __LINE__);
}


uint32 File::GetSize()
{
	struct stat s;
	if( stat( myName.c_str(), &s ) == -1 )
		throw FileException( "File::GetSize()", __LINE__ );
	return (uint32)s.st_size;
}


// returns -1 if failed 1 otherwise
int32 File::Seek(int32 moveBy, FilePosition from)
{
	int pos;
	if( lseek( myDiskFileHandle, moveBy, from ) == -1L )
		return -1;
	else
		return 1;
}


// returns -1 if failed 1 otherwise
int32 File::SeekToBegin()
{	
	return Seek(0,Start);
}


void File::Read(std::string& string)
{
	// First read in the length of the string
	int32 len,readsize;
	const int32 bufsize = 128;
	char buf[bufsize];

	Read(&len, sizeof(len) );
	len++;	// allow for null terminator
	string.reserve( len );
	while( len>0 )
	{
		readsize = (len >= bufsize) ? bufsize:len;
		Read( buf, readsize );
		string += buf;
		len -= readsize;
	}
}


void File::Write(std::string string)
{
	int32 length = string.size();

	Write(&length,sizeof(length));
	for(int32 i = 0; i< length;i++)
	{
	Write(&string[i],sizeof(string[i]));
	}

	// write the null terminator
	Write("\0",1);
}

bool File::FileExists(std::string& filename)
{
	struct stat s;
	if( stat( filename.c_str(), &s) == -1)
		return false;
	return true;
}

void File::Close()
{
	if (myDiskFileHandle!=INVALID_HANDLE_VALUE)
	{
		int res = close(myDiskFileHandle);
		myDiskFileHandle = INVALID_HANDLE_VALUE;
	}
	myName.empty();
}
