

// Skeleton class. Derived from Agent. Virtual parent for Creature class, which adds 
// brain, genetics, biochemistry and senses to the Skeleton

#ifndef SKELETON_H
#define SKELETON_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Definitions.h"
#include "Genome.h"
#include "../Entity.h"
#include "../AgentManager.h"
#include "../General.h"
#include "../Agents/Agent.h"
#include "../mfchack.h"
#include "../Display/CompressedGallery.h"


const int MAX_GAITS = 16;		// number of possible walking gaits (including default gait)
const int MAX_SKELETON_ANIMATION_STRING_LENGTH = ((30 * 3) + 2);  // 30 Poses + 'R' + '\0';
const int MAX_POSE_STRING_LENGTH = 15;					// # bytes in a Pose string

// Pose list index labels - indices into Poses[]
enum poses {

	// First set of poses are the Reach-out-and-touch poses...
	// See notes above for sequence
	POSE_TOUCH=0,				   			// start of touch poses
	POSE_TOUCH_NEAR_LOW = POSE_TOUCH,	// individual touch poses
	POSE_TOUCH_NEAR_LOWISH,
	POSE_TOUCH_NEAR_HIGHISH,
	POSE_TOUCH_NEAR_HIGH,
	POSE_TOUCH_MED_LOW,
	POSE_TOUCH_MED_LOWISH,
	POSE_TOUCH_MED_HIGHISH,
	POSE_TOUCH_MED_HIGH,
	POSE_TOUCH_FAR_LOW,
	POSE_TOUCH_FAR_LOWISH,
	POSE_TOUCH_FAR_HIGHISH,
	POSE_TOUCH_FAR_HIGH,

	// Pre-defined poses that might need to be referred to in code...
    POSE_STOP,              // stand & look at it <5>

    POSE_WALK1,
    POSE_WALK2,
    POSE_WALK3,
    POSE_WALK4,

    // All other poses are defined in Gene Editor

	MAX_POSES = 256			// Total number of poses allowed per creature
};											// (excl. terminator)

class TintManager;
class CreatureGallery;

class Skeleton : public Agent 
{
	typedef Agent base;

	CREATURES_DECLARE_SERIAL(Skeleton)

protected:

	// New style plot order array.

	EntityImage* myPlotOrders[4][4][NUMPARTS];

	bool myDoubleSpeedFlag;
	bool myJustAgedFlag;
	CreatureGallery*	myCompositeBodyPartGallery;
	CreatureGallery*	myCompositeBodyPartGalleryForNextAgeStage;
	int32				myNumSpritesInFile;
	int32				myNumberOfSpritesInFileforAgeing;
	CompressedGallery	myIndividualBodyPartGalleries[NUMPARTS];
	int32				myVariants[NUMPARTS];	// variant # to use for each part
    int32				myGenera[NUMPARTS];
	// which sprite# in final sprite file refers to the first pose of each body part
	// Filled in while copying sprites from library files. Used to construct Body & Limbs
	int32				myBodySprites[NUMPARTS];
	Body*				myBody;						// ptr to Body Ent
	Limb*				myLimbs[MAX_BODY_LIMBS];		// limbs (or NULL if none)
	uint32				myBodyPartToBeCreated;
	uint32				myBodyPartToBeCreatedForNextAgeStage;
	bool				myBodyPartsAreFullyFormedFlag;
	int					myBodyPartsAgeStage;
	int					myLastAge;
	int					myPreloadLastAge;
	bool				myAgeingDone;
	int					myAgeAlreadyLoaded;
	TintManager*		myTintin;

	bool myIsHoldingHandsWithThePointer;




	void GeneratePlotOrder();

	//
	// Movement
	//
	virtual void HandleMovementWhenAutonomous();
	//virtual void HandleMovementWhenFloating();
	virtual void HandleMovementWhenInVehicle();

	std::string myMotherMoniker;			// ditto to identify my mother
	std::string myFatherMoniker;			// ditto to identify my father

	bool myEntitiesAreCameraShy;

	uint8	myCurrentDirection;					// which dirn facing (ESWN)
	byte	myCurrentDownFoot;					// which foot is 'down' 0/1 LEFT/RIGHT
	Vector2D	myDownFootPosition;
	Vector2D	myUpFootPosition;
	int32   myNormalPlane;				// plot plane when not in a vehicle
	Vector2D  myPreviousUpFootPosition;
	float myPreviousMinY;
	float myMinX;
	float myMaxX;
	float myMinY;
	float myMaxY;
	Vector2D myExtremes[MAX_BODY_LIMBS];

	char	myCurrentPoseString[MAX_POSE_STRING_LENGTH+1];		// Pose$ representing current pose, used by UpdatePose()
	char	myTargetPoseString[MAX_POSE_STRING_LENGTH+1];	// Pose that UpdatePose() is currently striving towards

	char	myAnimationString[MAX_SKELETON_ANIMATION_STRING_LENGTH];		// current animation string
	char*	myAnimationPointer;	
	
	bool myStandStill;



	byte	myFacialExpression;			// 012: normal/happy/sad face
	uint8	myEarSet;
	int		myEyeState;					// 01: eyes closed/open
	bool	myBodyIsSuitedUp;
	bool myBeingTrackedFlag;


   	char	myPoseStringTable[MAX_POSES][MAX_POSE_STRING_LENGTH+1];// array of poses fr actions & reaching out
	char	myGaitTable[MAX_GAITS]			// array of walking animations ("010203R"), one for
					   [MAX_SKELETON_ANIMATION_STRING_LENGTH];	// each gait receptor (room fr 8 poses + 'R',\0)



	// Chemo-receptor and emitter loci that are controlled/monitored by skeleton
	// rather than creature
	float myMusclesLocus;				// emitter: amount of energy expended by movement this tick
    float myUpslopeLocus;				// emitter: how steep is the slope I'm facing
    float myDownslopeLocus;			// emitter: how steep is the slope I'm facing
	float myGaitLoci[MAX_GAITS];		// trigger different walking gaits ([0]=default)


	Vector2D myReach;				// Reach (inited from catalogue)

	// to do with IT (attended-to object)...
	AgentHandle	myIt;					// object of this creature's attention
									// NULL if none
	bool myIntrospectiveFlag;			// for extraspective actions like GO EAST and GO WEST

	Vector2D myItPosition;
	int	myHowToTouchIt;				// Msg you want to send to IT
									// (used to determine hotspot for Touch)

	void UpdatePositionsWithRespectToDownFoot();
	void CommitPositions()
	{
		myPreviousUpFootPosition = myUpFootPosition;
		myPreviousMinY = myMinY;
	}

	void UpdatePlotOrder();			// Set the display plane for each
									// of this body's limbs,
									// for so that all limbs are plotted
									// in the correct order
									// after a change of direction

	void UpdateFeet();				// swap 'down' foot if necessary

	void TrackIt();					// update ItX,ItY if IT has moved

	virtual void Update();			// update animation/blink/track (was Animate)
	char HeadAngle();				// used by SetPose() to look up/down
	bool HasBody();
		




	bool CreateBodyPart(int sex, int age,TintManager& tinty);
	bool PreloadBodyPartsForNextAgeStage(Genome& genome, int sex);

	bool ContinueLoadingDataForNextAgeStage();

	bool BuildGalleries(int sex, int age,uint32& currentPart, TintManager& tinty);



public:

	void GetExtremesForPose
	(char* pose, Vector2D extremes[MAX_BODY_LIMBS]);

	Vector2D GetMouthMapPosition();
	virtual void ChangePhysicalPlane(int plane);
	virtual void ChangeCameraShyStatus(bool shy);

	virtual int GetNormalPlane(int part = 0)
	{
		_ASSERT( !myGarbaged );
		return myNormalPlane;
	}

	bool ExpressGenes(const Genome& g, int age);// Express any genes relevant to current age
	
	bool PrepareForAgeing(int age);
	
	virtual int ReachOut(BOOL Point=FALSE);	// macro helper: touch IT or point to it. Made virtual - dsb

	virtual void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen = 0,
					uint8 lineColourBlue = 0,
					uint8 stippleon =0,
					uint8 stippleoff = 0,
					uint32 stipplestartAt =0);

	virtual void ResetLines();
	
	bool AreYourBodyPartsFullyFormed(){return myBodyPartsAreFullyFormedFlag;}



 	void Walk();						// helper fn for WALK macro

	bool CanReachIt();					// return true if creature within reach of IT
	bool CannotBeMuchCloser();

	inline void SetFacialExpression(int headset, int earset= -1) 
	{
		_ASSERT(!myGarbaged);

		myFacialExpression = headset;
		// nothing done with the ears as yet
	}
	inline int GetFacialExpression() 
	{
		_ASSERT(!myGarbaged);
		return myFacialExpression;
	}
	inline int GetEyeState() const 
	{
		_ASSERT(!myGarbaged);
		return myEyeState;
	}

	inline void SetEyeState(int i) 
	{
		_ASSERT(!myGarbaged);
		myEyeState = i;
	}

	inline std::string GetMoniker()
	{
		_ASSERT(!myGarbaged);
		return GetGenomeStore().MonikerAsString(0);
	}

	inline std::string GetMotherMoniker() const 
	{
		_ASSERT(!myGarbaged);
		return myMotherMoniker;
	}

	inline std::string GetFatherMoniker() const 
	{
		_ASSERT(!myGarbaged);
		return myFatherMoniker;
	}

	// this can be removed, along with myHowToTouchIt
	inline void SetHowToTouchIt(int i) 
	{
		_ASSERT(!myGarbaged);
		myHowToTouchIt=i;
	}

	inline void SetDirection(int d) 
	{
		_ASSERT(!myGarbaged);
		myCurrentDirection=d;
	}

	inline int GetDirection() const 
	{
		_ASSERT(!myGarbaged);
		return myCurrentDirection;
	}
  
	bool GetExtremePoint(int bodypart,Vector2D& point);

	//-- constructors
	Skeleton();								// Serialisation constr
	Skeleton(AgentHandle& gene, int gene_index);// Constructor called by Creature constr

	virtual ~Skeleton();					// destruct
	void Init();							// common initialisation


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


	//--- initialisers ---
	bool CreateSkeleton(Genome& genome,	
					int Age);

	bool ReloadSkeleton(Genome& genome,
						int Age);

	void ReadAppearanceGenes(Genome& genome, TintManager& tinty);

	void DestroySkeleton();						// Kill body & limb entities
	
	void AttachLimbChain(int position,Limb* limbChain);	// attach a chain of Limbs to
											// this joint (after constructn)


	// -- (virtual) helper functions for macro commands
	virtual bool SetAnim( const uint8* anim, int length,int part);	// helper fn for ANIM macro
	virtual bool AnimOver(int part);				// helper fn for OVER macro
	virtual bool ShowPose(int pose,int part=0);	// helper fn for POSE macro

	void SetGait(int gait);
	void SetCurrentPoseString(char* pose);			// go straight to this pose$
	void MoveTowardsTargetPoseString();					// go 1 more step towards TargetPose
	bool SetTargetPoseString(char* pose);		// define new TargetPose if necc. Retn TRUE if reached already
	bool HasTargetPoseStringBeenReached();				// retn TRUE if TargetPose has now been reached

	void SetAnimationString(char* animation);			// start a new animation, given a c++ string ("0123R")
	void ResetAnimationString();					// cancel any animation prior to special pose change

	void UpdateSkeleton(); // update all xys, sprite *s &
										// bounding box; avoid walls & swap
										// feet if requd. Generally init
										// body & limbs after a move/pose

	void MoveFootTo(float x,float y);
	bool TestMoveFootTo(float x, float y, bool forcePositionCheck);
	virtual void MoveBy(float xd,float yd);	
	virtual void MoveTo(float x, float y);		
	virtual bool TestMoveTo(float x, float y, bool forcePositionCheck);
	virtual bool TestMoveBy(float x, float y, bool forcePositionCheck);
	virtual bool TestCurrentLocation(bool forcePositionCheck);
	virtual bool TestRoomSystemLocation();
	virtual float GetDistanceToObstacle(int direction);
	virtual bool MoveToSafePlace(const Vector2D& positionPreferred);
	virtual bool MoveToSafePlaceGivenCurrentLocation();


	inline void SetItPosition(const Vector2D& position)
	{
		_ASSERT(!myGarbaged);
		myItPosition = position;
	}

	inline Vector2D GetItPosition()					// return curr IT's location
	{
		_ASSERT(!myGarbaged);
		return myItPosition;
	}


	inline void SetItAgent(AgentHandle& a) 
	{	// define object of your attention
		// stop script if extraspective and old it is being invalidated:
		_ASSERT(!myGarbaged);
		myIt = a;
		myHowToTouchIt = ACTIVATE1;     // default hotspot
		TrackIt();                      // find out where it is
	}

	inline AgentHandle GetItAgent() 
	{ 		// return curr IT object
		_ASSERT(!myGarbaged);
		return myIt;
	}



  	//--- reading data ---
	Vector2D GetDownFootPosition();
	Vector2D GetUpFootPosition();

	float GetReachX();
	float GetReachY();

	virtual int GetPlane(int part = 0);			// return *principal* plot plane

	virtual Vector2D GetMapCoordOfWhereIPickUpAgentsFrom(int pose = -1, bool bRelative = false);
	virtual Vector2D GetMyOffsetOfWhereIAmPickedUpFrom(int pose = -1);


	virtual bool HitTest(Vector2D& point);
	virtual bool Visibility(int scope);	// retn 1 if any part of agent is on screen


	inline void SetIntrospective(bool b) 
	{
		_ASSERT(!myGarbaged);
		myIntrospectiveFlag= b;
	}

	inline bool IsIntrospective() 
	{
		_ASSERT(!myGarbaged);
		return myIntrospectiveFlag;
	}


	virtual void Trash();

	EntityImage* GetEntityImage() 
	{
		_ASSERT(!myGarbaged);
		return myBody;
	}

	void SetPregnancyStage(float progesteroneLevel);
	void ChangeHairStateInSomeWay(int action);

	int GetOverlayIndex(int region);
	int GetClothingSet(int bodypart, int layer);

	bool WearOutfit(int set,int layer);
	bool WearItem(int bodypart, int set, int layer);

	void GetBodyPartFileName(int index, std::string& path);
};




// stop any animation - MUST call this before trying to change pose via any other means
// than an animation!!!!
inline void Skeleton::ResetAnimationString()
{
	_ASSERT(!myGarbaged);
	*myAnimationString = 0; 
	myAnimationPointer = myAnimationString;
}


// attach a chain of limbs to a joint
inline void Skeleton::AttachLimbChain(int position, Limb* limbChain)
{
	_ASSERT(!myGarbaged);
	myLimbs[position] = limbChain;
}



// read world xy of down FOOT
inline Vector2D Skeleton::GetDownFootPosition()
{
	_ASSERT(!myGarbaged);
	return myDownFootPosition;
}


// read world xy of up FOOT
inline Vector2D Skeleton::GetUpFootPosition()
{
	_ASSERT(!myGarbaged);
	return myUpFootPosition;
}


// return *principal* plot plane
inline int Skeleton::GetPlane(int part)
{
	_ASSERT(!myGarbaged);
	return(myBody->GetPlane());
}

#endif // SKELETON_H
