// --------------------------------------------------------------------------
// Filename:	Music Script.cpp
// Class:		MusicScript
// Purpose:		
//
// Description:
//
// Breaks up the script into individual tokens, which can be examined one at
// a time.
//
// Token types:
//
//   String			 Any alphanumeric string 
//   Scoped String	 String of the form layerName_variableName
//	 Start Argument	 (
//	 End Argument	 )
//	 Start Section   {
//	 End Section	 }
//	 Separator		 ,
//	 Assignment	     =
//   Constant		 Floating point value	
//   EndOfFile
//	 Unrecognised	 Anything else
// History:
// 8Apr98	PeterC	Created
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicScript.h"

// ----------------------------------------------------------------------
// Method:		MusicScript
// Arguments:	script - complete text of script
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicScript::MusicScript(LPCTSTR script)
	{
	if (script!=NULL)
		{
		// Copy the text of the script
		text = script;
		}

	// Point to the beginning
	currentPos = text.data();

	// Parse the first token, and point to the next
	Advance();
	}

// ----------------------------------------------------------------------
// Method:		Restart
// Arguments:	None
// Returns:		Nothing
// Description:	Point back to the beginning of the script
// ----------------------------------------------------------------------
void MusicScript::Restart()
	{
	// Point to the beginning
	currentPos = text.data();

	// Parse the first token, and point to the next
	Advance();

	}

// ----------------------------------------------------------------------
// Method:		Advance
// Arguments:	None
// Returns:		Nothing
// Description:	Move on to the next token
// ----------------------------------------------------------------------
void MusicScript::Advance()
	{
	SubAdvance();
	}

// ----------------------------------------------------------------------
// Method:		SubAdvance
// Arguments:	None
// Returns:		Nothing
// Description:	This does the actual work of Advance, and enables 
//				advance to display debug information
// ----------------------------------------------------------------------
void MusicScript::SubAdvance()
	{
	// Reset the current contents
	currentToken="";
	currentValue=0.0;

	// Default to an error, unless something claims otherwise
	currentType = Unrecognised;

	// We need to ignore any white space and comments
	bool ignore;
	do
		{
		ignore=FALSE;
		if (IsCarriageReturn( currentPos ) )
			{
			// Jump past this
			currentPos=SkipCarriageReturn( currentPos);
			ignore = true;
			}
		else
			{
			if (IsWhiteSpace( currentPos ) )
				{
				// Jump past this
				currentPos++;
				ignore = TRUE;
				}
			else
				{
				// Is this the start of a comment (//)
				if (*currentPos=='/' && *(currentPos+1)=='/' )
					{
					ignore = TRUE;

					// Skip past the comment (//)
					currentPos+=2;

					// then skip everything up to the end of the line
					BOOL foundEnd=FALSE;
					do
						{
						// Keep a look out for terminators mid comment!
						if (IsTerminator( currentPos) )
							{
							// our work here is done...
							currentType =  EndOfFile;
							return;
							}

						// Have we reached the end of the line?
						if (IsCarriageReturn(currentPos))
							{
							// Skip past these shady characters
							currentPos=SkipCarriageReturn(currentPos);

							// Flag that we can stop looking
							foundEnd=TRUE;
							}
						else
							{
							// Whatever it was, we weren't interested
							currentPos++;
							}
				
						} while (!foundEnd);
					}
				}
			}
		} while (ignore);

	if (IsTerminator( currentPos) )
		{
		// our work here is done...
		currentToken = "";
		currentType =  EndOfFile;
		return;
		}

	// Have we got a word ?
	if ( IsLetter( currentPos) )
		{
		while (IsLetter( currentPos) || IsDigit( currentPos) )
			{
			currentToken += *currentPos++;
			}

		// Is this a scoped string ( two strings separated by an underscore )
		if (*currentPos == '_' )
			{
			currentToken += *currentPos++;

			if (!IsLetter( currentPos ) )
				{
				// Don't know what's going on here!
				currentType = Unrecognised;
				return;
				}
			else
				{
				// Continue appending the second half
				while (IsLetter( currentPos) || IsDigit( currentPos) )
					{
					currentToken += *currentPos++;
					}
				}
			currentType = ScopedString;
			
			}
		else
			{
			currentType = String;
			}

		return;
		}

	// need to check for unary '-' before we parse a number
	bool unaryMinus = false;
	if (*currentPos == '-' )
		{
		// We'll need to check this was actually followed by a digit.
		unaryMinus = true;

		// Now skip past it
		// Add the symbol to the string
		currentToken += *currentPos++;

		}

	// Check for numerical constants (always floating point)
	if (IsDigit(currentPos))
		{
		// If it's a float, we need to make sure it's valid 
		// (not 0., 102. or 1.2.3 etc)
		bool justHadDot = FALSE;
		bool hadDot = FALSE;

		// If this turns out to be floating point, we'll need to keep
		// track of the number of places we've reached
		MusicValue multiplier=(MusicValue) 0.1;

		while(IsDigit(currentPos) || *currentPos=='.')
			{
			// Has a floating point occurred
			if (*currentPos=='.')
				{
				// Have we already had a floating point?
				if (hadDot)
					{
					// Yes - flag an error
					currentType=Unrecognised;
					return;
					}
				else
					{
					// We need to make sure a number follows this
					hadDot=true;
					justHadDot=true;
					}
				}
			else
				{
				justHadDot = false;

				// Incorporate this new digit into the stored contents
				if (hadDot)
					{
					// This is part of the fractional component
					currentValue += multiplier * ( (MusicValue) (*currentPos - '0') );

					// Prepare the multiplier to move down to the next 
					// decimal place
					multiplier *= (MusicValue) 0.1;
					
					}
				else
					{
					// Keep accumulating the integer component
					currentValue *= 10.0;
					currentValue += *currentPos - '0' ;
					}
				}

			// Add the symbol to the string
			currentToken += *currentPos++;

			}

		// If we've ended with a dot, something's gone wrong!
		if (justHadDot)
			{
			currentType=Unrecognised;
			}
		else
			{
			currentType=Constant;

			// If this had been preceded by a unary minus,
			// invert the value
			if (unaryMinus)
				{
				currentValue=-currentValue;
				}
			}

		// Whatever it was, we've finished it now.
		return;

		}

	// Check here for the case where we had a minus operator, with
	// no following digits
	if (unaryMinus)
		{
		// Not having that!
		currentType=Unrecognised;
		return;
		}

	// The remainder are all individual symbols
	switch( *currentPos)
		{
		case ( '(' ) :
			currentType = StartArgument;
			break;
		case ( ')' ) :
			currentType = EndArgument;
			break;
		case ( '{' ) :
			currentType = StartSection;
			break;
		case ( '}' ) :
			currentType = EndSection;
			break;
		case ( ',' ) :
			currentType = Separator;
			break;
		case ( '=' ) :
			currentType = Assignment;
			break;
		default:
			currentType = Unrecognised;
			break;
		}

	// Store this symbo, and move on
	currentToken += *currentPos++;
	
	}

// ----------------------------------------------------------------------
// Method:		ParseArgument
// Arguments:	value  - to hold parsed value
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to parse in a single value enclosed in brackets
// ----------------------------------------------------------------------
MusicError MusicScript::ParseArgument(MusicValue &value)
	{
	// First the open bracket
	if (GetCurrentType() != StartArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	Advance();

	// Now should have a floating point value (time between
	// updates, in seconds)
	if (GetCurrentType() != Constant)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	value = GetCurrentValue();

	Advance();

	// Now a closed bracket
	if (GetCurrentType() != EndArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Point to the next section
	Advance();

	// Successfully parsed
	return MUSIC_OK;
	}

// ----------------------------------------------------------------------
// Method:		ParseArgument
// Arguments:	value  - to hold parsed value
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to parse in a single value enclosed in brackets
// ----------------------------------------------------------------------
MusicError MusicScript::ParseArgument(std::string &value)
	{
	// First the open bracket
	if (GetCurrentType() != StartArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	Advance();

	// Now should have a string
	if (GetCurrentType() != String)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	value = GetCurrentToken();

	Advance();

	// Now a closed bracket
	if (GetCurrentType() != EndArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Point to the next section
	Advance();

	// Successfully parsed
	return MUSIC_OK;
	}
