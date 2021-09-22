// -------------------------------------------------------------------------
// Filename:    FileLocaliser.h
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


#ifndef FILELOCALISER_H
#define FILELOCALISER_H


#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER
#include <string>
#include <vector>


// fallback language to try for if requested language or langauge-neutral
// file not available (language codes are uppercase by tradition)
#define FALLBACK_LANG_PRIMARY "EN"




class FileLocaliser
{
public:
	// ----------------------------------------------------------------------
	// Method:		LocaliseFilename
	// Arguments:	filename - base name of file (can include path)
	//				langid - identifier for preferred language (and dialect)
	// Returns:		true if a localised file is available, else false.
	// Description:	
	// Given a particular base name, this function looks for the most
	// appropriate existing file. The base filename passed in does not need
	// to actually exist for this function to work.
	// For example, passing in "stuff.txt" and a langid of "en-gb", the
	// following files would all match (in order of preference):
	// "stuff-en-gb.txt"	(perfect match)
	// "stuff-en.txt"		(language matched, dialect neutral)
	// "stuff-en-*.txt"		(any dialect)
	// "stuff.txt"			(language-neutral)
	// "stuff-en*.txt"		(hardwired fallback to English)
	// "stuff-*.txt"		(desparate - any language will do!)
	// ----------------------------------------------------------------------
	bool LocaliseFilename( std::string& filename,
		const std::string& langid,
		const std::string& wildCard = "*");

	// ----------------------------------------------------------------------
	// Method:		LocaliseDirContents
	// Arguments:	dir - name of directory of interest
	//				langid - identifier for preferred language (and dialect)
	//				files - vector to receive processed filenames
	// Returns:		true for success, false upon error
	// Description:	
	// Scans a directory and tries to provide a localised version for each
	// unique base file name present. No recursion into subdirectories.
	// Each filename returned includes the path to the originally specified
	// directory.
	// ----------------------------------------------------------------------
	bool LocaliseDirContents( const std::string& dir,
		const std::string& langid,
		std::vector<std::string>& files, 
		const std::string& wildCard = "*");
	
	// ----------------------------------------------------------------------
	// Method:		SplitPath
	// Arguments:	fullname - full path and name of file
	//				path - string to receive path part
	//				name - string to receive name part
	//				extension - string to receive file extension part
	// Returns:		None
	// Description:	
	// Utility function to split a filename into its constituent components.
	// Examples:
	// "C:\wibble\pibble.txt" --> "C:\wibble\" "pibble" ".txt"
	// "/usr/foo/bar.tar.gz" --> "/usr/foo/" "bar" ".tar.gz"
	// ----------------------------------------------------------------------
	void SplitPath( const std::string& fullname, std::string& path,
		std::string& name, std::string& extension );

	// ----------------------------------------------------------------------
	// Method:		GetLangPrimary
	// Arguments:	langid - language identifier string
	// Returns:		primary language code
	// Description:	
	// Extracts the primary language code from a language identifier.
	// ----------------------------------------------------------------------
	std::string GetLangPrimary( const std::string& langid );

	// ----------------------------------------------------------------------
	// Method:		GetLangSecondary
	// Arguments:	langid - language identifier string
	// Returns:		secondary language code
	// Description:	
	// Extracts the secondary language code from a language identifier.
	// Note - according to the iso standard / rfc, the secondary id can
	// itself consist of multiple codes, separated by hyphens. We don't
	// bother with all that, and the secondary code(s) is/are just treated
	// as a single string.
	// ----------------------------------------------------------------------
	std::string GetLangSecondary( const std::string& langid );
private:
	struct FileInfo
	{
		std::string Path;
		std::string Name;
		std::string PriLang;
		std::string SecLang;
		std::string Ext;
	};
	typedef std::vector<FileInfo> file_list;

	bool GetFileList( const std::string& path, file_list& files, const std::string& wildCard = "*");
	file_list::iterator FindBestFile( file_list& files,
		const std::string path,
		const std::string name,
		const std::string prilang,
		const std::string seclang,
		const std::string ext );

};

#endif // FILELOCALISER_H
