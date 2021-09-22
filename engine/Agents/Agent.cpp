// Agent.cpp
//
//////////////////////////////////////////////
// Agent class: base class for all
// agent classes.
//////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Agent.h"
#include "../World.h"
#include "../App.h"
#include "../Display/MainCamera.h"	// for theMainView
#include "../Message.h"
#include "Agent.h"
#include "../Creature/Creature.h"
#include <string>
#include "../AgentManager.h"
#include "../Map/Map.h"
#include "../Display/ErrorMessageHandler.h"
#include "../Caos/DebugInfo.h"
#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#else
#include <sstream>
#endif
#include "../Caos/RuntimeErrorDialog.h"
#include "Vehicle.h"
#include <algorithm>
#include "../Caos/DisplayHandlers.h"
#include "../C2eServices.h"

#include "../Caos/AutoDocumentationTable.h"

CREATURES_IMPLEMENT_SERIAL( Agent )

#ifdef AGENT_PROFILER
	int Agent::ourAgentProfilerTicks = 0;
	double Agent::ourAgentProfilerPaceTotal = 0.0;
#endif

int theAgentCount = 0;

int GetAgentCount()
{
	return theAgentCount;
}

TableSpec ourAttributeFlags[] =
{
	TableSpec("Attribute Flags"),
	TableSpec("Number (for @#ATTR@)", "Name", "Description"),

	TableSpec("1", "Carryable", "Can be picked up by things other than creatures, vehicles and the pointer."),
	TableSpec("2", "Mouseable", "Can be picked up by the mouse."),
	TableSpec("4", "@Activateable@", "Can be activated using the mouse - otherwise @#CLAC@ and @#CLIK@ style events don't get sent when you click on the agent."),
	TableSpec("8", "@Greedy Cabin@", "When set on a vehicle, it will automatically pick up things dropped in its cabin."),
	TableSpec("16", "@Invisible@", "Creatures don't see you - applies to @#ESEE@ and @#STAR@ on creatures, as well as internal creature code."),
	TableSpec("32", "@Floatable@", "Agent floats relative to the screen.  Move it with @#FLTO@.  If you call @#FREL@, the agent will float relative to another agent instead of the screen."),
	TableSpec("64", "Suffer Collisions", "Will collide with room boundaries, according to its @#PERM@."),
	TableSpec("128", "Suffer Physics", "Agent falls with proper physics, including gravity, air resistance and friction.  Otherwise, it simply moves with velocity."),
	TableSpec("256", "Camera Shy", "Agent can't be seen on a remote camera (@#PAT: CMRA@) or photograph (@#SNAP@).  For vehicles, the contents is shy as well."),
	TableSpec("512", "Open Air Cabin", "When set on a vehicle, it allows creatures to see and activate its passengers.  The default behaviour is that they can't."),
};

int dummyAttributeFlags = AutoDocumentationTable::RegisterTable(ourAttributeFlags, sizeof(ourAttributeFlags));

TableSpec ourCreaturePermissions[] =
{
	TableSpec("Creature Permissions"),
	TableSpec("Number (for @#BHVR@)", "Name", "Description"),

	TableSpec("1", "Activate 1", "Creature can activate 1 this agent."),
	TableSpec("2", "Activate 2", "Creature can activate 2 this agent."),
	TableSpec("4", "Deactivate", "Creature can deactivate this agent."),
	TableSpec("8", "Hit", "Creature can hit this agent."),
	TableSpec("16", "Eat", "Creature can eat this agent."),
	TableSpec("32", "Pick Up", "Creature can pick up this agent."),
};

int dummyCreaturePermissions = AutoDocumentationTable::RegisterTable(ourCreaturePermissions, sizeof(ourCreaturePermissions));

//------------------------------------------------------------------
//
// Agent stuff proper
//


// Serialisation & subclass constructor
Agent::Agent():
	myAgentType(AgentHandle::agentNormal),
	myReferenceCount(0),
	myEntityImage(NULL),
	myVelocityXVariable(myVelocityVector.x, myStoppedFlag),
	myVelocityYVariable(myVelocityVector.y, myStoppedFlag)
{
	++theAgentCount;
	try
	{
		Init();
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0013: Unknown exception caught in agent constructor";
	}
	if (!myFailedConstructionException.empty())
	{
		myAgentType = 0;
		return;
	}

}


Agent::~Agent()
{
	--theAgentCount;
	_ASSERT(myGarbaged);
	_ASSERT(myReferenceCount == 0);
}



// Virtual
uint32 Agent::GetMagicNumber()
{
	return 3141592654U;
}


void Agent::SpeakSentence(std::string& thisSentence)
{
	_ASSERT(!myGarbaged);
	if (GetVoice().GetCurrentVoice() == "")
		return;
	if (!GetVoice().BeginSentence( thisSentence ))
		return;
	int32 delay = 0;
	uint32 sound;
	while (GetVoice().GetNextSyllable(sound,delay)) {
			SoundEffect(sound,delay);
	}

}


bool Agent::IsConnectionValid
	(Port* in, Port* out, bool& warning)
{
	static const float safeDist = theApp.GetMaximumDistanceBeforePortLineWarns();
	static const float maxDist = theApp.GetMaximumDistanceBeforePortLineSnaps();
	// Get two positions
	Vector2D inPosition = in->GetWorldPosition();
	Vector2D outPosition = out->GetWorldPosition();

	// Get distance between
	float dist = inPosition.SquareDistanceTo(outPosition);
	if (dist > safeDist*safeDist)
		warning = true;
	else
		warning = false;

	if (dist > maxDist*maxDist)
	{
		return false;
	}

	Agent& inAgent = in->GetOwner().GetAgentReference();
	Agent& outAgent = out->GetOwner().GetAgentReference();

	int inID;
	if (!theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
		(inAgent.GetCentre(), inID))
		return false;
	int outID;
	if (!theApp.GetWorld().GetMap().GetMetaRoomIDForPoint
		(outAgent.GetCentre(), outID))
		return false;

	if (inID != outID)
		return false;

	AgentHandle inCarrier = inAgent.GetCarrier();
	if ((inCarrier.IsVehicle()) && 
		(inCarrier.GetAgentReference().GetMovementStatus() == FLOATING))
		return false;
	AgentHandle outCarrier = outAgent.GetCarrier();
	if ((outCarrier.IsVehicle()) && 
		(outCarrier.GetAgentReference().GetMovementStatus() == FLOATING))
		return false;
	return true;
}


void Agent::InitialisePickupPointsAndHandles()
{
	_ASSERT(!myGarbaged);
	int i;
	if (GetPrimaryEntity())
	{
		// fill in invalid positions for all PUPT / PUHLs
		for(i = 0; i < GetPrimaryEntity()->GetAbsolutePoseMax(); i++)
		{
			myPointsWhereIamPickedUp.push_back(INVALID_POSITION);	// Indexed by pose.
			myPointsIUseToPickUpAgents.push_back(INVALID_POSITION);
		}
	}
}


// Basic initialisation used by constructors
void Agent::Init()
{
	int i;

#ifdef AGENT_PROFILER
	myAgentProfilerCumulativeTime = 0;
	myAgentProfilerCumulativeTicks = 0;
#endif

	myResetLines = false;
	myUpdateTick = 0;

	// Used for bounding box checks
	myWidthTemp = -1.0f;
	myHeightTemp = -1.0f;

	myHighlightedFlag = false;
	myHighlightColour.r = 0;
	myHighlightColour.g = 0;
	myHighlightColour.b = 0;
	myMarchingAntCount = 0;

	myDrawMirroredFlag = false;

    myMovementStatus = AUTONOMOUS;	// default to fixed locn (not floating etc)

    myAttributes = 0;				// default not carryable, etc.
	myCreaturePermissions = 0;		// default to creature can do nothing.

	myClickActionState = -1;		// default to no cycling of click state
		
	myCurrentWidth= 0.0f;
	myCurrentHeight =0.0f;


	for (i = 0; i < NUMBER_OF_STATES_I_CAN_HANDLE_FOR_CLICKACTIONS; i++)
	{
		// initialise the actions to activate1
		myDefaultClickActionCycles[i] =ACTIVATE1;
	}

	myCarrierAgent = NULLHANDLE;				// no carrier
	myCarriedAgent = NULLHANDLE;			// not carrying anything
	myInputMask = 0;				// not interested in any input events

	myTimerRate = 0;					// default = issue no TIMER events
	myTimer = 0;
	myRunning = true;           // Agent's update should be called

	mySoundHandle=-1;					// No controlled sound is active
	mySoundName=0;
	mySoundLooping=FALSE;


	myPermiability = 50;
	myLastWallHit = -1;
	myGeneralRange = 500.0f;		// how far away agents can hear and see things
								// by default.  Can change with RNGE command.
	myGravitationalAcceleration = 0.3f;
	myStoppedFlag = false;
	
	myPositionVector = Vector2D(-9876.0f, -9876.0f); // off the display
	myFloatingPositionVector = Vector2D(-9876.0f, -9876.0f); // off the display
	myInvalidPosition = true;

	myVelocityVector = ZERO_VECTOR;
	myElasticity = 100;
	myCollisionFactor = 1.0f;
	myAeroDynamicPercentage = 0;
	myAeroDynamicFactor = 1.0f;
	myFrictionFactor = 0.0f;
	myFrictionPercentage = 100;

			// set up the PortBundle

	myDefaultClickAction = ACTIVATE1;

	myPrevCAIndex = myCAIndex = -1;
	myPrevCAIncrease = myCAIncrease = 0.0;
	myPrevCAIsNavigable = myCAIsNavigable = false;
	myPrevRoomWhenEmitting = -1;
	myCAProcessingState = stateNotProcessing;

	for (i=0; i<GLOBAL_VARIABLE_COUNT; ++i) {
		myGlobalVariables[i].SetInteger(0);
	}

	// by default do pixel perfect testing for Hit Tests
	myPixelPerfectHitTestFlag = true;
	myFailedConstructionException = "";

	myGarbaged = false;
	myImpendingDoom = false;
	mySelf = AgentHandle(this);
	myPorts.Init( mySelf );
}


// virtual
bool Agent::MoveToSafePlaceGivenCurrentLocation()
{
	Vector2D positionSafe;
	bool ok = theApp.GetWorld().GetMap().FindSafeAgentLocation
			(myPositionVector, myCurrentWidth, myCurrentHeight, 
			 myPermiability, positionSafe);
	if (ok)
		MoveTo(positionSafe.x, positionSafe.y);
	return ok;
}

// virtual
bool Agent::MoveToSafePlace(const Vector2D& positionPreferred)
{
	Vector2D positionSafe;
	bool ok = theApp.GetWorld().GetMap().FindSafeAgentLocation
			(positionPreferred, myCurrentWidth, myCurrentHeight, 
			 myPermiability, positionSafe);
	if (ok)
		MoveTo(positionSafe.x, positionSafe.y);
	return ok;
}




Vector2D Agent::WhereShouldCreaturesPressMe() {
	_ASSERT(!myGarbaged);
	Vector2D pressPoint(myPositionVector.x + (GetWidth()/2.0f), myPositionVector.y + GetHeight());
	return pressPoint;
}



Port* Agent::IsPointOverPort(Vector2D& p, bool& inputnotoutput)
{
	_ASSERT(!myGarbaged);

	int in = myPorts.GetInputPortCount();
	int out = myPorts.GetOutputPortCount();
	if ((in == 0) && (out == 0))
		// Most agents don't have ports, bomb out straight away
		return NULL;

	int i;
	InputPort *ip;
	OutputPort *op;
	Vector2D pos;

	// Search for the point lying over an input port
	for (i=0;i<in;++i) {
		ip = myPorts.GetInputPort(i);
		if (ip == NULL) 
			continue;
		pos = myPositionVector + ip->GetRelativePosition();
		if ((fabsf(pos.x-p.x) > 10.0f) || (fabsf(pos.y-p.y) > 10.0f))
			continue;
		inputnotoutput = true;
		return ip;
	}

	// Search for the point lying over an output port
	for (i=0;i<out;++i) {
		op = myPorts.GetOutputPort(i);
		if (op == NULL) 
			continue;
		pos = myPositionVector + op->GetRelativePosition();
		if ((fabsf(pos.x-p.x) > 10.0f) || (fabsf(pos.y-p.y) > 10.0f))
			continue;
		inputnotoutput = false;
		return op;
	}
	return NULL;
}



// virtual
float Agent::GetDistanceToObstacle(int direction)
{
	_ASSERT(!myGarbaged);

	bool collision;
	float distanceCollision;

	if (myMovementStatus == AUTONOMOUS)
	{
		theApp.GetWorld().GetMap().TestForAgentCollisionInRoomSystem
			(myPositionVector, myCurrentWidth, myCurrentHeight, direction, 
			 myGeneralRange, myPermiability, collision, 
			 distanceCollision);
	}
	else if (myMovementStatus == INVEHICLE)
	{
		Box cabin;
		myCarrierAgent.GetVehicleReference().GetCabinExtent(cabin);
		theApp.GetWorld().GetMap().TestForAgentCollisionInRectangle
			(myPositionVector, myCurrentWidth, myCurrentHeight, direction, 
			 myGeneralRange, cabin.left, cabin.right, cabin.top, 
			 cabin.bottom, collision, distanceCollision);
	}
	else
		return FLT_MAX;

	if (collision)
		return distanceCollision;
	else
		return FLT_MAX;
}



int Agent::GetLastWallHit() 
{
	_ASSERT(!myGarbaged);
	return myLastWallHit;
}


// Handle a message from another agent
void Agent::HandleMessage(Message* Msg)
{
	_ASSERT(!myGarbaged);

	bool isCreature = Msg->GetFrom().IsCreature();
    switch (Msg->GetMsg()) {
	// activation events - call virtual function
    case ACTIVATE1:
		
		if (isCreature && !TestCreaturePermissions(permCanActivate1))
			break;
        HandleActivate1(Msg);
        break;

    case ACTIVATE2:
		// creaturecast
		if (isCreature && !TestCreaturePermissions(permCanActivate2))
			break;
        HandleActivate2(Msg);
        break;

    case DEACTIVATE:
		if (isCreature && !TestCreaturePermissions(permCanDeactivate))
			break;
        HandleDeactivate(Msg);
        break;

    case HIT:
		if (isCreature && !TestCreaturePermissions(permCanHit))
			break;
        HandleHit(Msg);
        break;

    case PICKUP:
		if (isCreature && !TestCreaturePermissions(permCanPickUp))
			break;
        HandlePickup(Msg);
        break;

    case DROP:
        HandleDrop(Msg);
        break;

	// system events - execute any script for this obj/event

    case EAT:
		if (isCreature && !TestCreaturePermissions(permCanEat))
			break;
        HandleEat(Msg);
        break;

	case START_HOLD_HANDS:
        HandleStartHoldHands(Msg);
		break;

	case STOP_HOLD_HANDS:
		HandleStopHoldHands(Msg);
		break;

// Please remember! (tie a knot in your pyjamas),
// update the documentation table in Message.cpp if
// you add a new message, or change the meaning
// of any of these

	default:
		// Message is not one of the fixed messages defined in Message.h.
		// This is used to implement generic inter-agent communication
		// for injected COBs.
		if( Msg->GetMsg() >= SCRIPTUIMIN && Msg->GetMsg() <= SCRIPTUIMAX )
			HandleUI( Msg );
		else
			HandleOther(Msg);
        break;
    }
}


// Default message handlers for agents (virtual, so may be
// overloaded by subclasses). As base class is abstract, these handlers
// do nothing. Subclasses must define their own handlers wherever an action
// is required for this message
// Message is passed as a param, so can determine sender & supp. data
// Default message handlers for Agents (virtual, so may be
// overloaded by subclasses)
void Agent::HandleActivate1(Message* Msg)
{
	_ASSERT(!myGarbaged);

	AgentHandle from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
   	int code =
		ExecuteScriptForEvent(SCRIPTACTIVATE1, from, p1, p2);
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(Msg->GetFrom().IsCreature()))
	{
		char s[1000];
		sprintf(s, "No activate 1 script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}


void Agent::HandleActivate2(Message* Msg)
{
	_ASSERT(!myGarbaged);
	AgentHandle from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
  	int code =
		ExecuteScriptForEvent(SCRIPTACTIVATE2, from, p1,p2);
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(Msg->GetFrom().IsCreature()))
	{
		char s[1000];
		sprintf(s, "No activate 2 script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}


void Agent::HandleDeactivate(Message* Msg)
{
	_ASSERT(!myGarbaged);

	AgentHandle from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
   	int code =
		ExecuteScriptForEvent(SCRIPTDEACTIVATE, from, p1, p2 );
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(Msg->GetFrom().IsCreature()))
	{
		char s[1000];
		sprintf(s, "No deactivate script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}


void Agent::HandleHit(Message* Msg)
{
	_ASSERT(!myGarbaged);

	AgentHandle from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
	int code =
		ExecuteScriptForEvent(SCRIPTHIT, from, p1, p2);
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(Msg->GetFrom().IsCreature()))
	{
		char s[1000];
		sprintf(s, "No hit script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}

void Agent::HandleStartHoldHands(Message* Msg)
{
	_ASSERT(!myGarbaged);
	// handled by the creature not here	
}

void Agent::HandleStopHoldHands(Message* Msg)
{
	_ASSERT(!myGarbaged);
	// handled by the creature not here
}

bool RecursiveCarrierTester(AgentHandle& what, AgentHandle& potentialCarrier)
{
	if (what == potentialCarrier)
		return false;
	// Jump up through the potentialCarrier's carrier chain until we hit nullhandle or "what"
	while (potentialCarrier.GetAgentReference().GetCarrier().IsValid() && potentialCarrier.GetAgentReference().GetCarrier() != what)
		potentialCarrier = potentialCarrier.GetAgentReference().GetCarrier();
	if (potentialCarrier.GetAgentReference().GetCarrier().IsInvalid())
		return true;
	return false;
}


bool Agent::TryToBePickedUp
	(AgentHandle& newCarrierAgent, CAOSVar& p1, CAOSVar& p2)
{
	if (newCarrierAgent.IsInvalid())
		// Fail
		return false;

	bool pickupOK =
		// pickup request from the pointer...
		newCarrierAgent.IsPointerAgent() ? (TestAttributes(attrMouseable) ? true : false) :
		// pickup request from a Creature...
		newCarrierAgent.IsCreature() ? (TestCreaturePermissions(permCanPickUp) ? true : false) :
		// pickup request from Vehicle... (they can pick up anything)
		newCarrierAgent.IsVehicle() ? RecursiveCarrierTester(mySelf,newCarrierAgent) :
		// pickup requst from anything else...
		(TestAttributes(attrCarryable) ? RecursiveCarrierTester(mySelf,newCarrierAgent) : false);
	if (!pickupOK)
		// Fail
		return false;

	if (myCarrierAgent == newCarrierAgent)
		// We are already being carried by the requesting agent, fail
		return false;

	// Ask the new carrier if it can carry me
	if (!newCarrierAgent.GetAgentReference().CanStartCarrying(mySelf))
		return false;

	// If I'm currently being carried by another agent, then make it
	// drop me
	AgentHandle oldCarrier(myCarrierAgent);
	if (oldCarrier.IsValid())
	{
		oldCarrier.GetAgentReference().StopCarrying(mySelf);
		if (oldCarrier.IsVehicle()) 
			ExecuteScriptForEvent(SCRIPTDROPBYVEHICLE, oldCarrier, INTEGERZERO, INTEGERZERO);
		else 
			ExecuteScriptForEvent(SCRIPTDROP, oldCarrier, INTEGERZERO, INTEGERZERO);
	}

	// Make the new carrier start carrying me
	newCarrierAgent.GetAgentReference().StartCarrying(mySelf);

	MarkValidPositionNow();
	
	if (newCarrierAgent.IsVehicle()) 
		ExecuteScriptForEvent(SCRIPTPICKUPBYVEHICLE, newCarrierAgent, p1, p2);
	else 
		ExecuteScriptForEvent(SCRIPTPICKUP, newCarrierAgent, p1, p2);

	return true;
}


bool Agent::TryToBeDropped
	(AgentHandle& fromAgent, CAOSVar& p1, CAOSVar& p2, bool forceDrop)
{
	if (myCarrierAgent.IsInvalid())
		return false;

	AgentHandle oldCarrier = myCarrierAgent;

	if (forceDrop)
	{
		// Make the current carrier drop me
		oldCarrier.GetAgentReference().StopCarrying(mySelf);
		// Move myself into a safe place on the map, relative to where I am now
		MoveToSafePlaceGivenCurrentLocation();
		if (oldCarrier.IsVehicle())
			ExecuteScriptForEvent( SCRIPTDROPBYVEHICLE, fromAgent, p1, p2 );
		else
			ExecuteScriptForEvent( SCRIPTDROP, fromAgent, p1, p2 );
	}
	else 
	{
		bool findSafePlace;
		AgentHandle v;
		if (!oldCarrier.GetAgentReference().CanStopCarrying(mySelf, v,
			findSafePlace))
		{
			// The pointer is unable to drop me where I currently am
			GoBackToLastClickActionState();
			// Cannot drop at all
			return false;
		}

		// Make the current carrier drop me
		oldCarrier.GetAgentReference().StopCarrying(mySelf);
		if (v.IsValid())
		{
			v.GetAgentReference().StartCarrying(mySelf);
		}
		else if (findSafePlace)
		{
			// Move myself into a safe place on the map, relative to where I am now
			MoveToSafePlaceGivenCurrentLocation();
		}
		if (oldCarrier.IsVehicle())
			ExecuteScriptForEvent( SCRIPTDROPBYVEHICLE, fromAgent, p1, p2 );
		else
			ExecuteScriptForEvent( SCRIPTDROP, fromAgent, p1, p2 );
		if (v.IsValid())
			ExecuteScriptForEvent( SCRIPTPICKUPBYVEHICLE, fromAgent, INTEGERZERO, INTEGERZERO);
	}
	return true;
}


void Agent::HandlePickup(Message* Msg)
{
	_ASSERT(!myGarbaged);
	AgentHandle from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
    (void)TryToBePickedUp( from, p1, p2 );
}

void Agent::HandleDrop(Message* Msg) 
{
	_ASSERT(!myGarbaged);

	AgentHandle from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
	(void)TryToBeDropped(from, p1, p2, false);
}








void Agent::HandleEat(Message* Msg)
{
	_ASSERT(!myGarbaged);
  
	AgentHandle from( Msg->GetFrom() );
	CAOSVar p1( Msg->GetP1() );
	CAOSVar p2( Msg->GetP2() );
	int code =
		ExecuteScriptForEvent(SCRIPTEAT, from, p1, p2 );
	if ((code == EXECUTE_SCRIPT_NOT_FOUND) && 
		(Msg->GetFrom().IsCreature()))
	{
		char s[1000];
		sprintf(s, "No eat script available for <%d, %d, %d> despite BHVR\n", 
 			myClassifier.Family(), myClassifier.Genus(), myClassifier.Species());
		ErrorMessageHandler::NonLocalisable(s, std::string("Agent::Handle[etc]"));
	}
}

void Agent::HandleUI(Message* Msg)
{
	_ASSERT(!myGarbaged);
	HandleOther( Msg );
}

/*********************************************************************
* Public: HandleOther.
*********************************************************************/
void Agent::HandleOther(Message* Msg)
{
	_ASSERT(!myGarbaged);

	// Execute macro event script Msg.
	ExecuteScriptForEvent(Msg->GetMsg(), Msg->GetFrom(), Msg->GetP1(), Msg->GetP2());
}

// If this agent is touching (within the bounds of) any other agent
// whose Attributes (CARRYABLE, etc) agree with that given, return the
// address of the topmost such agent. Else, return NULL

// The AttrFields & AttrState params are used to define the kind of agent
// I'm interested in. For example:
//      - if AttrFields==CARRYABLE and AttrState==CARRYABLE
//        then the fn will look for a carryable agent.
//      - if AttrFields==CARRYABLE|LIVING and AttrState==CARRYABLE
//        then fn will look for a CARRYABLE, non-LIVING thing.
//      - if AttrFields and AttrState ==0, then fn will look for ANY agent

AgentHandle Agent::IsTouching( DWORD AttrFields, // which Attr bit(s) I care about
                            DWORD AttrState)  // what state I want them to be
{
	_ASSERT(!myGarbaged);
    Vector2D ptThis;

	// Get my boundary...
	if (mySelf == thePointer) 
	{
        // If I am the pointer, use my hotspot.
		ptThis = thePointer.GetPointerAgentReference().GetHotSpot();
	}
	else
	{
        // Use centre base of agent.
        Box rectThis;
        GetAgentExtent(rectThis);
		ptThis.x = rectThis.left + (rectThis.Width() / 2.0f);
        ptThis.y = rectThis.bottom;
	}
	return theAgentManager.WhatAmITouching(ptThis,mySelf,AttrFields,AttrState);                              
}


// Calculate volume (-5000 -> 0) and panning values (-10000 -> 10000)
// Given the agents current location
void Agent::SoundLevels(long &volume, long &pan)
{
	_ASSERT(!myGarbaged);

	if(theApp.PlayAllSoundsAtMaximumLevel())
	{
// TODO: get rid of directsound constants!
#ifdef _WIN32
		volume = DSBVOLUME_MAX;
		pan = DSBPAN_CENTER;
#else
		// ARE THESE CORRECT?
		volume = 0;
		pan = 0;
#endif
		return;
	}

	int x=Map::FastFloatToInteger(myPositionVector.x);
	int y=Map::FastFloatToInteger(myPositionVector.y);

	Box screen;
	RECT temp;
	theMainView.GetViewArea(temp);
	screen = temp;

	int cx=(temp.left+temp.right)/2;
	int cy=(temp.top+temp.bottom)/2;

	x-=cx;
	y-=cy;

	if(screen.Width() == 0.0f)
	{
		volume = SoundMinVolume;
		return;
	}

	pan=Map::FastFloatToInteger
		((x*5000.0f) / Map::FastFloatToInteger(screen.Width()));
	if (pan<-10000)
	{
		pan=-10000;
	}
	else
	{
		if (pan>10000)
		{
			pan=10000;
		}
	}

	if (x<0)
	{
		x=-x;
	}

	if (y<0)
	{
		y=-y;
	}

	if (x>=Map::FastFloatToInteger(screen.Width()/2.0f))
	{
		x-=Map::FastFloatToInteger(screen.Width()/2.0f);
	}
	else
	{
		x=0;
	}
	
	if (y>=Map::FastFloatToInteger(screen.Height()/2.0f))
	{
		y-=Map::FastFloatToInteger(screen.Height()/2.0f);
	}
	else
	{
		y=0;
	}

	volume = ( SoundMinVolume * x) / ( Map::FastFloatToInteger(screen.Width()) )
		   + ( SoundMinVolume * y) / ( Map::FastFloatToInteger(screen.Height()) );
	
	if (volume < SoundMinVolume)
	{
		volume = SoundMinVolume;
	}

}


bool Agent::Visibility(int scope)
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
	{
		return myEntityImage->Visible(scope);
	}

	return false;
}

void Agent::Hide()
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
		myEntityImage->Unlink();

}

void Agent::Show()
{
	_ASSERT(!myGarbaged);

	// must force the entity to show because some entity Images
	// ingore the link command otherwise see cloned entity image and Guy
	if(myEntityImage)
		myEntityImage->Link(true);
}

Vector2D Agent::GetMapCoordOfWhereIPickUpAgentsFrom(int pose /*= -1*/, bool bRelative /* = false */)
{
	_ASSERT(!myGarbaged);

	// default is my position or global point
	Vector2D pos;
	if (bRelative)
	{
		pos = ZERO_VECTOR;
	}
	else
	{
		pos = myPositionVector;
	}

	if (GetPrimaryEntity())
	{
		int imageIndex = pose;
		if (imageIndex == -1)
			imageIndex = GetPrimaryEntity()->GetAbsolutePose();
	
		ASSERT(myPointsIUseToPickUpAgents.size() == GetPrimaryEntity()->GetAbsolutePoseMax());
		if (imageIndex >= 0 && imageIndex < myPointsIUseToPickUpAgents.size())
		{
			// the first point is 
			Vector2D currentpos = myPointsIUseToPickUpAgents[imageIndex];
			if (currentpos != INVALID_POSITION)
			{
				pos.x += currentpos.x;
				pos.y += currentpos.y;
				return pos;
			}
		}
	}
	return pos;
}

Vector2D Agent::GetMyOffsetOfWhereIAmPickedUpFrom(int pose /*= -1*/)
{
	_ASSERT(!myGarbaged);

	// default pickup point is near the top in the middle:
	Vector2D pos(GetWidth()/2.0f,GetHeight()/4.0f);

	if(GetPrimaryEntity())
	{
		int imageIndex = pose;
		if(imageIndex == -1)
			imageIndex = GetPrimaryEntity()->GetAbsolutePose();
		
		ASSERT(myPointsWhereIamPickedUp.size() == GetPrimaryEntity()->GetAbsolutePoseMax());
		if (imageIndex >= 0 && imageIndex < myPointsWhereIamPickedUp.size())
		{
			Vector2D currentpos = myPointsWhereIamPickedUp[imageIndex];
			if (currentpos != INVALID_POSITION)
				return currentpos;
		}
	}

	return pos;
}

EntityImage* Agent::GetPrimaryEntity()
{
	_ASSERT(!myGarbaged);

	return myEntityImage;
}


int32 Agent::GetPlaneForCarriedAgent()
{
	_ASSERT(!myGarbaged);

	// default is one less than your plane
	int32 plane = -1;
	if(myEntityImage)
	{
		plane = myEntityImage->GetPlane() -1;
	}
	return plane;
}

bool Agent::GetRoomID(int &roomID) //this would be much quicker if the agent kept track of 
				//current room
{
	_ASSERT(!myGarbaged);

	if (myMovementStatus == INVEHICLE)
	{
		int cabinRoom = myCarrierAgent.GetVehicleReference().GetCabinRoom();
		if (cabinRoom > -1)
		{
			roomID = cabinRoom;
			return true;
		}
	}

	return theApp.GetWorld().GetMap().GetRoomIDForPoint(GetCentre(), 
		roomID );
}


void Agent::HandleConnections()
{
	_ASSERT(!myGarbaged);

	int count = myPorts.GetInputPortCount();
	if (count == 0)
		return;

	InputPort *inport;
	OutputPort *outport;
	int i;
	Vector2D inpos;
	Vector2D outpos;
	bool valid, warning, broke=false;

	for (i=0; i<count; ++i) {
		inport = myPorts.GetInputPort(i);
		if (inport == NULL)
			continue;
		inport->Relax(20);
		outport = inport->GetConnectedPort();
		if (outport == NULL)
			continue;

		valid = IsConnectionValid(inport, outport, warning);
		if (!valid)
		{
			// Break the connection
			inport->DisconnectFromOutputPort();
			broke = true;
			theApp.GetWorld().WriteMessage( mySelf, 
				outport->GetOwner(), SCRIPTCONNECTIONBREAK, 
				INTEGERZERO, INTEGERZERO, 0 );
			continue;
		}

		inpos = myPositionVector + inport->GetRelativePosition();
		outpos = outport->GetOwner().GetAgentReference().GetPosition() +
			outport->GetRelativePosition();

		int r, g, b;
		if (warning)
		{
			r = 155 + inport->GetExcitementLevel()/2;
			g = 0;
			b = 0;
		}
		else 
		{
			r = 55 + inport->GetExcitementLevel();
			g = 155 + (inport->GetExcitementLevel()/2);
			b = 155 + (inport->GetExcitementLevel()/2);
		}
		DrawLine(Map::FastFloatToInteger(outpos.x), 
			Map::FastFloatToInteger(outpos.y), 
			Map::FastFloatToInteger(inpos.x), 
			Map::FastFloatToInteger(inpos.y), 
			r, g, b, 4, 4, myMarchingAntCount);
		myMarchingAntCount = (myMarchingAntCount - 1) % 8;
		myResetLines = true;
	}

	if (broke)
	{
		ExecuteScriptForEvent(SCRIPTCONNECTIONBREAK, mySelf,
								INTEGERZERO, INTEGERZERO);
	}
}


bool Agent::SetEmission(int caIndex, float value)
{
	_ASSERT(!myGarbaged);
	if ((caIndex < -1) || (caIndex > CA_PROPERTY_COUNT))
		return false;
	if ((myCAIndex == caIndex) && (myCAIncrease == value))
		return true;
	myCAIndex = caIndex;
	myCAIncrease = value;
	myCAProcessingState = stateSettingsChange;
	myCAIsNavigable = theApp.GetWorld().GetMap().IsCANavigable(caIndex);
	return true;
}


void Agent::HandleCA()
{
	_ASSERT(!myGarbaged);

	Map &map = theApp.GetWorld().GetMap();

	switch(myCAProcessingState)
	{
	case stateNotProcessing:
		return;

	case stateStableSettings:
		// We have stable settings. This means that we have not done a change.
		// This means that we only need check if the room we are (or are not) in has changed...
		if (myCAIsNavigable)
		{
			// We are emitting a navigable CA :)
			int roomID;
			GetRoomID(roomID);	// We can ignore the bool return code as roomID is -1 for no room.
			if (roomID != myPrevRoomWhenEmitting)
			{
				// Right then, we have moved room, so let's process it....
				if (myPrevRoomWhenEmitting != -1)
				{
					map.AlterCAEmission(myPrevRoomWhenEmitting,myPrevCAIndex,-myPrevCAIncrease);
				}
				// Now let's load our new values into the new room....
				if (roomID != -1)
				{
					map.AlterCAEmission(roomID,myCAIndex,myCAIncrease);
				}
				// As we are on stablesettings mode, simply propogate the new roomID...
				myPrevRoomWhenEmitting = roomID;
			}
		}
		else
		{
			// Not a navigable CA, so update if it's our turn to do so.
			if (map.GetCAIndex() == myCAIndex)
			{
				int roomID;
				if (GetRoomID(roomID))
					map.IncreaseCAInput(roomID, myCAIncrease);
			}
		}
		break;
	case stateSettingsChange:
		// We have a settings change which means one of four state changes.
		// Either:
		//         Non -> Nav
		//         Nav -> Non
		//         Nav -> Nav
		//         Non -> Non

		// State changes happen in two parts. First of all, we delete our old navigational effect.
		if (myPrevCAIsNavigable)
		{
			if (myPrevRoomWhenEmitting != -1)
				map.AlterCAEmission(myPrevRoomWhenEmitting,myPrevCAIndex,-myPrevCAIncrease);
		}

		// Now we propogate current CA settings into prev settings...
		myPrevCAIsNavigable = myCAIsNavigable;
		myPrevCAIncrease = myCAIncrease;
		myPrevCAIndex = myCAIndex;
		// myPrevRoomWhenEmitting = <WAIT>

		// Are we disabling the CA Emissions (I.E. is our CA index now -1)
		if (myPrevCAIndex == -1)
		{
			// We are disabling CA Emissions for this Agent.
			myCAProcessingState = stateNotProcessing;
			return;
		}

		// Right then, we need to decide what's happening with the CA....
		if (myCAIsNavigable)
		{
			// Okay then, we need to process an AlterCAEmission....
			Map &map = theApp.GetWorld().GetMap();
			int roomID;
			GetRoomID(roomID);
			myPrevRoomWhenEmitting = roomID;
			if (roomID != -1)
			{
				map.AlterCAEmission(roomID,myCAIndex,myCAIncrease);
			}
		}
		// Having processed the state change, we enter stable mode.
		myCAProcessingState = stateStableSettings;
	}
}


// Called by every agent every tick
// virtual
void Agent::Update()
{
	_ASSERT(!myGarbaged);

	if (myResetLines)
	{
		ResetLines();
		myResetLines = false;
	}

	myUpdateTick++;

	if (myUpdateTick % 5 == 0)
		HandleSound();

	// maybe extend this to do nothing if not in a valid position?
	if (!myInvalidPosition)
		HandleMovement();

	HandleCA();

	// Check for bounding box changes that take us out of the map
	float width = GetWidth();
	float height = GetHeight();
	bool ok = true;
	if (!myInvalidPosition && TestAttributes(attrSufferCollisions) &&
		(myMovementStatus == AUTONOMOUS) &&
		(myWidthTemp > 0.0f) && (myHeightTemp > 0.0f) && !mySelf.IsCreature()) 
	{
		if ((width != myWidthTemp) || (height != myHeightTemp)) {
			ok = TestCurrentLocation(false);
		}
	}
	if (!ok)
	{
		std::string message = ErrorMessageHandler::Format("invalid_map_error", 0, std::string("Agent::Update"), 
			Map::FastFloatToInteger(myWidthTemp), 
			Map::FastFloatToInteger(myHeightTemp),
			Map::FastFloatToInteger(width), 
			Map::FastFloatToInteger(height));
		CAOSMachine::RunError temporaryRunError(message.c_str());
		HandleRunError(temporaryRunError);
	}
	myWidthTemp = width;
	myHeightTemp = height;

	// Deal with input events if there are any
	if( theApp.GetInputManager().GetPendingMask() & myInputMask )
	{
		HandleInputEvents();
		_ASSERT(!myGarbaged);
	}

	// Deal with any input-to-output connections heading out of this agent
	HandleConnections();


	// Highlight for creature permissions:
	int creaturePerm = theApp.GetWhichCreaturePermissionToHighlight();
	if (creaturePerm && TestCreaturePermissions(creaturePerm)) {
		Box r;
		GetAgentExtent(r);
		DrawLine(r.left, r.top, r.right, r.bottom, 30,255,255, 0,0);		//yellow:
		DrawLine(r.right, r.top, r.left, r.bottom, 30,255,255, 0,0);
		myResetLines = true;
	}
	// Draw bounding box if this agent is highlighted:
	if (myHighlightedFlag) {
		Box r;
		GetAgentExtent(r);
		DrawLine(r.left, r.top, r.right, r.top, myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		DrawLine(r.left, r.top, r.left, r.bottom, myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		DrawLine(r.left, r.bottom, r.right, r.bottom, myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		DrawLine(r.right, r.top, r.right, r.bottom, myHighlightColour.r,myHighlightColour.g,myHighlightColour.b, 0,0);
		myResetLines = true;
	}

	_ASSERT(!myGarbaged);

	try
	{

		// update timer and issue a TIMER script event if timed out
		if( myTimerRate>0 )
		{
			if( ++myTimer >= myTimerRate )
			{

				ExecuteScriptForEvent(SCRIPTTIMER, mySelf, INTEGERZERO,
					INTEGERZERO);
				_ASSERT(!myGarbaged);
				myTimer = 0;
			}
		}

		if (IsRunning())
		{
			_ASSERT(!myGarbaged);
			myVirtualMachine.UpdateVM( 5 );	//  instructions per tick
		}
	}
	catch( CAOSMachine::InvalidAgentHandle& te)
	{
		// Right then, are we running an exception script already?!?!?!
		Classifier d;
		myVirtualMachine.GetScript()->GetClassifier(d);
		if (d.myEvent == SCRIPTAGENTEXCEPTION)
		{
			// Okay then, we were running an exception script already, so do it via HRE
			HandleRunError(te);
		}
		else
		{
			// We were not running an exception script, do we have one?
			Classifier c(myClassifier);
			c.SetEvent(SCRIPTAGENTEXCEPTION);
			CAOSVar p1,p2;
			p1.SetInteger(d.myEvent);
			p2.SetString(te.what());
		
			if (ExecuteScriptForClassifier(c,mySelf,p1,p2) != EXECUTE_SCRIPT_STARTED)				
				// We could not start an exception script
				HandleRunError(te);
			
		}
	}
	catch( CAOSMachine::RunError& e )
	{
		HandleRunError(e);
	}
	catch(BasicException& e)
	{
		myRunning = false;
		ErrorMessageHandler::Show(e, "Agent::Update");
	}
}



void Agent::MoveTo(float x, float y)
{
	_ASSERT(!myGarbaged);
	myPositionVector.x = x;
	myPositionVector.y = y;
}

void Agent::MoveBy(float xd, float yd)
{
	_ASSERT(!myGarbaged);

	if (GetMovementStatus() == Agent::FLOATING)
	{
		Vector2D floatPos = myFloatingPositionVector;
		FloatRelativeToWorld(floatPos);
		FloatTo(Vector2D(floatPos.x+xd, floatPos.y+yd));
	}
	else
		MoveTo(myPositionVector.x+xd, myPositionVector.y+yd);
}


// virtual
bool Agent::TestCurrentLocation(bool forcePositionCheck)
{
	_ASSERT(!myGarbaged);
	return TestMoveTo(myPositionVector.x, myPositionVector.y, forcePositionCheck);
}


//virtual
bool Agent::TestRoomSystemLocation()
{
	if (!TestAttributes(attrSufferCollisions))
		return true;
	return theApp.GetWorld().GetMap().IsAgentLocationValidInRoomSystem
		(myPositionVector, myCurrentWidth, myCurrentHeight,
			myPermiability);
}


// virtual
bool Agent::TestMoveTo(float x, float y, bool forcePositionCheck)
{
	_ASSERT(!myGarbaged);

	if (!forcePositionCheck && !TestAttributes(attrSufferCollisions))
		return true;
	if ((myMovementStatus == FLOATING) ||
		(myMovementStatus == CARRIED))
		// Can move everywhere
		return true;
	else if (myMovementStatus == INVEHICLE)
	{
		Box cabin;
		myCarrierAgent.GetVehicleReference().GetCabinExtent(cabin);
		Vector2D position(x, y);
		return theApp.GetWorld().GetMap().IsAgentLocationValidInRectangle
			(position, myCurrentWidth, myCurrentHeight, 
			cabin.left, cabin.right, cabin.top, cabin.bottom);
	}
	else
	{
		Vector2D position(x, y);
		return theApp.GetWorld().GetMap().IsAgentLocationValidInRoomSystem
			(position, myCurrentWidth, myCurrentHeight, myPermiability);
	}
}


// virtual
bool Agent::TestMoveBy(float x, float y, bool forcePositionCheck)
{
	_ASSERT(!myGarbaged);
	return TestMoveTo(myPositionVector.x+x,myPositionVector.y+y,forcePositionCheck);
}





//
//
// Movement Routines
//
//

void Agent::HandleMovement()
{
	_ASSERT(!myGarbaged);

	switch( myMovementStatus )
	{
	case AUTONOMOUS:
		HandleMovementWhenAutonomous();
		break;
	case FLOATING:
		HandleMovementWhenFloating();	
		break;
	case INVEHICLE:
		HandleMovementWhenInVehicle();
		break;
	case CARRIED:
		HandleMovementWhenCarried();
		break;
	}
}


void Agent::HandleMovementWhenAutonomous()
{
	if (myStoppedFlag)
		return;

	bool collision;
	bool stopped;
	int wall;
	Vector2D velocityCollision;
	int permiability;

	// Calculate permiability
	if (TestAttributes(attrSufferCollisions))
		permiability = myPermiability;
	else
		permiability = 0;

	Vector2D myNewPositionVector(myPositionVector);

	bool applyPhysics = 
		(myAttributes & attrSufferPhysics) == attrSufferPhysics;


	theApp.GetWorld().GetMap().MoveAgentInsideRoomSystem
		(myCurrentWidth, myCurrentHeight, applyPhysics,
		permiability,
		myCollisionFactor, myAeroDynamicFactor, 
		myFrictionFactor,
		myGravitationalAcceleration,
		myNewPositionVector, myVelocityVector, collision, wall,
		stopped, velocityCollision);

	MoveTo(myNewPositionVector.x, myNewPositionVector.y);

	if (stopped)
		myStoppedFlag = true;

	if (collision) {
		CAOSVar vcx, vcy;
		myLastWallHit = wall;
		vcx.SetFloat(velocityCollision.x);
		vcy.SetFloat(velocityCollision.y);
	
		ExecuteScriptForEvent(SCRIPTCOLLISION, mySelf, vcx, vcy); 
	}
}


void Agent::HandleMovementWhenFloating()
{
	bool collision;
	bool stopped;
	int wall;
	int permiability = 0;
	Vector2D velocityCollision;

	theApp.GetWorld().GetMap().MoveAgentInsideRoomSystem
		(myCurrentWidth, myCurrentHeight, false,
		permiability, 
		myCollisionFactor, myAeroDynamicFactor, 
		myFrictionFactor,
		myGravitationalAcceleration,
		myFloatingPositionVector, myVelocityVector,
		collision, wall,
		stopped, velocityCollision);
	
	// We move here, as well as in the camera.  This is to be
	// sure our position is reasonably up to date if another 
	// script looks at it in the meanwhile
	CameraPositionNotify();
}


void Agent::HandleMovementWhenInVehicle()
{
	_ASSERT(!myGarbaged);

	if (myStoppedFlag)
		return;

	float w,h;
	Vector2D p, vel;
	bool collision;
	int wall;
	bool stopped;
	Vector2D velocityCollision;
	float x,y;
	CAOSVar vcx, vcy;
	Box r;

	p = myPositionVector;
	vel = myVelocityVector;
	w = GetWidth();
	h = GetHeight();

	// vehicle cast
	ASSERT(GetCarrier().IsVehicle());
	
	Vehicle& v = GetCarrier().GetVehicleReference();

	v.GetCabinExtent(r);

	bool applyPhysics = (myAttributes & attrSufferPhysics) == attrSufferPhysics;
	theApp.GetWorld().GetMap().MoveAgentInsideRectangle
		(w, h, applyPhysics,
		 myCollisionFactor, myAeroDynamicFactor, 
			myFrictionFactor,
			myGravitationalAcceleration,
		 r.left,
		 r.right,
		 r.top,
		 r.bottom,
		 p, vel, 
		 collision, 
		 wall, 
		 stopped,
		 velocityCollision);

	myVelocityVector = vel;
	x = p.x;
	y = p.y;
	MoveTo(x, y);

	if (stopped)
		myStoppedFlag = true;

	if (collision) {
		myLastWallHit = wall;
		vcx.SetFloat(velocityCollision.x);
		vcy.SetFloat(velocityCollision.y);
		
		ExecuteScriptForEvent(SCRIPTCOLLISION, mySelf, vcx, vcy); 
	}
}


void Agent::HandleMovementWhenCarried()
{
	_ASSERT(!myGarbaged);
	_ASSERT( myCarrierAgent.IsValid() );
	_ASSERT( myCarrierAgent.GetAgentReference().myCarriedAgent == mySelf );


	Vector2D carrier = myCarrierAgent.GetAgentReference().GetMapCoordOfWhereIPickUpAgentsFrom();
	Vector2D offset = GetMyOffsetOfWhereIAmPickedUpFrom();
	Vector2D p = carrier - offset;

	MoveTo(p.x, p.y);
}








// scan through the pending input events and run scripts for any which
// the agent is listening for...
void Agent::HandleInputEvents()
{
	_ASSERT(!myGarbaged);

	InputManager& im = theApp.GetInputManager();
	int i;
	int scriptnum;
	const InputEvent* ev;
	int param1,param2;
	CAOSVar p1, p2;

	for( i=0; i<im.GetEventCount(); ++i )
	{
		ev = &im.GetEvent(i);

		if( ev->EventCode & myInputMask)
		{
			param1 = 0;
			param2 = 0;
			switch( ev->EventCode )
			{
			case InputEvent::eventKeyUp:
				scriptnum = SCRIPTRAWKEYUP;
				param1 = ev->KeyData.keycode;
				break;
			case InputEvent::eventKeyDown:
				scriptnum = SCRIPTRAWKEYDOWN;
				param1 = ev->KeyData.keycode;
				break;
			case InputEvent::eventTranslatedChar:
				scriptnum = SCRIPTRAWTRANSLATEDCHAR;
				param1 = ev->KeyData.keycode;
				break;
			case InputEvent::eventMouseMove:
				scriptnum = SCRIPTRAWMOUSEMOVE;
				param1 = ev->MouseMoveData.mx;
				param2 = ev->MouseMoveData.my;
				// translate into world coords
				theMainView.ScreenToWorld( param1, param2);
				break;
			case InputEvent::eventMouseDown:
				scriptnum = SCRIPTRAWMOUSEDOWN;
				param1 = ev->MouseButtonData.button;
				break;
			case InputEvent::eventMouseUp:
				scriptnum = SCRIPTRAWMOUSEUP;
				param1 = ev->MouseButtonData.button;
				break;
			case InputEvent::eventMouseWheel:
				scriptnum = SCRIPTRAWMOUSEWHEEL;
				param1 = ev->MouseWheelData.delta;
				break;
			default:
				ASSERT( false );	// unknown event type!
				break;
			}

			// no "from" agent for inputevents
			p1.SetInteger(param1);
			p2.SetInteger(param2);
			ExecuteScriptForEvent(scriptnum, NULLHANDLE, p1, p2);
			_ASSERT(!myGarbaged);
		}
	}
}





// helper for Update()
// This updates any active controlled sound
// (Altering their panning/volume as position changes
// and stopping/restarting them as they move in and
// out of range

void Agent::HandleSound()
{
	_ASSERT(!myGarbaged);

	if (!theSoundManager)
		return;
	
	// Is a sound active
	if (mySoundName)
	{

		// Is it currently playing
		if (mySoundHandle>=0)
		{
			
			// Sound is playing - adjust levels or finish it
			if (theSoundManager->FinishedControlledSound(mySoundHandle))
			{
				theSoundManager->StopControlledSound(mySoundHandle);
				mySoundHandle=-1;
				mySoundName=0;
			}
			else
			{
				if (Visibility(0))
				{
					long volume, pan;
					SoundLevels(volume,pan);
					theSoundManager->UpdateControlledSound(mySoundHandle,volume,pan);
				}
				else
				{
					// agent off screen - suspend/destroy it
					theSoundManager->StopControlledSound(mySoundHandle);
					mySoundHandle=-1;
					if (!mySoundLooping)
					{
						// Don't restart a one off sound
						mySoundName=0;
					}
				}
			}
		}
		else
		{
			// Sound is active, but not currently playing
			if (Visibility(0))
			{
				// restart it if it is in range
				SOUNDHANDLE h;
				SOUNDERROR err;
				long volume, pan;
				SoundLevels(volume,pan);
				err=theSoundManager->StartControlledSound(mySoundName,h,volume,pan,mySoundLooping);
				if (err || h==-1)
				{
					// "SOUND ERROR: %d PLAYING %s %d\n",err,tokn_str,mySoundHandle
				}
				else
				{
					mySoundHandle=h;
					// "Sound %s %d re-entered range\n",tokn_str,mySoundHandle
				}
			}
		}
	}
}



// retn message# (ACTIVATE1 etc) appropriate for a click at a given
// position, else return -1
// Since this is the default version of this virtual function, it should
// just return ACTIVATE1, which is the default for simple agents
// (compound agents will return different msgs according to which button
// was under xy)

int Agent::ClickAction(int x,int y)
{
	_ASSERT(!myGarbaged);

	return -1;
}


int Agent::ExecuteScriptForClassifier
	(const Classifier& c, const AgentHandle& from,
	 const CAOSVar& p1, const CAOSVar& p2)
{
	_ASSERT(!myGarbaged);

	MacroScript* m;
	try
	{
		if( myVirtualMachine.IsRunning() && (c.Event() != SCRIPTAGENTEXCEPTION))
		{
			// script-interruption rules. UGH.

			// - timer can't interrupt anything
			// - can't interrupt a LOCKed script
			if( c.Event() == SCRIPTTIMER ||
				myVirtualMachine.IsLocked() )
			{
				return EXECUTE_SCRIPT_NOT_INTERRUPTIBLE;
			}
		}

		m = theApp.GetWorld().GetScriptorium().FindScript(c);
		if( m == NULL )
			return EXECUTE_SCRIPT_NOT_FOUND;

		myVirtualMachine.StartScriptExecuting( m, mySelf ,from, p1, p2 );

		// Execute at least one instruction immediately.
		// This ensures that scripts starting with INST get executed
		// atomically, otherwise a little bit of the script gets done
		// immediately.
		myVirtualMachine.UpdateVM( 1 );
		return EXECUTE_SCRIPT_STARTED;
	}
	catch( CAOSMachine::InvalidAgentHandle& te)
	{
		// Right then, are we running an exception script already?!?!?!
		Classifier d;
		myVirtualMachine.GetScript()->GetClassifier(d);
		if (d.myEvent == SCRIPTAGENTEXCEPTION)
		{
			// Okay then, we were running an exception script already, so do it via HRE
			HandleRunError(te);
		}
		else
		{
			// We were not running an exception script, do we have one?
			Classifier c(myClassifier);
			c.SetEvent(SCRIPTAGENTEXCEPTION);
			CAOSVar p1,p2;
			p1.SetInteger(d.myEvent);
			p2.SetString(te.what());
			
			if (ExecuteScriptForClassifier(c,mySelf,p1,p2) != EXECUTE_SCRIPT_STARTED)				
				// We could not start an exception script
				HandleRunError(te);
			
		}
	}
	catch( CAOSMachine::RunError& e )
	{
		HandleRunError(e);
	}
	return EXECUTE_SCRIPT_STARTED;
}


int Agent::ExecuteScriptForEvent
	(int event, const AgentHandle& from,
	 const CAOSVar& p1, const CAOSVar& p2)
{
	_ASSERT(!myGarbaged);
	Classifier c(myClassifier);
	c.SetEvent(event);
	return ExecuteScriptForClassifier(c, from, p1, p2);
}










///////////////////// macro helper functions ///////////////////

// Default version of virtual helper function for POSE macro.
// Overloaded forms set a new pose & return TRUE if change is complete. I return TRUE always.
bool Agent::ShowPose(int pose,int part)
{
	_ASSERT(!myGarbaged);
	return true;
}

// Default version of virtual helper function for POSE
// Overloaded forms get current pose for given part#
int Agent::GetPose(int part)
{
	_ASSERT(!myGarbaged);
	return 0;
}




// default helper fn for BASE macro - set base sprite for obj or given part
bool Agent::SetBaseImage(int image,int part)
{
	_ASSERT(!myGarbaged);
	return true;
}













void Agent::Trash()
{
	_ASSERT(!myGarbaged);

	// Dump my previous CA if I need to.
	if (myCAProcessingState != stateNotProcessing)
	{
		if (myPrevCAIsNavigable)
		{
			theApp.GetWorld().GetMap().AlterCAEmission(myPrevRoomWhenEmitting,myPrevCAIndex,-myPrevCAIncrease);
		}
	}

	// Clear any messages that will be for us
	theApp.GetWorld().RemoveMessagesAbout(mySelf);

	BreakLinksToOtherAgents();

	// Delete genome files
	GetGenomeStore().ClearAllSlots();

	// And we're not registered as floating any more
	if ( myMovementStatus == FLOATING )
		theMainView.RemoveFloatingThing(mySelf);

	// Clear all global variables thus relinquishing any pointers
	for (int i=0; i<GLOBAL_VARIABLE_COUNT; ++i) 
		myGlobalVariables[i].SetInteger(0);

	myRunning = false;

	// stop any running script
	myVirtualMachine.StopScriptExecuting();

	myPorts.Trash();

	if (myEntityImage)
		delete myEntityImage;
	myEntityImage = NULL;

	// Clears the type to prevent RTTI from the AgentHandle's point 
	// of view.
	myAgentType = 0; 

	if (mySoundHandle>=0)
	{
		StopControlledSound(mySoundHandle);
	}

	// These must be the last two lines in the function
	mySelf = NULLHANDLE;
	myGarbaged = true;
}




// Play this sound effect if this agent is audible to the user
// The sound starts playing at the appropriate volume/panning
// but the agent has no further control over the sound
// (i.e. when it stops/altering its panning)
// the sound is played after 'delay' ticks
void Agent::SoundEffect(DWORD fsp, int delay)
{
	_ASSERT(!myGarbaged);

    // Just in case sound effect called before sound manager initialised!
    if (!theSoundManager)
        return;

	long volume,pan;

	if (Visibility(0))
	{
		SoundLevels(volume,pan);
		SOUNDERROR err;
		err = theSoundManager->PlaySoundEffect(fsp,delay,volume,pan);
	}

}

// Play this sound  if this agent is audible to the user
// The sound starts playing at the appropriate volume/panning
// and is continually updated by 'Agent::Update()'
// This alters its panning and volume as the agent moves
// and also checks if it goes in or out of range.
// Looped sounds are restarted when they come back in range
// while non-looped sounds cease to exist when out of range
void Agent::ControlledSound(DWORD fsp, BOOL loop)
{
	_ASSERT(!myGarbaged);

	if (!theSoundManager)
		return;

	long volume,pan;
				// determine volume according to location
	if (Visibility(0))
	{
		SoundLevels(volume,pan);
		SOUNDERROR err;
	
		SOUNDHANDLE h;
		err=theSoundManager->StartControlledSound(fsp,h,volume,pan,loop);
		if (err)
		{
			// SOUND ERROR: %d PLAYING %s\n",err,tokn_str
		}
		else
		{
			if (mySoundHandle>=0)
			{
				// Sound %s replacing %s\n",tokn_str,old_str

				// Fade out any sound currently playing
				theSoundManager->StopControlledSound(mySoundHandle,TRUE);
			}

			mySoundHandle=h;
			mySoundName=fsp;
			mySoundLooping=loop;
		}

	}
	else
	{
		if (loop)
		{
			if (mySoundHandle>=0)
			{
				// Sound %s (off-screen) replacing %s\n",tokn_str,old_str
				// Fade out any sound currently playing
				theSoundManager->StopControlledSound(mySoundHandle,TRUE);
			}

			mySoundHandle=-1;		// off range, so not playing
			mySoundName=fsp;
			mySoundLooping=loop;
			
		}
	}

}

// If a controlled sound is currently active,
// it is stopped (either abruptly or faded)
// Hence Stop...Controlled...Sound...
void Agent::StopControlledSound(BOOL fade)
{
	_ASSERT(!myGarbaged);


	if (mySoundHandle>=0)
	{
        if (theSoundManager)
		    theSoundManager->StopControlledSound(mySoundHandle,fade);
	}

	mySoundHandle=-1;
	mySoundName=0;
	mySoundLooping=FALSE;

}


void Agent::PauseControlledSound()				// Stop active sound when game is paused
{
	_ASSERT(!myGarbaged);

	if (mySoundHandle>=0)
	{
		if (theSoundManager)
			theSoundManager->StopControlledSound(mySoundHandle);
		mySoundHandle=-1;

		// If the sound is looping, we want it to start up again when unpaused
		if (!mySoundLooping)
		{
			mySoundName=0;
		}
	}

}



/*********************************************************************
* Public: CanHear.
* Return true if this agent can hear another
*********************************************************************/
bool Agent::CanHear(const AgentHandle& other)
{
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	_ASSERT( other2.IsValid() );

	// Can't hear self
	if (mySelf == other)
		return false;

	Vector2D v1(GetCentre());
	Vector2D v2(other.GetAgentReference().GetCentre());

	int v1id,v2id;
	if (theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(v1,v1id) && 
		theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(v2,v2id))
	{
		if (v1id == v2id)
		{
			float audioRange = GetAudibleRange();
			return v1.SquareDistanceTo(v2) <= (audioRange * audioRange);
		}
	}
	return false;
}


/*********************************************************************
* Public: CanSee.
* Return true if this agent can see another
*********************************************************************/
bool Agent::CanSee(const AgentHandle& other)
{
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	ASSERT( other2.IsValid() );

	// Can't see self
	if (mySelf == other)
		return false;

	Vector2D v1 = GetCentre();
	Vector2D v2 = other.GetAgentReference().GetCentre();

	// Check if in visual range
	float visualRange = GetVisualRange();
	if (v1.SquareDistanceTo(v2) > (visualRange * visualRange))
		return false;

	// Check if walls in the way
	int perm = GetPermiability();
	bool canSee;
	bool result = theApp.GetWorld().GetMap().CanPointSeePoint(v1, v2, perm, canSee);
	if (!result) {
		return false;
	}
	return canSee;
}





/*********************************************************************
* Public: CanTouch.
* Return true if this agent is touching other
*********************************************************************/
bool Agent::CanTouch(const AgentHandle& other)
{
	AgentHandle other2(other);
	_ASSERT(!myGarbaged);
	ASSERT( other2.IsValid() );

	if (mySelf == other)
		return false;

	Box r1;
	GetAgentExtent(r1);

	Box r2;
	other.GetAgentReference().GetAgentExtent(r2);

	if( r1.left > r2.right || r2.left > r1.right )
		return false;
	if( r1.top > r2.bottom || r2.top > r1.bottom )
		return false;

	return true;
}








// virtual
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool Agent::Write(CreaturesArchive &ar) const
{
	_ASSERT(!myGarbaged);

	int i;

	ar << (int)0;		// reserved for version hacking
	ar << myID;			// Unique ID.
	ar << myUpdateTick;
	myClassifier.Write(ar);
	ar << myAttributes;
	ar << myCreaturePermissions;
	ar << myMovementStatus;
	ar << myGarbaged;
	ar << myRunning;
	ar << myInputMask;
	ar << myTimer;
	ar << myTimerRate;
	ar << myDefaultClickAction;		// used by ClickAction()

	// display stuff
	ar << myEntityImage;


	ar << myCarriedAgent;
	ar << myCarrierAgent;

	ar << myGeneralRange;
	ar << myInvalidPosition;
	ar << myHighlightedFlag;
	ar << myHighlightColour.r << myHighlightColour.g << myHighlightColour.b;
	
	ar << myPointsWhereIamPickedUp;
	ar << myPointsIUseToPickUpAgents;


	// physics stuff
	ar << myPositionVector;
	ar << myFloatingPositionVector;
	ar << myVelocityVector;
	ar << myElasticity;
	ar << myFrictionPercentage;
	ar << myFrictionFactor;
	ar << myCollisionFactor;
	ar << myAeroDynamicFactor;	// % velocity lost due to media resistance.
	ar << myAeroDynamicPercentage;
	ar << myPermiability;		// Below which the agent cannot pass through.
	ar << myGravitationalAcceleration;	// Rate at which agent accelerates in pixels per tick.
	ar << myStoppedFlag;
	ar << myLastWallHit;
	ar << myFloatingWith;
	ar << myFloatees;

	ar << myCAIndex;		//Index of the CA property which this agent will effect
	ar << myCAIncrease;		//Amount by which the CA property will be increased
	ar << myCAIsNavigable;
	ar << myPrevCAIndex;
	ar << myPrevCAIncrease;
	ar << myPrevCAIsNavigable;
	ar << myPrevRoomWhenEmitting;
	// ar << myCAProcessingState;
	int tempState;
	if (myCAProcessingState == stateNotProcessing)
		tempState = 0;
	else if (myCAProcessingState == stateStableSettings)
		tempState = 1;
	else
		tempState = 2;
	ar << tempState;

	ar << myPixelPerfectHitTestFlag;
	ar << myClickActionState;
	for( i = 0; i < 3; ++i )
		ar << myDefaultClickActionCycles[i];

	if( !myVirtualMachine.Write(ar) )
		return false;
	if( !myPorts.Write(ar) )
		return false;


	// agent variables... OV00-OV99
	// ugly code, but don't want overhead of 100 persistentobjects...
	ar << (int)GLOBAL_VARIABLE_COUNT;			// to allow future expansion
	for( i=0; i<GLOBAL_VARIABLE_COUNT; ++i )
		myGlobalVariables[i].Write( ar );

	ar << myGenomeStore;

	ar << myAgentType;
	myVoice.Write(ar);
	ar << myImpendingDoom;
			
	ar << myCurrentWidth;
	ar << myCurrentHeight;
	ar << myResetLines;

	ar << myDrawMirroredFlag;


	// obsolete stuff, or stuff I'm not sure about (benc)
//	SOUNDHANDLE	mySoundHandle;	// Controlled Sound (-1=Not playing)
//	DWORD mySoundName;			// Name of active sound (0=none)
//	BOOL mySoundLooping;			// Controlled sound is looping

	ar << mySoundName;
	ar << mySoundLooping;

	// Serialise these if we want bounding box checks on
	ar << myWidthTemp << myHeightTemp;

	// bytecount - num of bytes to skip over (to allow extensions)
	ar << (int)0;		// leeway to allow extension hacking

	return true;
}



// virtual
bool Agent::Read( CreaturesArchive &ar )
{
	_ASSERT(!myGarbaged);

	int32 version = ar.GetFileVersion();
	int i;
	int varcount;
	int tmp_int;

	if(version >= 3)
	{
		
		ar >> tmp_int;
		if( tmp_int != 0 )
		{
			return false;
		}

		ar >> myID;			// Unique ID.
		ar >> myUpdateTick;
		
		if(!myClassifier.Read(ar))
			return false;

		ar >> myAttributes;
		ar >> myCreaturePermissions;
		ar >> myMovementStatus;
		ar >> myGarbaged;
		ar >> myRunning;
		ar >> myInputMask;
		ar >> myTimer;
		ar >> myTimerRate;
		ar >> myDefaultClickAction;		// used by ClickAction()

		// display stuff
		ar >> myEntityImage;

		ar >> myCarriedAgent;
		ar >> myCarrierAgent;

		ar >> myGeneralRange;
		ar >> myInvalidPosition;
		ar >> myHighlightedFlag;
		ar >> myHighlightColour.r >> myHighlightColour.g >> myHighlightColour.b;

		ar >> myPointsWhereIamPickedUp;
		ar >> myPointsIUseToPickUpAgents;

		// physics stuff
		ar >> myPositionVector;
		ar >> myFloatingPositionVector;
		ar >> myVelocityVector;
		ar >> myElasticity;
		ar >> myFrictionPercentage;
		ar >> myFrictionFactor;
		ar >> myCollisionFactor;
		ar >> myAeroDynamicFactor;	// % velocity lost due to media resistance.
		ar >> myAeroDynamicPercentage;
		ar >> myPermiability;		// Below which the agent cannot pass through.
		ar >> myGravitationalAcceleration;	// Rate at which agent accelerates in pixels per tick.
		ar >> myStoppedFlag;
		ar >> myLastWallHit;
		ar >> myFloatingWith;
		ar >> myFloatees;

		ar >> myCAIndex;
		ar >> myCAIncrease;
		ar >> myCAIsNavigable;
		ar >> myPrevCAIndex;
		ar >> myPrevCAIncrease;
		ar >> myPrevCAIsNavigable;
		ar >> myPrevRoomWhenEmitting;
		// ar >> myCAProcessingState;
		int tempState;
		ar >> tempState;
		if (tempState == 0)
			myCAProcessingState = stateNotProcessing;
		else if (tempState == 1)
			myCAProcessingState = stateStableSettings;
		else
			myCAProcessingState = stateSettingsChange;

		if( myCAIsNavigable )
		{
			myCAProcessingState = stateSettingsChange;
			myPrevCAIndex = -1;
			myPrevCAIncrease = 0.0f;
			myPrevCAIsNavigable = false;
			myPrevRoomWhenEmitting = -1;
		}

		ar >> myPixelPerfectHitTestFlag;
		ar >> myClickActionState;
		for( i = 0; i < 3; ++i )
			ar >> myDefaultClickActionCycles[i];

		if( !myVirtualMachine.Read(ar) )
			return false;
		if( !myPorts.Read(ar) )
			return false;

		ar >> varcount;
		ASSERT( varcount <= GLOBAL_VARIABLE_COUNT );

		for( i=0; i<varcount; ++i )
			myGlobalVariables[i].Read( ar );

		ar >> myGenomeStore;

		ar >> myAgentType;

		if(!myVoice.Read(ar))
			return false;

		ar >> myImpendingDoom;
		ar >> myCurrentWidth;
		ar >> myCurrentHeight;
		ar >> myResetLines;

		ar >> myDrawMirroredFlag;

		// obsolete stuff, or stuff I'm not sure about (benc)
	//	SOUNDHANDLE	mySoundHandle;	// Controlled Sound (-1=Not playing)
	//	DWORD mySoundName;			// Name of active sound (0=none)
	//	BOOL mySoundLooping;			// Controlled sound is looping

		ar >> mySoundName;
		ar >> mySoundLooping;

		ar >> myWidthTemp >> myHeightTemp;

		// bytecount - num of bytes to skip over (to allow extensions)
		ar >> tmp_int;
		//	ar.Skip( tmp_int );

	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success!

	if (myMovementStatus == FLOATING)
	{
		theMainView.AddFloatingThing(mySelf);
	//		CameraPositionNotify();
	}

	DrawMirrored(IsMirrored());
	return true;
}





bool Agent::CanStartCarrying(AgentHandle& a)
{
	// We can only carry one agent at a time
	if (myCarriedAgent.IsInvalid())
		return true;
	else
		return false;
}


bool Agent::CanStopCarrying(AgentHandle& a, AgentHandle& v, bool& findSafePlace)
{
	findSafePlace = false;
	v = NULLHANDLE;
	Agent& agentref = a.GetAgentReference();
	if (agentref.TestAttributes(attrFloatable))
		return true;
	v = theAgentManager.WhatVehicleCouldIDropInto(a, mySelf);
	bool validVehicle = v.IsValid();

	if ((myAgentType & AgentHandle::agentPointer) != AgentHandle::agentPointer)
	{
		// Not being dropped by the pointer
		if (validVehicle && (v.GetAgentReference().GetMovementStatus() != Agent::FLOATING))
			return true;
		else
			v = NULLHANDLE;
		if (agentref.TestRoomSystemLocation())
			return true;
		findSafePlace = true;
		return true;
	}
	else
	{
		// Being dropped by the pointer
		if (validVehicle)
		{
			if ((a.IsCreature()) && 
				(v.GetAgentReference().GetMovementStatus() == Agent::FLOATING))
			{
				// Can't drop creatures into the inventory
				v = NULLHANDLE;
				return false;
			}
			return true;
		}
		if (agentref.TestRoomSystemLocation())
			return true;
		return false;
	}
}


void Agent::StartCarrying(AgentHandle& a)
{
	_ASSERT(!myGarbaged);

	Agent& agentref = a.GetAgentReference();

	// Create parent-child relationships
	agentref.SetCarrier(mySelf);
	myCarriedAgent = a;

	// Agent is now being carried
	agentref.SetMovementStatus(CARRIED);

	// Clear the velocity. This isn't strictly necessary
	// because agents can't move on their own when in the 
	// carried state, but it seems correct to clear it.
	agentref.SetVelocity(ZERO_VECTOR);

	// Move the picked-up agent to the appropriate carry point
	Vector2D p = GetMapCoordOfWhereIPickUpAgentsFrom() -
		agentref.GetMyOffsetOfWhereIAmPickedUpFrom();
	agentref.MoveTo(p.x, p.y);
	
	// Place the carried agent into the appropriate carry plane
	int32 plane = GetPlaneForCarriedAgent();
	if (plane != -1)
		agentref.ChangePhysicalPlane(plane);

	// Make the picked-up agent have the same camera-shy status as the
	// carrier 
	agentref.ChangeCameraShyStatus((myAttributes & attrCameraShy) == attrCameraShy);
	
	// Inform the new carrier that it has gained a carried agent
	ExecuteScriptForEvent(SCRIPGOTCARRIEDAGENT, a, INTEGERZERO, INTEGERZERO);

	if (myAgentType & AgentHandle::agentPointer)
	{
		// I'm the pointer and I'm picking up an agent, so run the agent's
		// script on the pointer's VM
		Classifier c = agentref.GetClassifier();
		c.SetEvent(SCRIPTPOINTERPICKUP);
		ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);
	}
}


void Agent::StopCarrying(AgentHandle& a)
{
	_ASSERT(!myGarbaged);

	Agent& agentref = a.GetAgentReference();
	
	// Break the parent-child relationship
	agentref.SetCarrier(NULLHANDLE);
	myCarriedAgent = NULLHANDLE;

	// Restore the agent to its default movement status
	if (agentref.TestAttributes(attrFloatable))
		agentref.SetMovementStatus(FLOATING);
	else
		agentref.SetMovementStatus(AUTONOMOUS);

	// Restore the agent to its default plane
	agentref.ChangePhysicalPlane(agentref.GetNormalPlane(0));

	// Restore the agent to its default camera-shy status
	agentref.ChangeCameraShyStatus(agentref.TestAttributes(attrCameraShy) == attrCameraShy);

	// Inform the old carrier that it has lost its carried agent
	ExecuteScriptForEvent(SCRIPTLOSTCARRIEDAGENT, a, INTEGERZERO, INTEGERZERO);

	if (myAgentType & AgentHandle::agentPointer)
	{
		// I'm the pointer and I'm dropping an agent, so run the agent's
		// script on the pointer's VM
		Classifier c = agentref.GetClassifier();
		c.SetEvent(SCRIPTPOINTERDROP);
		ExecuteScriptForClassifier(c, a, INTEGERZERO, INTEGERZERO);


		if (((agentref.TestAttributes(attrSufferCollisions) & 
			Agent::attrSufferCollisions)
			== Agent::attrSufferCollisions) &&
			(agentref.GetMovementStatus() == AUTONOMOUS))
		{
			// Give mouse velocity to dropped agent, facilitates throwing 
			Vector2D v(theApp.GetInputManager().GetMouseVX(), 
			   theApp.GetInputManager().GetMouseVY());
			agentref.SetVelocity(v);
		}
		else 
		{
			// Stop moving
			agentref.SetVelocity(ZERO_VECTOR);
		}
	}
	else
		// Dropped agents stop moving by default
		agentref.SetVelocity(ZERO_VECTOR);
}



bool Agent::AddAPointWhereYouCanPickMeUp(int pose, int x, int y)
{
	_ASSERT(!myGarbaged);

	if (pose >= 0 && pose < myPointsWhereIamPickedUp.size())
	{
		myPointsWhereIamPickedUp[pose].x = x;	// Indexed by absolute pose
		myPointsWhereIamPickedUp[pose].y = y;	// Indexed by absolute pose
		return true;
	}
	else if (pose == -1)
	{
		for (int i = 0; i < myPointsWhereIamPickedUp.size(); ++i)
		{
			myPointsWhereIamPickedUp[i].x = x;	// Indexed by absolute pose
			myPointsWhereIamPickedUp[i].y = y;	// Indexed by absolute pose
		}
		return true;
	}
	else
		return false;
}

bool Agent::AddAPointWhereIPickAgentsUp(int pose, int x, int y, int part /*=0*/)
{
	_ASSERT(!myGarbaged);

	if (pose >= 0 && pose < myPointsIUseToPickUpAgents.size())
	{
		myPointsIUseToPickUpAgents[pose].x = x;	// Indexed by absolute pose
		myPointsIUseToPickUpAgents[pose].y = y;	// Indexed by absolute pose
		return true;
	}
	else if (pose == -1)
	{
		for (int i = 0; i < myPointsIUseToPickUpAgents.size(); ++i)
		{
			myPointsIUseToPickUpAgents[i].x = x;	// Indexed by absolute pose
			myPointsIUseToPickUpAgents[i].y = y;	// Indexed by absolute pose
		}
		return true;
	}
	else
		return false;
}

bool Agent::DoBoundsIntersect(const Box& rect)
{
	_ASSERT(!myGarbaged);

	Box intersectRect;
	Box bound;
	Box actrect(rect);

	GetAgentExtent(bound);

	return intersectRect.IntersectRect(actrect, bound) ? true : false;
}

bool Agent::HitTest(const Vector2D& point)
{
	_ASSERT(!myGarbaged);

	Box rect;
	GetAgentExtent(rect);           // otherwise get its boundary
	int px = Map::FastFloatToInteger(point.x);
	int py = Map::FastFloatToInteger(point.y);

				
	if (rect.PointInBox(point))
	{
		if(myPixelPerfectHitTestFlag)
		{
			if(myEntityImage)
			{
				return !(myEntityImage->IsPixelTransparent(px,py));
			}
		}
		else
		{
			return true;
		}
	}
	return false;
}


void Agent::SetGallery(FilePath const& galleryName, int baseimage, int part)
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
	{
		if(myEntityImage->GetGallery()->GetName() != galleryName)
		{
			myEntityImage->Unlink();
			myEntityImage->SetGallery(galleryName,baseimage);
			myEntityImage->Link();
			DoSetCameraShyStatus();
		}
	}

}


// virtual
void Agent::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stippleStart) 
{
	_ASSERT(!myGarbaged);
}


// virtual
void Agent::ResetLines()
{
	_ASSERT(!myGarbaged);
}

bool Agent::SetAttributes(uint32 attributes)
{ 
	_ASSERT(!myGarbaged);

	myAttributes = attributes;

	if (TestAttributes(attrSufferCollisions) && !myInvalidPosition) 
	{
		bool ok = TestCurrentLocation(true);
		if (!ok) {
			return false;
		}
	}
	if (TestAttributes(attrFloatable)) {
		if (GetMovementStatus() == AUTONOMOUS)
			SetMovementStatus(FLOATING);
	}
	else {
		if (GetMovementStatus() == FLOATING)
			SetMovementStatus(AUTONOMOUS);
	}

	if (TestAttributes(attrSufferPhysics))
		myStoppedFlag = false;

	DoSetCameraShyStatus();
	return true;
}

void Agent::DoSetCameraShyStatus()
{
	ChangeCameraShyStatus((myAttributes & attrCameraShy) == attrCameraShy);
}

void Agent::ChangeCameraShyStatus(bool shy)
{
	if (myEntityImage)
		myEntityImage->YouAreCameraShy(shy);
}

void Agent::SetMovementStatus(int type)
{ 
	_ASSERT(!myGarbaged);

	if (myMovementStatus != type)
	{
		if (myMovementStatus == FLOATING)
			theMainView.RemoveFloatingThing(mySelf);

		myMovementStatus = type;

		if (myMovementStatus == FLOATING)
		{
			myFloatingPositionVector = myPositionVector;
			WorldToFloatRelative(myFloatingPositionVector);
			theMainView.AddFloatingThing(mySelf);
		}
	}
}

void Agent::CameraPositionNotify()
{
	_ASSERT(!myGarbaged);

	if (myMovementStatus == FLOATING)
	{
		// We move here, as well as in the camera.  This is to be
		// sure our position is reasonably up to date if another 
		// script looks at it in the meanwhile
		Vector2D myNewPos = myFloatingPositionVector;
		FloatRelativeToWorld( myNewPos );
		MoveTo(myNewPos.x, myNewPos.y);
	}
	else
	{
		ASSERT(false);
	}
}

void Agent::FloatTo(const Vector2D& floatpos)
{
	_ASSERT(!myGarbaged);
	ASSERT(GetMovementStatus() == FLOATING);
	myFloatingPositionVector = floatpos;
	WorldToFloatRelative(myFloatingPositionVector);
	CameraPositionNotify();
}


void Agent::MarkValidPositionNow()
{
	_ASSERT(!myGarbaged);
	myInvalidPosition = false;
}


void Agent::HandleRunError(CAOSMachine::RunError& e)
{
	_ASSERT(!myGarbaged);

#ifdef C2E_OLD_CPP_LIB
	char hackbuf[1024];
	std::ostrstream out( hackbuf, sizeof( hackbuf) );
#else
	std::ostringstream out;
#endif

	// agent classifier
	out << theCatalogue.Get("runtime_error", 4);
	myClassifier.StreamClassifier(out, false);
	myClassifier.StreamAgentNameIfAvailable(out);
	out << " ";

	// script classifier
	if (myVirtualMachine.GetScript())
	{
		out << theCatalogue.Get("runtime_error", 7) << " ";
		myVirtualMachine.StreamClassifier(out);
		out << " ";
	}

	// unique id
	out << theCatalogue.Get("runtime_error", 5) << " " << GetUniqueID() << " ";

	// message
	out << std::endl << e.what();
	
	// script position
	if (myVirtualMachine.GetScript())
	{
		out << std::endl;
		myVirtualMachine.StreamIPLocationInSource(out);
	}

	// time/date/user/version
	out << std::endl << std::endl << ErrorMessageHandler::ErrorMessageFooter();
	out << '\0';

	int ret;
	bool camera;

	theFlightRecorder.Log(1, theCatalogue.Get("runtime_error_log", 0));
#ifdef C2E_OLD_CPP_LIB
	theFlightRecorder.Log(1,"%s",out.str() );
#else
	theFlightRecorder.Log(1,"%s",out.str().c_str());
#endif

	if (theApp.AutoKillAgentsOnError())
	{
		ret = RuntimeErrorDialog::RED_KILL;
		camera = false;
	}
	else
	{
		RuntimeErrorDialog dlg;
		dlg.SetText(std::string(out.str()));
		dlg.SetCameraPossible(myVirtualMachine.GetScript() != NULL);
#ifdef _WIN32
		ret = dlg.DisplayDialog(theApp.GetMainHWND());
#else
		ret = dlg.DisplayDialog();
#endif
		camera = dlg.myCameraChecked;
	}

	if (camera && myVirtualMachine.GetScript())
	{
		DisplayHandlers::CameraHelper(myVirtualMachine, GetCentre().x, 
			GetCentre().y, 0, true, NULL);
	}

	if (ret == RuntimeErrorDialog::RED_FREEZE)
	{
		theFlightRecorder.Log(1,theCatalogue.Get("runtime_error_log", 1));
		myRunning = false;
	}
	else if (ret == RuntimeErrorDialog::RED_KILL)
	{
		if (theApp.AutoKillAgentsOnError())
			theFlightRecorder.Log(1,theCatalogue.Get("runtime_error_log", 5));
		else
			theFlightRecorder.Log(1,theCatalogue.Get("runtime_error_log", 2));

		if( mySelf != thePointer)
		{
			myImpendingDoom = true;
			myVirtualMachine.StopScriptExecuting();
			return;
		}
	}
	else if (ret == RuntimeErrorDialog::RED_PAUSEGAME)
	{
		theFlightRecorder.Log(1,theCatalogue.Get("runtime_error_log", 3));
		#ifndef _WIN32
		#warning "TODO: Pause the game here"
		#else
		SetMultimediaTimer(false);
		#endif
	}
	else if (ret == RuntimeErrorDialog::RED_STOPSCRIPT)
	{
		theFlightRecorder.Log(1,theCatalogue.Get("runtime_error_log", 6));
		myVirtualMachine.StopScriptExecuting();
	}
	else if (ret == RuntimeErrorDialog::RED_IGNORE)
	{
		theFlightRecorder.Log(1,theCatalogue.Get("runtime_error_log", 4));
	}
	else
		ASSERT(false);
}

//These two functions are mutually exclusive an agent either cliks
// or clacs
// clic allows the agent engineer to set up a short cycle of activations
// for the left mouse button

	// set the default clickaction
void Agent::SetDefaultClickAction( int msgid )
{
	_ASSERT(!myGarbaged);

	myDefaultClickAction = msgid; 
	
	// minus one here means don't do any cycling
	myClickActionState =-1;
}


void Agent::SetDefaultClickActionCycle(int i0, int i1, int i2)
{
	_ASSERT(!myGarbaged);

	myClickActionState = 0;
	myDefaultClickActionCycles[0] = i0;
	myDefaultClickActionCycles[1] = i1;
	myDefaultClickActionCycles[2] = i2;
}


void Agent::DecideClickActionByState()
{
	_ASSERT(!myGarbaged);

	// if this value is minus one then ignore this
	// command
	if(myClickActionState !=-1)
	{
		// please make sure that this has been initialized correctly
		// otherwise 
		// we'll get a never ending loop
		myDefaultClickAction = -1;
		// skip over any unused slots
		while (myDefaultClickAction == -1)
		{

		myDefaultClickAction = myDefaultClickActionCycles[myClickActionState];
			myClickActionState++;

			if(myClickActionState >= NUMBER_OF_STATES_I_CAN_HANDLE_FOR_CLICKACTIONS)
			{
				myClickActionState =0;
			}

		}
	}
	
}

// undo the last action this is useful if the drop command
// was sent but was disallowed
void Agent::GoBackToLastClickActionState()
{
	// if this value is minus one then ignore this
	// command
	if(myClickActionState !=-1)
	{
		if(myClickActionState > 0)
		{
			
			myClickActionState--;
		}
		else
		{
			myClickActionState = NUMBER_OF_STATES_I_CAN_HANDLE_FOR_CLICKACTIONS - 1;
		}
	}

}

void Agent::BreakLinksToOtherAgents()
{
	_ASSERT(!myGarbaged);

	// Disconnect us from anything else
	myPorts.KillAllConnections();

	// Make sure we're not being carried
	if ( myCarrierAgent.IsValid() )
	{
		TryToBeDropped(mySelf, INTEGERZERO, INTEGERZERO, true);
	}

	// Make sure we're not carrying anything
	if ( myCarriedAgent.IsValid() )
	{
		myCarriedAgent.GetAgentReference().TryToBeDropped
			(mySelf, INTEGERZERO, INTEGERZERO, true);
	}

	// Make sure we're not floating with anyone, and noone is floating with us
	DetachFloatingWith();
	while(!myFloatees.empty())
	{
		if (myFloatees[0].IsValid())
			myFloatees[0].GetAgentReference().DetachFloatingWith();
		else
			myFloatees.erase(myFloatees.begin());
	}
}

void Agent::DetachFloatingWith()
{
	if (myFloatingWith.IsValid())
	{
		Agent& with = myFloatingWith.GetAgentReference();
		with.myFloatees.erase( std::remove(with.myFloatees.begin(), with.myFloatees.end(), mySelf),
			with.myFloatees.end());
		myFloatingWith = NULLHANDLE;
	}
}

void Agent::AttachFloatingWith(AgentHandle& with)
{
	DetachFloatingWith();
	if (with.IsValid())
	{
		myFloatingWith = with;
		with.GetAgentReference().myFloatees.push_back(mySelf);
	}
}


void Agent::WorldToFloatRelative(Vector2D& vec)
{
	if (myFloatingWith.IsValid())
	{
		Agent& agent = myFloatingWith.GetAgentReference();
		vec.x = vec.x - agent.myPositionVector.x;
		vec.y = vec.y - agent.myPositionVector.y;
	}
	else
		theMainView.WorldToScreen(vec);
}

void Agent::FloatRelativeToWorld(Vector2D& vec)
{
	if (myFloatingWith.IsValid())
	{
		Agent& agent = myFloatingWith.GetAgentReference();
		vec += agent.myPositionVector;
	}
	else
		theMainView.ScreenToWorld(vec);
}

bool Agent::IsThereAScript(int event)
{
	Classifier c(myClassifier);
	c.SetEvent(event);
	return theApp.GetWorld().GetScriptorium().FindScript(c) ? true : false;
}

bool Agent::SetCreaturePermissions(uint32 i)
{
	_ASSERT(!myGarbaged);

	// check all the scripts are there
	
	if ((i & permCanActivate1) && !IsThereAScript(SCRIPTACTIVATE1))
		return false;

	if ((i & permCanActivate2) && !IsThereAScript(SCRIPTACTIVATE2))
		return false;

	if ((i & permCanDeactivate) && !IsThereAScript(SCRIPTDEACTIVATE	))
		return false;

// Don't check pickup - there are often scripts without it, when 
// the creature can pick the agent up
//	if ((i & permCanPickUp) && !IsThereAScript(SCRIPTPICKUP))
//		return false;

	if ((i & permCanHit) && !IsThereAScript(SCRIPTHIT))
		return false;

	if ((i & permCanEat) && !IsThereAScript(SCRIPTEAT))
		return false;

	myCreaturePermissions = i;
	return true;
}

// turn mirroring for the current sprite on or off
void Agent::DrawMirrored(bool mirror)
{
	if(myEntityImage)
	{
		myEntityImage->DrawMirrored(mirror);
		myDrawMirroredFlag = mirror;
	}

}
