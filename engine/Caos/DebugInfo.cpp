// -------------------------------------------------------------------------
// Filename:    DebugInfo.cpp
// Class:       DebugInfo
// Purpose:     Debug information container for MacroScript
// Description:
// Holds source code and line number information to allow source-level
// debugging of caos macroscripts.
//
// Usage:
//
//
// History:
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "DebugInfo.h"

CREATURES_IMPLEMENT_SERIAL( DebugInfo )

void DebugInfo::AddAddressToPositionMapping( int address, int srcpos )
{
	myAddressMap[ address ] = srcpos;
}


int DebugInfo::MapAddressToSource( int address )
{
	std::map< int,int >::const_iterator it;
	int srcpos = 0;

	for( it=myAddressMap.begin(); it != myAddressMap.end(); ++it )
	{
		if( (*it).first > address )
			break;
		else
			srcpos = (*it).second;
	}

	return srcpos;
}




void DebugInfo::GetSourceCode( std::string& str )
{
	str = mySourceText;
}



// serialization stuff
// virtual
bool DebugInfo::Write(CreaturesArchive &ar) const
{
	ar << mySourceText;
	ar << myAddressMap;

	return true;
}

// virtual
bool DebugInfo::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();
	if(version >= 3)
	{
		ar >> mySourceText;
		ar >> myAddressMap;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}
