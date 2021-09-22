// --------------------------------------------------------------------------
// Filename:	Agent.h
// Class:		Agent
// Purpose:		Agent is the base class of all agents
//			
	
#ifndef AGENT_H
#define AGENT_H

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503 )
#endif


#include "../Creature/voice.h"
#include "../../common/C2eTypes.h"
#include "../Caos/CAOSMachine.h"
#include "InputPort.h"
#include "OutputPort.h"
#include "Port.h"
#include "PortBundle.h"
#include <map>
#include "../Sound/Soundlib.h"
#include "../Maths.h"
#include "../Classifier.h"
#include "../PersistentObject.h"
#include "../Message.h"
#include "../Display/EntityImage.h"
#include "../Display/EntityImageClone.h"
#include "../Map/Map.h"
#include "../../common/BasicException.h"
#include "../Caos/VelocityVariable.h"
#include "../AgentManager.h"
#include "../Creature/GenomeStore.h"
#include "AgentConstants.h"
#include "../mfchack.h"

const int GLOBAL_VARIABLE_COUNT = 100;
const Vector2D INVALID_POSITION(-1.0f,-1.0f);
const int NUMBER_OF_STATES_I_CAN_HANDLE_FOR_CLICKACTIONS = 3;


#include "AgentHandle.h"

// I don't want to have to include Voice here, so I'm simply blatting it as a forward declaration (DAN)
// [but isn't it already included right up the top there (GAV) :-)]
class Voice;

class Agent : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( Agent )

public:

	// Constants for Execute...() calls
	enum
	{
		EXECUTE_SCRIPT_STARTED,
		EXECUTE_SCRIPT_NOT_FOUND,
		EXECUTE_SCRIPT_NOT_INTERRUPTIBLE
	};

	// Should a constructor fail, we need to know why, so here's a string to manage it...
	std::string myFailedConstructionException;

	// Nice command to speak things :):)
	virtual void SpeakSentence(std::string& thisSentence);

	// Nice code to manage suicidal Agents
	void YouAreNowDoomed() { _ASSERT(!myGarbaged); myImpendingDoom = true; }
	bool AreYouDoomed() { _ASSERT(!myGarbaged); return myImpendingDoom; }


	bool IsConnectionValid
		(Port* in, Port* out, bool& warning);

	// 
	// Nice bits of code to manage the reference counting
	//

	// This function can be called on a garbaged agent
	uint32 GetAgentType() 
	{
		return myAgentType;
	}

	// This function can be called on a garbaged agent
	int GetReferenceCount() 
	{
		return myReferenceCount;
	}	

	// This function can be called on a garbaged agent
	int DecrementReferenceCount() 
	{
		myReferenceCount--;
		return myReferenceCount;
	}

	// This function can be called on a garbaged agent
	bool IsGarbage()
	{
		return myGarbaged;
	}

	// This is to override PersistentObject's method (avoiding rtti)
	virtual bool IsAgent() const {return true;}

	// We don't allow construction of agent handles on garbaged agents
	void IncrementReferenceCount()
	{
		_ASSERT(!myGarbaged);
		myReferenceCount++;
	}
	virtual uint32 GetMagicNumber();


	// ---------------------------------------------------------------------
	// Method:      SetTimerRate
	// Arguments:   ticks - number of ticks between timer events
	// Returns:     None
	// Description:	Starts a timer running which causes timer events to be
	//				sent to the agent at regular intervals.
	//				A 'ticks' value of 0 turns timer events off.
	// ---------------------------------------------------------------------
	void SetTimerRate( int ticks )
	{
		_ASSERT(!myGarbaged);
	    myTimerRate = ticks; 
		myTimer=0;
	}

	int GetTimerRate()
	{
		_ASSERT(!myGarbaged);
		return myTimerRate; 
	}

	int GetTimerCount()
	{
		_ASSERT(!myGarbaged);
		return myTimer; 
	}

	bool IsStopped()
	{
		return myStoppedFlag;
	}

	// ---------------------------------------------------------------------
	// Method:      SetClassifier()
	// Arguments:   c - new classifier for agent
	// Returns:     None
	// Description:	Sets the family,genus,species of an agent.
	// ---------------------------------------------------------------------
	void SetClassifier( const Classifier& c ) 
	{
		_ASSERT(!myGarbaged);
		myClassifier = c;
		theAgentManager.UpdateSmellClassifierList(myCAIndex, &myClassifier);
	}

	// ---------------------------------------------------------------------
	// Method:      GetClassifier()
	// Arguments:   None
	// Returns:     Classifier of agent
	// Description:	Gets the family,genus,species of an agent.
	// ---------------------------------------------------------------------
	const Classifier& GetClassifier() const
	{
		_ASSERT(!myGarbaged); 
		return myClassifier;
	}

	// reading
	uint32 GetFamily();	
	uint32 GetGenus();						
	uint32 GetSpecies();					

	void DoSetCameraShyStatus();
	virtual void ChangeCameraShyStatus(bool shy);
	bool SetAttributes(uint32 attributes);
	uint32 GetAttributes()
	{
		_ASSERT(!myGarbaged); 
		return myAttributes;
	}

	bool SetCreaturePermissions(uint32 i) ;
	inline uint32 GetCreaturePermissions() 
	{
		_ASSERT(!myGarbaged);
		return myCreaturePermissions;
	}
	bool IsThereAScript(int event);

	inline void SetWhetherHighlighted(bool f, int r=255, int g=255, int b=255) 
	{
		_ASSERT(!myGarbaged);
		myHighlightedFlag = f;
		myHighlightColour.r = r;
		myHighlightColour.g = g;
		myHighlightColour.b = b;
	}
	inline bool IsHighlighted() 
	{
		_ASSERT(!myGarbaged);
		return myHighlightedFlag;
	}



	void SetGravitationalAcceleration(float acceleration)
	{
		_ASSERT(!myGarbaged); 
		myGravitationalAcceleration = acceleration;
		if (TestAttributes(attrSufferPhysics) && (acceleration != 0.0f))
			myStoppedFlag = false;
	}

	float GetGravitationalAcceleration()
	{
		_ASSERT(!myGarbaged);
		return myGravitationalAcceleration;
	}

	void SetAeroDynamicPercentage(int aeroDynamicPercentage)
	{
		_ASSERT(!myGarbaged);
		myAeroDynamicPercentage = aeroDynamicPercentage;
		myAeroDynamicFactor = 1.0f - ((float)aeroDynamicPercentage/100.0f);
	}

	int GetAeroDynamicPercentage()
	{
		_ASSERT(!myGarbaged); 
		return myAeroDynamicPercentage;
	}


	void SetFrictionPercentage(int frictionPercentage)
	{
		_ASSERT(!myGarbaged);
		myFrictionPercentage = frictionPercentage;
		myFrictionFactor = 1.0f - ((float)frictionPercentage/100.0f);
	}

	int GetFrictionPercentage()
	{
		_ASSERT(!myGarbaged); 
		return myFrictionPercentage;
	}

	void SetElasticity(int elasticity)
	{ 
		_ASSERT(!myGarbaged);
		myElasticity = elasticity;
		myCollisionFactor = (float)elasticity/100.0f;
	}

	int GetElasticity()
	{
		_ASSERT(!myGarbaged);
		return myElasticity;
	}

	void SetInputMask(int inputMask)
	{
		_ASSERT(!myGarbaged); 
		myInputMask = inputMask;
	}

	int GetInputMask()
	{
		_ASSERT(!myGarbaged);
		return myInputMask;
	}

	bool SetPermiability(int permiability)
	{
		_ASSERT(!myGarbaged);
		if (permiability < 1) 
			myPermiability = 1;
		else if (permiability > 100)
			myPermiability = 100;
		else
			myPermiability = permiability; 

		if (TestAttributes(attrSufferCollisions) && !myInvalidPosition) 
		{
			bool ok = TestCurrentLocation(true);
			if (!ok) {
				return false;
			}
		}
		return true;
	}

	int GetPermiability()
	{
		_ASSERT(!myGarbaged); 
		return myPermiability;
	}

	void SetVelocity(const Vector2D& velocity) 
	{
		_ASSERT(!myGarbaged);
		myVelocityVector = velocity;
		myStoppedFlag = false;
	}

	Vector2D GetVelocity() 
	{
		_ASSERT(!myGarbaged);
		return myVelocityVector;
	}



	int GetLastWallHit();

	bool SetEmission(int caIndex, float value) ;

	int GetCAIndex() const 
	{
		_ASSERT(!myGarbaged);
		return myCAIndex;
	}

	float GetCAIncrease() const 
	{
		_ASSERT(!myGarbaged); 
		return myCAIncrease;
	}

	// OV00..OV99
	CAOSVar& GetReferenceToVariable( int varnum ) 
	{
		_ASSERT(!myGarbaged); 
	   return myGlobalVariables[ varnum ];
	}

	CAOSVar& GetReferenceToVelocityX() 
	{
		_ASSERT(!myGarbaged);
		return myVelocityXVariable;
	}

	CAOSVar& GetReferenceToVelocityY() 
	{
		_ASSERT(!myGarbaged);
		return myVelocityYVariable;
	}

	uint32 GetUniqueID(void)
	{
		_ASSERT(!myGarbaged);
		return myID;
	}

	void SetUniqueID(uint32 id)
	{
		_ASSERT(!myGarbaged);
		myID = id;
	}
	
	Agent();
	virtual ~Agent();

	virtual EntityImage* GetEntityImage()
	{
		_ASSERT(!myGarbaged); 
		return myEntityImage;
	}

	void MakeFastImage()
	{
		_ASSERT(!myGarbaged);
		if(myEntityImage) 
			myEntityImage->MakeFastImage();
	}

	virtual void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen = 0,
					uint8 lineColourBlue = 0,
					uint8 stippleon =0,
					uint8 stippleoff = 0,
					uint32 stippleStart = 0);

	virtual void ResetLines();




	AgentHandle GetCarrier()
	{
		_ASSERT(!myGarbaged);
		return myCarrierAgent;
	}

	void SetCarrier(AgentHandle& a)
	{
		_ASSERT(!myGarbaged); 
		myCarrierAgent = a;
	}

	virtual AgentHandle GetCarried() 
	{
		_ASSERT(!myGarbaged);
		return myCarriedAgent;
	}

	virtual void Trash();			
	virtual void BreakLinksToOtherAgents();

	void MarkValidPositionNow();
	virtual void MoveBy(float xd,float yd);		// adjust agent's position
	virtual void MoveTo(float x, float y);		// set agent's position
	virtual bool TestMoveTo(float x, float y, bool forcePositionCheck);
	virtual bool TestMoveBy(float x, float y, bool forcePositionCheck);
	virtual bool TestCurrentLocation(bool forcePositionCheck);
	virtual bool TestRoomSystemLocation();
	virtual float GetDistanceToObstacle(int direction);
	virtual bool MoveToSafePlace(const Vector2D& positionPreferred);
	virtual bool MoveToSafePlaceGivenCurrentLocation();

	
	void FloatTo(const Vector2D& position);

	void SoundLevels(long &volume, long &pan);			// retn 1 if any part of agent is on screen
	virtual bool Visibility(int scope);			// retn 1 if any part of agent is on screen
								// retn 2 if agent slightly off screen
								// retn 0 if agent is off screen
	virtual void Hide();
	virtual void Show();

	inline float GetVisualRange() const 
	{
		_ASSERT(!myGarbaged);
		return myGeneralRange;
	}

	inline float GetAudibleRange() const 
	{
		_ASSERT(!myGarbaged);
		return myGeneralRange;
	}

	inline float GetGeneralRange() 
	{
		_ASSERT(!myGarbaged); 
		return myGeneralRange;
	}

	inline bool SetGeneralRange(float newGeneralRange) 
	{
		_ASSERT(!myGarbaged); 
		if ((newGeneralRange > 1.0f) && (newGeneralRange < 2000.0f))
		{
			myGeneralRange = newGeneralRange;
			return true;
		}
		else
			return false;
	}

	void SetMovementStatus(int type);
	int GetMovementStatus()
	{
		_ASSERT(!myGarbaged); 
		return(myMovementStatus);
	}

	void SoundEffect(DWORD fsp, int delay=0);	// Play sound effect if this agent
												// within range, after 'delay' ticks
												// Sound will play once completely		
	void ControlledSound(DWORD fsp, BOOL loop=FALSE);
												// Play sound, maintaining control
												// for its duration
	void StopControlledSound(BOOL fade=FALSE);	// Stop the active sound (if any)
	void PauseControlledSound();				// Stop active sound when game is paused
	int TestAttributes(int bits);					// retn true if ANY of these attributes true
	int TestCreaturePermissions(int bits);

	// retn any agent that this agent is touching (with given attr)
	AgentHandle IsTouching(DWORD AttrFields, DWORD AttrState);

	// test if the point is on a non transparent pixel
	virtual bool HitTest(const Vector2D& point);




	// ---------------------------------------------------------------------
	// Method:      GetMapCoordOfWhereIPickUpAgentsFrom()
	// Arguments:
	// Returns:     The map coordinate of where I currently would like an agent
	//              I'm carrying to be attached to me.
	// ---------------------------------------------------------------------
	virtual Vector2D GetMapCoordOfWhereIPickUpAgentsFrom(int pose = -1, bool bRelative = false);


	// ---------------------------------------------------------------------
	// Method:      GetPlaneForCarriedAgent()
	// Arguments:
	// Returns:     The carrying plane i sholud move to when being carried 
	//				by this agent
	// ---------------------------------------------------------------------
	virtual int32 GetPlaneForCarriedAgent();

	
	// ---------------------------------------------------------------------
	// Method:      GetMyOffsetOfWhereIAmPickedUpFrom()
	// Arguments:
	// Returns:     The offset relative to the top left hand corner of my
	//				bounding box where I would like to attach to agents
	//				carrying me.  (Default value is to be carried like a 
	//				briefcase).
	// ---------------------------------------------------------------------
	virtual Vector2D GetMyOffsetOfWhereIAmPickedUpFrom(int pose = -1);

	virtual EntityImage* GetPrimaryEntity();

	Vector2D GetCentre() 
	{
		_ASSERT(!myGarbaged);
		return Vector2D
			(myPositionVector.x+myCurrentWidth/2.0f, 
			myPositionVector.y+myCurrentHeight/2.0f);
	};

	void GetAgentExtent(Box& extent)
	{
		_ASSERT(!myGarbaged);
		extent.left = myPositionVector.x;
		extent.top = myPositionVector.y;
		extent.right = myPositionVector.x + myCurrentWidth;
		extent.bottom = myPositionVector.y + myCurrentHeight;
	}

	virtual int GetPlane(int part = 0)
	{
		_ASSERT(!myGarbaged); 
		return 0;
	};

	// tinting
	virtual void Tint(const uint16* tintTable, int part = 0)
	{
		_ASSERT(!myGarbaged); 
	}

	// this resets the plane in total giving a new normal plane
	virtual void SetNormalPlane(int plane, int part = 0) 
	{
		_ASSERT(!myGarbaged);
	}

	virtual int GetNormalPlane(int part = 0) 
	{
		_ASSERT(!myGarbaged); 
		return 0;
	}

	// this gets the actual entity or entities, or skeleton
	// to change the physical display play
	virtual void ChangePhysicalPlane(int plane)
	{
		_ASSERT(!myGarbaged);
	}

	// ---------------------------------------------------------------------
	// Method:		ClickAction()
	// Arguments:	x,y - position on agent
	// Returns:		Preferred action (ACTIVATE1 etc) or -1 for no action.
	// Description:	Susses out what kind of message should be sent to this
	//				agent by the mouse pointer which it is clicked on.
	// ---------------------------------------------------------------------
	virtual int ClickAction(int x, int y);

	// set the default clickaction
	void SetDefaultClickAction( int msgid );

	void SetDefaultClickActionCycle(int i1, int i2, int i3);
	void GetClickActions(int& clac, int& clik1, int& clik2, int& clik3, int& curclak)
	{
		if (myClickActionState == -1)
		{
			clac = myDefaultClickAction;
			clik1 = clik2 = clik3 = curclak = -2;
		}
		else
		{
			clac = -2;
			clik1 = myDefaultClickActionCycles[0];
			clik2 = myDefaultClickActionCycles[1];
			clik3 = myDefaultClickActionCycles[2];
			curclak = myClickActionState;
		}
	}

	void DecideClickActionByState();

	// undo the last action this is useful if the drop command
	// was sent but was disallowed
	void GoBackToLastClickActionState();

 	virtual Vector2D WhereShouldCreaturesPressMe();

	Vector2D GetPosition() 
	{
		_ASSERT(!myGarbaged);
		return myPositionVector;
	}

	Voice& GetVoice()
	{
		_ASSERT(!myGarbaged);
		return myVoice;
	}

	virtual void CameraPositionNotify();

	bool GetRoomID(int &roomID); //this would be much quicker if the agent kept track of 
					//current room

	 float GetWidth() 
	{
		_ASSERT(!myGarbaged);
		return 	myCurrentWidth;
	}	

	 float GetHeight() 
	{
		_ASSERT(!myGarbaged);
		return 	myCurrentHeight;
	}

	int ExecuteScriptForClassifier(const Classifier& c, const AgentHandle& from,
		const CAOSVar& p1, const CAOSVar& p2);
	int ExecuteScriptForEvent(int event, const AgentHandle& from,
		const CAOSVar& p1, const CAOSVar& p2);


	virtual bool SetAnim( const uint8* anim, int length, int part) 
	{
		_ASSERT(!myGarbaged);
		return true;
	}

	virtual bool SetFrameRate( const uint8 rate, int part) 
	{
		_ASSERT(!myGarbaged); 
		return true; 
	}

	virtual bool AnimOver(int part) 
	{
		_ASSERT(!myGarbaged);
		return true;
	}

	virtual bool ShowPose(int pose,int part=0);	// default helper fn for POSE macro
	virtual int GetPose(int part);				// default helper function for POSE
	virtual bool SetBaseImage(int image,int part=0);	// default helper fn for BASE macro
	virtual int GetBaseImage(int part) 
	{
		_ASSERT(!myGarbaged);
		return -1;
	}

	virtual bool ValidatePart(int partid) 
	{
		_ASSERT(!myGarbaged); 
		return true;
	}

	Vector2D GetFloatingVector()
	{
		return myFloatingPositionVector;
	}

	// ---------------------------------------------------------------------
	// Method:      Update()
	// Arguments:   None
	// Returns:     None
	// Description: master update routine - called from clock tick.
	// ---------------------------------------------------------------------
	virtual void Update();			// called every tick


	void HandleMessage(Message* Msg);	// Deal with this message
	bool CanHear(const AgentHandle& other);
	virtual bool CanSee(const AgentHandle& other);
	bool CanTouch(const AgentHandle& other);

	bool AddAPointWhereYouCanPickMeUp(int pose, int x, int y);
	bool AddAPointWhereIPickAgentsUp(int pose, int x, int y, int part =0);

	virtual bool DoBoundsIntersect(const Box& rect);

	virtual void SetGallery(FilePath const& galleryName, int baseimage, int part = 0);

	// new serialization stuff
	virtual bool Write( CreaturesArchive &ar ) const;
	virtual bool Read( CreaturesArchive &ar );

	void DetachFloatingWith();
	void AttachFloatingWith(AgentHandle& with);
	void FloatRelativeToWorld(Vector2D& vec);
	void WorldToFloatRelative(Vector2D& vec);

	//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
class AgentException: public BasicException
{
public:
	AgentException(std::string what, uint32 line):
	 BasicException(what.c_str()),
	lineNumber(line){;}

	uint32 LineNumber(){return lineNumber;}
private:

	uint32 lineNumber;

};

	void HandleRunError(CAOSMachine::RunError& e);
	CAOSMachine& GetVirtualMachine() 
	{
		_ASSERT(!myGarbaged); 
		return myVirtualMachine; 
	}
	
	bool IsRunning()
	{
		_ASSERT(!myGarbaged);
		return myRunning;
	}

	void SetRunning(bool running) 
	{
		_ASSERT(!myGarbaged);
		myRunning = running;
	}

	bool IsMirrored(){return myDrawMirroredFlag;}


	void InitialisePickupPointsAndHandles();


	Port* IsPointOverPort(Vector2D& p, bool &inputnotoutput);

	bool GetPixelPerfectTesting()
	{
		_ASSERT(!myGarbaged);
		return myPixelPerfectHitTestFlag;
	}

	virtual bool SetPixelPerfectTesting(bool flag,int partNumber)
	{
		_ASSERT(!myGarbaged);
		return myPixelPerfectHitTestFlag = flag;
	}

	void DrawMirrored(bool mirror);

	// ---------------------------------------------------------------------
	// Method:		GetPorts()
	// Arguments:	None
	// Returns:		Portbundle containing ports for this agent.
	// Description:	Accessor function for agent InputPorts and OutputPorts.
	//				(The PortBundle agent is used simply to avoid
	//				cluttering up the agent class with port-handling
	//				functions).
	// ---------------------------------------------------------------------
	PortBundle& GetPorts()
	{
		_ASSERT(!myGarbaged);
		return myPorts; 
	}

	bool Agent::TryToBePickedUp
		(AgentHandle& newCarrierAgent, CAOSVar& p1, CAOSVar& p2);

	bool Agent::TryToBeDropped
		(AgentHandle& fromAgent, CAOSVar& p1, CAOSVar& p2, bool forceDrop);


	// Agent movement status values

	enum {
		AUTONOMOUS = 0,	  			
		MOUSEDRIVEN, // this is only used by the pointer agent			
		FLOATING,				
		INVEHICLE,				
        CARRIED
	};


	// Attribute flags:
	enum 
	{
		attrCarryable		= 0x00000001,	// 1 can be picked up
		attrMouseable		= 0x00000002,	// 2 mouse can pick up agent
		attrActivateable	= 0x00000004,	// 4 can be activated using mouse
		attrGreedyCabin		= 0x00000008,	// 8 does vehicle autopickup things dropped in cabin
		attrInvisible		= 0x00000010,	// 16 creatures don't see you - doesn't apply to ESEE or STAR, but only to internal creature code
		attrFloatable		= 0x00000020,	// 32 normally floating on screen
		attrSufferCollisions= 0x00000040,	// 64 will collide with room boundaries
		attrSufferPhysics	= 0x00000080,	// 128 
		attrCameraShy		= 0x00000100,   // 256
		attrOpenAirCabin	= 0x00000200,   // 512 this vehicle can be seen through and
		// if you change these or their meaning, please change autodocument tables in Agent.cpp ... pyjamas
	};


	// Creature permissions:
	enum {
		permCanActivate1	= 0x00000001,	// 1 creature can activate 1 this agent
		permCanActivate2	= 0x00000002,	// 2 creature can activate 2 this agent
		permCanDeactivate	= 0x00000004,	// 4 creature can deactivate this agent
		permCanHit			= 0x00000008,	// 8 creature can hit this agent
		permCanEat			= 0x00000010,	// 16 creature can eat this agent
		permCanPickUp		= 0x00000020,	// 32 creature can pick up this agent
		// if you change these or their meaning, please change autodocument tables in Agent.cpp ... pyjamas
	};


	

	GenomeStore& GetGenomeStore()
	{
		return myGenomeStore;
	}

	//
	// Movement
	//
	void HandleMovement();
	virtual void HandleMovementWhenAutonomous();
	virtual void HandleMovementWhenFloating();
	virtual void HandleMovementWhenInVehicle();
	virtual void HandleMovementWhenCarried();

	void HandleCA();

protected:

	virtual bool CanStartCarrying(AgentHandle& a);
	bool CanStopCarrying(AgentHandle& a, AgentHandle& v, bool& findSafePlace);
	virtual void StartCarrying(AgentHandle& a);
	virtual void StopCarrying(AgentHandle& a);
	
	void SoundUpdate();

	

	// Private functions...
	virtual void HandleActivate1(Message* Msg);	// Message handlers. Default to
	virtual void HandleActivate2(Message* Msg);	// NULL fns here, so must be
	virtual void HandleDeactivate(Message* Msg);// overloaded with valid fns in all
	virtual void HandlePickup(Message* Msg);	// subclasses that need action
	virtual void HandleDrop(Message* Msg);		// (& sub-subs where appropriate)
	virtual void HandleHit(Message* Msg);
    virtual void HandleEat(Message* Msg);
	virtual void HandleUI(Message* Msg); // message numbers between SCRIPTUIMIN & SCRIPTUIMAX
										 // default calls HandleOther
	virtual void HandleStartHoldHands(Message* Msg); // only for creatures
	virtual void HandleStopHoldHands(Message* Msg); // only for creatures

	void HandleOther(Message* Msg);				// Generic inter-agent message handler.


	void HandleSound();
	void HandleInputEvents();
	void HandleConnections();

	void Init();				// Basic initialisation used by constructors


	//
	// Member variables
	//

	bool myHighlightedFlag;
	struct {
		int r,g,b;
	} myHighlightColour;

	Voice myVoice;

	SOUNDHANDLE	mySoundHandle;	// Controlled Sound (-1=Not playing)
	DWORD mySoundName;			// Name of active sound (NULL=none)
	BOOL mySoundLooping;			// Controlled sound is looping


	AgentHandle mySelf;

	CAOSVar	myGlobalVariables[ GLOBAL_VARIABLE_COUNT ];	// OV00..OV99

	bool myRunning;				// if zero, do not bother calling this agent's Update()
								// function (agent is out of scope, or on
								// Death Row awaiting deletion)

	int32 myUpdateTick;

	uint32 myMarchingAntCount;


	// types of input events this agent is interested in
	int myInputMask;

	// a CAOSMachine to run scripts on...
	CAOSMachine			myVirtualMachine;

	// input and output ports
	PortBundle			myPorts;

	// where I am to be picked up by others
	std::vector<Vector2D> myPointsWhereIamPickedUp;	// Indexed by pose.

	// where I pick others up
	std::vector<Vector2D>  myPointsIUseToPickUpAgents;	// Indexed by pose.

	EntityImage*		myEntityImage;				// entity for this agent

	Classifier myClassifier; 	// Family/Genus/Species/00 identifier (used fr event macros & recognition)
	int myTimer;				// TIMER event countdown
	int myTimerRate;			// # ticks between TIMER event scripts (0=no timer active)
	uint32 myAttributes;		// attribute flags
	uint32 myCreaturePermissions;// creature attributes
	int myMovementStatus;		// autonomous, floating etc.	
	uint32 myID;				// Unique ID.

	AgentHandle myCarrierAgent;	
	AgentHandle myCarriedAgent;			// Agent being carried (or NULL)

	bool myGarbaged;
	bool myImpendingDoom;

	VelocityVariable		myVelocityXVariable;	// CAOS interface for x velocity
	VelocityVariable		myVelocityYVariable;	// CAOS interface for y velocity
	Vector2D myVelocityVector;

	Vector2D myPositionVector;
	Vector2D myFloatingPositionVector;
	AgentHandle myFloatingWith;
	// the floatees list is so the pointer can update things floating relative to
	// it from the depths of the camera code
	std::vector<AgentHandle> myFloatees;

	int myElasticity;	
	int myFrictionPercentage;
	float myFrictionFactor;
	float myCollisionFactor;
	float myAeroDynamicFactor;	// % velocity lost due to media resistance.
	int myAeroDynamicPercentage;
	int myPermiability;		// Below which the agent cannot pass through.
	float myGravitationalAcceleration;	// Rate at which agent accelerates in pixels per tick.
	bool myStoppedFlag;
	
	int myLastWallHit;

	int	myDefaultClickAction;		// used by ClickAction()
	int myClickActionState;			// used to decide whether we are cycling
									// click actions or not -1 if we are not
									// else 0 1 2
	int	myDefaultClickActionCycles[3];

	//
	// The following are for CA processing.
	//

	int myCAIndex;			//Index of the CA property which this agent will effect
	float myCAIncrease;		//Amount by which the CA property will be increased
	bool myCAIsNavigable;	//(may be -ve) - GuyT

	int myPrevCAIndex;				// 
	int myPrevRoomWhenEmitting;		// These for the Navigable CA's
	float myPrevCAIncrease;			//
	bool myPrevCAIsNavigable;		//

	enum
	{
		stateNotProcessing,
		stateStableSettings,
		stateSettingsChange,
	} myCAProcessingState;			// Allows rapid state switch determination


	bool myInvalidPosition; // set to true until the agent is moved
	float myGeneralRange;
	bool myPixelPerfectHitTestFlag;

	// Used for bounding box checks
	float myWidthTemp, myHeightTemp;

	float myCurrentWidth;
	float myCurrentHeight;
	bool myResetLines;

	bool myDrawMirroredFlag;
	
	// Reference counting done through the smart pointer
	unsigned myReferenceCount;

	// myAgentType is used to store the type of the agent. This allows the AgentHandles
	// to do smart rtti which is faster than the slower C++ rtti (Dan)
	uint32 myAgentType;

#ifdef AGENT_PROFILER
public:
	int64 myAgentProfilerCumulativeTime;
	int myAgentProfilerCumulativeTicks;
	static int ourAgentProfilerTicks;
	static double ourAgentProfilerPaceTotal;
#endif

	GenomeStore myGenomeStore;
};


inline uint32 Agent::GetFamily()
{
	_ASSERT(!myGarbaged);
	return myClassifier.Family();
} 

inline uint32 Agent::GetGenus()
{
	_ASSERT(!myGarbaged);
	return myClassifier.Genus();
} 

inline uint32 Agent::GetSpecies()
{
	_ASSERT(!myGarbaged);
	return myClassifier.Species();
} 

// Test one or more Attributes (attrCarryable, ACTIVEATABLE, etc.)
// Return non-zero if ANY of the given attrs are set
inline int Agent::TestAttributes(int bits)
{
	_ASSERT(!myGarbaged);
	return (myAttributes & bits);
}

// Test one or more creature permissions:
// Return non-zero if ANY of the given perms are set
inline int Agent::TestCreaturePermissions(int bits)
{
	_ASSERT(!myGarbaged);
	return(myCreaturePermissions & bits);
}




#endif // AGENT_H


