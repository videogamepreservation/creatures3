// MotorFaculty.h: interface for the MotorFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MotorFaculty_H
#define MotorFaculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CreatureConstants.h"
#include "Faculty.h"

class Creature;
class Agent;

struct InvoluntaryAction {
	float locus;							// receptor locus (recommendation strength)
	byte latency;							// counter to prevent instant reactivation
};


class MotorFaculty : public Faculty {
	CREATURES_DECLARE_SERIAL(MotorFaculty)
public:
	typedef Faculty base;
	MotorFaculty();
	virtual ~MotorFaculty();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	void Update();

	inline void SetAttentionOverride(int i) {
		myVoluntaryScriptOverrides.attentionScriptNo = i;
	}
	inline void SetDecisionOverride(int i) {
		myVoluntaryScriptOverrides.decisionScriptNo = i;
	}
	inline int GetCurrentDecisionId() {	return myCurrentAction;		}
	inline int GetCurrentAttentionId() {return myCurrentFocusOfAttention;	}


	inline void SetLatencyOfInvoluntaryAction(int i, int l) {
		myInvoluntaryActions[i].latency = l;
	}
	inline void StopCurrentAction(){myCurrentAction=-1;}
	inline void StopCurrentInvoluntaryAction(){myCurrentInvoluntaryAction=-1;}
	inline int GetCurrentInvoluntaryAction(){return myCurrentInvoluntaryAction;}


	float* GetLocusAddress(int type, int organ, int tissue, int locus);

protected:
	int myCurrentFocusOfAttention;		// current brain attention lobe output id
	int myCurrentAction;				// currently executing voluntary action#
	int myCurrentInvoluntaryAction;		// currently executing reflex action (flinch, throw a fit...)
										// or -1 if not doing one
	struct {							// CAOS can override the brain using these
		int attentionScriptNo;			// -1 (or any negative value)
		int decisionScriptNo;			//   means no override
	} myVoluntaryScriptOverrides;
	InvoluntaryAction myInvoluntaryActions[NUMINVOL];// trigger involuntary actions (flinching, fits, langour...)
};

#endif//MotorFaculty_H
