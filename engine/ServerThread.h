// -------------------------------------------------------------------------
// Filename:    ServerThread.h
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

#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"
#include <windows.h>

class ServerThread
{
public:
	ServerThread();
	~ServerThread() { Die(); }

	// ---------------------------------------------------------------------
	// Method:      Run
	// Arguments:   fenster - Handle of window to send messages to
	//				eventobject - win32 object to monitor
	//				msgid - ID of message to send to the window when an
	//					event occurs (probably WM_USER + x).
	// Returns:     true for Success
	// Description:	Creates a thread which waits on "eventobject", sending
	//				a message to "fenster" whenever the object is signaled.
	// ---------------------------------------------------------------------
	bool Run( HWND fenster, HANDLE eventobject, UINT msgid );


	// ---------------------------------------------------------------------
	// Method:      Die
	// Arguments:   None
	// Returns:     None
	// Description:	Stops the auxilary thread and cleans up.
	// ---------------------------------------------------------------------
	void Die();

private:
	static DWORD WINAPI HackedThreadStart( LPVOID ob );
	UINT ThreadMain();

	HWND	myWindow;
	HANDLE	myObjectToWatch;
	UINT	myMessageID;
	HANDLE	myThread;
	HANDLE	myDieNowObject;		// used to tell the thread to shut down
	HANDLE	myUpAndRunningObject;
};

#endif // SERVERTHREAD_H
