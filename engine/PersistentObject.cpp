
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "PersistentObject.h"



// ----------------------------------------------------------------------
// Static Member functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Method:		New
// Arguments:	name - name of class
// Returns:		A new instance of the named class
// Description:	Searches through stored classes and, if a match is found,
//				creates a new instance of the class.  The class must
//				have been derived from PersistentObject, and registered
//				with the CREATURES_DECLARE/IMPLEMENT_SERIAL macros
// ----------------------------------------------------------------------
PersistentObject *PersistentObject::New(LPCTSTR name)
	{
	// Just pass this through to the static list of declared classes
	return GetClassList().New(name);
	}

// ----------------------------------------------------------------------
// Method:		AddClass
// Arguments:	name  - name of class
//				function - function returning a new object of the
//						   given class
// Returns:		zero (for implementation reasons, a dummy value must be
//					  returned)
// Description:	Adds the named class to 
// ----------------------------------------------------------------------
int PersistentObject::AddClass(LPCTSTR name, NewObjectFunction function)

	{
	// Add the class to the static list
	GetClassList().Add(name,function);

	// We have to return a dummy value (for use in CREATURES_IMPLEMENT_SERIAL)
	return 0;
	}


// ----------------------------------------------------------------------
// Method:		New
// Arguments:	name - name of class
// Returns:		A new instance of the named class (or NULL if no match)
// Description:	Searches through stored classes and, if a match is found,
//				creates a new instance of the class.  The class must
//				have been derived from PersistentObject, and registered
//				with the CREATURES_DECLARE/IMPLEMENT_SERIAL macros
// ----------------------------------------------------------------------
PersistentObject *PersistentObject::DeclaredClassList::New(LPCTSTR name)
	{
	// Iterate through the linked list of declared classes, until
	// a matching entry is found
	DeclaredClass *test = list;

	while (test != NULL)
		{
		// Does it match ?
		if (test -> Match (name) )
			{
			return test -> New();
			}

		// Move on to the next entry
		test = test -> GetNext();
		}

	// No matching entry could be found
	return NULL;
	}

// ----------------------------------------------------------------------
// Method:		AddClass
// Arguments:	name  - name of class
//				function - function returning a new object of the
//						   given class
// Returns:		Nothing
// Description:	Adds the named class to the stored list
// ----------------------------------------------------------------------
void PersistentObject::DeclaredClassList::Add(LPCTSTR name,
											  NewObjectFunction function)
	{
	// Create a new link, and place it at the head of the list
	list = new DeclaredClass(name,function,list);
	}
