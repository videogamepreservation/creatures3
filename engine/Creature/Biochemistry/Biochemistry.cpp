/*********************************************************************
* File:     Biochemistry.cpp
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifdef _MSC_VER
#pragma warning(disable: 4503 4786)
#endif

#include "Biochemistry.h"

#include "Organ.h"
#include "../Genome.h"
#include "../Creature.h"
#include "../LifeFaculty.h"


CREATURES_IMPLEMENT_SERIAL(Biochemistry)




float Biochemistry::GetChemical(int chem)					// return current concentration of a chemical
{	return myChemicalConcs[chem];}

void Biochemistry::SetChemical(int chem, float amount)	// set abs concentration of a chemical
{
	myChemicalConcs[chem] = BoundIntoZeroOne(amount);
}

void Biochemistry::AddChemical(int chem,float amount)		// add to the concentration of given chemical
{											// UNLESS chem=0 ("none")
	if (chem) 
		myChemicalConcs[chem] = BoundIntoZeroOne(myChemicalConcs[chem]+amount); 
} 

void Biochemistry::SubChemical(int chem,float amount)		// reduce the concentration of given chemical
{											// UNLESS chem=0 ("none")
	if (chem) 
		myChemicalConcs[chem] = BoundIntoZeroOne(myChemicalConcs[chem]-amount);
} 


/*********************************************************************
* Public: Default constuctor.
*********************************************************************/
Biochemistry::Biochemistry()
{
    myATPRequirement = 0.0f;
	myNoOfOrgans = 0;
	for (int i=0; i<MAXORGANS; i++)
		myOrgans[i] = NULL;
	myNoOfNeuroEmitters = 0;
	for (int k=0; k<NUMCHEM; k++) {
		myChemicalConcs[k] = 0.0f;
		myChemicalDecayRates[k] = 0.0f;
	}
}



/*********************************************************************
* Public: Destructor.
*********************************************************************/
Biochemistry::~Biochemistry()
{
	int i;
	for ( i=0; i<myNoOfOrgans; i++) {
		delete myOrgans[i];
		myOrgans[i] = NULL;
	}
}



bool Biochemistry::Write(CreaturesArchive &archive) const {
	int k,o,i;

	Faculty::Write( archive );
	for ( k=0; k<NUMCHEM; k++) {
		archive << myChemicalDecayRates[k];
		archive << myChemicalConcs[k];
	}

	for ( o=0; o<MAX_NEUROEMITTERS; o++)
		myNeuroEmitters[o].Write( archive );
	archive << myNoOfNeuroEmitters;

	archive << myNoOfOrgans;
	for ( i=0; i<myNoOfOrgans; i++)
		archive << myOrgans[i];
	return true;
}

bool Biochemistry::Read(CreaturesArchive &archive) 
{
	int k,o,i;
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!Faculty::Read( archive ))
			return false;

		for ( k=0; k<NUMCHEM; k++) {
			archive >> myChemicalDecayRates[k];
			archive >> myChemicalConcs[k];
		}

		for ( o=0; o<MAX_NEUROEMITTERS; o++)
		{
			if(!myNeuroEmitters[o].Read( archive ))
				return false;
		}

		archive >> myNoOfNeuroEmitters;

		archive >> myNoOfOrgans;
		for ( i=0; i<myNoOfOrgans; i++)
			archive >> myOrgans[i];
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}



// Warning:  GetLocusAddress is not overrided from Faculty and so returns NULL.
// The only biochemical loci are in organ.

float* Biochemistry::GetInvalidCreatureLocusAddress() {
	return myCreature.GetCreatureReference().GetInvalidLocusAddress();
}


float* Biochemistry::GetCreatureLocusAddress(int type,	// ID relates to RECEPTOR or EMITTER locus?
							 int organ,
							 int tissue,		// tissue containing locus
							 int locus) {		// locus on object
	return myCreature.GetCreatureReference().GetLocusAddress(type, organ, tissue, locus);
}





/*********************************************************************
* Public: ReadFromGenome.
* Define all chemical properties, receptor sites, 
* emitter sites & reaction sites using DNA
*********************************************************************/
void Biochemistry::ReadFromGenome(Genome& genome)
{
	float ClockRate, RepairRate, LifeForce, InitClock, ZeroEnergyDamage;
	int c;
	NeuroEmitter* n = NULL;


	// Read the NEUROEMITTER genes to determine the chemo-emitters
	// Therefore late-switching genes only replace existing ones if they share the
	// same neuronal loci (regardless of the chemicals they emit).
	// Otherwise, they are added to the list.
	genome.Reset();
	while (genome.GetGeneType(BIOCHEMISTRYGENE, G_NEUROEMITTER, NUMBIOCHEMSUBTYPES, SWITCH_AGE, ORGANGENE)) {
		if	(myNoOfNeuroEmitters == MAX_NEUROEMITTERS) {	// ignore if too many genes!
			break;
		}
		int i,o;
		float* neuronalInputs[NeuroEmitter::noOfNeuronalInputs];
		for ( i=0; i<NeuroEmitter::noOfNeuronalInputs; i++) {
			int lobeId = genome.GetByte();
			int neuronId = genome.GetByte();
			neuronalInputs[i] = 
				GetCreatureLocusAddress(EMITTER,ORGAN_BRAIN,lobeId,neuronId*noOfVariablesAvailableAsLoci);
		}

		// if there exists a neuroemitter already using the same neurons replace it
		n = NULL;
		for	(i=0; i<myNoOfNeuroEmitters; i++) {
			int noOfMatches = 0;
			for ( o=0; o<NeuroEmitter::noOfNeuronalInputs; o++) {
				if (myNeuroEmitters[i].myNeuronalInputs[o]==neuronalInputs[o])
					noOfMatches++;
			}
			if (noOfMatches==NeuroEmitter::noOfNeuronalInputs) {
				n = &myNeuroEmitters[i];
				break;
			}
		}

		// otherwise add to end of list
		if (n==NULL) {
			n = &myNeuroEmitters[myNoOfNeuroEmitters++];
			for ( o=0; o<NeuroEmitter::noOfNeuronalInputs; o++) {
				n->myNeuronalInputs[o] = neuronalInputs[o];
			}
        }

		n->bioTickRate = genome.GetFloat();
		for ( o=0; o<NeuroEmitter::noOfChemicalEmissions; o++) {
			n->myChemicalEmissions[o].chemicalId = genome.GetByte();
			n->myChemicalEmissions[o].amount = genome.GetFloat();
		}
	}



	// Read any HALFLIFE genes to determine the natural decay rates of chemicals
	// Late-switching genes overwrite their predecessor always 
	genome.Reset();
	while ((genome.GetGeneType(BIOCHEMISTRYGENE, G_HALFLIFE, NUMBIOCHEMSUBTYPES)) != false) {
		for (c=0; c<256; c++) {
			float inputFloat = genome.GetFloat()*32.0f;

			if (inputFloat==0.0f) {
				// allow completely instant decays:
				myChemicalDecayRates[c] = 0.0f;
			}
			else 
			{
				float halfLifeInTicks = pow(2.2f, inputFloat);
				myChemicalDecayRates[c] = (float)pow(0.5f, 1.0f/halfLifeInTicks);
			}
		}
	}


	// Read any INJECT genes to determine the initial concentration of a chemical
	// C1: Late-switching genes just reset the concentration to the new value 
    // C2: NO! it is wrong to reset chemical levels just because a norn has aged.
	// C2e: Really?  Surely the C1 way gives more scope: (gtb)
//    if (myCreatureOwner->GetAge() == AGE_BABY) {
	genome.Reset();
	while ((genome.GetGeneType(BIOCHEMISTRYGENE, G_INJECT, NUMBIOCHEMSUBTYPES)) != false) {
		c = genome.GetByte();							// get chemical name
		myChemicalConcs[c] = genome.GetFloat();// read initial concentration
	}
//    }

	// Read organ specification.
	genome.Reset();

    // Process 'hidden' body organ. 
    // All biochemistry genes that are not preceded by an organ gene will be gathered up 
    // into this implicit organ. This can be considered the 'body'.
    int iOrgan = 0; // Process all organs form start.
    if (genome.GetAge() == AGE_BABY) {
        iOrgan = myNoOfOrgans;
		myOrgans[iOrgan] = new Organ();
        myOrgans[iOrgan]->SetOwner(this);
		myNoOfOrgans++;
    }
	myOrgans[iOrgan]->InitFromGenome(genome);
	myOrgans[iOrgan++]->BindToLoci();

	// Read organ specification.
    // Do none cloned genes first.
    int iGen = 0;
    bool bGeneFound;
    do {
        bGeneFound = false;
		genome.Reset();
		while(genome.GetGeneType(ORGANGENE, G_ORGAN, NUMORGANSUBTYPES, SWITCH_ALWAYS) != false) 
		{
			if (myNoOfOrgans == MAXORGANS) {
				break;
			}

            if (genome.Generation() == iGen) {
                bGeneFound = true;

				// Ignore organ gene age expression time. Do all at baby.
                if (genome.GetAge() == AGE_BABY)
                {
                    //TRACE("Newly expressed organ: ");
                    // Newly expressed organ gene.
		            ClockRate = genome.GetFloat();
		            RepairRate = genome.GetFloat();
		            LifeForce = genome.GetFloat();
		            InitClock = genome.GetFloat();
		            ZeroEnergyDamage = genome.GetFloat();
    	            myOrgans[iOrgan] = new Organ;
                    myOrgans[iOrgan]->Init(ClockRate, RepairRate, LifeForce, InitClock, ZeroEnergyDamage);
		            myOrgans[iOrgan]->SetOwner(this);
//					OutputFormattedDebugString("Organ: clock rate %lf\n", ClockRate);
		            myOrgans[iOrgan]->InitFromGenome(genome);
		            myOrgans[iOrgan++]->BindToLoci();
		            myNoOfOrgans++;
                } else {
                    // Cellotape & blue tack.
                    // Occasionally a mystery organ gene is found which pushed this beyond the ends
                    // of the array.
                    if (iOrgan < myNoOfOrgans) {
						ASSERT(myOrgans[iOrgan]);
                        // Previously expressed organ gene.
		                myOrgans[iOrgan]->InitFromGenome(genome);
		                myOrgans[iOrgan++]->BindToLoci();
                    }
                }
            }
		}
        iGen++;
    }
    while(bGeneFound);
}



/*********************************************************************
* Public: Update.
* Update entire biochemistry. Call every tick.
*********************************************************************/
void Biochemistry::Update() {
	NeuroEmitter* n;
	int i;
	// update neuroemitters:
	for (i=0, n=myNeuroEmitters; i<myNoOfNeuroEmitters; i++, n++) {
		n->bioTick += n->bioTickRate;
		if (n->bioTick > 1.0f) {						// adjust according to sample rate
			n->bioTick -= 1.0f;

			float neuronInputsMultiplied = 1.0f;		// get multiplied activations
			int o;
			for (o=0; o<NeuroEmitter::noOfNeuronalInputs; o++) {
				neuronInputsMultiplied *= *(n->myNeuronalInputs[o]);
			}											// release chemicals in proportion to total activation:
			for (o=0; o<NeuroEmitter::noOfChemicalEmissions; o++) {
				AddChemical(
					n->myChemicalEmissions[o].chemicalId,
					n->myChemicalEmissions[o].amount*neuronInputsMultiplied
				);
			}
		}
	}

	
	// Update all organs.
	for (i=0; i<myNoOfOrgans; i++) {
		myOrgans[i]->Update();
	}

	// Reduce chemical concentrations according to half-lives
	for (i=0; i<NUMCHEM; i++)
		myChemicalConcs[i] *= myChemicalDecayRates[i];
}


Organ* Biochemistry::GetOrgan(int organNumber) const
{
	if (organNumber < 0 || organNumber >= myNoOfOrgans)
		return NULL;

	return myOrgans[organNumber];
}
