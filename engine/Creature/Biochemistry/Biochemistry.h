/*********************************************************************
* File:     Biochemistry.h
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifndef _BIOCHEMISTRY
#define _BIOCHEMISTRY

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <vector>
//#include "Chemical.h"
#include "BiochemistryConstants.h"
#include "NeuroEmitter.h"
#include "../Faculty.h"

class Creature;
class Organ;
class Genome;

/*********************************************************************
* Biochemistry.
* Each Creature must have a Biochemistry object, containing all the  
* biochemicals, and organs.
*********************************************************************/
class Biochemistry : public Faculty {
	CREATURES_DECLARE_SERIAL(Biochemistry)
public:
    float myATPRequirement;

	Biochemistry();							// serialisation constr
	virtual ~Biochemistry();

	virtual void ReadFromGenome(Genome& genome);	// Define whole biochemistry from DNA
	virtual void Update();				// call this every tick to update chemicals,
								// receptors & emitters

	float GetChemical(int chem);				// return current concentration of a chemical
	void SetChemical(int chem, float amount);	// set abs concentration of a chemical
	void AddChemical(int chem,float amount);	// add to the concentration of given chemical
	void SubChemical(int chem,float amount);	// reduce the concentration of given chemical UNLESS chem=0 ("none")


	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	inline int NeuroEmitterCount() const {				return myNoOfNeuroEmitters;}
	inline float* GetChemicalConcs() {
		return &(myChemicalConcs[0]);
	}

	virtual float* GetCreatureLocusAddress(int type, int tissue, int organ, int locus);
	float* GetInvalidCreatureLocusAddress();

	int GetOrganCount() const { return myNoOfOrgans; }
	Organ* GetOrgan(int organNumber) const;

private:
	float myChemicalDecayRates[NUMCHEM];		
	float myChemicalConcs[NUMCHEM];				// array of chemical concentrations

	NeuroEmitter myNeuroEmitters[MAX_NEUROEMITTERS];
	int myNoOfNeuroEmitters;

	Organ* myOrgans[MAXORGANS];
	int myNoOfOrgans;
};
#endif
