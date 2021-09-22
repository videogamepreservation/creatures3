// BenC 30Dec99

#ifdef _WIN32
#error // don't need this file under windows.
#endif

#include "FileFuncs.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


// win32 replacement
bool DeleteFile( const char* filename )
{
	int i=unlink( filename );
	return i==0 ? true:false;
}



// win32 replacement
// hmmm... thought there would be a better way...
bool CopyFile( const char* src, const char* dest, bool overwrite )
{
	int infd = -1;
	int outfd = -1;
	void* p = NULL;
	struct stat statbuf;
	bool success = false;	// positive attitude.
	int flags;

	// open input file and map it into memory
	infd = open( src, O_RDONLY );
	if( infd == -1 )
		goto cleanup;

	if( fstat( infd, &statbuf ) != 0 )
		goto cleanup;

	p = mmap( 0, statbuf.st_size, PROT_READ, MAP_PRIVATE, infd, 0 );
	if( p == MAP_FAILED )
		goto cleanup;


	// create output file
	flags = O_CREAT|O_WRONLY;
	if( overwrite )
		flags |= O_TRUNC;
	else
		flags |= O_EXCL;	// fail if file exists

	outfd = open( dest, O_WRONLY );
	if( outfd == -1 )
		goto cleanup;

	// blam.
	if( write( outfd, p, statbuf.st_size ) == statbuf.st_size )
		success = true; 


cleanup:
	if( outfd != -1 )
		close( outfd );
	if( p )
		munmap( p, statbuf.st_size );
	if( infd != -1 )
		close( infd );

	return success;
}

void CreateDirectory(const char* name, void* ignored)
{
	int i = mkdir(name,0xffff);	
}


// win32 replacement
bool MoveFile( const char* src, const char* dest )
{
	int i = rename( src, dest );
	return i==0 ? true:false;
}



bool FileExists( const char* filename )
{
	struct stat buf;
	int i = stat( filename, &buf );
	return i==0 ? true:false;
}




