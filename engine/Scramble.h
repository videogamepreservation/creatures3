// --------------------------------------------------------------------------
// Filename:	scramble.cpp
// History:		11June98	Peter Chilvers	Created
// --------------------------------------------------------------------------

#ifndef _SCRAMBLE_H

#define _SCRAMBLE_H

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
void Scramble(char *string, int length);

#endif
