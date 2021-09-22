// BenC 30Dec99

// NOTE: win32 version not yet tested(!)

#ifndef _WIN32
#error // use platform-specific version!
#endif

#include <windows.h>

bool FileExists( conat char* filename )
{
	if( GetFileAttributes( filename ) == -1 )
		return false;
	else
		return true;
}


