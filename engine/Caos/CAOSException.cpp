#ifdef OBSOLETE

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <stdio.h>
#include <stdarg.h>
#include "CAOSException.h"

CAOSException::CAOSException( const char* fmt, ... )
{
	va_list args;
	va_start( args, fmt );
	vsprintf( myMessage, fmt, args );
	va_end( args );
}

#endif // OBSOLETE
