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

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "StringIntGroup.h"

// ----------------------------------------------------------------------------------
// Constructor
// Arguments:	(None)
// Returns:		(None)
// Description:	Creates an empty StringIntGroup
// ----------------------------------------------------------------------------------
StringIntGroup::StringIntGroup()
{

}

//Helpers
inline int ReadInt(std::istream& src)
{
	char temp[4];
	src.read(temp,4);
	return *((int*)(temp));
}

inline void ReadString(std::istream& src, std::string& str)
{
	int leng = ReadInt(src);
	str.resize(leng);
	for(int i=0;i<leng;i++)
		src.read(&(str.at(i)),1);
}

// ----------------------------------------------------------------------------------
// Constructor
// Arguments:	fromThis - std::istream& - The stream to read from
// Returns:		(None)
// Description:	Creates a StringIntGroup and initialises it from the stream passed.
// ----------------------------------------------------------------------------------
StringIntGroup::StringIntGroup(std::istream& fromThis)
{
	int loop,num,val;
	std::string name,sval;
	num = ReadInt(fromThis);
	for(loop=0;loop<num;loop++)
	{
		ReadString(fromThis,name);
		val = ReadInt(fromThis);
		myStringIntMap[name] = val;
	}
	num = ReadInt(fromThis);
	for(loop=0;loop<num;loop++)
	{
		ReadString(fromThis,name);
		ReadString(fromThis,sval);
		myStringStringMap[name] = sval;
	}
}

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
bool StringIntGroup::FindInt(std::string name, int& dest)
{
	std::map<std::string,int>::iterator it = myStringIntMap.find(name);
	if (it == myStringIntMap.end())
		return false;
	dest = (*it).second;
	return true;
}

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
bool StringIntGroup::FindString(std::string name, std::string& dest)
{
	std::map<std::string,std::string>::iterator it = myStringStringMap.find(name);
	if (it == myStringStringMap.end())
		return false;
	dest = (*it).second;
	return true;
}

// ----------------------------------------------------------------------------------
// Method:		AddInt
// Arguments:	name - std::string - the name of the int
//				valu - int         - the value to add.
// Returns:		(None)
// Description:	Adds/Replaces the entry for "name" with the value "valu"
// ----------------------------------------------------------------------------------
void StringIntGroup::AddInt(std::string name, int valu)
{
	myStringIntMap[name] = valu;
}

// ----------------------------------------------------------------------------------
// Method:		AddString
// Arguments:	name - std::string - the name of the int
//				valu - std::string - the value to add.
// Returns:		(None)
// Description:	Adds/Replaces the entry for "name" with the value "valu"
// ----------------------------------------------------------------------------------
void StringIntGroup::AddString(std::string name, std::string valu)
{
	myStringStringMap[name] = valu;
}

// Helper functions...
inline void WriteInt(std::ostream& d, int i)
{
	char* temp;
	temp = (char*)&i;
	d.write(temp,4);
}

inline void WriteString(std::ostream& d, std::string s)
{
	WriteInt(d,s.length());
	for(int i=0;i<s.length();i++)
		d.write(&(s.at(i)),1);
}

// ----------------------------------------------------------------------------------
// Method:      SaveToStream
// Arguments:	dest - std::ostream& - the stream to save to
// Returns:		(None)
// Description:	Saves the maps to the stream specified.
// ----------------------------------------------------------------------------------
void StringIntGroup::SaveToStream(std::ostream& dest)
{
	std::map<std::string,int>::iterator sii;
	std::map<std::string,std::string>::iterator ssi;
	WriteInt(dest,myStringIntMap.size());
	for(sii = myStringIntMap.begin(); sii != myStringIntMap.end(); sii++ )
	{
		WriteString(dest,(*sii).first);
		WriteInt(dest,(*sii).second);
	}
	WriteInt(dest,myStringStringMap.size());
	for(ssi = myStringStringMap.begin(); ssi != myStringStringMap.end(); ssi++)
	{
		WriteString(dest,(*ssi).first);
		WriteString(dest,(*ssi).second);
	}
}
