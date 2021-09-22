// LifeFaculty.h: interface for the LifeFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LifeFaculty_H
#define LifeFaculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Faculty.h"
#include "CreatureConstants.h"

enum LifeState {
	zombieState,
	alertState,
	asleepState,
	dreamingState,
	unconsciousState,
	deadState
};

class LifeFaculty : public Faculty {
	CREATURES_DECLARE_SERIAL(LifeFaculty)
public:
	typedef Faculty base;
	LifeFaculty();
	virtual ~LifeFaculty();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	void Update();
	virtual float* GetLocusAddress(int type, int organ, int tissue, int locus);

	inline bool GetWhetherZombie() const {		return myState==zombieState;}
    inline bool GetWhetherAlert() const {		return myState==alertState;}
	inline bool GetWhetherAsleep() const {		return (myState==asleepState || myState==dreamingState);}
	inline bool GetWhetherDreaming() const {	return myState==dreamingState;}
	inline bool GetWhetherUnconscious() const {	return myState==unconsciousState;}
	inline bool GetWhetherDead() const {		return myState==deadState;}

	void SetWhetherZombie(bool b);
	void SetWhetherAlert(bool b);
	void SetWhetherAsleep(bool a);
	void SetWhetherDreaming(bool d);
	void SetState(LifeState s);

	void SetWhetherUnconscious(bool u);
	void SetWhetherDead(bool d);

	inline bool GetProperlyBorn() const {	return myProperlyBorn; }
	void SetProperlyBorn();

	inline int GetSex() const {		return mySex;}
	inline int GetVariant() const {	return myVariant;}
	inline int GetAge() const {		return myAge;}
	inline int GetNextAge() const {return myNextAge;}

	inline void SetSex(int i) {		mySex = i;}
	inline void SetVariant(int i) {	myVariant = i;}

	void ForceAgeing();
    float Health() const;

	int GetTickAge() const { return myAgeInTicks; }

private:
	void SendAgeEvent();

protected:
	float myAsleepLocus;				// 255 if asleep, else 0
	float myDeathTriggerLocus;			// creature dies if this recep switches on
	float myAgeingLoci[NUMAGES];		// [receptor] controls ageing - each triggers start of a new phase

	bool myProperlyBorn;
	LifeState myState;
	int myNextAge;

	int mySex;							// Sex: 1=male 2=female (controls sex-linked genes)
	int myAge;							// Current phase of life (used to control gene expression)
	int myVariant;						// behav variant for genome
	int myAgeInTicks;					// Age in clock ticks since properly born
	int myNumberOfForceAgeingRequestsPending;
};
#endif//LifeFaculty_H