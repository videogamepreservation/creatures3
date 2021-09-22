#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MacroScript.h"
#include "DebugInfo.h"
#include "../C2eServices.h"	// for logging
#include "Orderiser.h"

#include <stdlib.h>
#include <memory.h>	// for memcpy()


CREATURES_IMPLEMENT_SERIAL( MacroScript )


MacroScript::MacroScript( unsigned char* code, int size, DebugInfo* dbinfo )
{
	myCode = new unsigned char[ size ];
	mySize = size;
	myDebugInfo = dbinfo;
	myReferenceCount = 0;

	memcpy( myCode, code ,size );
}


MacroScript::MacroScript()
{
	mySize = 0;
	myCode = NULL;
	myDebugInfo = NULL;
	myReferenceCount = 0;
}




MacroScript::~MacroScript()
{
	_ASSERT( myReferenceCount == 0 );

	if( myCode != NULL && mySize > 0 )
	{
		delete [] myCode;
		myCode = NULL;
		mySize = 0;
	}

	if( myDebugInfo )
	{
		delete myDebugInfo;
		myDebugInfo = NULL;
	}
	
	myReferenceCount = 0;

}


void MacroScript::Lock()
{
	++myReferenceCount;
	if( myReferenceCount != 1 )
		return;

}

void MacroScript::Unlock()
{
	--myReferenceCount;
	ASSERT( myReferenceCount >= 0 );
	if( myReferenceCount != 0 )
		return;
}


bool MacroScript::IsLocked()
{
	return( myReferenceCount > 0 );
}





// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes state to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool MacroScript::Write(CreaturesArchive &archive) const
{
	archive << (int)0;			// reserved
	myClassifier.Write( archive );
	archive << myReferenceCount;
	
	archive << mySize;
	archive.Write( myCode, mySize );
	
	archive << myDebugInfo;

	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads obhect state from archive
// ----------------------------------------------------------------------
bool MacroScript::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{

		int crap;

		archive >> crap;
		myClassifier.Read( archive );
		archive >> myReferenceCount;
		
		archive >> mySize;
		myCode = new uint8[mySize];
		archive.Read( myCode, mySize );
		
		archive >> myDebugInfo;
		
		// Now re-orderise the CAOS
		Orderiser o;
		std::string src;
		myDebugInfo->GetSourceCode(src);
		MacroScript* newCAOS = o.OrderFromCAOS(src.c_str());
		
		// If the new CAOS is a different size to this CAOS then problems may ensue
		_ASSERT(newCAOS);
		_ASSERT(newCAOS->mySize == mySize);

		delete [] myCode;
		delete myDebugInfo;

		myCode = newCAOS->myCode;
		mySize = newCAOS->mySize;
		myDebugInfo = newCAOS->myDebugInfo;
		newCAOS->myCode = NULL;
		newCAOS->myDebugInfo = NULL;
		newCAOS->mySize = 0;		
		delete newCAOS;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}

