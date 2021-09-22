// -------------------------------------------------------------------------
// Filename:    DebugInfo.h
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

#ifndef DEBUGINFO_H
#define DEBUGINFO_H

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif


#include <string>
#include <map>

#include "../PersistentObject.h"

class DebugInfo : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( DebugInfo )
public:
	DebugInfo::DebugInfo( const char* srctext )
		{ mySourceText = srctext; }
	DebugInfo() {}

	void AddAddressToPositionMapping( int address, int srcpos );

	int MapAddressToSource( int address );

	void GetSourceCode( std::string& str );

	// serialization stuff
	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);

private:
	std::string mySourceText;
	std::map< int, int > myAddressMap;	// map address to position in source
};


#endif // DEBUGINFO_H


