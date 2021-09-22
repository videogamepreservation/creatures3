#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "General.h"
#include "App.h"
#include "Creature/Creature.h"	// for MALE/FEMALE
#include "Display/ErrorMessageHandler.h"

#include "../common/Catalogue.h"
#include "World.h"
#include "md5.h"

#ifdef _WIN32
#include "rpcdce.h"
#endif

#include <stdarg.h>
#ifdef _WIN32
#include <shellapi.h>
#else
#include "unix/FileFuncs.h"
#endif

#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#else
#include <sstream>
#endif



// Return an ascii filespec in the form xxxx.yyy, given fsp as a 4-char token (eg 'NORN')
// and a 3 character string for the suffix.
// Eg. BuildFsp(Tok('g','r','e','n'),"spr") returns "gren.spr"
// SubDir is a constant referring to an optional path to the directory, eg. 
// BuildFsp(0x30303030,"tst",BODY_DATA_DIR) might return "C:\SFC\Body Data\0000.tst"

// changed behaviour to accomodate world directories
// first check whether the file is in the local directory
// *if* it is *there* then choose it over the main directory
// otherwise *always* return the main directory version
char* BuildFsp(DWORD fsp,char const* ext, int SubDir /* =-1 */,
			    bool isOverlay/* = false*/)
{
		// template for filespecs
    static char ourlocalTemplate[MAX_PATH]= "####.###";
	
	static char SFPath[MAX_PATH];				// temp buffer holding path	for caller
	int i;

	char* AscFsp = (char*)&fsp;					// treat DWORD token as 4 chars
  	for (i=0; i<4; i++)							// copy chars into filespec from lo to hi
    	ourlocalTemplate[i] = *AscFsp++;

  	for (i=0; i<3; i++)							// copy extension
		ourlocalTemplate[5+i] = ext[i];

	if	(SubDir!=-1) 							// if a subdirectory is required
	{
		// first check whether the file is in the local directory
		// if it is there then choose it over the main directory
		// otherwise always return the main directory version
		// also get the local catalogue files
	
		std::string path;
		if(theApp.GetWorldDirectoryVersion(SubDir,path))
		{
		strcpy(SFPath,path.data());

		if(isOverlay)
			strcat(SFPath,"over_");

		strcat(SFPath,ourlocalTemplate);
#ifdef _WIN32
		if(GetFileAttributes(SFPath) != -1)
#else
		if(FileExists(SFPath))
#endif	
			return SFPath;
		}

		
	
		theApp.GetDirectory(SubDir, SFPath);		// copy it to start of path
		
	}
	else
		SFPath[0] = '\0';						// else path starts with nothing

	if(isOverlay)
			strcat(SFPath,"over_");

	strcat(SFPath,ourlocalTemplate);					// add filename to optional path

    // Error handling info.
//	strcpy(dderr_extra, SFPath);

	return SFPath;
 }


// Find the most suitable available attachment or sprite file, given the specification
// of the creature & part number. Return the MONIKER of this file (not the path).
// If the explicit file is not found, try Variant 0. If still no good, try the previous age.
DWORD ValidFsp(int Part,			// part number
			   int Genus,			// Genus (NORN, GRENDEL, ETTIN, SIDE)
			   int Sex,				// IDEAL Sex to look for (MALE/FEMALE)
			   int Age,				// IDEAL age & variant to look for
			   int Variant,
			   char* Ext,			// file extension ("spr" or "att")
			   int Dir,				// subdirectory type, eg. BODY_DATA_DIR
			   bool isOverlay /*= false*/)
{
	int s,v,g,a;
	DWORD Fsp;
	Sex-=MALE;										// cvt sex into range 0 to 1
	std::string string;

	for	(s=0; s<2; s++) {							// do ideal sex first, else opposite sex

		if	(s==0)
			g = Genus + Sex * 4;					// 0123 = male norn/gren/ett/side 4567=female
		else
			g = Genus + (1-Sex) * 4;				// 2nd time through, look for opposite sex

		for	(v=Variant; v>=0; v--) {				// variant is next most important item
	
			for	(a=Age; a>=0; a--) {				// if can't find this age, use previous age

				Fsp = (Part+'A') | 
							((g+'0')<<8) | 
							((a+'0')<<16) | 
							((v+'a')<<24);

				if	(FindFile(string = BuildFsp(Fsp,Ext,Dir,isOverlay),1)) {	// found a file? return its moniker
					return Fsp;
				}

			}										// else try previous age
		}											// and previous variant
	}												// and opposite sex

	if (Genus != 0)
	{
		DWORD nfsp = ValidFsp(Part,Genus-1,Sex+MALE,Age,Variant,Ext,Dir,isOverlay);
		return nfsp;
	}
	return 0;
}

// !!!!!!!!!!!! to do:
// just look on the hard drive for now but sort it out later
bool FindFile(std::string& filename,
			   bool hardDriveOnly)	
{
#ifdef _WIN32
	uint32 attributes = GetFileAttributes(filename.data());
	if (attributes == -1)
	{
		return false;
	}
#else
	ASSERT( false );	// TODO - non win32 version

#endif



	return true;												// return temporary full path to file
}


// IsValidTime, IsValidDate, IsValidGameTime moved to TimeFuncs.cpp - BenC

#ifdef _WIN32
bool GetFilesInDirectory(const std::string& path, std::vector<std::string>& files, const std::string& wildcard /* = "*" */)
{
	std::string str = path + wildcard;
	WIN32_FIND_DATA finddata;
	HANDLE h = FindFirstFile( str.c_str(), &finddata );
	if( h == INVALID_HANDLE_VALUE )
		return false;
	do
	{
		if( !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
			files.push_back( finddata.cFileName  );
	}
	while( FindNextFile( h, &finddata ) );
	DWORD err = GetLastError();
	FindClose( h );

	if( err != ERROR_NO_MORE_FILES )
		return false;
	else
		return true;
}

bool GetDirsInDirectory(const std::string& path, std::vector<std::string>& dirs)
{
	std::string str = path + "*";
	WIN32_FIND_DATA finddata;
	HANDLE h = FindFirstFile( str.c_str(), &finddata );
	if( h == INVALID_HANDLE_VALUE )
		return false;
	do
	{
		if( (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			(*finddata.cFileName != '.'))
			dirs.push_back( finddata.cFileName  );
	}
	while( FindNextFile( h, &finddata ) );
	DWORD err = GetLastError();
	FindClose( h );

	if( err != ERROR_NO_MORE_FILES )
		return false;
	else
		return true;
}

// only load the directories that do not appear in the specified vector to test against
bool GetMissingDirsInDirectory(const std::string& path,
							   const std::vector<std::string>& dirsToTestAgainst,
							   std::vector<std::string>& dirs)
{

	// first get them all
	if(GetDirsInDirectory(path,dirs))
	{
		std::vector<std::string>::const_iterator it;
		std::vector<std::string>::iterator itToGo;

		// then weed out the ones we don't want
		for( it = dirsToTestAgainst.begin(); it != dirsToTestAgainst.end(); it++ )
		{
			itToGo = std::remove( dirs.begin(), dirs.end(), (*it) );
			if(itToGo != dirs.end() )
			{
				dirs.erase(itToGo);
			}
		}
		return true;
	}

	return false;
}

bool CopyDirectory( std::string const &source, std::string const &destination )
{
	std::vector<std::string> files, dirs;
	std::vector<std::string>::const_iterator i;

	CreateDirectory( destination.c_str(), NULL );

	GetFilesInDirectory( source + "\\", files );

	for( i = files.begin(); i != files.end(); ++i )
		CopyFile( (source + "\\" + *i).c_str(),
			(destination + "\\" + *i).c_str(), FALSE );

	GetFilesInDirectory( source + "\\", dirs );
	for( i = dirs.begin(); i != dirs.end(); ++i )
		CopyDirectory( (source + "\\" + *i).c_str(),
			(destination + "\\" + *i).c_str() );

	return true;
}

bool DeleteDirectory( std::string directory )
{
	SHFILEOPSTRUCT fileOp;

	//this struct requires a *double* zero terminated string!
	char *buffer = new char[ directory.size() + 2 ];
	directory.copy( buffer, directory.size() );
	buffer[ directory.size() ] = 0;
	buffer[ directory.size() + 1 ] = 0;

	fileOp.hwnd = 0;
	fileOp.wFunc = FO_DELETE;
	fileOp.pFrom = buffer;
	fileOp.pTo = 0;
	fileOp.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;

	if( SHFileOperation( &fileOp ) != 0 )
	{
		ErrorMessageHandler::Show("archive_error", 8, "World::Delete");
		return false;
	}

	delete [] buffer;
	return true;
}
#else
	#warning non-win32 versions missing...
	// TODO - fix.
#endif //_WIN32


std::string GenerateUniqueIdentifier(std::string extraFood1, std::string extraFood2)
{
	// Set up MD5 
	md5_state_t state;
	md5_init(&state);
	 
	// Feed MD5 various data to be munged, to guarantee universal uniqueness ...

	// ... time stamp
	int timeStamp = GetTimeStamp();
	md5_append(&state, (const md5_byte_t *)&timeStamp , sizeof(timeStamp));

	// ... extra strings passed in (parent genomes, for monikers)
	md5_append(&state, (const md5_byte_t *)extraFood1.data(), extraFood1.size());
	md5_append(&state, (const md5_byte_t *)extraFood2.data(), extraFood2.size());

	// ... error message header has date and user name in it
	std::string errorHeader = ErrorMessageHandler::ErrorMessageFooter();
	md5_append(&state, (const md5_byte_t *)errorHeader.data(), errorHeader.size());

	// ... number of agents in the world
	AgentList aList;
	theAgentManager.FindByFGS(aList, Classifier(0, 0, 0));
	int agentCount = aList.size();
	md5_append(&state, (const md5_byte_t *)&agentCount, sizeof(agentCount));

	// ... system and world ticks
	uint32 systemTick = theApp.GetSystemTick();
	md5_append(&state, (const md5_byte_t *)&systemTick, sizeof(systemTick));
	uint32 worldTick = theApp.GetWorld().GetWorldTick();
	md5_append(&state, (const md5_byte_t *)&worldTick, sizeof(worldTick));

	// ... mouse position and velocity
	int mopX = theApp.GetInputManager().GetMouseX();
	md5_append(&state, (const md5_byte_t *)&mopX, sizeof(mopX));
	int mopY = theApp.GetInputManager().GetMouseY();
	md5_append(&state, (const md5_byte_t *)&mopY, sizeof(mopY));
	float movX = theApp.GetInputManager().GetMouseVX();
	md5_append(&state, (const md5_byte_t *)&movX, sizeof(movX));
	float movY = theApp.GetInputManager().GetMouseVY();
	md5_append(&state, (const md5_byte_t *)&movY, sizeof(movY));

	// ... current pace
	float tickFactor = theApp.GetTickRateFactor();
	md5_append(&state, (const md5_byte_t *)&tickFactor, sizeof(tickFactor));

	// ... some stuff from our random number generator
	for (int i = 0; i < Rnd(200, 300); ++i)
	{
		md5_byte_t randomByte = Rnd(0, 255);
		md5_append(&state, &randomByte, sizeof(randomByte));
	}

	// ... platform dependent information (hardware profiles etc.)
#ifdef _WIN32
	UUID uuid;
	UuidCreate(&uuid);
	md5_append(&state, (const md5_byte_t *)&uuid, sizeof(uuid));
#else
	#warning TODO: Extract some unique ID from the operating system
#endif

	// ... high performance time stamp (later than time stamp, so more likely to be different)
	int64 highTimeStamp = GetHighPerformanceTimeStamp();
	md5_append(&state, (const md5_byte_t *)&highTimeStamp, sizeof(highTimeStamp));

	// Retrieve our digest
	md5_byte_t digest[16];
	md5_finish(&state, digest);

	
	unsigned int i1 = digest[0] + (digest[1] << 8) + (digest[2] << 16) + (digest[3] << 24);
	unsigned int i2 = digest[4] + (digest[5] << 8) + (digest[6] << 16) + (digest[7] << 24);
	unsigned int i3 = digest[8] + (digest[9] << 8) + (digest[10] << 16) + (digest[11] << 24);
	unsigned int i4 = digest[12] + (digest[13] << 8) + (digest[14] << 16) + (digest[15] << 24);

	std::string uniqueId;

	// We need some way to extract 5 entries from this dictionary...

	std::string dictionary = "abcdefghjklmnpqrstuvwxyz23456789"; // Length is 32 :)

	int loopy;
	for(loopy=0; loopy<5; loopy++) { uniqueId += dictionary.at(i1 % 31); i1 >>= 5; }
	uniqueId += "-";
	for(loopy=0; loopy<5; loopy++) { uniqueId += dictionary.at(i2 % 31); i2 >>= 5; }
	uniqueId += "-";
	for(loopy=0; loopy<5; loopy++) { uniqueId += dictionary.at(i3 % 31); i3 >>= 5; }
	uniqueId += "-";
	for(loopy=0; loopy<5; loopy++) { uniqueId += dictionary.at(i4 % 31); i4 >>= 5; }

	return uniqueId;
}

std::string WildSearch(int f, int g, int s, const std::string& tag_stub)
{
	std::string final_tag;

#ifdef C2E_OLD_CPP_LIB
	char buf1[32];
	std::ostrstream out1(buf1,sizeof(buf1));
#else
	std::ostringstream out1;
#endif
	out1 << tag_stub << " " << f << " " << g << " " << s;
	if (theCatalogue.TagPresent(out1.str()))
		final_tag = out1.str();
	else
	{
#ifdef C2E_OLD_CPP_LIB
		char buf2[32];
		std::ostrstream out2(buf2, sizeof(buf2));
#else
		std::ostringstream out2;
#endif
		out2 << tag_stub << " " << f << " " << g << " 0";
		if (theCatalogue.TagPresent(out2.str()))
			final_tag = out2.str();
		else
		{
#ifdef C2E_OLD_CPP_LIB
			char buf3[32];
			std::ostrstream out3(buf3, sizeof(buf3));
#else
			std::ostringstream out3;
#endif
			out3 << tag_stub << " " << f << " 0 0";
			if (theCatalogue.TagPresent(out3.str()))
				final_tag = out3.str();
		}
	}

	return final_tag;
}
