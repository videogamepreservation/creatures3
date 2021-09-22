// -------------------------------------------------------------------------
// Filename:    Scriptorium.h
// Class:       Scriptorium
// Purpose:     Repository for macroscripts used by agents in the game.
// Description: This stores all of the macroscripts for the game.
//				This class is heavily used every time the game tries to
//				execute an event, or trigger a script.
//
// Usage:		MacroScript* FindScript(Classifier&) (need I say more?)
//
// History:
//		(Someone)	Created / upgraded to STL
//	18Aug99	Daniel	Made intensely better with nice vectors
//  20Aug99 Daniel	Made (hopefully) up to coding standards
// -------------------------------------------------------------------------

#ifndef SCRIPTORIUM_H
#define SCRIPTORIUM_H


// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <vector>
#include <set>

#include "../PersistentObject.h"
#include "../Classifier.h"

// Forward Declaration
class MacroScript;

// Typedefs
typedef std::set<int> IntegerSet;


class Scriptorium : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( Scriptorium )
public:
	// ---------------------------------------------------------------------
	// Method:      InstallScript
	// Arguments:   script - classified MacroScript to install
	// Returns:     true if the script was successfully installed.
	// Description: Assigns ownership of a script to the scriptorium
	//				if a script with the same classifier already exists,
	//				it will be replaced, and the old script will be
	//				deleted.
	//				However, if the existing script is still in use (ie
	//				locked), this function will fail.
	// ---------------------------------------------------------------------
	bool InstallScript( MacroScript* script );

	// ---------------------------------------------------------------------
	// Method:      ZapScript
	// Arguments:   c - classifier of script to remove
	// Returns:     true for success, or false if an attempt was made to
	//				remove a locked script.
	// Description:	Remove/delete script matching the classifier.
	//				NOTE - if no matching script is found, this function
	//				will return success.
	// ---------------------------------------------------------------------
	bool ZapScript( const Classifier& c );

	// ---------------------------------------------------------------------
	// Method:      FindScript
	// Arguments:   c - classifier of script(s) to find
	// Returns:     Best matching script.
	// Description:	find a suitable event script within the scriptorium
	//				If a perfect matching script is not present,
	//				FindScript() tries to fall back to general event
	//				scripts for the entire genus, or even family.
	//				Returns NULL if no suitable script is available.
	// ---------------------------------------------------------------------
	MacroScript* FindScript( const Classifier& c );


	// ---------------------------------------------------------------------
	// Method:      FindScriptExact
	// Arguments:   c - classifier of script(s) to remove
	// Returns:     ptr to script or null
	// Description:	Find a script with a particular classifier.
	//				Unlike FindScript(), there is no falling back to more
	//				general scripts.
	// ---------------------------------------------------------------------
	MacroScript* Scriptorium::FindScriptExact( const Classifier& c );


	// ---------------------------------------------------------------------
	// Method:		DumpFamilyIDs
	// Arguments:	IntegerSet& s
	// Returns:		(None)
	// Description:	Puts the families for which there are scripts, into "s"
	// ---------------------------------------------------------------------
	void DumpFamilyIDs( IntegerSet& s );

	// ---------------------------------------------------------------------
	// Method:		DumpGenusIDs
	// Arguments:	IntegerSet& s, int family
	// Returns:		(None)
	// Description:	Puts the genuses for "family" into "s"
	// ---------------------------------------------------------------------
	void DumpGenusIDs( IntegerSet& s, int family );

	// ---------------------------------------------------------------------
	// Method:		DumpSpeciesIDs
	// Arguments:	IntegerSet& s, int family, int genus
	// Returns:		(None)
	// Description:	Puts the species for "family/genus" into "s"
	// ---------------------------------------------------------------------
	void DumpSpeciesIDs( IntegerSet& s, int family, int genus );

	// ---------------------------------------------------------------------
	// Method:		DumpEventIDs
	// Arguments:	IntegerSet& s, int family, int genus, int species
	// Returns:		(None)
	// Description:	Puts the events for "family/species/genus" into "s"
	// ---------------------------------------------------------------------
	void DumpEventIDs( IntegerSet& s, int family, int genus, int species );

	
	// ---------------------------------------------------------------------
	// Constructor:	Builds a new Scriptorium
	// ---------------------------------------------------------------------
	Scriptorium();

	// ---------------------------------------------------------------------
	// Destructor:	Deletes all the scripts in, and the, scriptorium
	// ---------------------------------------------------------------------
	~Scriptorium();

	// ---------------------------------------------------------------------
	// Method:		Clear
	// Arguments:	(None)
	// Returns:		(None)
	// Description:	Deletes all the scripts and empties the scriptorium
	// ---------------------------------------------------------------------
	void Clear();

	// serialization
	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);

private:
	typedef std::vector< MacroScript* > ScriptList;
	typedef ScriptList::iterator ScriptIterator;
	typedef ScriptList::const_iterator ScriptConstIterator;
	typedef std::vector< int32 > EventList;
	typedef std::vector< EventList > SpeciesList;
	typedef std::vector< SpeciesList > GenusList;
	typedef std::vector< GenusList > FamilyList;
	
	ScriptList myScripts;
	FamilyList myScriptoriumEntries;

};


#endif // SCRIPTORIUM_H
