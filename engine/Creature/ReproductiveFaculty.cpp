// ReproductiveFaculty.cpp: implementation of the ReproductiveFaculty class.
//
//////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "ReproductiveFaculty.h"
#include "Creature.h"
#include "../Agents/Agent.h"
#include "Biochemistry/Biochemistry.h"
#include "../App.h"
#include "../World.h"


CREATURES_IMPLEMENT_SERIAL(ReproductiveFaculty)

#include "../Agents/AgentHandle.h"


// Threshold values for loc_ovulate receptor, controlling whether an egg/sperm is to be
// prepared or destroyed:
// The receptor acts like a thermostat with hysteresis - if the chemical level is high,
// the creature becomes fertile; if low, he/she becomes infertile. The hysteresis
// allows females to have cyclic ovulation - eg. oestrogen level can be caused to cycle
// between OVULATEOFF and OVULATEON by producing oestrogen when the loc_fertile emitter
// is OFF and allowing it to decay when it is ON.
// Males also use this receptor to control sperm generation - for example, a hormone
// level can rise and cause sperm to be produced, then fall to zero after sex to provide a
// recovery period.
const float OVULATEOFF = 0.314f;// become infertile if receptor o/p is less than this
const float OVULATEON = 0.627f;	// become fertile if receptor o/p is more than this


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       ReproductiveFaculty
// Description: 
// ------------------------------------------------------------------------
ReproductiveFaculty::ReproductiveFaculty()
{
	myGamete = false; // not fertile

	// clear biochemical emitter & receptor loci...
	myFertileLocus = 0.0f;
    myPregnancyLocus = 0.0f;
	myOvulateLocus = 0.0f;
	myReceptiveLocus = 0.0f;
	myChanceOfMutationLocus = 0.5f;
	myDegreeOfMutationLocus = 0.5f;
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       ReproductiveFaculty
// Description: 
// ------------------------------------------------------------------------
ReproductiveFaculty::~ReproductiveFaculty()
{

}






// ------------------------------------------------------------------------
// Function:    Update
// Class:       ReproductiveFaculty
// Description: 
// ------------------------------------------------------------------------
void ReproductiveFaculty::Update() {
    // Ovulate, fertility, pregnancy.
	// Respond to changes in myOvulateLocus receptor, by adding or removing an egg/sperm
	// from .Gamete, thus making the creature fertile or unfertile in response to chemistry.
	// The receptor acts like a thermostat with hysteresis - if the chemical level is high,
	// the creature becomes fertile; if low, he/she becomes infertile. The hysteresis
	// allows females to have cyclic ovulation - eg. oestrogen level can be caused to cycle
	// between OVULATEOFF and OVULATEON by producing oestrogen when the myFertileLocus emitter
	// is OFF and allowing it to decay when it is ON.
	// Males also use this receptor to control sperm generation - for example, a hormone
	// level can rise and cause sperm to be produced, then fall to zero after sex to provide a
	// recovery period.
	if	((myGamete) && (myOvulateLocus < OVULATEOFF))
		myGamete = false;
	else if	((!myGamete) && (myOvulateLocus > OVULATEON))
		myGamete = true;
	// If .myGamete is present (sperm or egg), then you are fertile
	myFertileLocus = (myGamete) ? 1.0f : 0;

	// Note in locus if pregnant
	myPregnancyLocus = (IsPregnant()) ? 1.0f : 0;
}

// Pregnant if our genome store slot 1 has a zygote in it
// Twins and triplets and so on would be in slots 2, 3, 4, ...
// But slot 1 should always be full when pregnant
// ------------------------------------------------------------------------
// Function:    IsPregnant
// Class:       ReproductiveFaculty
// Description: 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool ReproductiveFaculty::IsPregnant()
{
	return !(GetCreatureOwner().GetAgentReference().GetGenomeStore().MonikerAsString(1).empty());
}

//////////////////////////////////////////////////////////////////////////////
// REPRODUCTION:

// Reproduction: Attempt to inseminate the IT object (if IT is a female of
// the same genus!). Do this by calling her AcceptSperm() function.
// ------------------------------------------------------------------------
// Function:    DonateSperm
// Class:       ReproductiveFaculty
// Description: 
// ------------------------------------------------------------------------
void ReproductiveFaculty::DonateSperm()
{
	// IT object's classifier must be CREATURE:same genus as you:FEMALE:00
	// if she's an acceptable target!
	Classifier C(myCreature.GetCreatureReference().GetClassifier().Family(), myCreature.GetCreatureReference().GetClassifier().Genus(), FEMALE);
	// if IT is a female of your genus

	AgentHandle it = myCreature.GetCreatureReference().GetItAgent();
	if ((it.IsValid()) && it.GetAgentReference().GetClassifier() == C) {
		// send her any prepared sperm
		it.GetCreatureReference().Reproductive()->AcceptSperm(GetCreatureOwner(), myChanceOfMutationLocus, myDegreeOfMutationLocus);
		myGamete = false;					// shot your bolt now
	}
}

// Reproduction: A male is passing possible sperm to a female.
// The function is only called for females. The parameter is a
// male's handle.
// The female should decide whether fertilisation is possible, and do this
// by calling GenomeStore::CrossoverFrom() to create and store a child's moniker
// in genome store 1.

// NOTE: As well as the various biological necessities such as sperm and egg, another
// control over successful fertilisation is the value on the female's
// loc_receptive receptor. This allows chemistry to give the female some say in the
// issue. For example, this receptor could be driven by the sex drive, which might be
// raised by some pheromone emitted by the male during a courtship dance. Alternatively,
// the Nineties female might self-excite this receptor during her own mating act, thus
// preventing conception unless BOTH partners are actively engaged in sex.
//#include <map>
// ------------------------------------------------------------------------
// Function:    AcceptSperm
// Class:       ReproductiveFaculty
// Description: 
// Arguments:   AgentHandle dad = 
//              byte DadChanceOfMutation = 
//              byte DadDegreeOfMutation = 
// ------------------------------------------------------------------------
void ReproductiveFaculty::AcceptSperm(AgentHandle dad, byte DadChanceOfMutation, byte DadDegreeOfMutation)
{
	if ((myGamete)&&							// If a fertile egg is present
		(dad.GetCreatureReference().Reproductive()->myGamete)&&								// and a sperm is present
		(!IsPregnant()))							// and not already pregnant
	{											
		if (RndFloat() < myReceptiveLocus)			// then probablistically,
		{
			int birthCount;
//std::map <int, int> count;
//for (int k = 0; k < 100000; ++k)
//{
			birthCount = 1;
			int maxBirths = theApp.GetWorld().GetGameVar("engine_multiple_birth_maximum").GetInteger();
			if (maxBirths < 1)
				maxBirths = 1;
			if (RndFloat() < theApp.GetWorld().GetGameVar("engine_multiple_birth_first_chance").GetFloat())
			{
				birthCount++;
				while (birthCount < maxBirths
					&& RndFloat() < theApp.GetWorld().GetGameVar("engine_multiple_birth_subsequent_chance").GetFloat())
				{
					birthCount++;
				}
			}
			if (birthCount > maxBirths)
				birthCount = maxBirths;

//if (count.find(birthCount) == count.end())
//  count[birthCount] =0;
//else
//  count[birthCount]++;
//}
//for (std::map <int, int>::iterator it = count.begin(); it != count.end(); ++it)
//{
//OutputFormattedDebugString("Multiple %d birth occured %d\n", it->first, it->second);
//}

			bool identical = RndFloat() < theApp.GetWorld().GetGameVar("engine_multiple_birth_identical_chance").GetFloat();
			for (int birth = 0; birth < birthCount; ++birth)
			{
				// a happy event is expected!
				if (birth == 0 || !identical)
				{
					// non-identical twin, or first of identical set
					bool result = GetCreatureOwner().GetAgentReference().GetGenomeStore().CrossoverFrom(birth + 1,		// genome store to contain zygote
								GetCreatureOwner().GetAgentReference().GetGenomeStore(), 0,		// cross our actual genome
								dad.GetAgentReference().GetGenomeStore(), 0, // with dad's actual one
								myChanceOfMutationLocus, myDegreeOfMutationLocus,
								DadChanceOfMutation, DadDegreeOfMutation,
								true);
					ASSERT(result);
				}
				else
				{
					// identical twin
					bool result = GetCreatureOwner().GetAgentReference().GetGenomeStore().IdenticalTwinFrom(birth + 1,		// genome store to contain zygote
								GetCreatureOwner().GetAgentReference().GetGenomeStore(), 0,		// cross our actual genome
								dad.GetAgentReference().GetGenomeStore(), 0, // with dad's actual one
								GetCreatureOwner().GetAgentReference().GetGenomeStore().MonikerAsString(1));
					ASSERT(result);
				}
			}
		}
	}
}





// ------------------------------------------------------------------------
// Function:    GetLocusAddress
// Class:       ReproductiveFaculty
// Description: 
// Arguments:   int type = 
//              int organ = 
//              int tissue = 
//              int locus = 
// Returns:     float* = 
// ------------------------------------------------------------------------
float* ReproductiveFaculty::GetLocusAddress(int type, int organ, int tissue, int locus) {
	if (organ!=ORGAN_CREATURE)
		return NULL;

	if (tissue==TISSUE_REPRODUCTIVE) {
		if (type==RECEPTOR) {
			switch (locus) {
			case LOC_OVULATE:
				return &myOvulateLocus;
			case LOC_RECEPTIVE:
				return &myReceptiveLocus;
			case LOC_CHANCEOFMUTATION:
				return &myChanceOfMutationLocus;
			case LOC_DEGREEOFMUTATION:
				return &myDegreeOfMutationLocus;
			}
		} else {
			switch (locus) {
				case LOC_FERTILE:
					return &myFertileLocus;
				case LOC_PREGNANT:
					return &myPregnancyLocus;
				case LOC_E_OVULATE:
					return &myOvulateLocus;
				case LOC_E_RECEPTIVE:
					return &myReceptiveLocus;
				case LOC_E_CHANCEOFMUTATION:
					return &myChanceOfMutationLocus;
				case LOC_E_DEGREEOFMUTATION:
					return &myDegreeOfMutationLocus;

			}
		}
	}
	return NULL;
}


// ------------------------------------------------------------------------
// Function:    Write
// Class:       ReproductiveFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool ReproductiveFaculty::Write(CreaturesArchive &archive) const {
	base::Write( archive );
	archive << myGamete;

	archive.WriteFloatRefTarget( myFertileLocus );
	archive.WriteFloatRefTarget( myPregnancyLocus );
	archive.WriteFloatRefTarget( myOvulateLocus );
	archive.WriteFloatRefTarget( myReceptiveLocus );
	archive.WriteFloatRefTarget( myChanceOfMutationLocus );
	archive.WriteFloatRefTarget( myDegreeOfMutationLocus );
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       ReproductiveFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool ReproductiveFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!base::Read( archive ))
			return false;

		archive >> myGamete;

		archive.ReadFloatRefTarget( myFertileLocus );
		archive.ReadFloatRefTarget( myPregnancyLocus );
		archive.ReadFloatRefTarget( myOvulateLocus );
		archive.ReadFloatRefTarget( myReceptiveLocus );
		archive.ReadFloatRefTarget( myChanceOfMutationLocus );
		archive.ReadFloatRefTarget( myDegreeOfMutationLocus );
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


// ------------------------------------------------------------------------
// Function:    GetProgesteroneLevel
// Class:       ReproductiveFaculty
// Description: 
// Returns:     float = 
// ------------------------------------------------------------------------
float ReproductiveFaculty::GetProgesteroneLevel()
{
	// calculate a pregnancy state from the level of progesterone.
	// the chemical level rises rapidly and then tails off towards 255
	// so these stages will need tweaking
	float progesterone = myCreature.GetCreatureReference().GetBiochemistry()->GetChemical(CHEM_PROGESTERONE);
	return progesterone;
}
 
