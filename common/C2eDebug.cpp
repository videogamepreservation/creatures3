// -------------------------------------------------------------------------
// Filename:    C2eDebug.cpp
// Class:       None (but see also C2eDebug.h)
// Purpose:     Provide ASSERT and output of debug strings.
// Description:
//
// Usage:
//
//
// History:
// 13Aug99  BenC  Pulled out of C2eServices.
// -------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include <stdarg.h>
#include <stdio.h>

#include "C2eDebug.h"


#ifndef _WIN32
void OutputDebugString( const char* lpOutputString )
{
	fprintf( stderr, "%s", lpOutputString );
}

#endif


void OutputFormattedDebugString(const char* fmt, ... )
{
#ifdef _DEBUG
	char buf[512];
	va_list args;

	va_start(args, fmt);
	int len = vsprintf(buf, fmt, args);
	va_end(args);
	OutputDebugString(buf);
#endif
}


