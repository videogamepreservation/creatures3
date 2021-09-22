// --------------------------------------------------------------------------
// Filename:	ServerSide.h
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
// diversion...
#include "unix/ServerSide.h"
#else
// rest of this file is win32-specific.



#ifndef SERVERSIDE_H
#define SERVERSIDE_H



#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <windows.h>

class ServerSide
{
public:
	// ----------------------------------------------------------------------
	// Method:		Create
	// Arguments:	servername - an identifying name for the server
	//				maxsize - size of largest allowable data (request and
	//				          result sizes are both limited by this value)
	// Returns:		Success
	// Description:	Gets the server object ready to handle incoming requests
	// ----------------------------------------------------------------------
	bool Create( const char* servername, unsigned int maxsize );

	// ----------------------------------------------------------------------
	// Method:		Close
	// Arguments:	None
	// Returns:		None
	// Description:	Shuts down the server and cleans up
	// ----------------------------------------------------------------------
	void Close() { Cleanup(); }

	// ----------------------------------------------------------------------
	// Method:		Wait
	// Arguments:	None
	// Returns:		Success
	// Description:	Blocks until a request arrives.
	// ----------------------------------------------------------------------
	bool Wait() { return WaitForSingleObject( myRequestEvent,
		INFINITE ) == WAIT_OBJECT_0; }

	// ----------------------------------------------------------------------
	// Method:		GetRequestEvent
	// Arguments:	None
	// Returns:		A win32 object handle
	// Description:	The returned object will be signaled on incoming events
	// ----------------------------------------------------------------------
	HANDLE	GetRequestEvent() { return myRequestEvent; }

	// ----------------------------------------------------------------------
	// Method:		GetRequestSize
	// Arguments:	None
	// Returns:		The size of the request in bytes
	// Description:	Simple accessor to query a pending request
	//				NOTE: the request is _binary_ data rather than text.
	//				(ie text request sizes will include the '\0' byte)
	// ----------------------------------------------------------------------
	unsigned int GetRequestSize() { return mySharedMem->DataSize; }

	// ----------------------------------------------------------------------
	// Method:		GetBuffer
	// Arguments:	None
	// Returns:		Address of request buffer
	// Description:	For performance and size reasons, this buffer is used
	//				both to hold the incoming request and for returning
	//				results. The results should be simply written over the
	//				original request.
	//				NOTE: the buffer is for binary data rather than text, so
	//				when using text you'll need to cast to char*.
	// ----------------------------------------------------------------------
	unsigned char* GetBuffer()
		{ return (unsigned char*)(mySharedMem+1); }

	// ----------------------------------------------------------------------
	// Method:		Respond
	// Arguments:	None
	// Returns:		resultsize - size of result data in the buffer
	//				returncode - error code to return to client (0=no error)
	// Description: Respond to the pending request and send the contents of
	//				the buffer back to the client.
	//				Don't forget to allow for the '\0' in the resultsize if
	//				returning text!
	//				The pending request must be Respond()ed to before
	//				any other requests will be accepted.
	// ----------------------------------------------------------------------
	void Respond( unsigned int resultsize, int returncode=0 );

	// ----------------------------------------------------------------------
	// Method:		GetMaxBufferSize
	// Arguments:	None
	// Returns:		Size of request/result buffer
	// Description: Use this to figure out max size for result data.
	// ----------------------------------------------------------------------
	unsigned int GetMaxBufferSize()
		{return mySharedMem->BufferSize;}

	ServerSide();
	~ServerSide() { Cleanup(); }

private:
	// !!! Need to keep this struct in sync with ClientSide version !!!
	struct TransferHeader
	{
		char			MagicCookie[4];		// "c2e!"
		DWORD			ServerProcessId;	// id of server process
		int				ReturnCode;	// result code returned from server
		unsigned int	DataSize;	// size of data currently in buf
									// (both for requests and results)
		unsigned int	BufferSize;	// size of allocated buffer
									// (excluding this header struct)
		int				Pad;		// pad out to 8 byte boundary, just
									// to be pedantic :-)
		// Data area follows on from here.
	};

	void Cleanup();

	HANDLE	myMutex;
	HANDLE	myRequestEvent;
	HANDLE	myResultEvent;
	HANDLE	myMappedFile;
	TransferHeader*	mySharedMem;	// ptr to shared mem
};



#endif // SERVERSIDE_H

#endif // _WIN32

