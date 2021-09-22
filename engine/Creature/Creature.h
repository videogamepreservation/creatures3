
#ifndef Creature_H
#define Creature_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Skeleton.h"

#include "Biochemistry/BiochemistryConstants.h"
#include "CreatureConstants.h"


class Faculty;

class LinguisticFaculty;
class MotorFaculty;
class SensoryFaculty;
class ReproductiveFaculty;
class ExpressiveFaculty;
class LifeFaculty;
class MusicFaculty;

class Brain;
class Biochemistry;


class Creature : public Skeleton 
{
	CREATURES_DECLARE_SERIAL( Creature )
	typedef Skeleton base;

public:
	Creature();							// Serialisation constr
	virtual ~Creature();				// destruct an obj & remove it from objlib
	Creature(	int family, uint32 id,
				AgentHandle gene, int gene_index,			// your genome
				int   sex,				// your sex 0=randomly choose 1=male 2=female
				int variant);		// only the agent manager can create Creatures (except serialization)
	void Init();						// common initialisation


	virtual float* GetLocusAddress(int type, int tissue, int organ, int locus);
	virtual bool ExpressGenes();		// Express any genes relevant to current age
	float GetDriveLevel(int i);
	int GetHighestDrive();
	void RememberThatYouAreBeingTracked(bool amibeingtracked);

	bool FormBodyParts();


////////////////////////////////////////////////////////////////////////////////
// AGENT:



	virtual void HandlePickup(Message* Msg);

	virtual void SpeakSentence(std::string& thisSentence);
	virtual void Update();
   	virtual void Trash();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	virtual int ClickAction(int x, int y);
 	virtual Vector2D WhereShouldCreaturesPressMe();

	// Holding hands
	bool AreYouHoldingHandsWithThePointer()
	{
		return myIsHoldingHandsWithThePointer;
	}
	virtual void HandleStartHoldHands(Message* Msg);
	virtual void HandleStopHoldHands(Message* Msg);
	void StartHoldingHandsWithThePointer
		(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2);
	void HoldHandsWithThePointer();
	void StopHoldingHandsWithThePointer
		(const AgentHandle& from, const CAOSVar& p1, const CAOSVar& p2);



	virtual bool CanSee(const AgentHandle& other);

	void MakeYourselfTired();



	inline ExpressiveFaculty* Expressive() 
	{
		_ASSERT(!myGarbaged);
		return myExpressiveFaculty;
	}

	inline LinguisticFaculty* Linguistic() 
	{
		_ASSERT(!myGarbaged);
		return myLinguisticFaculty;
	}

	inline ReproductiveFaculty* Reproductive() 
	{
		_ASSERT(!myGarbaged);
		return myReproductiveFaculty;
	}

	inline SensoryFaculty* Sensory() 
	{
		_ASSERT(!myGarbaged);
		return mySensoryFaculty;
	}

	inline MotorFaculty* Motor() 
	{
		_ASSERT(!myGarbaged);
		return myMotorFaculty;
	}

	inline LifeFaculty* Life() 
	{
		_ASSERT(!myGarbaged);
		return myLifeFaculty;
	}

	inline MusicFaculty* Music() 
	{
		_ASSERT(!myGarbaged);
		return myMusicFaculty;
	}

	inline Brain* GetBrain() 
	{
		_ASSERT(!myGarbaged);
		return myBrain;
	}

	inline Biochemistry* GetBiochemistry() 
	{
		_ASSERT(!myGarbaged);
		return myBiochemistry;
	}

	enum {noOfFaculties=9};


	inline float* GetInvalidLocusAddress() 
	{
		_ASSERT(!myGarbaged);
		return &myInvalidLocus;
	}

	// this is grotesque
	void RemakeSkeletonAfterSerialisation();

protected:
	ExpressiveFaculty* myExpressiveFaculty;
	LinguisticFaculty* myLinguisticFaculty;
	ReproductiveFaculty* myReproductiveFaculty;
	SensoryFaculty* mySensoryFaculty;
	MotorFaculty* myMotorFaculty;
	LifeFaculty* myLifeFaculty;
	MusicFaculty* myMusicFaculty;
	Brain* myBrain;
	Biochemistry* myBiochemistry;

	Faculty* myFaculties[noOfFaculties];

	float myDriveLoci[NUMDRIVES];
	float myConstantLocus;
	float myFloatingLoci[NUM_FLOATING_LOCI];
	float myAirQualityLocus;
	float myCrowdedLocus;
	float myInvalidLocus;

	// these two don't need to be serialised:
	int myUpdateTickOffset;
	static int ourNextUpdateTickOffsetToUse;
};
#endif //Creature_H
