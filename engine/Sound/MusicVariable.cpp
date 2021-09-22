// --------------------------------------------------------------------------
// Filename:	Music Variable.cpp
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

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicVariable.h"
#include "MusicScript.h"

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script - script, pointing to the start of the variable
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to Parse the variable from the script
// ----------------------------------------------------------------------
MusicError MusicVariable::Parse(MusicScript &script)
	{
	// First Token should always be "Variable" ...
	std::string token(script.GetCurrentToken());

	if (strcmp(token.data(),"Variable")!=0)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// ...followed by an open bracket
	script.Advance();

	if (script.GetCurrentType() != MusicScript::StartArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Next comes the variable's name
	script.Advance();

	if (script.GetCurrentType() != MusicScript::String)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	SetName(script.GetCurrentToken());

	// Then a comma
	script.Advance();

	if (script.GetCurrentType() != MusicScript::Separator)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Then should be a constant value
	script.Advance();

	if (script.GetCurrentType() != MusicScript::Constant)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Store this both as the contents and initial contents,
	// so that the variable can be reset later
	initialContents = contents = script.GetCurrentValue();

	// Finally, close the declaration with a ')'
	script.Advance();

	if (script.GetCurrentType() != MusicScript::EndArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// And leave this pointing to the next thing
	script.Advance();

	return (MUSIC_OK);
	}



