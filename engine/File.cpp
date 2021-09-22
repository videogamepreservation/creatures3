// --------------------------------------------------------------------------
// Filename:	File.cpp
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
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#ifndef _WIN32
#error Try some other File.cpp...
#endif

#include "File.h"
#include "Display/ErrorMessageHandler.h"



File::File(const std::string& name,
		   uint32 desiredAccessFlags/* = GENERIC_READ|GENERIC_WRITE*/,
		   uint32 shareModeFlag /*= FILE_SHARE_READ*/ )
	:myPosition(0),
	myDiskFileHandle(INVALID_HANDLE_VALUE),
	myLength(0),
	myName(name)
	{
	Open(name,desiredAccessFlags,shareModeFlag);
	}

// ----------------------------------------------------------------------
// Method:      Open 
// Arguments:   name		- filename
//				flags		- denote how the file will be used e.g read
//								only
// Returns:     None
//			
// Description: Opens the file only if it exists
// ----------------------------------------------------------------------
void File::Open(const std::string& name,   
			uint32 desiredAccessFlags/* = GENERIC_READ|GENERIC_WRITE*/,
		   uint32 shareModeFlag /*= FILE_SHARE_READ)*/)
{
	if(myDiskFileHandle != INVALID_HANDLE_VALUE)
	{
	//	std::string string = ErrorMessageHandler::Format("file_error", 1, "File::Open");
		//throw FileException(string, __LINE__);
		Close();
	}

	myDiskFileHandle=CreateFile(name.data(),
						desiredAccessFlags,
						shareModeFlag,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);

	if (myDiskFileHandle!=INVALID_HANDLE_VALUE)
	{
		myLength=GetFileSize(myDiskFileHandle,NULL);
		
		if(myLength == 0)
		{
			std::string string = ErrorMessageHandler::Format("file_error", 3, "File::Open", name.c_str());
			throw FileException(string, __LINE__);
		}
	}
	else
	{
		myLength=0;
		char buf[200];
		
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					0,
					buf,
					200,
					NULL);

		std::string string = ErrorMessageHandler::Format("file_error", 2, "File::Open", name.c_str(), buf);
		throw FileException(string, __LINE__);
	}
}


// ----------------------------------------------------------------------
// Method:      Creates 
// Arguments:   name		- filename
//				flags		- denote how the file will be used e.g read
//								only
// Returns:     None
//			
// Description: Creates and opens the file
// ----------------------------------------------------------------------
void File::Create(const std::string& name,   
			uint32 desiredAccessFlags/* = GENERIC_READ|GENERIC_WRITE*/,
		   uint32 shareModeFlag /*= FILE_SHARE_READ)*/)
{
	if(myDiskFileHandle != INVALID_HANDLE_VALUE)
	{
	//	std::string string = ErrorMessageHandler::Format("file_error", 1, "File::Create");
	//	throw FileException(string, __LINE__);
		Close();
	}

	myDiskFileHandle=CreateFile(name.data(),
						desiredAccessFlags,
						shareModeFlag,
						NULL,
						OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);


	if (myDiskFileHandle!=INVALID_HANDLE_VALUE)
	{
		myLength=GetFileSize(myDiskFileHandle,NULL);
	}
	else
	{
		myLength=0;
		char buf[200];
		
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					0,
					buf,
					200,
					NULL);
		std::string string = ErrorMessageHandler::Format("file_error", 4, "File::Create", name.c_str(), buf);
		throw FileException(string, __LINE__);
	}
}

// ----------------------------------------------------------------------
// Method:      Seek  
// Arguments:   None
//			     moveBy - number of bytes to move the file pointer
//				from - from the end begining or current position in the 
//						file.
// Returns:     -1 if move failed, 1 if move was success
// Description: 
// ----------------------------------------------------------------------
int32 File::Seek(int32 moveBy, FilePosition from)
{
	//assume failure
	int32 success = -1;
 
	switch(from)
	{
	case(Start): 
		{
			if(moveBy > myLength) 
				return success; // this failed
			else
				myPosition = moveBy;
			break;
		}

	case(Current):
		{
			if((moveBy + myPosition > myLength) || (myPosition - moveBy < 0) )
				return success;
			else
				myPosition += moveBy;
			break;
		}
	case(End):
		{
		if(abs(moveBy) > myLength)
			return success;
		else
			myPosition = myLength - moveBy;
		break;
		}
	}
	success = SetFilePointer(  myDiskFileHandle,          // handle of file
						moveBy,  // number of bytes to move file pointer
						NULL,// pointer to high-order DWORD of distance to move
						from);    // how to move);

	return success;
}

// ----------------------------------------------------------------------
// Method:      SeekToBegin  
// Arguments:   None
//
// Returns:    -1 if failed 1 otherwise
//			
// Description: Moves the file pointer to the start of the file
// ----------------------------------------------------------------------
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
	uint32 attributes = GetFileAttributes(filename.c_str());
	
	return attributes == -1 ?  false: true;

}

void File::Close()
{
	if (myDiskFileHandle!=INVALID_HANDLE_VALUE)
	{
	int res = 	CloseHandle(myDiskFileHandle);
		myDiskFileHandle = INVALID_HANDLE_VALUE;
	}
	myPosition = 0;
	myName.empty();
}
