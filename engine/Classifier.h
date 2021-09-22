/*********************************************************************
* File:     Classifier.h
* Created:  5/01/98
* Author:   Robin E. Charlton
*********************************************************************/

#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "PersistentObject.h"

#include <iostream>

/*********************************************************************
* Constants.
*********************************************************************/

// G_NORN, G_GRENDEL, G_ETTIN - I think special behaviour for
// these three should be genetically specified, or be in the scripts.
// There's only the music code where they're still used, but I've got
// a feeling they might be explicit constants elsewhere.  Oh well.
const uint8		G_NORN				= 0x01;	// all pets are of this genus
const uint8		G_GRENDEL			= 0x02;
const uint8		G_ETTIN 			= 0x03;

/*********************************************************************
* class Classifier.
*********************************************************************/
class Classifier : public PersistentObject
{
	CREATURES_DECLARE_SERIAL(Classifier);

public:
	uint32 myFamily;
	uint32 myGenus;
	uint32 mySpecies;
	uint32 myEvent;

public:
	Classifier();
	Classifier(uint32 Family, uint32 Genus = 0, uint32 Species = 0, uint32 Event = 0);
	Classifier(const Classifier& rOther);

	// ----------------------------------------------------------------------
	// Method:		Write
	// Arguments:	archive - archive being written to
	// Returns:		true if successful
	// Description:	Overridable function - writes details to archive,
	//				taking serialisation into account
	// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;


	// ----------------------------------------------------------------------
	// Method:		Read
	// Arguments:	archive - archive being read from
	// Returns:		true if successful
	// Description:	Overridable function - reads detail of class from archive
	// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);

	// Access methods.
	inline uint8 Family() const
	{	return myFamily;}

	inline uint8 Genus() const
	{	return myGenus;}

	inline uint16 Species() const
	{	return mySpecies;}

	inline uint16 Event() const
	{	return myEvent;}

	// Wildcard support.
	Classifier GenericSpecies() const
	{	return Classifier(Family(), Genus(), 0, Event());}

	Classifier GenericGenus() const
	{	return Classifier(Family(), 0, Species(), Event());}

	Classifier GenericGenusSpecies() const
	{	return Classifier(Family(), 0, 0, Event());}

	void SetEvent(uint16 wEvent);


	inline bool EqualsZeroingSpeciesOf(const Classifier& other) const {
		return (myFamily == other.myFamily) && (myGenus == other.myGenus) && 
			(mySpecies == 0) && (myEvent == other.myEvent);
			
	}
	inline bool EqualsZeroingGenusOf(const Classifier& other) const {
		return (myFamily == other.myFamily) && (myGenus == 0) &&
			(mySpecies == other.mySpecies) && (myEvent == other.myEvent);
			
	}
	inline bool EqualsZeroingSpeciesAndGenusOf(const Classifier& other) const {
		return (myFamily == other.myFamily) && (myGenus == 0) &&
			(mySpecies == 0) && (myEvent == other.myEvent);
	}

	bool operator ==(const Classifier& rOther) const;
	bool operator <(const Classifier& rOther) const;
	BOOL GenericMatchForWildCard(const Classifier& rOther, 
		BOOL bFamily = TRUE, BOOL bGenus = TRUE, 
		BOOL bSpecies = TRUE, BOOL bEvent = TRUE) const;

	// const Classifier& operator =(const Classifier& rOther);

	void StreamClassifier(std::ostream& out, bool event = true) const;
	void StreamAgentNameIfAvailable(std::ostream& out) const;
};


#endif // CLASSIFIER_H