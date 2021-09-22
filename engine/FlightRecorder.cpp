// -------------------------------------------------------------------------
// Filename:    FlightRecorder.cpp
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

// Categories so far:
// 1 - error message logging
// 16 - shutdown sequence logging

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "C2eServices.h"	// to get around circular dependency problem
#include "FlightRecorder.h"
#include "Display/ErrorMessageHandler.h"

FlightRecorder::FlightRecorder()
{
	// all categories on by default
	myEnabledCategories = 0xFFFFFFFF;
	myOutFile = NULL;
	myOutFilename[0] = '\0';
	strcpy( myOutFilename, "creatures_engine_logfile.txt" );
}


FlightRecorder::~FlightRecorder()
{
	if( myOutFile )
	{
		Log( myEnabledCategories, "");
		std::string ended = std::string("LOG ENDED ") + ErrorMessageHandler::ErrorMessageFooter();
		Log( myEnabledCategories, ended.c_str());
		fclose( myOutFile );
	}
}

void FlightRecorder::SetOutFile( const char* filename )
{
	// close existing file if any
	if( myOutFile )
	{
		fclose( myOutFile );
		myOutFile = NULL;
	}

	strcpy( myOutFilename, filename );
}


void FlightRecorder::Log( uint32 categorymask, const char* fmt, ... )
{
	char buf[512];
	va_list args;
	int len;

	if( !( categorymask & myEnabledCategories ) )
		return;

	// open file if needed...
	bool madeFile = false;
	if( !myOutFile )
	{
		if( myOutFilename[0] )
			myOutFile = fopen( myOutFilename, "a+tc" );
		madeFile = true;
	}

	if( !myOutFile )
		return;

	if (madeFile)
	{
		Log( myEnabledCategories, "----------------------------------------------------" );
		std::string started = std::string("LOG STARTED ") + ErrorMessageHandler::ErrorMessageFooter();
		Log( myEnabledCategories, started.c_str());
		Log( myEnabledCategories, "");
	}
	va_start(args, fmt);
	len = vsprintf( buf, fmt, args);
	va_end(args);

	// append a linefeed
	buf[len] = '\n';
	buf[++len] = '\0';
	fwrite( buf, 1, len, myOutFile );
	fflush( myOutFile );
}

void FlightRecorder::SetCategories( uint32 enablemask )
{
	myEnabledCategories = enablemask;
}
