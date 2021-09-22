/*********************************************************************
* File:     Organ.h
* Created:  13/01/98
* Author:   Robin E. Charlton
* (split dec 1998 by gtb)
*********************************************************************/

#ifndef _ORGAN
#define _ORGAN

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include <vector>
#include "ChemicallyActive.h"
#include "BiochemistryConstants.h"

#include "Reaction.h"
#include "Emitter.h"
#include "Receptor.h"

class Biochemistry;
class Genome;


typedef std::vector<Receptor*> Receptors;

class Organ : public PersistentObject, public ChemicallyActive {
	friend Biochemistry;

	CREATURES_DECLARE_SERIAL(Organ)
public:
	// Construction & Destruction:
	Organ();
	virtual ~Organ();

	void Init(float ClockRate, float RateOfRepair, float LifeForce, float InitialClock, float DamageDueToZeroEnergy);
    void BindToLoci();
	void InitFromGenome(Genome& genome);

    inline void SetOwner(Biochemistry* pOwner){	myBiochemistryOwner = pOwner;}

	// Processes:
	virtual bool Update();
	virtual void Injure(float damage);
	float LocToLf(float l);

	// Access:
	virtual float* GetLocusAddress(int type, int organ, int tissue, int locus);
	virtual float* GetLocusAddress(int type,			// is this an EMITTER or RECEPTOR locus?
								int tissue, 			// tisse containing locus (eg. lobe#)
								int locus);			// locus on object (eg. .Activity)

	inline const Emitter& GetEmitter(int iIndex) const	 {	return myEmitters[iIndex];}
	inline const Reaction& GetReaction(int iIndex) const{	return myReactions[iIndex];}
	inline int ReceptorCount() const{					return myNoOfReceptors;}
	inline int EmitterCount() const	{					return myNoOfEmitters;}
	inline int ReactionCount() const{					return myNoOfReactions;}

	inline float LocusClockRate() const { return loc_ClockRate;}
	inline float LocusLifeForce() const { return loc_LifeForce;}
    inline float LocusLongTermRateOfRepair() const { return loc_LongTermRateOfRepair;}
    inline float LocusInjuryToApply() const { return loc_InjuryToApply;}
	inline float InitialLifeForce() const { return myInitialLifeForce;}
	inline float ShortTermLifeForce() const { return myShortTermLifeForce;}
	inline float LongTermLifeForce() const { return myLongTermLifeForce;}
	inline float LongTermRateOfRepair() const { return myLongTermRateOfRepair;}
	inline float EnergyCost() const { return myEnergyCost;}
	inline float DamageDueToZeroEnergy() const { return myDamageDueToZeroEnergy;}

    inline bool Failed() const		{					return (myLongTermLifeForce <= myMinLifeForce);}
    inline bool Functioning() const	{					return myEnergyAvailableFlag && !Failed();}

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

protected:
	virtual void InitEnergyCost();
	virtual bool ConsumeEnergy();
	void DecayLifeForce();
	virtual bool RepairInjury(bool bEnergyAvailable);
	bool ProcessAll();
	bool ProcessReaction(int iIndex);
	bool ProcessReceptors(bool onlyDoClockRateReceptors);

	Biochemistry* myBiochemistryOwner;

	float loc_ClockRate;	// Internal ticks per update.
	float myClock;			// Incremented by loc_ClockRate each Update().

	float loc_LifeForce;                        // Locus representation of life-force.
    float loc_LongTermRateOfRepair;             // Locus modifier to rate of repair.
    float loc_InjuryToApply;

	float myInitialLifeForce;
	float myShortTermLifeForce;
	float myLongTermLifeForce;
	float myLongTermRateOfRepair;               // Genetically specified rate of repair.
	float myEnergyCost;
	float myDamageDueToZeroEnergy;
    bool myEnergyAvailableFlag;

	Emitter myEmitters[MAXEMITTERS];			// array of chemo-emitters
	Reaction myReactions[MAXREACTIONS];			// array of chemical reaction sites
	Receptors myReceptorGroups[MAXRECEPTORGROUPS];

	int myNoOfReceptorGroups;
	int myNoOfReceptors;						// # active receptors
	int myNoOfEmitters;							// # active emitters
	int myNoOfReactions;						// # active reaction sites

	static const float myBaseOrganADPCost;
	static const float myRateOfDecay;
	static const float myBaseLifeForce;
	static const float myMinLifeForce;
};

#endif
