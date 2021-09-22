// --------------------------------------------------------------------------
// Filename:	scramble.cpp
// History:		11June98	Peter Chilvers	Created
// --------------------------------------------------------------------------

#include "Scramble.h"

// ----------------------------------------------------------------------
// Method:		Scramble
// Arguments:	string - string to be replaced (not necessarily null
//						 terminated)
//				length - number of characters in string (this is included
//						 as the scrambled version may change some of the
//						 letters into zeros, playing havoc with 
//						 termination
// Returns:		Nothing
// Description:	Uses a quick and crude method to render text unreadable.
//				Calling the function a second time will unscramble.
// ----------------------------------------------------------------------
void Scramble(char *string, int length)
	{
	// Arbitrary byte against which the first letter will be EORed
	char scramble = 5;

	// Move through each letter in turn...
	for (int i = 0; i<length;i++)
		{
		// EORing it with the scramble value
		string[i] = string[i] ^ scramble ;

		// Change the scramble value to some other arbitrary value.
		// As 31 is prime, and not a factor of 256, this will cycle
		// through all values from 0 to 255, in a seemingly random
		// order
		scramble += (char) 193;
		}
	}
