#ifndef		FILE_PATH_H
#define		FILE_PATH_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
#include "General.h"
#include "../common/BasicException.h"

class CreaturesArchive;

class FilePath
{
public:
	FilePath() : myDirectory( 0 ),myUseLocalWorldDirectoryVersionOfTheFile(0) {}
	FilePath( std::string name, int directory, bool checkLocallyFirst = true, bool forceLocal = false );
	FilePath( DWORD fsp, char const *ext, int directory );

	void SetFilePath( std::string name, int directory );

	bool GetWorldDirectoryVersionOfTheFile(std::string& path, bool forcecreate = false) const;

	std::string GetFullPath() const;

	std::string GetFileName() const;

	bool empty() const {return myName.empty();}
	void SetExtension( std::string extension );

	FilePath operator+( std::string const &append ) const
	{
		return FilePath( myName + append, myDirectory );
	}
	//////////////////////////////////////////////////////////////////////////
	// Exceptions
	//////////////////////////////////////////////////////////////////////////
	class FilePathException: public BasicException
	{
	public:
		FilePathException(std::string what, uint16 line):
		BasicException(what.c_str()),
		lineNumber(line){;}

		uint16 LineNumber(){return lineNumber;}
	private:

		uint16 lineNumber;

	};


	friend CreaturesArchive &operator<<( CreaturesArchive &archive, FilePath const &filePath );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, FilePath &filePath );
	friend bool operator==( FilePath const &l, FilePath const &r );
	friend bool operator!=( FilePath const &l, FilePath const &r );
	friend bool operator<( FilePath const &l, FilePath const &r );
private:
	int myDirectory;
	std::string myName;
	bool myUseLocalWorldDirectoryVersionOfTheFile;
};

int CaseInsensitiveCompare( const char *l, const char *r );

#endif