/*********************************************************************
* File:     Classifier.cpp
* Created:  5/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"
#include "Classifier.h"
#include "General.h"
#include "../common/Catalogue.h"

CREATURES_IMPLEMENT_SERIAL( Classifier )


/*********************************************************************
* Public: Default Constructor.
*********************************************************************/
Classifier::Classifier()
{
	myFamily = myGenus = 0;
	mySpecies = myEvent = 0;
}


/*********************************************************************
* Public: Constructor.
*********************************************************************/
Classifier::Classifier(uint32 Family, uint32 Genus, uint32 Species, uint32 Event)
{
	myFamily = Family;
	myGenus = Genus;
	mySpecies = Species;
	myEvent = Event;
}

/*********************************************************************
* Public: Copy constructor.
*********************************************************************/
Classifier::Classifier(const Classifier& rOther)
{
	myFamily = rOther.myFamily;
	mySpecies = rOther.mySpecies;
	myGenus = rOther.myGenus;
	myEvent = rOther.myEvent;
}


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool Classifier::Write(CreaturesArchive &rArchive) const
	{

	rArchive << myFamily << myGenus << mySpecies << myEvent;

	return true;
	}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Classifier::Read(CreaturesArchive &rArchive)
{
	// Check version info
	int32 version = rArchive.GetFileVersion();

	if ( version >= 3 )
	{
		rArchive >> myFamily >> myGenus >> mySpecies >> myEvent;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;

}

/*********************************************************************
* Public: SetEvent.
*********************************************************************/
void Classifier::SetEvent(uint16 wEvent)
{
	myEvent = wEvent;
}


/*********************************************************************
* Public: Equality operator.
*********************************************************************/
bool Classifier::operator ==(const Classifier& rOther) const
{
	return (myFamily == rOther.myFamily) && (mySpecies == rOther.mySpecies) &&
		(myGenus == rOther.myGenus) && (myEvent == rOther.myEvent);
}


/*********************************************************************
* Public: Less than operator.
*********************************************************************/
bool Classifier::operator <(const Classifier& rOther) const
{
	if ((myFamily < rOther.myFamily) || (myFamily == 0))
		return true;
	if (myFamily > rOther.myFamily)
		return false;
	if ((myGenus < rOther.myGenus)  || (myGenus == 0))
		return true;
	if (myGenus > rOther.myGenus)
		return false;
	if ((mySpecies < rOther.mySpecies) || (mySpecies == 0))
		return true;
	if (mySpecies > rOther.mySpecies)
		return false;

	// Do we want this?
	if (myEvent < rOther.myEvent)
		return true;
	if (myEvent > rOther.myEvent)
		return false;

	_ASSERT(*this == rOther);

	return false;
}

/*********************************************************************
* Public: GenericMatchFor.
* rOther can contain 0x00 as wildcard value.
*********************************************************************/
BOOL Classifier::GenericMatchForWildCard(const Classifier& rOther,
	BOOL bFamily, BOOL bGenus, BOOL bSpecies, BOOL bEvent) const
{
	BOOL bEqual = TRUE;

	if (bFamily)
		bEqual &= (Family() == rOther.Family()) || (rOther.Family() == 0);

	if (bGenus)
		bEqual &= (Genus() == rOther.Genus()) || (rOther.Genus() == 0);

	if (bSpecies)
		bEqual &= (Species() == rOther.Species()) || (rOther.Species() == 0);

	if (bEvent)
		bEqual &= (Event() == rOther.Event()) || (rOther.Event() == 0);

	return bEqual;
}


/*********************************************************************
* Public: Assignment operator.
*********************************************************************/
/*
const Classifier& Classifier::operator =(const Classifier& rOther)
{
	myFamily = rOther.myFamily;
	myGenus = rOther.myGenus;
	mySpecies = rOther.mySpecies;
	myEvent = rOther.myEvent;

	return rOther;
}
*/

void Classifier::StreamClassifier(std::ostream& out, bool event) const
{
	out << (int)Family() << " " << (int)Genus() << " "
		<< (int)Species();
	if (event)
		out << " " << (int)Event();
}


// This is here to prevent the need to include practically the entire engine to compile the
// CAOS Debugger tool.
#ifndef _CAOSDEBUGGER

void Classifier::StreamAgentNameIfAvailable(std::ostream& out) const
{
	std::string final_tag = WildSearch(Family(), Genus(), Species(), "Agent Help");
	if (!final_tag.empty())
	{
		out << " (" << theCatalogue.Get(final_tag, 0) << ")";
	}

}
#endif
