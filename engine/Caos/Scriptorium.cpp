// -------------------------------------------------------------------------
// Filename:    Scriptorium.cpp
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

// GCC + SGI STL NOTE:
// Compiling this file under gcc (which comes with SGIs stl) causes the
// ansi template instantiation depth limit (17) to be exceeded.
//
// Not sure if this is due to gcc or SGIs stl implementation.
// Not sure what Visual C is doing - either not hitting the ansi limit
// or not saying anything when it does (I'd suspect the latter).
//
// Quick fix for gcc is to compile with -ftemplate-depth-30
// (using gcc 2.91.66)
//
// BenC


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"
#include "Scriptorium.h"
#include "MacroScript.h"


CREATURES_IMPLEMENT_SERIAL( Scriptorium )

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
bool Scriptorium::InstallScript( MacroScript* script)
{
	Classifier c;
	script->GetClassifier(c);
	// Try finding it - if it's not locked, we can replace, else abort :)
	MacroScript* m = FindScriptExact(c);
	bool fail = true;
	if (m == NULL)
		fail = false;						// No script, so can use it :)
	if ((m!=NULL) && (!m->IsLocked()))
		fail = false;						// Script here but not locked - can replace :)
	
	// There is a script, and it is locked, so we fail :)
	if (fail)
		return false;
	if (m == NULL)
	{
		// We have to ensure that the vectors are okay :)
		if (myScriptoriumEntries.size() <= c.Family())
			myScriptoriumEntries.resize(c.Family()+1);

		if (myScriptoriumEntries[c.Family()].size() <= c.Genus())
			myScriptoriumEntries[c.Family()].resize(c.Genus() + 1);

		if (myScriptoriumEntries[c.Family()][c.Genus()].size() <= c.Species())
			myScriptoriumEntries[c.Family()][c.Genus()].resize(c.Species() + 1);

		if (myScriptoriumEntries[c.Family()][c.Genus()][c.Species()].size() <= c.Event())
			myScriptoriumEntries[c.Family()][c.Genus()][c.Species()].resize(c.Event() + 1, -1);
		
		// And add the script
		myScripts.push_back(script);
		myScriptoriumEntries[c.Family()][c.Genus()][c.Species()][c.Event()] = myScripts.size() - 1;
		ASSERT(myScripts[myScriptoriumEntries[c.Family()][c.Genus()][c.Species()][c.Event()]] == script);
		return true; // Woohoo - done and diddled :):)
	}

	// Else it is already there, so all we need to do is get the iterator on the list & replace :)
	int si = myScriptoriumEntries[c.Family()][c.Genus()][c.Species()][c.Event()];
	delete myScripts[si]; // Remove the old script
	myScripts[si] = script; // Add new one in
	return true; // Wasn't that easy then?

}

// ---------------------------------------------------------------------
// Method:      ZapScript
// Arguments:   c - classifier of script to remove
// Returns:     true for success, or false if an attempt was made to
//				remove a locked script.
// Description:	Remove/delete script matching the classifier.
//				NOTE - if no matching script is found, this function
//				will return success.
// ---------------------------------------------------------------------
bool Scriptorium::ZapScript( const Classifier& c )
{
	// Try finding it - if it's not locked, we can delete it
	MacroScript* m = FindScriptExact(c);
	if (m == NULL)
		return false; // Not found :(
	if (m->IsLocked())
		return false; // Script Locked :(
	// Hmm we can zap it :):)
	myScripts[myScriptoriumEntries[c.Family()][c.Genus()][c.Species()][c.Event()]] = NULL; // Remove from list
	myScriptoriumEntries[c.Family()][c.Genus()][c.Species()][c.Event()] = -1;// Make mapping to end()
	delete m; // and delete that macroscript :)
	return true;
}


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
MacroScript* Scriptorium::FindScript( const Classifier& c )
{
	// Must use FindScriptExact falling back through the classification :)
	MacroScript* returnValue = NULL;
	Classifier d = c;

	returnValue = FindScriptExact(d);
	if (returnValue != NULL)
		return returnValue;

	d.mySpecies = 0;
	returnValue = FindScriptExact(d); // If not exact, what about speciesWild?
	if (returnValue != NULL)
		return returnValue;

	d.myGenus = 0;
	returnValue = FindScriptExact(d); // Darn - how about GenusWild?
	if (returnValue != NULL)
		return returnValue;

	d.myFamily = 0;
	returnValue = FindScriptExact(d); // And How about Family Wild too?

	return returnValue; // Report what we have :)
}

// ---------------------------------------------------------------------
// Method:      FindScriptExact
// Arguments:   c - classifier of script(s) to remove
// Returns:     ptr to script or null
// Description:	Find a script with a particular classifier.
//				Unlike FindScript(), there is no falling back to more
//				general scripts.
// ---------------------------------------------------------------------
MacroScript* Scriptorium::FindScriptExact( const Classifier& c )
{
	if (c.Family() >= (myScriptoriumEntries.size()))
		return NULL; // No family by that number
	if (c.Genus() >= (myScriptoriumEntries[c.Family()].size()))
		return NULL; // No Genus in that family
	if (c.Species() >= (myScriptoriumEntries[c.Family()][c.Genus()].size()))
		return NULL; // No Species in that Genus
	if (c.Event() >= (myScriptoriumEntries[c.Family()][c.Genus()][c.Species()].size()))
		return NULL; // No event in that species
	if (myScriptoriumEntries[c.Family()][c.Genus()][c.Species()][c.Event()] == -1)
		return NULL; // No entry for that FGSE
	return myScripts[(myScriptoriumEntries[c.Family()][c.Genus()][c.Species()][c.Event()])];
}

// ---------------------------------------------------------------------
// Constructor:	Builds a new Scriptorium
// ---------------------------------------------------------------------
Scriptorium::Scriptorium()
{
}

// ---------------------------------------------------------------------
// Destructor:	Deletes all the scripts in, and the, scriptorium
// ---------------------------------------------------------------------
Scriptorium::~Scriptorium()
{
	Clear();
}

// ---------------------------------------------------------------------
// Method:		Clear
// Arguments:	(None)
// Returns:		(None)
// Description:	Deletes all the scripts and empties the scriptorium
// ---------------------------------------------------------------------
void Scriptorium::Clear()
{
	ScriptIterator it;
	myScriptoriumEntries.clear();
	for(it = myScripts.begin(); it != myScripts.end(); ++it)
	{
		delete *it;
	}
	myScripts.clear();

}

// ---------------------------------------------------------------------
// Method:		DumpFamilyIDs
// Arguments:	IntegerSet& s
// Returns:		(None)
// Description:	Puts the families for which there are scripts, into "s"
// ---------------------------------------------------------------------
void Scriptorium::DumpFamilyIDs( IntegerSet& s )
{
	ScriptIterator it;
	Classifier c;

	for( it=myScripts.begin(); it!=myScripts.end(); ++it )
	{
		if (*it)
		{
			(*it)->GetClassifier( c );
			s.insert( c.Family() );
		}
	}
}

// ---------------------------------------------------------------------
// Method:		DumpGenusIDs
// Arguments:	IntegerSet& s, int family
// Returns:		(None)
// Description:	Puts the genuses for "family" into "s"
// ---------------------------------------------------------------------
void Scriptorium::DumpGenusIDs( IntegerSet& s, int family )
{
	ScriptIterator it;
	Classifier c;

	for( it=myScripts.begin(); it!=myScripts.end(); ++it )
	{
		if (*it)
		{
			(*it)->GetClassifier( c );
			if( c.Family() == family )
			{
				s.insert( c.Genus() );
			}
		}
	}
}

// ---------------------------------------------------------------------
// Method:		DumpSpeciesIDs
// Arguments:	IntegerSet& s, int family, int genus
// Returns:		(None)
// Description:	Puts the species for "family/genus" into "s"
// ---------------------------------------------------------------------
void Scriptorium::DumpSpeciesIDs( IntegerSet& s, int family, int genus )
{
	ScriptIterator it;
	Classifier c;

	for( it=myScripts.begin(); it!=myScripts.end(); ++it )
	{
		if (*it)
		{
			(*it)->GetClassifier( c );
			if( c.Family() == family &&
				c.Genus() == genus )
			{
				s.insert( c.Species() );
			}
		}
	}
}

// ---------------------------------------------------------------------
// Method:		DumpEventIDs
// Arguments:	IntegerSet& s, int family, int genus, int species
// Returns:		(None)
// Description:	Puts the events for "family/species/genus" into "s"
// ---------------------------------------------------------------------
void Scriptorium::DumpEventIDs( IntegerSet& s, int family, int genus, int species )
{
	ScriptIterator it;
	Classifier c;

	for( it=myScripts.begin(); it!=myScripts.end(); ++it )
	{
		if (*it)
		{
			(*it)->GetClassifier( c );
			if( c.Family() == family &&
				c.Genus() == genus &&
				c.Species() == species )
			{
				s.insert( c.Event() );
			}
		}
	}
}


// serialization
bool Scriptorium::Write(CreaturesArchive &ar) const
{
	ScriptConstIterator it;

	ar << (int)0;	// reserved
	ar << (int)myScripts.size();
	for( it = myScripts.begin(); it != myScripts.end(); ++it )
	{
		ar << (*it);
	}
	return true;
}

// serialization
bool Scriptorium::Read(CreaturesArchive &ar)
{
	int crap;
	int count;
	int i;
	MacroScript* m;

	myScripts.clear();

	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{
		ar >> crap;	// reserved
		ar >> count;
		for( i=0; i<count; ++i )
		{
			ar >> m;
			// Now insert it properly into the lists :)
			if (m != NULL)
				InstallScript(m);
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}

