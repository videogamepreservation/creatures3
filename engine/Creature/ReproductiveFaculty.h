// ReproductiveFaculty.h: interface for the ReproductiveFaculty class.
//
//////////////////////////////////////////////////////////////////////
#ifndef ReproductiveFaculty_H
#define ReproductiveFaculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Faculty.h"


class Creature;

class ReproductiveFaculty : public Faculty {
	CREATURES_DECLARE_SERIAL(ReproductiveFaculty)
public:
	typedef Faculty base;
	ReproductiveFaculty();
	virtual ~ReproductiveFaculty();

	void Update();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	bool IsPregnant();
	void AcceptSperm(AgentHandle dad, BYTE DadChanceOfMutation, BYTE DadDegreeOfMutation);
	void DonateSperm();					// Reproduction: A male passes possible sperm to a female by
										// calling DonateSperm(), which calls the female's AcceptSperm()

	float GetProgesteroneLevel();

	virtual float* GetLocusAddress(int type, int organ, int tissue, int locus);


protected:
	bool myGamete;						// whether we have egg/sperm available

	float myFertileLocus;				// 255 if .Gamete contains an egg/sperm
	float myPregnancyLocus;				// 255 if .Zygote contains a fertilised egg
	float myOvulateLocus;				// [receptor] control fertility: see notes on definition of OVULATEON/OVULATEOFF constants
	float myReceptiveLocus;				// [receptor] control fertilisation process in female
	float myChanceOfMutationLocus;		// [receptor] probability of mutation during conception
	float myDegreeOfMutationLocus;		// [receptor] size of mutation during conception
};
#endif//ReproductiveFaculty_H
