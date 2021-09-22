// --------------------------------------------------------------------------
// Filename:	Music Variable Container.cpp
// Class:		MusicVariableContainer
// Purpose:		Named Class with a list of named variables
//
// Description:
//
// Class containing a number of named variables, all of type "MusicValue".
// Their contents can be exposed to expressions, actions and conditions.
// 
// History:
// 7Apr98	PeterC	Created
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicVariable.h"
#include "MusicVariableContainer.h"
#include "MusicScript.h"

// ----------------------------------------------------------------------
// Method:		MusicVariableContainer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicVariableContainer::MusicVariableContainer() 
	{
	// **** Probably should initialise size of variables array here
	}

// ----------------------------------------------------------------------
// Method:		~MusicVariableContainer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicVariableContainer::~MusicVariableContainer()
	{
	// Iterate through each variable in the array
	variableIterator it;
	for (it = variables.begin(); it != variables.end(); it++)
		{
		// delete the space, but not the entry (this will be deleted
		// by 'variables' destructor
		delete (*it);
		}
	}

// ----------------------------------------------------------------------
// Method:		ParseVariable
// Arguments:	script - script pointing to the beginning of the
//						 variable declaration
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Searches through list of named / pre-defined variables
// ----------------------------------------------------------------------
MusicError MusicVariableContainer::ParseVariable(MusicScript &script)
	{
	// Create a new variable, and attempt to Parse it
	MusicVariable *variable = new MusicVariable;
	MusicError error = variable -> Parse (script);

	// Success?
	if (error == MUSIC_OK)
		{
		// Yes - add it to the existing list
		variables.push_back( variable );
		}
	else
		{
		// No - Don't add this to the list
		delete variable;
		}
	return (error);
	}

// ----------------------------------------------------------------------
// Method:		GetVariable
// Arguments:	varName - name of the variable
//				lvalue  - true if value can be written to
// Returns:		Pointer to variable, or NULL if not defined
// Description:	Searches through list of named / pre-defined variables
// ----------------------------------------------------------------------
MusicValue *MusicVariableContainer::GetVariable(LPCTSTR varName,
												bool lvalue) 
	{
	// We can ignore the 'lvalue' part - more for the benefit of
	// derived classes

	// Iterate through each of the variables, checking for a match
	variableIterator it;

	for (it = variables.begin(); it != variables.end(); it++)
		{
		// Store the variable first, to void recalculating
		MusicVariable *current = (*it);

		_ASSERT( current );
		if (current->MatchName(varName))
			{
			// Yes - return the value this points to
			return ( current -> GetContents() );
			}
		}

	// Didn't find anything
	return ( NULL );
	}

// ----------------------------------------------------------------------
// Method:		Reset
// Arguments:	None
// Returns:		Nothing
// Description:	Resets all variables to their initial values
// ----------------------------------------------------------------------
void MusicVariableContainer::Reset()
	{
	// Iterate through each of the variables, reseting them
	variableIterator it;
	for (it = variables.begin(); it != variables.end(); it++)
		{
		_ASSERT ( (*it));
		(*it) -> Reset();
		}

	}
