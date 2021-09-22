// --------------------------------------------------------------------------
// Filename:	Music Random.h
// Purpose:		Functions used in providing random floating point values
//
// Description:
//
// All functions are made inline, for efficiency.  If a zero random range is
// given, this will equate to simply returning a given number.
//
// History:
// 17Apr98	PeterC	Created
// --------------------------------------------------------------------------

#ifndef _MUSIC_RANDOM_H

#define _MUSIC_RANDOM_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"

// ----------------------------------------------------------------------
// Method:		MusicRandom
// Arguments:	size - maximum* value of random number
// Returns:		Random number
// Description:	Returns a random number in the range [0.0, size ]
//				* or [ size, 0.0 ] if size is negative
//				If size is zero, zero will be returned
// ----------------------------------------------------------------------
inline MusicValue MusicRandom(MusicValue size)
	{
	if (size == 0.0)
		{
		return 0.0;
		}
	else
		{
		// Multiply size by the ratio of the random number to the maximum
		// value it could have
		return size * ( (MusicValue) (rand()) / ( (MusicValue) RAND_MAX ) ) ;
		}
	};

// ----------------------------------------------------------------------
// Method:		MusicRandom
// Arguments:	min - minimum value of the random number
//				max - maximum value of the number
// Returns:		Random number
// Description:	Returns a random number in the range [min, max]
// ----------------------------------------------------------------------
inline MusicValue MusicRandom(MusicValue min, MusicValue max)
	{
	if (min == max)
		{
		return min;
		}
	else
		{
		// Multiply the difference by the ratio of the random number to 
		// the maximum value it could have
		return min + ( max - min) * ( (MusicValue) (rand()) / ( (MusicValue) RAND_MAX ) ) ;
		}
	};


#endif
