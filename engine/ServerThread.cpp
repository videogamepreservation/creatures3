// -------------------------------------------------------------------------
// Filename:    ServerThread.cpp
// Class:       ServerThread
// Purpose:     Support for external interface system
// Description:	This class provides a way for a standard windows message
//				pump to handle event signals from win32 objects.
//
//				A thread is created which waits for an event to trigger
//				on a win32 HANDLE. When this happens, a windows
//				message is sent to the messagepump via PostMessage().
//
// Usage:
//
//
// History:
//   Nov98  BenC	Initial version
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ServerThread.h"
#include "../common/CStyleException.h"


ServerThread::ServerThread()
{
	myWindow = NULL;
	myObjectToWatch = NULL;
	myMessageID = 0;
	myThread = NULL;
	myUpAndRunningObject = NULL;
	myDieNowObject = NULL;
}



bool ServerThread::Run( HWND fenster, HANDLE eventobject, UINT msgid )
{
	DWORD dw;

	myWindow = fenster;
	myObjectToWatch = eventobject;
	myMessageID = msgid;

	myDieNowObject = CreateEvent( NULL, TRUE, FALSE, NULL );
	myUpAndRunningObject = CreateEvent( NULL, TRUE, FALSE, NULL );

	if( !myDieNowObject || !myUpAndRunningObject || !myWindow )
		return false;

	myThread = CreateThread( NULL,	// security
		0,							// stacksize
		HackedThreadStart,			// startpoint
		(LPVOID)this,				// param
		0,							// flags
		&dw );						// returned id

	if( !myThread )
		return false;

	dw = WaitForSingleObject( myUpAndRunningObject, 5000 );
	if( dw != WAIT_OBJECT_0 )
		return false;

	return true;
}


void ServerThread::Die()
{
	if( myDieNowObject )
	{
		SetEvent( myDieNowObject );
		if (WaitForSingleObject( myThread, 10000 ) == WAIT_TIMEOUT)
			ASSERT(false);
	}

	myWindow = NULL;
	if( myThread )
		CloseHandle( myThread );
	if( myDieNowObject )
		CloseHandle( myDieNowObject );
	if( myUpAndRunningObject )
		CloseHandle( myUpAndRunningObject );
	myThread = NULL;
	myUpAndRunningObject = NULL;
	myDieNowObject = NULL;
}




// static entry point for the thread - just passes control on
// to the non-static ThreadMain().
DWORD WINAPI ServerThread::HackedThreadStart( LPVOID ob )
{
	return ((ServerThread*)ob)->ThreadMain();	// ob contains a "this" pointer
}


UINT ServerThread::ThreadMain()
{
	// Catch C style exceptions as C++ style ones
	// for more informative messages
	ConvertCStyleExceptionsToCppStyle();

	DWORD dw;
	bool quit=false;
	HANDLE objects[2];

	PulseEvent( myUpAndRunningObject );

	objects[0] = myDieNowObject;
	objects[1] = myObjectToWatch;

	while( !quit )
	{
		dw = WaitForMultipleObjects( 2, objects, false, INFINITE );
		if( dw == WAIT_OBJECT_0 + 1 )
		{
		
			PostMessage( myWindow, myMessageID, 0, 0 );
		
		}
		else
			quit = true;

	}


	return 0;
}

