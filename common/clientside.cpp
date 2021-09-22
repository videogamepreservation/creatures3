// --------------------------------------------------------------------------
// Filename:	ClientSide.cpp
// Class:		ClientSide
//
// Purpose:		Provide a high performance client-side interface
//
// Description:
//
// Usage:
//
//
// History:
// --------------------------------------------------------------------------

#include "clientside.h"
#include "basicexception.h"

ClientSide::ClientSide()
{
	myMutex = NULL;
	myRequestEvent = NULL;
	myResultEvent = NULL;
	myMappedFile = NULL;
	mySharedMem = NULL;
}


void ClientSide::Cleanup()
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


bool ClientSide::Open( const char* name )
{
	if (myMutex != NULL)
		throw BasicException("Internal IPC mutex error");

	char buf[128];

	sprintf( buf, "%s_mutex", name );
	myMutex = OpenMutex( MUTEX_ALL_ACCESS, FALSE, buf );
	if( !myMutex )
	{
		Cleanup();
		return false;
	}

	DWORD dwFlags;
	BOOL bResult = GetHandleInformation(myMutex, &dwFlags);
  
	// maybe should obtain mutex before continuing?
	// (in reality, unlikely to be a problem)

	////////////////////////////////////////////////
	// open existing request event
	sprintf( buf, "%s_request", name );
	myRequestEvent = OpenEvent( EVENT_ALL_ACCESS, FALSE, buf );
	if( !myRequestEvent )
	{
		Cleanup();
		return false;
	}

	//////////////////////////////////
	// open existing result event
	sprintf( buf, "%s_result", name );
	myResultEvent = OpenEvent( EVENT_ALL_ACCESS, FALSE, buf );
	if( !myResultEvent )
	{
		Cleanup();
		return false;
	}

	///////////////////////////////////
	// find existing shared memory area

	sprintf( buf, "%s_mem", name );
	myMappedFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,
		false,
		buf );

	if( !myMappedFile )
	{
		Cleanup();
		return false;
	}

	/////////////////////////////
	// Set up the TransferBuffer

	mySharedMem = (TransferHeader*)MapViewOfFile( myMappedFile,
		FILE_MAP_ALL_ACCESS,
		0,0,
		0 );		// map the entire taco

	if( !mySharedMem )
	{
		Cleanup();
		return false;
	}

	// sanity check
	if( mySharedMem->MagicCookie[0] != 'c' ||
		mySharedMem->MagicCookie[1] != '2' ||
		mySharedMem->MagicCookie[2] != 'e' ||
		mySharedMem->MagicCookie[3] != '@' )
	{
		Cleanup();
		return false;
	}

	return true;
}




bool ClientSide::StartTransaction( const unsigned char* data, unsigned int size )
{
//	ASSERT(myTransferBuffer->Buffer);
	if( size > mySharedMem->BufferSize )
		return false;	// tooooooo much data.

	// check the magic cookie
	if( mySharedMem->MagicCookie[0] != 'c' ||
		mySharedMem->MagicCookie[1] != '2' ||
		mySharedMem->MagicCookie[2] != 'e' ||
		mySharedMem->MagicCookie[3] != '@' )
	{
		return false;
	}               

	if( WaitForSingleObject( myMutex, FIRST_TIMEOUT ) != WAIT_OBJECT_0 )
		return false;

	// copy request to buffer
	memcpy( (unsigned char*)(mySharedMem+1), data, size );
	mySharedMem->DataSize = size;

	// tell the server to process it...

	ResetEvent( myResultEvent );
//	OutputDebugString("Set event...\n");
	SetEvent( myRequestEvent );
//	OutputDebugString("... Set ended\n");

	HANDLE serverProcessHandle =
		OpenProcess(PROCESS_ALL_ACCESS, FALSE, mySharedMem->ServerProcessId);

	// wait for result or for the engine to die:
	const HANDLE twoHandles[2] = {myResultEvent, serverProcessHandle};
	if( WaitForMultipleObjects( 2, twoHandles, FALSE, INFINITE ) != WAIT_OBJECT_0 )
	{
//		OutputDebugString(".. failed for result\n");
		CloseHandle(serverProcessHandle);
		ReleaseMutex( myMutex );
		return false;
	}
//	OutputDebugString(".. OK for result\n");
	CloseHandle(serverProcessHandle);

	return true;
}




unsigned int ClientSide::GetResultSize()
{
	return mySharedMem->DataSize;
}


const unsigned char* ClientSide::GetResultBuffer()
{
	// data area appears immediately after header struct
	return (unsigned char*)(mySharedMem+1);
}


int ClientSide::GetReturnCode()
{
	return mySharedMem->ReturnCode;
}


void ClientSide::EndTransaction()
{
	ReleaseMutex( myMutex );
}
