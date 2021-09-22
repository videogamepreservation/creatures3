/*********************************************************************
* File:     Organ.cpp
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Organ.h"
#include "Biochemistry.h"

#include "../Genome.h"
#include "Chemical.h"
#include "Reaction.h"
#include "../Brain/SVRule.h"

CREATURES_IMPLEMENT_SERIAL(Organ)


/*********************************************************************
* Statics.
*********************************************************************/
const float Organ::myBaseOrganADPCost = 0.0078f;	//???
const float Organ::myRateOfDecay = 1.0e-5f;
const float Organ::myBaseLifeForce = 1.0e6f;
const float Organ::myMinLifeForce = 0.5;


/*********************************************************************
* Public: Default constructor.
*********************************************************************/
Organ::Organ()
{
    myEnergyAvailableFlag = true;
	myBiochemistryOwner = NULL;
	loc_ClockRate = 0.5f;
	myClock = 0.0f;

	loc_LifeForce = 0.5f;
	myInitialLifeForce = loc_LifeForce * myBaseLifeForce;
	myShortTermLifeForce = myInitialLifeForce;
	myLongTermLifeForce = myInitialLifeForce;

	myLongTermRateOfRepair = 10.0f/255.0f;
	loc_LongTermRateOfRepair = 0.0f; 
    loc_InjuryToApply = 0.0f;

	myEnergyCost = 0;
	myDamageDueToZeroEnergy = myInitialLifeForce/128.0f;
		// Organ will fail after at most 128 activations.

	myNoOfReceptors = 0;
	myNoOfEmitters = 0;
	myNoOfReactions = 0;
	myNoOfReceptorGroups = 0;
}


/*********************************************************************
* Public: Helper fn.
*********************************************************************/
void Organ::Init(float ClockRate, float RateOfRepair, float LifeForce, 
				 float InitialClock, float DamageDueToZeroEnergy) {
	loc_ClockRate = ClockRate;
	myClock = InitialClock;

	loc_LifeForce = max(1.0f, LifeForce);
	myInitialLifeForce = loc_LifeForce * myBaseLifeForce;
	myShortTermLifeForce = myInitialLifeForce;
	myLongTermLifeForce = myInitialLifeForce;

	myLongTermRateOfRepair = RateOfRepair;
	if (DamageDueToZeroEnergy>1.0f) DamageDueToZeroEnergy=1.0f;
	myDamageDueToZeroEnergy = (myInitialLifeForce *
		DamageDueToZeroEnergy) / 255.0f;
}


/*********************************************************************
* Public: Init.
*********************************************************************/
void Organ::BindToLoci()
{
//    ASSERT(myBiochemistryOwner); 

	Receptor* r = NULL;
	Emitter* e = NULL;
	int i;

	for	(i = 0; i < myNoOfReceptorGroups; i++) 
	{
		Receptors& rGroup = myReceptorGroups[i];
		int iNumberInGroup = rGroup.size();

		for (int j = 0; j < iNumberInGroup; j++)
		{
			r = rGroup[j];
			// send in reaction ID to locus address function if need be:
			r->Dest = GetLocusAddress(RECEPTOR, r->IDOrgan, r->IDTissue, r->IDLocus);
			r->isClockRateReceptor = 
				(r->IDOrgan==ORGAN_ORGAN && r->IDLocus==RLOCUS_CLOCKRATE);
			ASSERT(r->Dest);
		}
	}
	for	(i=0, e=myEmitters; i<myNoOfEmitters; i++, e++) {
		e->Source = GetLocusAddress(EMITTER, e->IDOrgan, e->IDTissue, e->IDLocus);
	}
}

/*********************************************************************
* Public: Destructor.
*********************************************************************/
Organ::~Organ()
{
	// Destroy receptors...
	// this needs updating to use the vector more efficiently
	for	(int i = 0; i < myNoOfReceptorGroups; i++) 
	{
		while(myReceptorGroups[i].size())
		{
			Receptors& rGroup = myReceptorGroups[i];
			Receptor* pR = rGroup[0];
			rGroup.erase(rGroup.begin());
			delete pR;
		}
	}
}


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool Organ::Write(CreaturesArchive &archive) const {

    const Receptor* r = NULL;
    const Emitter* e = NULL;
	const NeuroEmitter* n = NULL;
    const Reaction* rn = NULL;
    int i;

	// Saving.
	archive.WriteFloatRefTarget( loc_ClockRate );
	archive.WriteFloatRefTarget( loc_LifeForce );
	archive.WriteFloatRefTarget( loc_LongTermRateOfRepair );
	archive.WriteFloatRefTarget( loc_InjuryToApply );
    archive << myClock;
	archive << myInitialLifeForce << myShortTermLifeForce << myLongTermLifeForce;
	archive << myLongTermRateOfRepair << myEnergyCost;
	archive << myDamageDueToZeroEnergy << myEnergyAvailableFlag;
	archive << myNoOfReceptors << myNoOfReceptorGroups;


	// got to serialize reactions before receptors as receptors can bind to the rate
	// locus now on reactions:
	archive << myNoOfReactions;
	for	(i = 0, rn = myReactions; i < myNoOfReactions; i++, rn++) {
		archive << rn->propR1 << rn->R1;
		archive << rn->propR2 << rn->R2;
		archive.WriteFloatRefTarget( rn->Rate );
		archive << rn->propP1 << rn->P1;
		archive << rn->propP2 << rn->P2;
	}


    for	(i = 0; i < myNoOfReceptorGroups; i++) {
		// ReceptorPtrArray is not directly serialisable in this format, so we'll have to do it explicitly.
		int size = myReceptorGroups[i].size(); 
		archive << size;
		for (int j=0;j<size;j++) {
			archive << myReceptorGroups[i][j];
		}
	}

	archive << myNoOfEmitters;
	for	(i = 0, e = myEmitters; i < myNoOfEmitters; i++, e++) {
		archive << e->IDOrgan << e->IDTissue << e->IDLocus;
		archive << e->Chem << e->Threshold << e->bioTickRate << e->Gain << e->Effect;
		archive.WriteFloatRef( e->Source );
	}
	archive << myBiochemistryOwner;
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Organ::Read(CreaturesArchive &archive) 
{
	// Check version info
	Receptor* r = NULL;
	Emitter* e = NULL;
	NeuroEmitter* n = NULL;
	Reaction* rn = NULL;
	int i;

	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		// Loading.
		archive.ReadFloatRefTarget( loc_ClockRate );
		archive.ReadFloatRefTarget( loc_LifeForce );
		archive.ReadFloatRefTarget( loc_LongTermRateOfRepair );
		archive.ReadFloatRefTarget( loc_InjuryToApply );
		archive >> myClock;
		archive >> myInitialLifeForce >> myShortTermLifeForce >> myLongTermLifeForce;
		archive >> myLongTermRateOfRepair >> myEnergyCost;
		archive >> myDamageDueToZeroEnergy >> myEnergyAvailableFlag;
		archive >> myNoOfReceptors >> myNoOfReceptorGroups;


		archive >> myNoOfReactions;
		for	(i = 0, rn = myReactions; i < myNoOfReactions; i++, rn++) {
			archive >> rn->propR1 >> rn->R1;
			archive >> rn->propR2 >> rn->R2;
			archive.ReadFloatRefTarget( rn->Rate );
			archive >> rn->propP1 >> rn->P1;
			archive >> rn->propP2 >> rn->P2;
		}


		for	(i = 0; i < myNoOfReceptorGroups; i++) {
			// ReceptorPtrArray is not directly serialisable in this
			// format, so we'll have to do it explicitly.
			int size;
			archive >> size;
			myReceptorGroups[i].resize(size);
			for (int j=0;j<size;j++) {
				archive >> myReceptorGroups[i][j];
			}
		}

		archive >> myNoOfEmitters;
		for	(i = 0, e = myEmitters; i < myNoOfEmitters; i++, e++) {
			archive >> e->IDOrgan >> e->IDTissue >> e->IDLocus;
			archive >> e->Chem >> e->Threshold >> e->bioTickRate >> e->Gain >> e->Effect;
			archive.ReadFloatRef( e->Source );
		}
		archive >> myBiochemistryOwner;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}



/*********************************************************************
* Public: InitFromGenome.
*********************************************************************/
void Organ::InitFromGenome(Genome& genome)
{
	// Taken from C1 CBiochemistry embryology.
	Receptor* r = NULL;
	Emitter* e = NULL;
	Reaction* rn = NULL;
	int c,i;
	int o,t,l;

	// Store this position in the genome.
	genome.Store();


	int thisReactionNo = -1;
	while (true) {
		genome.Store2();
		// Receptors can now attach to reactions so that's why we load them now:

		// Read the RECEPTOR genes to determine the chemo-receptors
		// Late-switching genes overwrite equivalent predecessors (since can't 
		// have 2 receptors on the same locus)
		// (stop at next reaction or organ gene)
		while ((genome.GetGeneType(
			BIOCHEMISTRYGENE, G_RECEPTOR, NUMBIOCHEMSUBTYPES, 
			SWITCH_AGE, BIOCHEMISTRYGENE, G_REACTION, ORGANGENE)) != false) {

			if (myNoOfReceptors == MAXRECEPTORS) {		// ignore if too many genes!		
				break;
			}

			// Creatures2: Allow more than one receptor to attach to a locus.
			r = new Receptor;
			myNoOfReceptors++;

			r->IDOrgan = genome.GetCodon(0,NUM_RECEPTOR_ORGANS-1);// organ that receptor binds to
			r->IDTissue = genome.GetCodon(0,255);		// tissue in organ that receptor binds to

			if (r->IDOrgan==ORGAN_REACTION)				// for current reaction replace tissue no by reaction no
				r->IDTissue = thisReactionNo;

			r->IDLocus = genome.GetCodon(0,255);		// locus in that tissue that receptor binds to

			r->Chem = genome.GetCodon(0, NUMCHEM-1);// receptor chemical
			r->Threshold = genome.GetFloat();		// receptor threshold
			r->Nominal = genome.GetFloat();			// nominal o/p signal
			r->Gain = genome.GetFloat();			// gain
			r->Effect = genome.GetByte();			// flags

			// Creatures2: Is locus shared by more then one receptor?
			int g;
			for	(g = 0; g < myNoOfReceptorGroups; g++) {
				// Only need to check the first one in the list.
				Receptor* pR = myReceptorGroups[g][0];
				ASSERT(pR);

				if (pR->IDOrgan == r->IDOrgan && pR->IDTissue == r->IDTissue &&
					pR->IDLocus == r->IDLocus) break;
			}

			// Add receptor to group.
			myReceptorGroups[g].push_back(r);

			// If no group was found... create new group.
			if (g == myNoOfReceptorGroups) {
				myNoOfReceptorGroups++;
				ASSERT(myNoOfReceptorGroups <= MAXRECEPTORGROUPS);
			}
		}
		genome.Restore2();


		// Read a REACTION gene
		// Late-switching genes are just added to the list of reactions - they never
		// replace pre-existing ones
		if ((genome.GetGeneType(
			BIOCHEMISTRYGENE, G_REACTION, NUMBIOCHEMSUBTYPES, 
			SWITCH_AGE, ORGANGENE)) == false) {
			// no more reactions so break:
			break;
		}
		if (myNoOfReactions == MAXREACTIONS - 1) 		// ignore if too many genes! (save space for ATP->ADP)
		{								
			break;
		}

		thisReactionNo = myNoOfReactions++;
		rn = &myReactions[thisReactionNo];				// get reaction addr & tally

		rn->propR1 = genome.GetCodon(1, MAXREACTANTS);	// proportion of reactant1 (1-MAXREACTANTS)
		rn->R1 = genome.GetByte();				// reactant1 (0=none)
		rn->propR2 = genome.GetCodon(1, MAXREACTANTS);	// proportion of reactant2
		rn->R2 = genome.GetByte();				// reactant2 (0=none)
		rn->propP1 = genome.GetCodon(1, MAXREACTANTS);	// proportion of product1 (1-MAXREACTANTS)
		rn->P1 = genome.GetByte();				// product1 (0=none)
		rn->propP2 = genome.GetCodon(1, MAXREACTANTS);	// proportion of product2 (1-MAXREACTANTS)
		rn->P2 = genome.GetByte();				// product2 (0=none)

		rn->Rate = 1.0f - genome.GetFloat();
	}




	// Read the EMITTER genes to determine the chemo-emitters
	// It is acceptable for several emitters to be attached to the same locus.
	// Therefore late-switching genes only replace existing ones if they share the
	// same locus AND emit the same chemical. Otherwise, they are added to the list.
	genome.Restore();
	while ((genome.GetGeneType(
        BIOCHEMISTRYGENE, G_EMITTER, NUMBIOCHEMSUBTYPES, 
        SWITCH_AGE, ORGANGENE)) != false) 
	{
		if	(myNoOfEmitters == MAXEMITTERS)				// ignore if too many genes!
		{									
			break;
		}

		o = genome.GetCodon(0, NUM_EMITTER_ORGANS - 1);		// organ that receptor binds to
		t = genome.GetByte();				// tissue in organ that receptor binds to
		l = genome.GetByte();				// locus in that tissue that receptor binds to
		c = genome.GetCodon(0, NUMCHEM - 1);		// emitter chemical

//		OutputFormattedDebugString("Emitter chemical %d\n", c);

		// Search for any pre-existing emitter at this locus and replace this only if it
		// also emits the same chemical
		for	(i = 0; i < myNoOfEmitters; i++) 
		{
			if (myEmitters[i].IDOrgan == o && 
				myEmitters[i].IDTissue == t && 
				myEmitters[i].IDLocus == l && 
				myEmitters[i].Chem == c)
			{
				e = &myEmitters[i];
				break;
			}
		}
		// else add to end of list
		if (i >= myNoOfEmitters)
        {
			e = &myEmitters[myNoOfEmitters++];
            e->IDOrgan = o;
            e->IDTissue = t;
            e->IDLocus = l;
            e->Chem = c;
        }

		e->Threshold = genome.GetFloat();		// emitter threshold
		e->bioTickRate = 1.0f/((float)genome.GetCodon(1,255));
		e->Gain = genome.GetFloat();			// gain
		e->Effect = genome.GetCodon(0,255);		// flags
		e->IDOrgan = o;							// organ that receptor binds to
		e->IDTissue = t;						// tissue in organ that receptor binds to
		e->IDLocus = l;							// locus in that tissue that receptor binds to
	}


	// Restore position. Necessary to prevent organs begin infinitely read from the genome.
	genome.Restore();
	// Initialize energy cost.
	InitEnergyCost();
}


/*********************************************************************
* Public: Update.
*********************************************************************/
bool Organ::Update() {
	bool bOrganFunction = false;

	// Update only if organ alive!
	if (!Failed()) {
		bool workWasDone = false;

		myClock += loc_ClockRate;
		if (myClock>=1.0f) {
			myClock-=1.0f;

			// If time to process biochemistry...
			// and if energy available...
			myEnergyAvailableFlag = ConsumeEnergy();
            if (myEnergyAvailableFlag) {
				// If energy is consumed, work is done!
				workWasDone = true;

				// Process emitters & reactions.
				workWasDone |= ProcessAll();
			} else {
				// Damage organ.
				Injure(myDamageDueToZeroEnergy);
			}
			workWasDone |= RepairInjury(myEnergyAvailableFlag);

            // Process applied injury.
            // If full injury applied, organ can fail in a second (clock-rate depending).
            // 10 = activations necessary to remove all life-force.
            if (loc_InjuryToApply) {
                Injure(LOC_TO_LF(loc_InjuryToApply) / 10.0f);
                loc_InjuryToApply = 0.0f;
            }
			workWasDone |= ProcessReceptors(false);	// process all receptors
		} else {
			workWasDone |= ProcessReceptors(true);	// process just clock-rate receptors
		}

		// Only decay organs if work was done.
		if (workWasDone) DecayLifeForce();

		bOrganFunction = true;
	}

	return bOrganFunction;
}


float Organ::LocToLf(float l)
{
	return LOC_TO_LF(l);
}



/*********************************************************************
* Public: Injure.
*********************************************************************/
void Organ::Injure(float damage)
{
    ASSERT(myBiochemistryOwner); 

	// Reduce life force.
	myShortTermLifeForce = BoundedSub(myShortTermLifeForce, damage);
	loc_LifeForce = myShortTermLifeForce / myInitialLifeForce;

    // Emit drive chemical.
    myBiochemistryOwner->AddChemical(CHEM_INJURY, LF_TO_LOC(damage));
}


/*********************************************************************
* Protected: DecayLifeForce.
*********************************************************************/
void Organ::DecayLifeForce()
{
	// Decay.
	myShortTermLifeForce -= myShortTermLifeForce*myRateOfDecay;
	myLongTermLifeForce  -= myLongTermLifeForce *myRateOfDecay;
	loc_LifeForce = myShortTermLifeForce / myInitialLifeForce;
}


/*********************************************************************
* Protected: RepairInjury.
*********************************************************************/
bool Organ::RepairInjury(bool isEnergyAvailable)
{
    ASSERT(myBiochemistryOwner); 

	// Moving average.
	float delta = myLongTermLifeForce - myShortTermLifeForce;

	// Always do LongTerm damage.
    // LongTermLifeforce is NOT affected by healing.
	myLongTermLifeForce -= delta * myLongTermRateOfRepair;

	// Repair only if energy is available...
	if (isEnergyAvailable) {
        float repair = delta * loc_LongTermRateOfRepair;
		myShortTermLifeForce += repair;

        // Emit drive chemical (1.5 is to compensate for * 2 ST rate of repair).
        myBiochemistryOwner->SubChemical(CHEM_INJURY, LF_TO_LOC(repair));
    }

	loc_LifeForce = myShortTermLifeForce / myInitialLifeForce;

	return (delta>0.5f && isEnergyAvailable);
}


/*********************************************************************
* Public: GetLocusAddress.
* Helper function. Find either internal locus or external locus.
*********************************************************************/
float* Organ::GetLocusAddress(int type, int organ, int tissue, int locus)
{
    ASSERT(myBiochemistryOwner); 

	switch (organ) {
		case ORGAN_ORGAN:
			// Locus is internal to organ.
			return GetLocusAddress(type, tissue, locus);

		case ORGAN_REACTION:
			if (tissue<0 || tissue>=myNoOfReactions)
				return myBiochemistryOwner->GetInvalidCreatureLocusAddress();
			return &myReactions[tissue].Rate;// all loci return the Reaction Rate

		default:
			// Locus is external to organ.
			return myBiochemistryOwner->GetCreatureLocusAddress(type, organ, tissue, locus);
	}
}


/*********************************************************************
* Public: GetLocusAddress.
*********************************************************************/
float* Organ::GetLocusAddress(int type, int tissue, int locus)
{
	ASSERT(type == RECEPTOR || type == EMITTER);
	if (type == RECEPTOR)
	{
		switch(locus)
		{
		case RLOCUS_CLOCKRATE:
			return &loc_ClockRate;
			break;

        case RLOCUS_RATEOFREPAIR:
            return &loc_LongTermRateOfRepair;
            break;

        case RLOCUS_INJURY:
            return &loc_InjuryToApply;
            break;
		}
	}
	else
	{
		switch(locus)
		{
		case ELOCUS_CLOCKRATE:
			return &loc_ClockRate;
			break;

        case ELOCUS_RATEOFREPAIR:
            return &loc_LongTermRateOfRepair;
            break;

		case ELOCUS_LIFEFORCE:
			return &loc_LifeForce;
			break;
		}
	}

    // Unrecognised ID.
	return myBiochemistryOwner->GetInvalidCreatureLocusAddress();
}


/*********************************************************************
* Protected: ProcessAll.
*********************************************************************/
bool Organ::ProcessAll() {
    ASSERT(myBiochemistryOwner); 
	bool workWasDone = false;

	int i;
	float conc, sig;
	Emitter* e;

	try {
		// First, update all emitters from their source loci
		for	(i = 0, e = myEmitters; i < myNoOfEmitters; i++, e++) {
			// Invert signal if required.
			sig = (e->Effect & EM_INVERT) ? 1.0f-*e->Source : *e->Source; 

			// If there's any signal at locus, and it's time to emit...
			e->bioTick += e->bioTickRate;
			if (e->bioTick > 1.0f) {
				e->bioTick -= 1.0f;
				if (sig) {
					if ((conc = sig - e->Threshold) > 0) {			// no o/p if sig<threshold
						// If response is digital then o/p = gain regardless of signal.
						// If response is analogue then o/p is proportional to signal.
						if (e->Effect & EM_DIGITAL)					
							myBiochemistryOwner->AddChemical(e->Chem,e->Gain);
						else										
							myBiochemistryOwner->AddChemical(e->Chem,conc * e->Gain);

						if (e->Effect & EM_REMOVE)					// wipe locus if reqd
							*e->Source = 0;

						workWasDone |= true;
					}
				}
			}
		}

		// Next, update all reaction sites.
		for	(i=0; i<myNoOfReactions; i++) {
			workWasDone |= ProcessReaction(i);
		}
	}
	catch(...) 
	{	
		ASSERT(false);
	}
	return workWasDone;
}


// Update all receptors to modulate their loci...
bool Organ::ProcessReceptors(bool onlyDoClockRateReceptors) {
	bool workWasDone = false;
	Receptor* r;

	// Process each group of receptors (that share a locus)...
	for	(int g = 0; g < myNoOfReceptorGroups; g++) {
		Receptors& rGroup = myReceptorGroups[g];
		int noOfReceptorsProcessed = 0;
		float totalOfAllNominals = 0.0f;
		int noOfTermsToAdd = 0;
		int noOfTermsToSub = 0;
		float termToAddSoFar = 0.0f;
		float termToSubSoFar = 0.0f;

		// Sum the changes the receptors make to the locus...
		for (int rIndex=0; rIndex<rGroup.size(); rIndex++) {
			r = rGroup[rIndex];
			ASSERT(r->Dest);


			if (r->isClockRateReceptor==false && onlyDoClockRateReceptors)
				break;		// these receptors are not clock-rate receptors
							// so don't update them now

			if (r->Chem) {										 // if receptor is used...
				workWasDone |= true;

				noOfReceptorsProcessed++;
				totalOfAllNominals += r->Nominal;

				float inputSignal =
					myBiochemistryOwner->GetChemical(r->Chem) - // get current chemical concentration
					r->Threshold; 								// get excess of conc over threshold
				if (inputSignal<0.0f) inputSignal=0.0f;			// ignore changes below threshold

				if (inputSignal) {						
					if (r->Effect & RE_DIGITAL)		// If response is digital
						inputSignal = r->Gain;		// then o/p = gain regardless of signal
					else							// If response is analogue
						inputSignal *= r->Gain;		// then o/p is proportional to sig
				}

				if (r->Effect & RE_REDUCE) {		// add or subtract from nominal, 
					termToSubSoFar += inputSignal;
					noOfTermsToSub++;
				} else {
					termToAddSoFar += inputSignal;
					noOfTermsToAdd++;
				}

			}
		}
		if (onlyDoClockRateReceptors && noOfReceptorsProcessed==0)
			continue;
		float result = 0.0f;
		if (noOfReceptorsProcessed>0)
			result = totalOfAllNominals/(float)noOfReceptorsProcessed;
		if (noOfTermsToAdd>0)
			result = BoundedAdd(result, termToAddSoFar/(float)noOfTermsToAdd);
		if (noOfTermsToSub>0)
			result = BoundedSub(result, termToSubSoFar/(float)noOfTermsToSub);

		// for trigger death locus do an OR:
		if (r->IDOrgan==ORGAN_CREATURE && r->IDTissue==TISSUE_IMMUNE && r->IDLocus==LOC_DIE)
		{
			result = (totalOfAllNominals + termToAddSoFar - termToSubSoFar)>0.0f ? 1.0f : 0.0f;
		}
		*r->Dest = result;
	}
	return workWasDone;
}


/*********************************************************************
* Protected: ProcessReaction.
*********************************************************************/
bool Organ::ProcessReaction(int iIndex)
{
    ASSERT(myBiochemistryOwner); 

    float avail = 1.0f, avail2 = 1.0f;
	Reaction* rn = &myReactions[iIndex];
	bool workWasDone = false;

	if (rn->R1) // amount of R or 2R or 3R in blood
		avail = myBiochemistryOwner->GetChemical(rn->R1) / rn->propR1;
	if (rn->R2) // (if reactant not reqd, assume 1)
		avail2 = myBiochemistryOwner->GetChemical(rn->R2) / rn->propR2;	

	if (avail2 < avail)									// amount available for reaction is
		avail = avail2;									// the lesser of the two reactants

	if (avail) {
		// rn->Rate is 0 for slow, 1 for fast (reverse from when loaded in from the genome!)
		float inputFloat = (1.0f-rn->Rate)*32.0f;
		float halfLifeInTicks = powf(2.2f, inputFloat);
		float rate = 1.0f - powf(0.5f, 1.0f/halfLifeInTicks);

		avail *= rate;													// see how much reacts this tick
																		// rate=1 says all (i.e.fast)
																		// rate=0 says none (i.e.slow)

		myBiochemistryOwner->SubChemical(rn->R1, avail * rn->propR1);	// reduce any reactants by this much
		myBiochemistryOwner->SubChemical(rn->R2, avail * rn->propR2);	// modulated by proportions [19/2/96]
		
		myBiochemistryOwner->AddChemical(rn->P1, avail * rn->propP1);	// increase any products by this much
		myBiochemistryOwner->AddChemical(rn->P2, avail * rn->propP2);	// (in proportion as specified) 

		workWasDone = true;
	}

	return workWasDone;

}


/*********************************************************************
* Protected: InitEnergyCost.
*********************************************************************/
void Organ::InitEnergyCost()
{
    ASSERT(myBiochemistryOwner); 

    // Initialize energy cost.
	myEnergyCost = myBaseOrganADPCost +
		((myNoOfReceptors + myNoOfEmitters + 
		myNoOfReactions) / 2550.0);

	// Global organ wide total:
	myBiochemistryOwner->myATPRequirement = myEnergyCost*loc_ClockRate;
}


/*********************************************************************
* Protected: ConsumeEnergy.
*********************************************************************/
bool Organ::ConsumeEnergy()
{
    ASSERT(myBiochemistryOwner); 

    bool isSuccess = false;
	if (myBiochemistryOwner->GetChemical(CHEM_ATP) >= myEnergyCost) {
 		myBiochemistryOwner->SubChemical(CHEM_ATP, myEnergyCost);
		myBiochemistryOwner->AddChemical(CHEM_ADP, myEnergyCost);
		isSuccess = true;
	}

	return isSuccess;
}

