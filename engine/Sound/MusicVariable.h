// --------------------------------------------------------------------------
// Filename:	Music Variable.h
// Class:		MusicVariable
// Purpose:		Stores named variables within the music system, for use 
//				within expressions
//
// Description:
//
// Labels contain their names for use when compiling the initial soundtrack
// script.  For efficiency during execution, their values are pointed to
// directly.
//
// History:
// 02Apr98	Peter	Created
// --------------------------------------------------------------------------

#ifndef _MUSIC_VARIABLE_H

#define _MUSIC_VARIABLE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicErrors.h"
#include "MusicNamedItem.h"

class MusicScript;

class MusicVariable : public MusicNamedItem
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicVariable
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicVariable() {}

		// ----------------------------------------------------------------------
		// Method:		~MusicVariable
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicVariable() {}

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script - script, pointing to the start of the variable
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the variable from the script
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script);

		// ----------------------------------------------------------------------
		// Method:		GetContents
		// Arguments:	None
		// Returns:		Pointer to contents
		// Description:	Allows read/write access to variable
		// ----------------------------------------------------------------------
		MusicValue *GetContents()  {return &contents;}

		// ----------------------------------------------------------------------
		// Method:		Reset
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Restores initial value of variable
		// ----------------------------------------------------------------------
		void Reset() {contents = initialContents;}

	private:

		// ----------------------------------------------------------------------
		// Attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Contents of variable
		// ----------------------------------------------------------------------
		MusicValue contents;

		// ----------------------------------------------------------------------
		// intialContents
		// Initial value is stored to enable restarting of soundtrack
		// (Use Reset() )
		// ----------------------------------------------------------------------
		MusicValue initialContents;

	};

#endif
