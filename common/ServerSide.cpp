// --------------------------------------------------------------------------
// Filename:	ServerSide.cpp
// Class:		ServerSide
//
// Purpose:		Provide a high performance server-side interface
//
// Description:
//
// Usage:
//
//
// History:
// Nov98	BenC	Created
// --------------------------------------------------------------------------


#ifndef _WIN32
#error "this is the win32 version."
#endif 


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ServerSide.h"

#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////

ServerSide::ServerSide()
{
	myMutex = NULL;
	myRequestEvent = NULL;
	myResultEvent = NULL;
	myMappedFile = NULL;
	mySharedMem = NULL;
}



/////////////////////////////////////////////////////////////////////////////
bool ServerSide::Create( const char* servername, unsigned int maxsize )
{
	char buf[128];


	//////////////////////////////////////////////////////////////////
	// Create a mutex to make sure only one client at a time connects.

	sprintf( buf, "%s_mutex", servername );
	myMutex = CreateMutex( NULL, TRUE, buf );	// initial ownership
	if(	!myMutex )
	{
		Cleanup();
		return false;
	}

	// mutex already exists?
	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		Cleanup();
		return false;
	}

	//////////////////////////////////
	// create request event for incoming stuff
	sprintf( buf, "%s_request", servername );
	myRequestEvent = CreateEvent( NULL, FALSE, FALSE, buf );
	if( !myRequestEvent )
	{
		Cleanup();
		return false;
	}

	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		Cleanup();
		return false;
	}

	//////////////////////////////////
	sprintf( buf, "%s_result", servername );
	myResultEvent = CreateEvent( NULL, FALSE, FALSE, buf );
	if( !myResultEvent )
	{
		Cleanup();
		return false;
	}

	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		Cleanup();
		return false;
	}


	////////////////////////////
	// Create shared memory area

	sprintf( buf, "%s_mem", servername );
	myMappedFile = CreateFileMapping(
		(HANDLE)0xFFFFFFFF,			// use system pagefile
		NULL,						// default security
		PAGE_READWRITE,				//
		0, sizeof( TransferHeader ) + maxsize,
		buf							// public name
		);

	if( !myMappedFile )
	{
		Cleanup();
		return false;
	}

	if( GetLastError() == ERROR_ALREADY_EXISTS )
	{
		Cleanup();
		return false;
	}

	////////////////////////////

	mySharedMem = (TransferHeader*)MapViewOfFile( myMappedFile,
		FILE_MAP_ALL_ACCESS,
		0,0,						// no offset
		sizeof( TransferHeader ) + maxsize );

	if( !mySharedMem )
	{
		Cleanup();
		return false;
	}


	// magic cookie
	mySharedMem->MagicCookie[0] = 'c';
	mySharedMem->MagicCookie[1] = '2';
	mySharedMem->MagicCookie[2] = 'e';
	mySharedMem->MagicCookie[3] = '@';
	mySharedMem->ServerProcessId = GetCurrentProcessId();

	mySharedMem->DataSize = 0;
	mySharedMem->BufferSize = maxsize;

	// let rip!
	ReleaseMutex( myMutex );

	return true;		// wahay!
}


/////////////////////////////////////////////////////////////////////////////
void ServerSide::Respond( unsigned int resultsize, int returncode )	
{
	mySharedMem->DataSize = resultsize;
	mySharedMem->ReturnCode = returncode;
	SetEvent( myResultEvent );
}


/////////////////////////////////////////////////////////////////////////////
// private

void ServerSide::Cleanup()
{
	if( mySharedMem )
	{
		UnmapViewOfFile( mySharedMem );
		mySharedMem = NULL;
	}

	if( myMappedFile )
	{
		CloseHandle( myMappedFile );
		myMappedFile = NULL;
	}

	if( myRequestEvent )
	{
		CloseHandle( myRequestEvent );
		myRequestEvent = NULL;
	}

	if( myResultEvent )
	{
		CloseHandle( myResultEvent );
		myResultEvent = NULL;
	}

	if( myMutex )
	{
		CloseHandle( myMutex );
		myMutex = NULL;
	}
}


