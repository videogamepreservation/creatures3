#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Creature.h"

#include "../World.h"		// for pointer!
#include "../App.h"
#include "../Display/ErrorMessageHandler.h"

#include "Brain/Brain.h"
#include "Biochemistry/Biochemistry.h"
#include "LinguisticFaculty.h"
#include "MotorFaculty.h"
#include "SensoryFaculty.h"
#include "ReproductiveFaculty.h"
#include "ExpressiveFaculty.h"
#include "LifeFaculty.h"
#include "MusicFaculty.h"
#include "../Display/MainCamera.h"

#ifndef _WIN32
// VK_* defs
#include "../unix/KeyScan.h"
#endif


CREATURES_IMPLEMENT_SERIAL( Creature )



// to facilitate creature drowning:
const int WATER_ROOM_TYPE_1 = 8;
const int WATER_ROOM_TYPE_2 = 9;

int theCreatureCount = 0;

int GetCreatureCount()
{
	return theCreatureCount;
}

int Creature::ourNextUpdateTickOffsetToUse = 0;

void Creature::HandlePickup(Message* Msg)
{
	// Simple tests..
	if (Msg->GetFrom().IsPointerAgent())
	{
		// Pickup is from Pointer....
		if ((theApp.GetCreaturePickupStatus() == 1) ||
		    ((theApp.GetCreaturePickupStatus() == 2) && (!theApp.GetInputManager().IsKeyDown(VK_SHIFT))) ||
		    ((theApp.GetCreaturePickupStatus() == 3) && ((!theApp.GetInputManager().IsKeyDown(VK_SHIFT)) || !Music()->SelectableByUser())))
		{
			// Convert pickup message to hold hands message
			HandleStartHoldHands(Msg);
			return;
			
		}
	}

	base::HandlePickup(Msg);
}



/*********************************************************************
* Public: CanSee.
* Return true if this agent can see another
*********************************************************************/
bool Creature::CanSee(const AgentHandle& other)
{
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	_ASSERT(other2.IsValid());

	if (!base::CanSee(other))
		return false;
	Agent& agentref = other2.GetAgentReference();

	// Can't see invisible things
	if (agentref.TestAttributes(attrInvisible))
		return false;

	Vector2D pressed(agentref.WhereShouldCreaturesPressMe());
	// too high?
	if (pressed.y < myPositionVector.y-myCurrentHeight)
		return false;
	// too low?
	if (pressed.y > myPositionVector.y+2.0f*myCurrentHeight)
		return false;

	// The loop is to get the creature and the agent's base vehicle, e.g. the root vehicle of a creature on
	// a hoverboard (an open air cabin) is NULLHANDLE, i.e. the world, whereas the root vehicle of a creature
	// in a lift is just the lift.

	// Get the creature's real vehicle (ignoring open air cabins) - using NULLHANDLE to mean the world:
	AgentHandle creaturesEnclosingVehicle = AgentHandle(this);
	do {
		creaturesEnclosingVehicle = myMovementStatus==INVEHICLE ? creaturesEnclosingVehicle.GetAgentReference().GetCarrier() : NULLHANDLE;
	} while (creaturesEnclosingVehicle.IsValid() &&
		creaturesEnclosingVehicle.GetAgentReference().TestAttributes(attrOpenAirCabin));

	// Get the agent's real vehicle (ignoring open air cabins) - using NULLHANDLE to mean the world:
	AgentHandle agentsEnclosingVehicle = other;
	do {
		agentsEnclosingVehicle = myMovementStatus==INVEHICLE ? agentsEnclosingVehicle.GetAgentReference().GetCarrier() : NULLHANDLE;
	} while (agentsEnclosingVehicle.IsValid() &&
		agentsEnclosingVehicle.GetAgentReference().TestAttributes(attrOpenAirCabin));

	// if you're not in the same root vehicle you can't see the agent:
	if (creaturesEnclosingVehicle!=agentsEnclosingVehicle)
		return false;

	return true;
}


void Creature::SpeakSentence(std::string& thisSentence)
{
	myLinguisticFaculty->Say( thisSentence );
}


// retn message# (ACTIVATE1 etc) appropriate for a click at a given
// position, else return -1
// Currently set up so that a click over ANY creature's head will
// cause ACTIVATE1 = pleasure (pat), and over his body will
// cause DEACTIVATE = pain (slap)
int Creature::ClickAction(int x, int y)
{
	_ASSERT(!myGarbaged);

	// if the default action is holding hands then
	// stop right here
	if ((myDefaultClickAction == START_HOLD_HANDS) || (myDefaultClickAction == STOP_HOLD_HANDS))
		return myDefaultClickAction;

    Vector2D pt(x, y);
	Box r;
	RECT rr;

	GetAgentExtent(r);

    // Teaching pointer.  Is click over head?
    myLimbs[BODY_LIMB_HEAD]->GetBound(rr);
	if( y < (r=rr).CenterPoint().y )
        return ACTIVATE1;

    // Is click over legs?
    myLimbs[BODY_LIMB_LEFT_LEG]->GetBound(rr);
	if( (r = rr).PointInBox( pt ) )
        return DEACTIVATE;

    myLimbs[BODY_LIMB_RIGHT_LEG]->GetBound(rr);
	if( (r = rr).PointInBox( pt ) )
        return DEACTIVATE;

    return INVALID;
}


// where creatures should reach out to when trying to Activate you.
Vector2D Creature::WhereShouldCreaturesPressMe() 
{
	_ASSERT(!myGarbaged);
	return myExtremes[BODY_LIMB_HEAD];
}



void Creature::Trash()
{
	_ASSERT(!myGarbaged);
	for(int i = 0; i < noOfFaculties; i++)
	{
		myFaculties[i]->Trash();
	}

	for(int j = 0; j < noOfFaculties; j++)
	{
		delete myFaculties[j];
		myFaculties[j] = NULL;
	}

	// This must be the last line in the function
	base::Trash();
}

void Creature::Update() 
{    
	_ASSERT(!myGarbaged);

	base::Update();

	_ASSERT(!myGarbaged);

	if (myStoppedFlag)
	{
		// Only age when on the floor
		if(ContinueLoadingDataForNextAgeStage())
		{
			if(Life()->GetAge() <= NUMAGES)
			{
				Genome g(GetGenomeStore(), 0, Life()->GetSex(),	myAgeAlreadyLoaded, Life()->GetVariant());
				PreloadBodyPartsForNextAgeStage(g, Life()->GetSex());
			}
		}
	}

	if ((myUpdateTick%4)!=myUpdateTickOffset)
	{
		if (myIsHoldingHandsWithThePointer) 
		{			
			HoldHandsWithThePointer();
		}
		return;
	}

	if (!Life()->GetWhetherDead()) {
		for (int i=0; i<noOfFaculties; i++) 
		{
			myFaculties[i]->Update();
		}
		base::SetPregnancyStage(Reproductive()->GetProgesteroneLevel());
		
		if (myIsHoldingHandsWithThePointer) 
		{
			HoldHandsWithThePointer();
		}

	
		myAirQualityLocus = 1.0f;
		int roomId;
		if (theApp.GetWorld().GetMap().GetRoomIDForPoint(myLimbs[BODY_LIMB_HEAD]->CentrePoint(), roomId))
		{
			int roomType;
			if (theApp.GetWorld().GetMap().GetRoomType(roomId, roomType))
			{
				if (roomType==WATER_ROOM_TYPE_1 || roomType==WATER_ROOM_TYPE_2)
					myAirQualityLocus = 0.0f;
			}
		}

		// update crowdedness:
		myCrowdedLocus = 0.0f;
		float value;
		if (theApp.GetWorld().GetMap().GetRoomPropertyMinusMyContribution(AgentHandle(*this), value))
			myCrowdedLocus = value;
	}
}





float Creature::GetDriveLevel(int i) 
{
	_ASSERT(!myGarbaged);
	if (i<0 || i>=NUMDRIVES)
		return -1.0f;
	return myDriveLoci[i];
}

int Creature::GetHighestDrive() 
{
	_ASSERT(!myGarbaged);

	int d = 0;
	for	(int i=0; i<NUMDRIVES; i++)
	{
		if(myDriveLoci[i] > myDriveLoci[d])
			d = i;
	}
	return d;
}

bool Creature::Write(CreaturesArchive &archive) const {

	_ASSERT(!myGarbaged);

	base::Write(archive);

	archive.WriteFloatRefTarget( myInvalidLocus );
	for (int o=0; o<NUMDRIVES; o++) {
		archive.WriteFloatRefTarget( myDriveLoci[o] );
	}

	archive.WriteFloatRefTarget( myConstantLocus );
	archive.WriteFloatRefTarget( myAirQualityLocus );
	archive.WriteFloatRefTarget( myCrowdedLocus );

	for (int k=0; k<NUM_FLOATING_LOCI; k++) {
		archive.WriteFloatRefTarget( myFloatingLoci[k] );
	}

	//Save 4 (Biochemistry for last)
	for (int i=0; i<noOfFaculties; i++)
		if( i != 4 )
			archive << myFaculties[i];
	archive << myFaculties[4];

	return true;
}

bool Creature::Read(CreaturesArchive &archive) 
{
	_ASSERT(!myGarbaged);

	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		if(!base::Read(archive))
			return false;

		archive.ReadFloatRefTarget( myInvalidLocus );

		for (int o=0; o<NUMDRIVES; o++) {
			archive.ReadFloatRefTarget( myDriveLoci[o] );
		}
		archive.ReadFloatRefTarget( myConstantLocus );
		archive.ReadFloatRefTarget( myAirQualityLocus );
		archive.ReadFloatRefTarget( myCrowdedLocus );

		for (int k=0; k<NUM_FLOATING_LOCI; k++) {
			archive.ReadFloatRefTarget( myFloatingLoci[k] );
		}

		//Save 4 (Biochemistry for last)
		for (int i=0; i<noOfFaculties; i++)
			if( i != 4 )
				archive >> myFaculties[i];
		archive >> myFaculties[4];

		mySensoryFaculty		= (SensoryFaculty*)		(myFaculties[0]);
		myBrain					= (Brain*)				(myFaculties[1]);
		myMotorFaculty			= (MotorFaculty*)		(myFaculties[2]);
		myLinguisticFaculty		= (LinguisticFaculty*)	(myFaculties[3]);
		myBiochemistry			= (Biochemistry*)		(myFaculties[4]);
		myReproductiveFaculty	= (ReproductiveFaculty*)(myFaculties[5]);
		myExpressiveFaculty		= (ExpressiveFaculty*)	(myFaculties[6]);
		myMusicFaculty			= (MusicFaculty*)		(myFaculties[7]);
		myLifeFaculty			= (LifeFaculty*)		(myFaculties[8]);

		// resolve known agents in this world and time stamp (must be done after life
		// cos it is called within)
		mySensoryFaculty->ResolveFriendAndFoe();
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success
	myBrain->RegisterBiochemistry(myBiochemistry->GetChemicalConcs());

	// if cloning a creature, we have to remake our skeleton later,
	// when the moniker is known
	if (!archive.GetCloningACreature())
		RemakeSkeletonAfterSerialisation();

	return true;
}

void Creature::RemakeSkeletonAfterSerialisation()
{
	Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());

	Skeleton::CreateSkeleton(g, Life()->GetAge());
}


///////////////////////////////////////////////////////////////////////////////
//  Initialisation:

// General Initialisation
// Called during both birth and serialization.
void Creature::Init() 
{
	int i;

	myConstantLocus = 1.0f;						// constant output emitter locus
	for	(i=0; i<NUM_FLOATING_LOCI; i++)		// recep/emitter loci
		myFloatingLoci[i] = 0.0f;
	for	(i=0; i<NUMDRIVES; i++)
		myDriveLoci[i] = 0.0f;
	myAirQualityLocus = 0.0f;
	myCrowdedLocus = 0.0f;
	myInvalidLocus = 0.0f;

	myBeingTrackedFlag = false;
	myUpdateTickOffset = ourNextUpdateTickOffsetToUse;
	ourNextUpdateTickOffsetToUse = (ourNextUpdateTickOffsetToUse+1)%4;
}




// Serialisation constructor
Creature::Creature() : Skeleton() {
	++theCreatureCount;
	Init();									// set defaults
	myAgentType = AgentHandle::agentNormal | AgentHandle::agentSkeleton | AgentHandle::agentCreature;
}

// Destructor.
Creature::~Creature()
{
	--theCreatureCount;
}

// 'Birth' constructor. 
// Called during "new: crea <moniker> <sex>" macro to create a new creature.
Creature::Creature(	int family, uint32 id,
				    AgentHandle gene, int gene_index,	// your genome
					int   sex,		// your sex 0=randomly choose 1=male 2=female
					int   variant)  // 0 means choose randomly from 1 to NUM_BEHAVIOUR_VARIANTS)
		: Skeleton(gene, gene_index) // construct skeleton
{
	++theCreatureCount;
	try
	{
		ASSERT(sex==1 || sex==2);
		ASSERT(variant>=1 && variant<=8);

		myID = id;

		myMovementStatus=AUTONOMOUS;

		myFaculties[0] = (Faculty*)(mySensoryFaculty		= new SensoryFaculty());
		myFaculties[1] = (Faculty*)(myBrain					= new Brain());
		myFaculties[2] = (Faculty*)(myMotorFaculty			= new MotorFaculty());
		myFaculties[3] = (Faculty*)(myLinguisticFaculty		= new LinguisticFaculty());
		myFaculties[4] = (Faculty*)(myBiochemistry			= new Biochemistry());
		myFaculties[5] = (Faculty*)(myReproductiveFaculty	= new ReproductiveFaculty());
		myFaculties[6] = (Faculty*)(myExpressiveFaculty		= new ExpressiveFaculty());
		myFaculties[7] = (Faculty*)(myMusicFaculty			= new MusicFaculty());
		myFaculties[8] = (Faculty*)(myLifeFaculty			= new LifeFaculty());

		myBrain->RegisterBiochemistry(myBiochemistry->GetChemicalConcs());

		Init();
		Life()->SetSex(sex);
		Life()->SetVariant(variant);

		myAgentType = AgentHandle::agentNormal | AgentHandle::agentSkeleton | AgentHandle::agentCreature;
		Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());

		// make classifier
		Classifier c(family, g.GetGenus() + 1, g.GetSex());
		myClassifier = c;
	
		// put stuff in history
		std::string moniker = g.GetMoniker();
		HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
		CreatureHistory& history = historyStore.GetCreatureHistory(moniker);

		history.myGender = g.GetSex();
		history.myVariant = variant;

		ASSERT(g.GetSex() == sex);
		for (int i=0; i<noOfFaculties; i++) 
		{
			myFaculties[i]->Init(mySelf);
		}

		if (!ExpressGenes())
			myFailedConstructionException = ErrorMessageHandler::Format("creature_error", 0, "Creature::Creature", GetMoniker().c_str());

		Sensory()->PostInit();

	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0012: Unknown exception caught in creature constructor";
	}
}









// Express any genes relevant to current age:
bool Creature::ExpressGenes() 
{
	_ASSERT(!myGarbaged);

	try
	{
		// Temporary genome object read from file:
		Genome g(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant());

		for (int i=0; i<noOfFaculties; i++) {
			myFaculties[i]->ReadFromGenome(g);
		}

		if(!Skeleton::ExpressGenes(g, Life()->GetAge()))
			return false;
	}
	catch(Genome::GenomeException& e)
	{
		ErrorMessageHandler::Show(e, std::string("Creature::ExpressGenes"));
		return false;
	}
	return true;
}



void Creature::HoldHandsWithThePointer()
{
	// Sanity Check...
	if (thePointer.GetPointerAgentReference().GetHandHeldCreature() != mySelf)
	{
		StopHoldingHandsWithThePointer(mySelf,INTEGERZERO,INTEGERZERO);
		return;
	}
	// If falling, don't need to make any effort to move towards pointer
	if (!myStoppedFlag)
		return;

	// Get the location of the physical mouse
	int x = theApp.GetInputManager().GetMouseX();
	int dummy=0;
	// Convert to world coords
	theMainView.ScreenToWorld(x, dummy);

	static const float shift = 5.0f;

	float px = (float)x;
	float delta;

	if ((px >= myDownFootPosition.x-30.0f) && 
		(px <= myDownFootPosition.x+30.0f))
	{
		myStandStill = true;
		return;
	}

	myStandStill = false;
	if (px < myDownFootPosition.x)
	{
		SetDirection(WEST);
		delta = myDownFootPosition.x - px;
		if (delta > shift)
			delta = shift;
		delta = -delta;
	}
	else
	{
		SetDirection(EAST);
		delta = px - myDownFootPosition.x; 
		if (delta > shift)
			delta = shift;
	}

	Vector2D downFootNew;
	bool collision, footChange, fall;
	int wall;
	theApp.GetWorld().GetMap().ShiftCreatureAlongFloor
		(myDownFootPosition, myUpFootPosition, myMinX, myMaxX, myMinY,
			myPermiability, delta, downFootNew,
			collision, wall, footChange, fall);

	myDownslopeLocus = 0.0f;
	myUpslopeLocus = 0.0f;

	if (footChange)
	{
		if (myCurrentDownFoot == LEFT)
			myCurrentDownFoot = RIGHT;
		else
			myCurrentDownFoot = LEFT;
	}
	myDownFootPosition = downFootNew;
	UpdatePositionsWithRespectToDownFoot();
	CommitPositions();

	if (fall) 
	{
		myStoppedFlag = false;
		myVelocityVector = ZERO_VECTOR;
	}

	if (collision) 
	{
		myLastWallHit = wall;
		ExecuteScriptForEvent(SCRIPTBUMP, mySelf, INTEGERZERO, 
			INTEGERZERO);
	}
}


void Creature::StartHoldingHandsWithThePointer
	(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2)
{
	if (myIsHoldingHandsWithThePointer)
		return;

	if (!Life()->GetWhetherAlert() || Life()->GetWhetherZombie() || (myMovementStatus == INVEHICLE))
		return;

	myDoubleSpeedFlag = true;
	myIsHoldingHandsWithThePointer = true;
	myLifeFaculty->SetWhetherZombie(true);	
	myDefaultClickAction = STOP_HOLD_HANDS;
	SetAnimationString(myGaitTable[0]);
	thePointer.GetPointerAgentReference().StartHoldingHandsWithCreature(mySelf);
	ExecuteScriptForEvent(SCRIPTSTARTHOLDHANDS, from, p1, p2);
}


void Creature::HandleStartHoldHands(Message* Msg)
{
	StartHoldingHandsWithThePointer(Msg->GetFrom(), Msg->GetP1(), Msg->GetP2());
}


void Creature::StopHoldingHandsWithThePointer
	(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2)
{
	// If I'm not currently holding hands, then there's nothing to do
	if (!myIsHoldingHandsWithThePointer)
		return;
	myIsHoldingHandsWithThePointer = false;
	myDoubleSpeedFlag = false;
	myStandStill = false;
	myLifeFaculty->SetWhetherZombie(false);
	// Make the pointer stop holding hands with me
	_ASSERT(mySelf == thePointer.GetPointerAgentReference().GetHandHeldCreature());
	thePointer.GetPointerAgentReference().StopHoldingHandsWithCreature();
	ExecuteScriptForEvent(SCRIPTSTOPHOLDHANDS, from, p1, p2);
}


void Creature::HandleStopHoldHands(Message* Msg)
{
	StopHoldingHandsWithThePointer(Msg->GetFrom(), Msg->GetP1(), Msg->GetP2());
}




///////////////////////////////////////////////////////////////////////////////
// LOCI FACULTY:

// respond to an enquiry by a biochemical Receptor/Emitter object for its locus address
float* Creature::GetLocusAddress(int type,			// ID relates to RECEPTOR or EMITTER locus?
							 int organ,
							 int tissue,		// tissue containing locus
							 int locus) {		// locus on object
	_ASSERT(!myGarbaged);

	for (int i=0; i<noOfFaculties; i++)
	{
		float* locusPointer = myFaculties[i]->GetLocusAddress(type, organ, tissue, locus);
		if (locusPointer)
			return locusPointer;
	}

	if (organ==ORGAN_CREATURE)
	{
		if (type==RECEPTOR)
		{
			// Receptor loci (locus is modulated according to chemical concentration)
			switch (tissue)
			{
				case TISSUE_CIRCULATORY:
					if (locus>=LOC_FLOATING_FIRST && locus<=LOC_FLOATING_LAST)
						return &myFloatingLoci[locus-LOC_FLOATING_FIRST];
					break;

				case TISSUE_SENSORIMOTOR:
					if (locus>=LOC_GAIT0 && locus<=LOC_GAIT15)
						return &myGaitLoci[locus-LOC_GAIT0];
					break;

				case TISSUE_DRIVES:
					if (locus>=LOC_DRIVE0 && locus<LOC_DRIVE0+NUMDRIVES)
						return &myDriveLoci[locus-LOC_DRIVE0];
					break;
			}
		}
		else
		{
 			// Emitter loci (locus is a source of chemical emission)
			switch (tissue)
			{							// according to tissue...
				case TISSUE_SOMATIC:
					if (locus==LOC_MUSCLES)
						return &myMusclesLocus;
					break;

				case TISSUE_CIRCULATORY:
					if (locus>=LOC_FLOATING_FIRST && locus<=LOC_FLOATING_LAST)
						return &myFloatingLoci[locus-LOC_FLOATING_FIRST];
					break;

				case TISSUE_SENSORIMOTOR:
					if(locus>=LOC_E_GAIT0 && locus<=LOC_E_GAIT15)
						return &myGaitLoci[locus-LOC_E_GAIT0];

					switch (locus)
					{
						case LOC_CONST:
							return &myConstantLocus;
						case LOC_UPSLOPE:
							return &myUpslopeLocus;
						case LOC_DOWNSLOPE:
							return &myDownslopeLocus;
						case LOC_AIRQUALITY:
							return &myAirQualityLocus;
						case LOC_CROWDEDNESS:
							return &myCrowdedLocus;
					}
					break;

				case TISSUE_DRIVES:
					if (locus>=LOC_DRIVE0 && locus<LOC_DRIVE0+NUMDRIVES)
						return &myDriveLoci[locus-LOC_DRIVE0];
					break;
			}
		}
	}

    // Unrecognised ID.
	return &myInvalidLocus;
}

// blame Alima for these...
// when a creature ages it gets removed as the tracking object
// but if this flag is set then it will be reset
void Creature::RememberThatYouAreBeingTracked(bool amibeingtracked)
{
	_ASSERT(!myGarbaged);
	myBeingTrackedFlag = amibeingtracked;
}

// this is a helper for CAV at shut down time our norns need to
// fall asleep.  These tiredness and sleepiness values were recommended
// by Helen.  Call this 5 minutes before shutdown.
void Creature::MakeYourselfTired()
{
	_ASSERT(!myGarbaged);
	// maybe trun these values into game variables ?
myBiochemistry->SetChemical(CHEM_TIREDNESS,float(0.8));
myBiochemistry->SetChemical(CHEM_SLEEPINESS,float(0.6));
}

bool Creature::FormBodyParts()
{
	if(!Skeleton::ExpressGenes
		(Genome(GetGenomeStore(), 0, Life()->GetSex(), Life()->GetAge(), Life()->GetVariant()),
			Life()->GetAge()))
			return false;
	return true;
}





