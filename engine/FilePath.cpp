#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "FilePath.h"
#include "App.h"
#include "CreaturesArchive.h"
#include "Display/ErrorMessageHandler.h"
#include <algorithm>

#ifndef _WIN32
#include "unix/FileFuncs.h"
#endif

FilePath::FilePath( std::string name, int directory , bool checkLocallyFirst/*= true*/, bool forceLocal /*= false*/)
	: myName( name ),
	  myDirectory( directory ),
	  myUseLocalWorldDirectoryVersionOfTheFile(0)
{

	// For all requests we must search for the files in
	// both in the local world  directory version of the folder and the 
	// main directory versions that was installed (these files are global to all
	// worlds)
	// If the file is definitely found in the local directory then it will flagged
	// for use instead of any main directory versions.
	if(checkLocallyFirst)
	{
		std::string path;
		if(forceLocal || GetWorldDirectoryVersionOfTheFile(path))
		{
			myUseLocalWorldDirectoryVersionOfTheFile = true;
		}
	}
}

FilePath::FilePath( DWORD fsp, char const *ext, int directory )
	: myDirectory( directory ),
	myUseLocalWorldDirectoryVersionOfTheFile(0)
{
	myName = BuildFsp( fsp, ext );
}

std::string FilePath::GetFullPath() const
{
	if( myDirectory < 0 )
		return myName;

	std::string localpath;
	if(myUseLocalWorldDirectoryVersionOfTheFile /* && 
		GetWorldDirectoryVersionOfTheFile(localpath) this has already been checked*/ )
	{
		GetWorldDirectoryVersionOfTheFile(localpath);
		return localpath; 
	}
	else
	{
		return theApp.GetDirectory( myDirectory ) + myName;
	}
}

std::string FilePath::GetFileName() const
{
	return myName;
}

CreaturesArchive &operator<<( CreaturesArchive &archive, FilePath const &filePath )
{
	archive << filePath.myDirectory << filePath.myName 
		<< filePath.myUseLocalWorldDirectoryVersionOfTheFile;
	return archive;
}

CreaturesArchive &operator>>( CreaturesArchive &archive, FilePath &filePath )
{
	archive >> filePath.myDirectory >> filePath.myName 
		>> filePath.myUseLocalWorldDirectoryVersionOfTheFile;

	// just do a quickie check to check that the file is still
	//  (or is now suddenly) available locally
	std::string path;
	if(filePath.GetWorldDirectoryVersionOfTheFile(path))
	{
		filePath.myUseLocalWorldDirectoryVersionOfTheFile = true;
	}
	else
	{
		filePath.myUseLocalWorldDirectoryVersionOfTheFile = false;
	}
	return archive;
}

// This should be generally usefull!
// Indeed! (put in header)
int CaseInsensitiveCompare( const char *l, const char *r )
{
	if( l == r ) return 0;
	if( !l ) return -1;
	if( !r ) return 1;
	while( *l )
	{
		int diff = tolower( *l ) - tolower( *r );
		if( diff < 0 ) return -1;
		if( diff > 0 ) return 1;
		++l, ++r;
	}
	if (*r != 0)
		return -1;
	return 0;
}

bool operator==( FilePath const &l, FilePath const &r )
{
	return l.myDirectory == r.myDirectory  &&
		(CaseInsensitiveCompare( l.myName.c_str(), r.myName.c_str() ) == 0 )
		&&l.myUseLocalWorldDirectoryVersionOfTheFile ==
		r.myUseLocalWorldDirectoryVersionOfTheFile; 
}

bool operator!=( FilePath const &l, FilePath const &r )
{
	return !(l == r);
}

bool operator<( FilePath const &l, FilePath const &r )
{
	if( l.myDirectory < r.myDirectory ) return true;
	return l.myName < r.myName;
}

void FilePath::SetExtension( std::string extension )
{
	int x = myName.find_last_of(".");
	if( x == -1 )
		myName += "." + extension;
	else
		myName = myName.substr( 0, x + 1 ) + extension;
}


void FilePath::SetFilePath( std::string name, int directory )
{
	myName = name;
	myDirectory = directory;
}


bool FilePath::GetWorldDirectoryVersionOfTheFile(std::string& path, bool forcecreate) const
{

	if(theApp.GetWorldDirectoryVersion(myDirectory,path,forcecreate))
	{
	
		std::string tempPath = path;
		path+= myName;

#ifdef _WIN32
		if(GetFileAttributes(path.data()) != -1)
			return true;
#else
		ASSERT( false );	//TODO: non-win32 version
#endif

	
		// now check for C16 version as the call would have
		// sent S16 by default
		if(myDirectory == IMAGES_DIR)
		{
			std::string tempName;

			// get the name without the extension in case we have to alter it
			int x = myName.find_last_of(".");
				if( x != -1 )
			{
				tempName = myName.substr( 0, x + 1 );
			}

			tempName+= "C16";
			tempPath+= tempName;
#ifdef _WIN32
			if(GetFileAttributes(tempPath.data()) != -1)
#else
			if(FileExists(tempPath.data()))
#endif	
			{
				path = tempPath;
				return true;
			}
			
		}
	}
	return false;
}

