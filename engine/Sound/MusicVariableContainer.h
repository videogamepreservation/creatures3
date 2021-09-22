// --------------------------------------------------------------------------
// Filename:	Music Variable Container.h
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

#ifndef _MUSIC_VARIABLE_CONTAINER_H

#define _MUSIC_VARIABLE_CONTAINER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicErrors.h"
#include "MusicNamedItem.h"
#include <vector>



class MusicScript;
class MusicVariable;


class MusicVariableContainer : public MusicNamedItem
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicVariableContainer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicVariableContainer();

		// ----------------------------------------------------------------------
		// Method:		~MusicVariableContainer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicVariableContainer();

		// ----------------------------------------------------------------------
		// Method:		ParseVariable
		// Arguments:	script - script pointing to the beginning of the
		//						 variable declaration
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Searches through list of named / pre-defined variables
		// ----------------------------------------------------------------------
		MusicError ParseVariable(MusicScript &script);

		// ----------------------------------------------------------------------
		// Method:		GetVariable
		// Arguments:	varName - name of the variable
		//				lvalue  - true if value can be written to
		// Returns:		Pointer to variable, or NULL if not defined
		// Description:	Searches through list of named / pre-defined variables
		// ----------------------------------------------------------------------
		virtual MusicValue *GetVariable(LPCTSTR varName, bool lvalue = false);

		// ----------------------------------------------------------------------
		// Method:		Reset
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Resets all variables to their initial values
		// ----------------------------------------------------------------------
		void Reset();

	private:

		// ----------------------------------------------------------------------
		// Member attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Array of named variables
		// ----------------------------------------------------------------------
		std::vector<MusicVariable*> variables;
		typedef std::vector<MusicVariable*>::iterator variableIterator;
	};

#endif
