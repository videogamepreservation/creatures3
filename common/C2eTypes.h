// -------------------------------------------------------------------------
// Filename:    C2eTypes.h
// Class:       None
// Purpose:     Basic datatype definitions for C2e
// Description:
//
//
// Usage:
//
//
// History:
// 03Dev98	BenC	Initial version
// -------------------------------------------------------------------------

#ifndef C2ETYPES_H
#define C2ETYPES_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif



#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#else
	typedef unsigned char BYTE;
	typedef unsigned short int WORD;
	typedef unsigned long int DWORD;
	typedef const char* LPCTSTR;
	typedef const char* LPTSTR;
	// also need BOOL?

	// better estimate for this?
	#define MAX_PATH 512
	#define _MAX_PATH MAX_PATH
#endif




// unambiguous basic types
typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int int16;
typedef unsigned short int uint16;
typedef signed long int int32;
typedef unsigned long int uint32;

#ifdef _WIN32
	typedef __int64 int64;
#else
	typedef long long int64;
#endif

// #include "Vector2D.h"

// CRect CSize etc...
#include "../engine/mfchack.h"
#include "../engine/Display/Position.h"


#define ConvertByteToFloat(aByte) (((float)aByte)/255.0f)
#define ConvertFloatToByte(aFloat) (int)(aFloat*255.0f)

#define GetUINT32At(bptr) (*(uint32*) (bptr) )
#define GetUINT16At(bptr) (*(uint16*) (bptr) )
#define GetUINT8At(bptr) (*(uint8*) (bptr) )


#endif // C2ETYPES_H

