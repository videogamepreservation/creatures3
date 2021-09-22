// -------------------------------------------------------------------------
// Filename:    
// Class: 
// Purpose:     
// Description:
//
// Usage:
//
// History:
// -------------------------------------------------------------------------


#include "TimeFuncs.h"
#include <time.h>


uint32 GetRealWorldTime()
{
	return (uint32)(time(NULL));
}


#ifdef _WIN32
int GetTimeStamp()
{
	return timeGetTime();
}

int64 GetHighPerformanceTimeStamp()
{
	LARGE_INTEGER stamp;
	if (QueryPerformanceCounter(&stamp))
		return stamp.QuadPart;
	else
		return 0;		
}

int64 GetHighPerformanceTimeStampFrequency()
{
	LARGE_INTEGER stamp;
	if (QueryPerformanceFrequency(&stamp))
		return stamp.QuadPart;
	else
		return 0;		
}

#else
// Non-windows version

// TODO: implementation :-)

int GetTimeStamp()
{
	return 0;
}

int64 GetHighPerformanceTimeStamp()
{
	return 0;
}

int64 GetHighPerformanceTimeStampFrequency()
{
	return 0;
}

// win32 replacement function
void GetLocalTime( SYSTEMTIME* t )
{
	memset( t,0,sizeof( SYSTEMTIME ) );
	#warning TODO: implement GetLocalTime()
}

#endif




//Check for invalid time components
bool IsValidTime(SYSTEMTIME& time)
{
	// TODO: Is this correct?
	if(time.wHour < 1 || time.wHour > 24)
		return false;

	if( time.wMinute > 59)
		return false;

	if( time.wSecond > 59)
		return false;

	if( time.wMilliseconds > 999)
		return false;

	return true;
}

//Check for invalid time components
bool IsValidDate(SYSTEMTIME& time)
{
	if(time.wDay == 0 || time.wMonth == 0)
		return false;

	return true;
}

// game time must be at least one second!!
bool IsValidGameTime(SYSTEMTIME& time)
{
	if(time.wHour==0 && time.wMinute == 0 && time.wSecond ==0)
	return false;

	return true;
}

