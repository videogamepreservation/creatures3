// BrainComponent.h: interface for the BrainComponent class.
//
//////////////////////////////////////////////////////////////////////

#ifndef BrainComponent_H
#define BrainComponent_H


#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include "SVRule.h"

#include <vector>
class BrainComponent;
typedef std::vector<BrainComponent*> BrainComponents;

class BrainComponent : public PersistentObject {
	CREATURES_DECLARE_SERIAL(BrainComponent)
public:
	BrainComponent();
	virtual ~BrainComponent();

	virtual void DoUpdate() {}
	virtual void TraceDebugInformation() {}
	virtual void Initialise() {};
	void RegisterBiochemistry(float* chemicals);

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	static bool xIsProcessedBeforeY(BrainComponent* x, BrainComponent* y);
	int GetUpdateAtTime();

	inline int* GetPointerToIdInList() {return &myIdInList;}
	inline void SetIdInList(int i) {			myIdInList = i;}
	inline int GetIdInList() {			return myIdInList;}

	inline bool SupportsReinforcement() {	return mySupportReinforcementFlag;}

protected:
	int myIdInList;

	// 'true' if the initialisation SV Rule should run on every update
	// instead of upon creation and migration.
	// If the Init rule is run always then default initialisation takes
	// place on creation and migration, e.g. for tracts weights are cleared
	bool myRunInitRuleAlwaysFlag;
	// 'true' if a brain component supports ST/LT 
	// weights and Reward/Punishment (i.e. tracts)
	bool mySupportReinforcementFlag;	 
	int myUpdateAtTime;
	SVRule myInitRule;
	SVRule myUpdateRule;
	float *myPointerToChemicals;
	bool myInitialised;
};


#endif//BrainComponent_H
