// LifeFaculty.cpp: implementation of the LifeFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "LifeFaculty.h"
#include "Creature.h"
#include "MotorFaculty.h"
#include "SensoryFaculty.h"
#include "Biochemistry/Biochemistry.h"
#include "Brain/Brain.h"
#include "../App.h"
#include "../World.h"

CREATURES_IMPLEMENT_SERIAL(LifeFaculty)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       LifeFaculty
// Description: 
// ------------------------------------------------------------------------
LifeFaculty::LifeFaculty()
{
	myState = alertState;
	myAge = 0;							// embryo
	myNextAge = 1;
	myAgeInTicks = 0;					// creatures myAge in ticks
	myProperlyBorn = false;
	myNumberOfForceAgeingRequestsPending =0;

	for	(int i=0; i<NUMAGES; i++)
		myAgeingLoci[i] = 0.0f;
	myAsleepLocus = 0.0f;				// clear emitter loci
	myDeathTriggerLocus = 0.0f;
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       LifeFaculty
// Description: 
// ------------------------------------------------------------------------
LifeFaculty::~LifeFaculty()
{

}


// ------------------------------------------------------------------------
// Function:    Update
// Class:       LifeFaculty
// Description: 
// ------------------------------------------------------------------------
void LifeFaculty::Update() {
	if (myProperlyBorn && !GetWhetherDead())
		myAgeInTicks+=4;

	myAsleepLocus = myState==asleepState? 1.0f : 0.0f;// asleep or awake?

	// Now that ageing in terms of appearance takes a number of ticks if during debugging
	// we try to age the norn too many times in one go it'll be bad so we now
	// count up the pending ageing requests and do them when we can
	if(myNumberOfForceAgeingRequestsPending)
	{
		if(myCreature.GetCreatureReference().PrepareForAgeing(myNextAge))
		{
			myAge++;							// select new myAge & therefore a new, lower threshold
			myCreature.GetCreatureReference().ExpressGenes();
			myNumberOfForceAgeingRequestsPending--;
			myNextAge++;
			SendAgeEvent();

			if (myAge>=NUMAGES)			// if final phase is reached (senile->death)
				SetWhetherDead(true);	// then force creature to die of old myAge regardless of health
										// (this last recep is optional & hopefully won't be needed)
		
		}
		else
		{
			return;
		}
	}

	if (myAge>=NUMAGES)			// if final phase is reached (senile->death)
	SetWhetherDead(true);	// then force creature to die of old myAge regardless of health
										// (this last recep is optional & hopefully won't be needed)


	// If it's time to move into next phase of life, do so & express new genes.
	// A series of receptors is used to trigger movement to the next phase. Each phase
	// will only and always be entered once, and only in the right sequence.
	if(myAge < NUMAGES)
	{
		if ((myAgeingLoci[myAge]))
		{											// if NEXT myAge receptor has turned on
			ForceAgeing();							// select new myAge & therefore a new, lower threshold
		}
	}

	if (myDeathTriggerLocus>0.0f)
		SetWhetherDead(true);



}



// ------------------------------------------------------------------------
// Function:    ForceAgeing
// Class:       LifeFaculty
// Description: 
// ------------------------------------------------------------------------
void LifeFaculty::ForceAgeing()				
{
	// If it's time to move into next phase of life, do so & express new genes.
	// A series of receptors is used to trigger movement to the next phase. Each phase
	// will only and always be entered once, and only in the right sequence.


	if	(myAge<NUMAGES)	{					// if NEXT myAge receptor has turned on
		if(myCreature.GetCreatureReference().PrepareForAgeing(myNextAge))
		{
			myAge++;							// select new myAge & therefore a new, lower threshold
			myCreature.GetCreatureReference().ExpressGenes();		// then search genome for new genes to express
			myNextAge++;
			SendAgeEvent();
		}
		else
			myNumberOfForceAgeingRequestsPending++;
	}
	if (myAge>=NUMAGES)			// if final phase is reached (senile->death)
		SetWhetherDead(true);	// then force creature to die of old myAge regardless of health
								// (this last recep is optional & hopefully won't be needed)
}


// ------------------------------------------------------------------------
// Function:    Health
// Class:       LifeFaculty
// Description: 
// Returns:     float = 
// ------------------------------------------------------------------------
float LifeFaculty::Health() const {
    return myCreature.GetCreatureReference().GetBiochemistry()->GetChemical(CHEM_GLYCOGEN);
}




// Kill this creature (death from old myAge, ill health or wounding).
// Lie down, switch off brain, allow chemistry to fade, initiate Funeral kit,
// Enter Death Row, etc.
// ------------------------------------------------------------------------
// Function:    SetWhetherDead
// Class:       LifeFaculty
// Description: 
// Arguments:   bool d = 
// ------------------------------------------------------------------------
void LifeFaculty::SetWhetherDead(bool d) {
	if (d!=true) return;						// can't rejuvenate.
	if (myState!=deadState) {					// unless already started

		// Make sure we're not carrying anything
		if (myCreature.GetCreatureReference().GetCarried().IsValid() )
		{
			myCreature.GetCreatureReference().GetCarried().GetAgentReference().TryToBeDropped
				(myCreature, INTEGERZERO, INTEGERZERO, true);
		}
	
		AgentHandle a = myCreature.GetAgentReference();
		SensoryFaculty::RemoveFromAllFriendAndFoe(a);

		SetState(deadState);					// prevent re-entry (also acts as chemo-emitter)
		myCreature.GetCreatureReference().GetVirtualMachine().StopScriptExecuting();
		myCreature.GetCreatureReference().ExecuteScriptForEvent(SCRIPTDIE,myCreature, INTEGERZERO, INTEGERZERO);// do script, overwriting any existing
		myCreature.GetCreatureReference().SetEyeState(0);				// Close creatures eyes.

		// trigger and store death event
		Creature& creature = GetCreatureOwner().GetCreatureReference();
		std::string moniker = creature.GetMoniker();
		HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
		CreatureHistory& history = historyStore.GetCreatureHistory(moniker);
		history.AddEvent(LifeEvent::typeDied, "", "");
	}
}

// ------------------------------------------------------------------------
// Function:    SetWhetherZombie
// Class:       LifeFaculty
// Description: 
// Arguments:   bool u = 
// ------------------------------------------------------------------------
void LifeFaculty::SetWhetherZombie(bool u) {
	if (myState==deadState)
		return;
	// not allowed unzombie if we're not in a zombie state:
	if (!u && myState!=zombieState)
		return;
	SetState(u ? zombieState : alertState);
}

// ------------------------------------------------------------------------
// Function:    SetWhetherUnconscious
// Class:       LifeFaculty
// Description: 
// Arguments:   bool u = 
// ------------------------------------------------------------------------
void LifeFaculty::SetWhetherUnconscious(bool u) {
	if (myState==deadState)
		return;
	if (myState==zombieState)
		return;
	// not allowed conscious if we're not in an unconscious state:
	if (!u && myState!=unconsciousState)
		return;
	SetState(u ? unconsciousState : alertState);
}

// ------------------------------------------------------------------------
// Function:    SetWhetherAlert
// Class:       LifeFaculty
// Description: 
// Arguments:   bool b = 
// ------------------------------------------------------------------------
void LifeFaculty::SetWhetherAlert(bool b) {
	if (myState==deadState)
		return;
	if (myState==zombieState)
		return;
	SetState(b ? alertState : unconsciousState);
}
// ------------------------------------------------------------------------
// Function:    SetWhetherAsleep
// Class:       LifeFaculty
// Description: 
// Arguments:   bool a = 
// ------------------------------------------------------------------------
void LifeFaculty::SetWhetherAsleep(bool a) {
	if (myState==deadState)
		return;
	if (myState==zombieState)
		return;
	// not allowed wake if we're not asleep:
	if (!a && (myState!=asleepState && myState!=dreamingState))
		return;
	SetState(a ? asleepState : alertState);
}
// ------------------------------------------------------------------------
// Function:    SetWhetherDreaming
// Class:       LifeFaculty
// Description: 
// Arguments:   bool d = 
// ------------------------------------------------------------------------
void LifeFaculty::SetWhetherDreaming(bool d) {
	if (myState==deadState)
		return;
	if (myState==zombieState)
		return;
	// not allowed undream if we're not dreaming:
	if (!d && myState!=dreamingState)
		return;
	SetState(d ? dreamingState : asleepState);
}

// ------------------------------------------------------------------------
// Function:    SetState
// Class:       LifeFaculty
// Description: 
// Arguments:   LifeState s = 
// ------------------------------------------------------------------------
void LifeFaculty::SetState(LifeState s) {
	if (s==myState)
		return;

	Creature& c = myCreature.GetCreatureReference();


	// if norn is going not alert:
	if (myState==alertState)
	{
		c.Motor()->StopCurrentAction();	// reset action so it gets restarted later
		c.SetIntrospective(true);
		c.ResetAnimationString();
	}

	// if norn is going unconscious give it a suitable pose:
	if (s==unconsciousState || s==zombieState)
	{
		c.GetVirtualMachine().StopScriptExecuting();
		c.ShowPose(58, 0);
	}

	// if the norn is waking up make sure it stops its involuntary action too:
	if ((myState==asleepState || myState==dreamingState) && s==alertState)
	{
		c.Motor()->StopCurrentInvoluntaryAction();
		c.GetVirtualMachine().StopScriptExecuting();
	}

	if (s==dreamingState)
		c.GetBrain()->SetWhetherToProcessInstincts(true);
	if (myState==dreamingState)
		c.GetBrain()->SetWhetherToProcessInstincts(false);

	myState = s;
}



// ------------------------------------------------------------------------
// Function:    GetLocusAddress
// Class:       LifeFaculty
// Description: 
// Arguments:   int type = 
//              int organ = 
//              int tissue = 
//              int locus = 
// Returns:     float* = 
// ------------------------------------------------------------------------
float* LifeFaculty::GetLocusAddress(int type, int organ, int tissue, int locus) {
	if (organ!=ORGAN_CREATURE)
		return NULL;

	if (tissue==TISSUE_SOMATIC) {
		if (type==RECEPTOR) {
			if (locus>=LOC_AGE0 && locus<=LOC_AGE6)
				return &myAgeingLoci[locus-LOC_AGE0];
		}
	}
	if (tissue==TISSUE_IMMUNE) {
		if (type==RECEPTOR) {
			if (locus==LOC_DIE)
				return &myDeathTriggerLocus;
		} else {
			if (locus==LOC_DEAD) {
				static float myDeadFlagAsFloat = myState==deadState ? 1.0f : 0.0f;
				return &myDeadFlagAsFloat;
			}
		}
	}
	if (tissue==TISSUE_SENSORIMOTOR) {
		if (type==EMITTER) {
			switch (locus) {
				case LOC_ASLEEP:
					return &myAsleepLocus;
			}
		}
	}
	return NULL;
}

// N.B. don't serialize out the pending force age counter as the creature
// will be recreated at the correct age!
// ------------------------------------------------------------------------
// Function:    Write
// Class:       LifeFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool LifeFaculty::Write(CreaturesArchive &archive) const {
	base::Write( archive );
	archive.WriteFloatRefTarget( myAsleepLocus );
	archive.WriteFloatRefTarget( myDeathTriggerLocus );
	for (int i=0; i<NUMAGES; i++) {
		archive.WriteFloatRefTarget( myAgeingLoci[i] );
	}
	archive << mySex;
	archive << myAge;
	archive << myVariant;
	archive << myAgeInTicks;
	archive << (int16)myState;
	archive << myProperlyBorn;
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       LifeFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool LifeFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!base::Read( archive ))
			return false;

		archive.ReadFloatRefTarget( myAsleepLocus );
		archive.ReadFloatRefTarget( myDeathTriggerLocus );
		for (int i=0; i<NUMAGES; i++) {
			archive.ReadFloatRefTarget( myAgeingLoci[i] );
		}
		archive >> mySex;
		archive >> myAge;
		archive >> myVariant;
		archive >> myAgeInTicks;
		int16 state;
		archive >> state;
		myState = (LifeState)state;
		archive >> myProperlyBorn;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	myNextAge = myAge + 1;
	return true;
}

// ------------------------------------------------------------------------
// Function:    SetProperlyBorn
// Class:       LifeFaculty
// Description: 
// ------------------------------------------------------------------------
void LifeFaculty::SetProperlyBorn()
{
	ASSERT(!myProperlyBorn);
	myProperlyBorn = true;

	Creature& child = GetCreatureOwner().GetCreatureReference();

	std::string monikerChild = child.GetMoniker();
	std::string monikerMum = child.GetMotherMoniker();
	std::string monikerDad = child.GetFatherMoniker();

	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	
	CreatureHistory& childHistory = historyStore.GetCreatureHistory(monikerChild);
	CreatureHistory& mumHistory = historyStore.GetCreatureHistory(monikerMum);
	CreatureHistory& dadHistory = historyStore.GetCreatureHistory(monikerDad);

	childHistory.AddEvent(LifeEvent::typeBorn, monikerMum, monikerDad);
	mumHistory.AddEvent(LifeEvent::typeChildBorn, monikerChild, monikerDad);
	dadHistory.AddEvent(LifeEvent::typeChildBorn, monikerChild, monikerMum);
}


// ------------------------------------------------------------------------
// Function:    SendAgeEvent
// Class:       LifeFaculty
// Description: 
// ------------------------------------------------------------------------
void LifeFaculty::SendAgeEvent()
{
	// trigger and store age event
	Creature& creature = GetCreatureOwner().GetCreatureReference();
	std::string moniker = creature.GetMoniker();
	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	CreatureHistory& history = historyStore.GetCreatureHistory(moniker);
	history.AddEvent(LifeEvent::typeNewLifeStage, "", "");
}
