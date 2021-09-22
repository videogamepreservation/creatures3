// --------------------------------------------------------------------------
// Filename:	File.h
// Class:		File
// Purpose:		A general class for reading files
//				
//
// Description: Add functions as required.
//			
//				
//
// History:
// -------  Chris Wylie		created
// 11Nov98	Alima			Added Seek.
// 09Aug99  BenC            Portability work.
// --------------------------------------------------------------------------

// TODO
// - better error handling on win32 Read/Write? throw exception?
// - are string Read/Write fns used?


#ifndef		FILE_H
#define		FILE_H

#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER


//#include	"../engine/Display/System.h"
#include "../common/C2eTypes.h"
#include "../common/BasicException.h"

#include	<string>


#ifndef _WIN32
//#include <sys/io.h>
#include <unistd.h>
#define INVALID_HANDLE_VALUE -1
// access flags:
#define GENERIC_READ 1
#define GENERIC_WRITE 2
// sharemode:
#define FILE_SHARE_READ 1
#define FILE_SHARE_WRITE 2
#endif





class File
{
public:
#ifdef _WIN32
	typedef HANDLE oshandle;
	enum FilePosition
	{
		Start = FILE_BEGIN,
		Current = FILE_CURRENT,
		End = FILE_END,
	};
#else
	typedef int oshandle;
	enum FilePosition
	{
		Start=SEEK_SET,
		Current=SEEK_CUR,
		End=SEEK_END
	};
#endif

	File() : myDiskFileHandle(INVALID_HANDLE_VALUE)
	{
#ifdef _WIN32
		myPosition = 0;
		myLength = 0;
#endif
	};

	File(const std::string& name, 
		uint32 desiredAccessFlags = GENERIC_READ|GENERIC_WRITE,
		uint32 shareModeFlags = FILE_SHARE_READ);

	virtual void Open( const std::string& name, 
			uint32 desiredAccessFlags = GENERIC_READ|GENERIC_WRITE,
			uint32 shareModeFlags = FILE_SHARE_READ);

	virtual void Create( const std::string& name,   
			uint32 desiredAccessFlags = GENERIC_READ|GENERIC_WRITE,
		   uint32 shareModeFlag = FILE_SHARE_READ);

	virtual ~File()
	{
		Close();
	}

#ifdef _WIN32
	uint32 GetSize(void)
	{
		return myLength;
	}
#else
	uint32 GetSize(void);
#endif

	const char* GetName()
		{return myName.c_str();}

	// ----------------------------------------------------------------------
	// Method:      Read 
	// Arguments:   buffer		- buffer to receive file data
	//				size		- size of buffer
	// Returns:     the number of bytes read 
	//			
	// Description: 
	// ----------------------------------------------------------------------
	uint32 Read(void* buffer,uint32 size);

	void Read(std::string& string);

	// ----------------------------------------------------------------------
	// Method:      Write 
	// Arguments:   buffer		- buffer holding file data
	//				size		- size of buffer
	// Returns:     the number of bytes written 
	//			
	// Description: This method needs to be written!!!
	// ----------------------------------------------------------------------
	uint32 Write(const void* buffer,uint32 size);

	void Write(std::string string);

	// ----------------------------------------------------------------------
	// Method:      Seek  
	// Arguments:   None
	// Returns:     moveBy - number of bytes to move the file pointer
	//				from - from the end begining or current position in the 
	//						file.
	// Description: 
	// ----------------------------------------------------------------------
	int32 Seek(int32 moveBy, FilePosition from);
	// ----------------------------------------------------------------------
	// Method:      SeekToBegin  
	// Arguments:   None
	// Returns:    -1 if failed 1 otherwise
	//			
	// Description: Moves the file pointer to the start of the file
	// ----------------------------------------------------------------------
	int32 SeekToBegin();

	// ----------------------------------------------------------------------
	// Method:      Close  
	// Arguments:   None
	// Returns:     None
	//			
	// Description: Closes the file
	// ----------------------------------------------------------------------
	void Close();

	bool Valid()
		{return (myDiskFileHandle !=INVALID_HANDLE_VALUE);}

	oshandle GetHandle()
		{return myDiskFileHandle;}

	static bool FileExists(std::string& filename);

	// Exceptions
	class FileException: public BasicException
	{
	public:
		FileException(std::string what, uint16 line):
		BasicException(what.c_str()),
		lineNumber(line){;}

		uint16 LineNumber()
			{return lineNumber;}
	private:

		uint16 lineNumber;
	};

protected:

	// Copy constructor and assignment operator
	// Declared but not implemented
	File (const File&);
	File& operator= (const File&);

#ifdef _WIN32
	uint32	myPosition;
	uint32	myLength;
#endif
	oshandle	myDiskFileHandle;
	std::string myName;

};



//------------------
// Inlines
//------------------



#ifdef _WIN32
inline uint32 File::Read(void* buffer,uint32 size)
{
	_ASSERT(myPosition + size <= myLength);
	uint32	bytes_read;

	if (myPosition+size > myLength)
	{
		size=myLength-myPosition;
		if (size==0) return 0;
	}

	_ASSERT(myDiskFileHandle);
	ReadFile(myDiskFileHandle,buffer,size,&bytes_read,NULL);
	myPosition+=bytes_read;
	return bytes_read;
}

inline uint32 File::Write(const void* buffer,uint32 size)
{
	uint32	bytesWritten;

	_ASSERT(myDiskFileHandle);
	WriteFile(myDiskFileHandle,buffer,size,&bytesWritten,NULL);
		
	myPosition+=bytesWritten;
	return bytesWritten;
}
#endif // _WIN32





#ifndef _WIN32
// posix version

inline uint32 File::Read(void* buffer,uint32 size)
{
	ASSERT( myDiskFileHandle != INVALID_HANDLE_VALUE );
	int count = read( myDiskFileHandle, buffer, size );
	if( count == -1 )
		throw FileException( "File::Read() failed", __LINE__ );
	return (uint32)count;
}

inline uint32 File::Write(const void* buffer,uint32 size)
{
	ASSERT( myDiskFileHandle != INVALID_HANDLE_VALUE );
	int count = write( myDiskFileHandle, buffer, size );
	if( count == -1 )
		throw FileException( "File::Write() failed", __LINE__ );

	return (uint32)count;
}
#endif


#endif		// FILE_H

