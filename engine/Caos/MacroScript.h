// -------------------------------------------------------------------------
// Filename:    MacroScript.h
// Class:       MacroScript
// Purpose:     An Orderised (compiled) CAOS Macro
// Description:
//
// Simple holder class for containing and accessing a CAOS macro.
// Also stores a classifier (family/genus/species/event) for the macro.
// TODO - some bounds-checking ASSERTS might be nice.
//
// Usage:
//
//
// History:
// 30Nov98  BenC	Initial version
// 22Mar99  Robert	Added floating-point support/tidy up
// -------------------------------------------------------------------------


#ifndef MACROSCRIPT_H
#define MACROSCRIPT_H

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>
#include "../Classifier.h"
#include "../PersistentObject.h"

class DebugInfo;


class MacroScript : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( MacroScript )
public:
	// ---------------------------------------------------------------------
	// Method:      Constructor
	// Arguments:   code - Orderised macro data
	//				size - size of code, in bytes
	//				dbinfo - extra debugging information to store
	// Returns:     None
	// Description:	MacroScript makes a copy of the code data. However, the
	//				DebugInfo object (if non-null) is transferred to the
	//				MacroScript. ie the ownership of the DebugInfo is
	//				assigned to the MacroScript.
	// ---------------------------------------------------------------------
	MacroScript( unsigned char* code, int size, DebugInfo* dbinfo=NULL );

	// contructor for serialisation
	MacroScript();

	~MacroScript();

	// ---------------------------------------------------------------------
	// Method:      FetchInteger
	// Arguments:   addr - address to read from (will be updated)
	// Returns:     the read integer
	// Description: Reads an integer from the macroscript stream and updates
	//				the address.
	// ---------------------------------------------------------------------
	inline int FetchInteger ( int& addr ) const;


	// ---------------------------------------------------------------------
	// Method:      FetchFloat
	// Arguments:   addr - address to read from (will be updated)
	// Returns:     the read float
	// Description: Reads a float from the macroscript stream and updates
	//				the address.
	// ---------------------------------------------------------------------
	inline float FetchFloat ( int& addr ) const;

	// ---------------------------------------------------------------------
	// Method:      FetchOp
	// Arguments:   addr - address to read from (will be updated)
	// Returns:     an op id.
	// Description: TODO: use optype typedef instead of unsigned short!
	// ---------------------------------------------------------------------
	inline unsigned short FetchOp( int& addr ) const;

	
	// ---------------------------------------------------------------------
	// Method:		PeekOp
	// Arguments:   addr - address to read from (won't change)
	// Returns:     an op id.
	// Description: TODO: use optype typedef instead of unsigned short!
	// ---------------------------------------------------------------------
	inline unsigned short PeekOp( int addr ) const;


	// ---------------------------------------------------------------------
	// Method:      FetchString
	// Arguments:   addr - address of start of string (will be updated)
	//				str - where to store the string
	// Returns:     addr and str params are both updated.
	// Description: Reads out a string encoded in the macroscript, and
	//				updates the address counter (addr) appropriately.
	// ---------------------------------------------------------------------
	inline void FetchString( int& addr, std::string& str ) const;


	// ---------------------------------------------------------------------
	// Method:      RawData
	// Arguments:   addr - address of data to access
	// Returns:     pointer to address specified (will require a cast)
	// Description: Allows direct access to data stored in the macroscript.
	//				Should be used sparingly. NOTE: no bounds checking -
	//				it's assumed that the caller knows how much data it
	//				needs to read and that the macroscript doesn't terminate
	//				prematurely.
	// ---------------------------------------------------------------------
	const void* RawData( int addr ) const { return (const void*)(&myCode[ addr ]); }

	// ---------------------------------------------------------------------
	// Method:      GetClassifier
	// Arguments:   c - Classifier reference to store result
	// Returns:     classifier is returned in c
	// Description: Gets the family/genus/species/event classifier of the
	//				script.
	// ---------------------------------------------------------------------
	void GetClassifier( Classifier& c ) const
		{ c=myClassifier; }

	// ---------------------------------------------------------------------
	// Method:      SetClassifier
	// Arguments:   c - the new classifier
	// Returns:     None
	// Description: Sets the family/genus/species/event classifier of the
	//				script.
	// ---------------------------------------------------------------------
	void SetClassifier( const Classifier& c )
		{ myClassifier=c; };

	// ---------------------------------------------------------------------
	// Method:      GetDebugInfo
	// Arguments:   None
	// Returns:     A DebugInfo object ptr or NULL
	// Description: Returns the DebugInfo object held by this macro (if
	//				there is one)
	// ---------------------------------------------------------------------
	DebugInfo* GetDebugInfo()
		{ return myDebugInfo; }

	void Lock();
	void Unlock();
	bool IsLocked();



	// serialization
	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);


private:
	Classifier		myClassifier;
	unsigned char*	myCode;
	int32				mySize;
	DebugInfo*		myDebugInfo;
	int32				myReferenceCount;
};


inline int MacroScript::FetchInteger( int& addr )  const
{
	_ASSERT((addr >= 0) && (addr < mySize));

	int i = *((int*)(&myCode[ addr ]));
	addr += sizeof(int);
	return i;
}

inline float MacroScript::FetchFloat( int& addr )  const
{
	_ASSERT((addr >= 0) && (addr < mySize));

	float f = *((float*)(&myCode[ addr ]));
	addr += sizeof(float);
	return f;
}

inline unsigned short MacroScript::FetchOp( int& addr )  const
{
	_ASSERT((addr >= 0) && (addr < mySize));

	unsigned short u = *((unsigned short*)(&myCode[ addr ]));
	addr += sizeof(unsigned short);
	return u;
}


inline unsigned short MacroScript::PeekOp( int addr )  const
{
	_ASSERT((addr >= 0) && (addr < mySize));

	unsigned short u = *((unsigned short*)(&myCode[ addr ]));
	return u;
}


inline void MacroScript::FetchString( int& addr, std::string& str ) const
{
	_ASSERT((addr >= 0) && (addr < mySize));

	int len;
	str = (const char*)&myCode[ addr ];

	len = str.size() + 1;	// allow for null...
	if( len & 1 )			// ...and pad if odd
		len++;

	addr += len;
}



#endif	// MACROSCRIPT_H

