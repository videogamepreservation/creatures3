// -------------------------------------------------------------------------
// Filename:    FileLocaliser.cpp
// Class:       FileLocaliser
// Purpose:
// To simplify the handling of localisable files.
//
// Description:
//
// Class to handle figuring out the most appropriate files for a
// specific language.
// Standard language tags are used, as defined by RFC 1766, "Tags for the
// Identification of Languages". Details can be found at -
//
// http://src.doc.ic.ac.uk/computing/internet/rfc/rfc1766.txt
//
// Quick summary...
// 
// A language tag consists of a two-letter primary tag. Zero or more subtags
// can follow, separated by hyphens.
// Subtags are used to specify country codes or dialects.
//
// Some examples:
//
// en           (plain English)
// en-US        (United States English)
// en-GB        (British English)
// en-cockney	(pretty obvious)
// fr           (plain french)
//
// In this class, all subtags are considered as a single string - the
// primary language is specified by the primary tag, and the secondary
// tags together form the secondary language (ie dialect/regional) specifier.
//
// Usage:
//
//
// History:
// 18Feb99	Initial version		BenC
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "FileLocaliser.h"

#include <algorithm>	// for find()

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>	// for dir-scanning calls
#endif // _WIN32



#ifndef _WIN32	// should be #ifdef C2E_POSIX maybe?
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fnmatch.h>
#endif







bool FileLocaliser::LocaliseDirContents( const std::string& dir,
	const std::string& langid,
	std::vector<std::string>& files,
	const std::string& wildCard /* = "*" */)
{
	std::string path=dir;
	file_list allfiles;
	file_list::iterator allfiles_it;
	std::string afile;
	std::vector<std::string>::iterator files_it;

	// make sure the path has a trailing path separator
	if( !path.empty() && path.find_last_of("/:\\") != path.size()-1 )
		path += '/';

	if( !GetFileList( path, allfiles, wildCard ) )
		return false;

	// suss out a unique list of base files.
	for( allfiles_it = allfiles.begin();
		allfiles_it != allfiles.end();
		++allfiles_it )
	{
		afile = (*allfiles_it).Path +
			(*allfiles_it).Name +
			(*allfiles_it).Ext;

		if( std::find( files.begin(), files.end(), afile ) == files.end() )
			files.push_back( afile );
	}

	// find best version for each file...
	for( files_it = files.begin();
		files_it != files.end();
		++files_it )
	{
		if( !LocaliseFilename( (*files_it), langid, wildCard ) )
			return false;
	}


	return true;
}




bool FileLocaliser::LocaliseFilename( std::string& filename,
	const std::string& langid,
	const std::string& wildCard /* = "*" */)
{
	std::string path;
	std::string name;
	std::string ext;
	std::string str;
	std::string prilang;
	std::string seclang;
	file_list files;
	file_list::iterator file_it;

	prilang = GetLangPrimary( langid );
	seclang = GetLangSecondary( langid );

	SplitPath( filename, path, name, ext );

	if( !GetFileList( path, files, wildCard ) )
		return false;

	file_it = FindBestFile( files, path, name, prilang, seclang, ext );
	if( file_it == files.end() )
		return false;	// no match found

	// rebuild the filename

	filename = (*file_it).Path + (*file_it).Name;
	if( !(*file_it).PriLang.empty() )
	{
		filename += '-';
		filename += (*file_it).PriLang;
	}
	if( !(*file_it).SecLang.empty() )
	{
		filename += '-';
		filename += (*file_it).SecLang;
	}

	filename += ext;
	return true;
}





std::string FileLocaliser::GetLangPrimary( const std::string& langid )
{
	return langid.substr( 0, langid.find_first_of( '-' ) );
}


std::string FileLocaliser::GetLangSecondary( const std::string& langid )
{
	int pos;

	pos = langid.find_first_of( '-' );
	if( pos == std::string::npos )
		return "";
	else
		return langid.substr( pos+1 );
}


void FileLocaliser::SplitPath( const std::string& fullname,
	std::string& path,
	std::string& name,
	std::string& extension )
{
	int pos;
	std::string str;

	// extract path:
	str = fullname;
	pos = str.find_last_of( "/:\\" );
	if( pos == std::string::npos )
		path = "";
	else
	{
		path = str.substr( 0, pos+1 );	// include path separator
		str = str.substr( pos+1 );
	}

	// split extension and base name
	pos = str.find_first_of( '.' );
	if( pos == std::string::npos )
	{
		extension = "";
		name = str;
	}
	else
	{
		extension = str.substr( pos );
		name = str.substr( 0, pos );
	}
}




#ifdef _WIN32
// path must include trailing slash (if non-empty)
bool FileLocaliser::GetFileList( const std::string& path, file_list& files, const std::string& wildCard /* = "*" */ )
{
	HANDLE h;
	WIN32_FIND_DATA finddata;
	DWORD err;
	std::string str;
	std::string fullfilename;
	int pos;

	str = path + wildCard;
	h = FindFirstFile( str.c_str(), &finddata );
	if( h == INVALID_HANDLE_VALUE )
		return false;
	do
	{
		if( !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			files.push_back( FileInfo() );

			fullfilename = path + finddata.cFileName;

			SplitPath(
				fullfilename,
				files.back().Path,
				str,
				files.back().Ext );

			// separate name and langid parts
			pos = str.find_first_of( '-' );
			files.back().Name = str.substr( 0, pos );
			if( pos != std::string::npos )
			{
				files.back().PriLang =
					GetLangPrimary( str.substr( pos+1 ) );
				files.back().SecLang =
					GetLangSecondary( str.substr( pos+1 ) );
			}
		}
	}
	while( FindNextFile( h, &finddata ) );
	err = GetLastError();
	FindClose( h );

	if( err != ERROR_NO_MORE_FILES )
		return false;
	else
		return true;
}
#else

// POSIX version

// path must include trailing slash (if non-empty)
bool FileLocaliser::GetFileList( const std::string& path, file_list& files, const std::string& wildCard /* = "*" */ )
{
	DIR*	dir;
	struct dirent* dir_entry;


	dir = opendir( path.c_str() );
	if( dir == NULL )
		return false;

	while( (dir_entry = readdir(dir) ) != NULL)
	{
		std::string fullfilename;
		struct stat s;

		fullfilename = path + dir_entry->d_name;

		int fnresult = fnmatch( wildCard.c_str(),fullfilename.c_str(), 0 );

		if( fnresult == FNM_NOMATCH )
		{
			// not a matching filename
			continue;
		}
		else if ( fnresult != 0 )
			return false;	// error


		if( stat( fullfilename.c_str(), &s ) != 0 )
			return false;

		if( !S_ISDIR( s.st_mode ) )
		{
			std::string str;

			files.push_back( FileInfo() );

			SplitPath(
				fullfilename,
				files.back().Path,
				str,
				files.back().Ext );

			// separate name and langid parts
			int pos = str.find_first_of( '-' );
			files.back().Name = str.substr( 0, pos );
			if( pos != std::string::npos )
			{
				files.back().PriLang =
					GetLangPrimary( str.substr( pos+1 ) );
				files.back().SecLang =
					GetLangSecondary( str.substr( pos+1 ) );
			}
		}
	}

	closedir( dir );

	return true;
}

#endif




FileLocaliser::file_list::iterator FileLocaliser::FindBestFile( file_list& files,
	const std::string path,
	const std::string name,
	const std::string prilang,
	const std::string seclang,
	const std::string ext )
{
	file_list::iterator it;

	// (deep breath)

	// try for perfect match (eg "foo/stuff-en-cockney.txt"):
	for( it = files.begin(); it != files.end(); ++it )
	{
		if( (*it).Path == path &&
			(*it).Name == name &&
			(*it).PriLang == prilang &&
			(*it).SecLang == seclang &&
			(*it).Ext == ext )
		{
			// wahay!
			return it;
		}
	}

	// try for prilang without seclang (eg "foo/stuff-en.txt"):
	for( it = files.begin(); it != files.end(); ++it )
	{
		if( (*it).Path == path &&
			(*it).Name == name &&
			(*it).PriLang == prilang &&
			(*it).SecLang.empty() &&
			(*it).Ext == ext )
		{
			return it;
		}
	}

	// This check should really be here, but had to be removed as we
	// didn't correctly name the catalogue files -en for the US 
	// master.  Oh well!

	// try for prilang with any seclang (eg "foo/stuff-en-*.txt"):
	/*for( it = files.begin(); it != files.end(); ++it )
	{
		if( (*it).Path == path &&
			(*it).Name == name &&
			(*it).PriLang == prilang &&
			(*it).Ext == ext )
		{
			return it;
		}
	}
	*/
	// try for no language specification (eg "foo/stuff.txt"):
	for( it = files.begin(); it != files.end(); ++it )
	{
		if( (*it).Path == path &&
			(*it).Name == name &&
			(*it).PriLang.empty() &&
			(*it).SecLang.empty() &&
			(*it).Ext == ext )
		{
			return it;
		}
	}

	// try for fallback language (probably english) with no
	// seclang (eg "foo/stuff-en.txt"):
	for( it = files.begin(); it != files.end(); ++it )
	{
		if( (*it).Path == path &&
			(*it).Name == name &&
			(*it).PriLang == FALLBACK_LANG_PRIMARY &&
			(*it).SecLang.empty() &&
			(*it).Ext == ext )
		{
			return it;
		}
	}

	// try for fallback language and any seclang (eg "foo/stuff-en-*.txt"):
	for( it = files.begin(); it != files.end(); ++it )
	{
		if( (*it).Path == path &&
			(*it).Name == name &&
			(*it).PriLang == FALLBACK_LANG_PRIMARY &&
			(*it).Ext == ext )
		{
			return it;
		}
	}

	// we're desparate - try for anything! (eg "foo/stuff*.txt"):
	for( it = files.begin(); it != files.end(); ++it )
	{
		if( (*it).Path == path &&
			(*it).Name == name &&
			(*it).Ext == ext )
		{
			return it;
		}
	}

	// finally admit defeat!
	return files.end();
}


