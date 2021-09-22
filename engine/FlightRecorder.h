// -------------------------------------------------------------------------
// Filename:    FlightRecorder.h
// Class:       FlightRecorder
// Purpose:     Basic logging system
// Description:
// Each log entry should be a single line of text.
//
// Usage:
//
//
// History:
// Jan99	  BenC	Initial version
// -------------------------------------------------------------------------
#ifndef FLIGHTRECORDER_H
#define FLIGHTRECORDER_H

// Ideas for improvement
// - allow for redirection out to debug console or external process
// (eg caos debugger).
// - category naming

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"

#include <stdio.h>


class FlightRecorder
{
public:
	// ---------------------------------------------------------------------
	// Method:		Log
	// Arguments:	categorymask - to which category(s) does this msg belong
	//				fmt - printf-style format string (should be single line)
	// Returns:		None
	// Description: Logs the message to the specified category(s).
	// ---------------------------------------------------------------------
	void Log( uint32 categorymask, const char* fmt, ... );

	// ---------------------------------------------------------------------
	// Method:		SetOutFile
	// Arguments:	filename
	// Returns:		None
	// Description: Specifies a file to send log output to
	// ---------------------------------------------------------------------
	void SetOutFile( const char* filename );

	// ---------------------------------------------------------------------
	// Method:		SetCategories
	// Arguments:	enablemask - categories to enable
	// Returns:		None
	// Description: Allows masking-out of specific categories.
	//				Any log data sent to a category with a zero bit in the
	//				enablemask will be ignored.
	// ---------------------------------------------------------------------
	void SetCategories( uint32 enablemask );

	FlightRecorder();
	~FlightRecorder();
private:
#ifdef _WIN32
	char	myOutFilename[ MAX_PATH ];
#else
	// TODO: need to suss out a proper definition for MAX_PATH
	char	myOutFilename[ 512 ];
#endif
	FILE*	myOutFile;
	uint32	myEnabledCategories;
};

#endif // FLIGHTRECORDER_H
