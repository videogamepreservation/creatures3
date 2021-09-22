// --------------------------------------------------------------------------
// Filename:	Music Named Item.h
// Class:		MusicNamedItem
// Purpose:		Base class used within music system
//
// Description:
//
// Since most objects in the music system are named for compilation and 
// debugging purposes, many items are derived from this class. 
//
// History:
// 03Apr98	PeterC	Created
// 14Apr98	PeterC  Derived from CObject, to allow MusicManager to serialise
// 13Aug98	PeterC	Derived from PersistentObject, to support dual format
//					Serialisation
// --------------------------------------------------------------------------

#ifndef _MUSIC_NAMED_ITEM_H

#define _MUSIC_NAMED_ITEM_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../PersistentObject.h"
#include <string>


class MusicNamedItem : public PersistentObject
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicNamedItem
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicNamedItem() {}

		// ----------------------------------------------------------------------
		// Method:		MusicNamedItem
		// Arguments:	newName - name of the object
		// Returns:		Nothing
		// Description:	Constructor
		// ----------------------------------------------------------------------
		MusicNamedItem(LPCTSTR newName) : name(newName) {}

		// ----------------------------------------------------------------------
		// Method:		~MusicNamedItem
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicNamedItem() {}

		// ----------------------------------------------------------------------
		// Method:		SetName
		// Arguments:	newName - new name assigned to object
		// Returns:		Nothing
		// Description:	Sets the object's name, for music parser and debugging
		// ----------------------------------------------------------------------
		void SetName(LPCTSTR newName) {name=newName;}

		// ----------------------------------------------------------------------
		// Method:		MatchName
		// Arguments:	compare - name being compared with
		// Returns:		true if the name matches
		// Description:	Compares given string with name of variable
		// ----------------------------------------------------------------------
		bool MatchName(LPCTSTR compare) const
		{
// standard gnu c lib has strcasecmp not stricmp...
#ifdef __GLIBC__
			return (strcasecmp(name.data(), compare) == 0); 
#else
			return (stricmp(name.data(), compare) == 0); 
#endif
		}

		// ----------------------------------------------------------------------
		// Method:		GetName
		// Arguments:	None
		// Returns:		Pointer to name of string
		// Description:	Returns variable name
		// ----------------------------------------------------------------------
		LPCTSTR GetName() const {return name.data();}

	private:

		// ----------------------------------------------------------------------
		// Attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Variable name
		// ----------------------------------------------------------------------
		std::string name;

	};

#endif
