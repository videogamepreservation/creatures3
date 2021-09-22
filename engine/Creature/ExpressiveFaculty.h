// ExpressiveFaculty.h: interface for the ExpressiveFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ExpressiveFaculty_H
#define ExpressiveFaculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SkeletonConstants.h"
#include "CreatureConstants.h"
#include "Faculty.h"

class Genome;
class Creature;

class ExpressiveFaculty : public Faculty {
	CREATURES_DECLARE_SERIAL(ExpressiveFaculty)
public:
	typedef Faculty base;
	ExpressiveFaculty();
	virtual ~ExpressiveFaculty();

	void Update();
	void ReadFromGenome(Genome& g);

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);
protected:
    int CalculateExpressionFromDrives();
    float myDriveWeightingsForExpressions[EXPR_COUNT][NUMDRIVES];
	float myExpressionWeightings[EXPR_COUNT];
};

#endif//ExpressiveFaculty_H
