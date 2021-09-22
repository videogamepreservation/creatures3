// --------------------------------------------------------------------------
// Filename:	ClientSide.h
// Class:		ClientSide
//
// Purpose:		Provide a high performance client-side interface
//
// Description:
//
// Usage:
//
// void servertest()
// {
//
//   ClientSide c;
//   c.Open( "a_server" );
//   if( c.StartTransaction( "hello" ) )
//     printf("Result: %s\n",s.GetResultBuffer() );
//   c.EndTransaction();
//   c.Close();
//	}
//
//
//
// History:
// --------------------------------------------------------------------------


#ifndef CLIENTSIDE_H
#define CLIENTSIDE_H

#include <stdio.h>
#include <windows.h>


// timeouts in milliseconds
#define FIRST_TIMEOUT 500		// time to wait to obtain server mutex
//#define SECOND_TIMEOUT_DEFAULT 5000	// time to wait for request to be processed

class ClientSide
{
public:
	// ----------------------------------------------------------------------
	// Method:		Open
	// Arguments:	servername - server to connect to
	// Returns:		Success
	// Description:	Connects to an existing server
	// ----------------------------------------------------------------------
	bool Open( const char* servername );

	// ----------------------------------------------------------------------
	// Method:		Close
	// Arguments:	None
	// Returns:		None
	// Description:	Shuts down, cleans up.
	// ----------------------------------------------------------------------
	void Close() { Cleanup(); }

	// ----------------------------------------------------------------------
	// Method:		StartTransaction
	// Arguments:	data - the request to send (binary data)
	//				size - size of request in bytes (need allow for '\0'
	//				       char if sending text!)
	// Returns:		Returns false if the communication failed.
	// Description:	Sends a request to the server and blocks until a result
	//				is sent back.
	//				Must be Paired with EndTransaction.
	//				NOTE: this call effectively locks the server until
	//				EndTransaction() is called.
	// ----------------------------------------------------------------------
	bool StartTransaction( const unsigned char* data, unsigned int size );

	// ----------------------------------------------------------------------
	// Method:		GetReturnCode()
	// Arguments:	None
	// Returns:		Error code from server
	// Description:	The server will return a non-zero error code here if
	//				the resquest crapped out.
	// ----------------------------------------------------------------------
	int GetReturnCode();

	// ----------------------------------------------------------------------
	// Method:		GetResultSize()
	// Arguments:	None
	// Returns:		Size of result data (in bytes)
	// Description:	Gets the number of bytes returned after a
	//				StartTransaction()
	// ----------------------------------------------------------------------
	unsigned int GetResultSize();

	// ----------------------------------------------------------------------
	// Method:		GetResultBuffer()
	// Arguments:	None
	// Returns:		Address of result buffer
	// Description: Allows reading of a result returned after a
	//				StartTransaction()
	// ----------------------------------------------------------------------
	const unsigned char* GetResultBuffer();

	// ----------------------------------------------------------------------
	// Method:		EndTransaction()
	// Arguments:	None
	// Returns:		None
	// Description:	Unlocks the result buffer and allows the server to accept
	//				other requests again.
	// ----------------------------------------------------------------------
	void EndTransaction();

	// ----------------------------------------------------------------------
	// Method:		GetBufferSize()
	// Arguments:	None
	// Returns:		Size of buffer being used (both requests and results are
	//				limited to this size)
	// ----------------------------------------------------------------------
	unsigned int GetBufferSize() { return mySharedMem->BufferSize; }

	ClientSide();
	~ClientSide() { Cleanup(); }
private:
	// !!! Need to keep this struct in sync with ServerSide version !!!
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
	TransferHeader* mySharedMem;
};

#endif // CLIENTSIDE_H

