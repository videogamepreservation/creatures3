// --------------------------------------------------------------------------
// Filename:	MemoryMappedFile.cpp
// Class:		MemoryMappedFile
// Purpose:		Allows client to map to a file on disk or map to part
//				of an already existing memory mapped file.
//			
//				
//
// Description: The flags are file share so that different views of
//				the same memory mapped file can be written to.
//				Note that if the memory mapped file is a view of another
//				memory mapped file then there is no need to know about the
//				original file on disk
//			
//				note myFileData can be moved to the relevant part of the 
//				memory mapped view e.g. in a composite file like creature
//				gallery I like to skip the offset and moniker key.
//				However myConstantPtrToViewOfFile will _always_ point to
//				the start of the file mapping.  Always use this pointer to
//				unmap the file.
//
// History:
// -------  
// 11Nov98	Alima			Created
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MemoryMappedFile.h"
#include "ErrorMessageHandler.h"
#include "../resource.h"

MemoryMappedFile::MemoryMappedFile()
:myMemoryFile(0),
myLength(0),
myPosition(0),
myFileData(0),
myConstantPtrToViewOfFile(0)
{
}

MemoryMappedFile::MemoryMappedFile(std::string filename,
				uint32 desiredAccessflags/* =GENERIC_READ|GENERIC_WRITE*/,
				uint32 sharemodeFlags /*= FILE_SHARE_READ|FILE_SHARE_WRITE*/)
:myFile(filename.data(),desiredAccessflags,sharemodeFlags),
myMemoryFile(0),
myLength(0),
myPosition(0),
myFileData(0),
myConstantPtrToViewOfFile(0)
{
Open(filename,desiredAccessflags,sharemodeFlags);
}

void MemoryMappedFile::Open(std::string& filename,
							uint32 desiredAccessflags, /* =GENERIC_READ|GENERIC_WRITE*/
							uint32 shareModeflags/* = FILE_SHARE_READ|FILE_SHARE_WRITE*/,
							uint32 fileSize/*=0*/)
{
	// in case of reload
	Close();
	// check that the file was opened
	if(	!myFile.Valid())
		myFile.Create(filename,desiredAccessflags,shareModeflags);
	
	// memory map the whole file or the amount we were given
	// throw an exception at any stage if we fail
	if (myFile.Valid())
	{
		myLength = fileSize;

		if(myLength == 0)
			myLength =  myFile.GetSize();

		// if the client wants to be able to write to the file
		// he better have requested a writable file then or else
		// it will 
		uint32 access = 0;
		if(desiredAccessflags & GENERIC_WRITE)
		{
		access = PAGE_READWRITE;
		}
		else
		{
			access = PAGE_READONLY;
		}

		myMemoryFile =	CreateFileMapping (myFile.GetHandle(),
							   NULL,
							   access,
							   0,        
							  myLength,
							   NULL);

		if(myMemoryFile == NULL)
		{
				
			std::string string = ErrorMessageHandler::Format("file_error", 0,
				"MemoryMappedFile::Open", filename.c_str());
		
			throw MemoryMappedFileException(string,__LINE__);
		}


		if(desiredAccessflags & GENERIC_WRITE)
		{
			access = FILE_MAP_WRITE;
		}
		else
		{
			access = FILE_MAP_READ;
		}

		myFileData =  (unsigned char*)MapViewOfFile(myMemoryFile,  // file-mapping object  		
						access,      // access mode
						 0,     // high-order 32 bits of file offset
						0,      // low-order 32 bits of file offset
						0);  // number of bytes to map - zero means map all

		if(myFileData == NULL)
		{
			char msg[_MAX_PATH];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						0,
						msg,
						200,
						NULL);
			std::string string = ErrorMessageHandler::Format("file_error", 5,
				"MemoryMappedFile::Open", msg);

			throw MemoryMappedFileException(string,__LINE__);
		}

		myConstantPtrToViewOfFile = myFileData;

	}
	else
	{	
		std::string string = ErrorMessageHandler::Format("file_error", 0,
			"MemoryMappedFile::Open", filename.c_str());
	
		throw MemoryMappedFileException(string,__LINE__);
	}	
}


// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   memoryFile - handle to a memory file to map to
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
MemoryMappedFile::MemoryMappedFile(HANDLE memoryFile, 
				 uint32 highOffset,
				 uint32 lowOffset,
				 uint32 numBytesToMap,
				 uint32 accessRights /*FILE_MAP_WRITE*/)
				 :myMemoryFile(0), // you don't have your own memory file
myLength(numBytesToMap),
myPosition(0)
{
//	_ASSERT(Valid());
	//we have to map according to the systems granularity
	SYSTEM_INFO systemInfo;

	GetSystemInfo(&systemInfo);

	// set the first step
	uint32 granularity = systemInfo.dwAllocationGranularity;

	uint32 multiplyBy = 1;
	// if it is a multiple already then fine
	// else find the next biggest multiple that will include
	// our position in the file
	// for example if low offset is 1300 and the granularity is 500
	// then we are looking for the boundary starting at 1000
	if(lowOffset % systemInfo.dwAllocationGranularity)
		{
		while(granularity < lowOffset) 
			{
			multiplyBy++;
			granularity = 
			multiplyBy * systemInfo.dwAllocationGranularity;
			}
		}

	// ours is the last boundary
	multiplyBy--;

	_ASSERT((systemInfo.dwAllocationGranularity * multiplyBy) 
		<= lowOffset );

	myFileData =  
		(unsigned char*)MapViewOfFile(memoryFile,  // file-mapping object  	
			// access mode
			accessRights,      
			// high-order 32 bits of file offset
			highOffset,     
			// low-order 32 bits of file offset
			systemInfo.dwAllocationGranularity * multiplyBy, 
			// number of bytes to map - zero means map all
			numBytesToMap);  

	_ASSERT(myFileData);


	if(myFileData == NULL)
	{		
		char msg[_MAX_PATH];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				0,
				msg,
				200,
				NULL);

		std::string string = ErrorMessageHandler::Format("file_error", 5,
			"MemoryMappedFile::Open", msg);

		throw MemoryMappedFileException(string,__LINE__);
	}

	myConstantPtrToViewOfFile = myFileData;
		// now move the data pointer to the exact start of the file
	// which is the low offset (taking account of our granularity
	// boundary) which in our example would be 1300%500 = 300

	myFileData += lowOffset %systemInfo.dwAllocationGranularity;
	myLength -= (lowOffset %systemInfo.dwAllocationGranularity )- myPosition;
	myPosition = 0;

}

MemoryMappedFile::~MemoryMappedFile()
{
	Close();
}

void MemoryMappedFile::Close()
{
	
	if(myConstantPtrToViewOfFile)
	{
		if(!UnmapViewOfFile(myConstantPtrToViewOfFile))
		{
			ASSERT(false);
		}

		myConstantPtrToViewOfFile = NULL;
		myFileData = NULL;
	}

	if(myMemoryFile)
	{
		CloseHandle(myMemoryFile);
		myMemoryFile = NULL;
	}

	myPosition =0;
	myFile.Close();
}



int32 MemoryMappedFile::Seek(int32 moveBy, File::FilePosition from)
{
	//assume failure
	int32 success = -1;
 
	switch(from)
	{
	case(File::Start): 
		{
			if(moveBy > myLength) return success;

			myPosition = moveBy;
			break;
		}

	case(File::Current):
		{
			if((moveBy + myPosition > myLength) || (myPosition - moveBy < 0) )
				return success;

			myPosition += moveBy;
			break;
		}
	case(File::End):
		{
			if(abs(moveBy) > myLength)
				return success;

			myPosition = myLength - moveBy;
			break;
		}
	}

	return success;
}