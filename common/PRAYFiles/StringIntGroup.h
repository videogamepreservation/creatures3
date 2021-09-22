// --------------------------------------------------------------------------------------
// Filename:	StringIntGroup.h
// Class:		StringIntGroup
// Purpose:		The StringIntGroup class provides two maps - one str/str one str/int
// Description:
//  The StringIntGroup is designed to allow for two maps to me loaded and stored from/to
//  streams. The class basically wraps the two maps it has, into one nice interface.
//
// History:
//  24Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------


#ifndef STRINGINTGROUP_H
#define STRINGINTGROUP_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif
#include <map>
#include <string>
#include <iostream>

class StringIntGroup
{
public:
	// ----------------------------------------------------------------------------------
	// Constructor
	// Arguments:	(None)
	// Returns:		(None)
	// Description:	Creates an empty StringIntGroup
	// ----------------------------------------------------------------------------------
	StringIntGroup();

	// ----------------------------------------------------------------------------------
	// Constructor
	// Arguments:	fromThis - std::istream& - The stream to read from
	// Returns:		(None)
	// Description:	Creates a StringIntGroup and initialises it from the stream passed.
	// ----------------------------------------------------------------------------------
	StringIntGroup(std::istream& fromThis);

	// ----------------------------------------------------------------------------------
	// Method:		FindInt
	// Arguments:	name - std::string - the name of the int to search for.
	//				dest - int&		   - a reference to the integer to store the int in.
	// Returns:		true if found, false if not/error
	// Description: This method looks in the string->int map for the entry specified.
	//				if the entry is found, it is loaded into the integer passed in,
	//				and the function returns true. If it isn't found, the method returns
	//				false and the integer is unchanged.	
	// ----------------------------------------------------------------------------------
	bool FindInt(std::string name, int& dest);

	// ----------------------------------------------------------------------------------
	// Method:		FindString
	// Arguments:	name - std::string  - the name of the string to search for
	//				dest - std::string& - a reference to the string to store the result.
	// Returns:		true of found, false if not/error
	// Description:	This method looks in the string->string map for the entry required.
	//				If the entry is found, it is loaded into the string passed in,
	//				and the function returns true. If it isn't found, the method returns
	//				false and the string is unchanged.
	// ----------------------------------------------------------------------------------
	bool FindString(std::string name, std::string& dest);

	// ----------------------------------------------------------------------------------
	// Method:		AddInt
	// Arguments:	name - std::string - the name of the int
	//				valu - int         - the value to add.
	// Returns:		(None)
	// Description:	Adds/Replaces the entry for "name" with the value "valu"
	// ----------------------------------------------------------------------------------
	void AddInt(std::string name, int valu);

	// ----------------------------------------------------------------------------------
	// Method:		AddString
	// Arguments:	name - std::string - the name of the int
	//				valu - std::string - the value to add.
	// Returns:		(None)
	// Description:	Adds/Replaces the entry for "name" with the value "valu"
	// ----------------------------------------------------------------------------------
	void AddString(std::string name, std::string valu);

	// ----------------------------------------------------------------------------------
	// Method:      SaveToStream
	// Arguments:	dest - std::ostream& - the stream to save to
	// Returns:		(None)
	// Description:	Saves the maps to the stream specified.
	// ----------------------------------------------------------------------------------
	void SaveToStream(std::ostream& dest);

	// ----------------------------------------------------------------------------------
	// Method:		Clear
	// Arguments:	(None)
	// Returns:		(None)
	// Description:	Clears the StringToInt and StringToString Maps for the group
	// ----------------------------------------------------------------------------------
	void Clear() { myStringIntMap.clear(); myStringStringMap.clear(); }

private:
	std::map<std::string,int> myStringIntMap;
	std::map<std::string,std::string> myStringStringMap;

};

#endif //STRINGINTGROUP_H
