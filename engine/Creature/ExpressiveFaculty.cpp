// ExpressiveFaculty.cpp: implementation of the ExpressiveFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "ExpressiveFaculty.h"
#include "Genome.h"
#include "Creature.h"
#include "LifeFaculty.h"


const int SCRIPT_ILL_FACE = 199;


CREATURES_IMPLEMENT_SERIAL(ExpressiveFaculty)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       ExpressiveFaculty
// Description: 
// ------------------------------------------------------------------------
ExpressiveFaculty::ExpressiveFaculty()
{
	for (int e=0; e<EXPR_COUNT; e++) {
		for (int d=0; d<NUMDRIVES; d++) {
			myDriveWeightingsForExpressions[e][d] = 0.0f;
		}
		myExpressionWeightings[e] = 0.0f;
	}
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       ExpressiveFaculty
// Description: 
// ------------------------------------------------------------------------
ExpressiveFaculty::~ExpressiveFaculty()
{

}


// ------------------------------------------------------------------------
// Function:    ReadFromGenome
// Class:       ExpressiveFaculty
// Description: 
// Arguments:   Genome& g = 
// ------------------------------------------------------------------------
void ExpressiveFaculty::ReadFromGenome(Genome& g) {

	// facial expression genes:
	g.Reset();
	while ((g.GetGeneType(CREATUREGENE,G_EXPRESSION,NUMCREATURESUBTYPES))!=false) {
		int expressionId = g.GetCodonLessThan(EXPR_COUNT);
		g.GetByte();						// spare
		myExpressionWeightings[expressionId] = g.GetFloat();

		// reset drive expression weightings:
		for (int d=0; d<NUMDRIVES; d++)
			myDriveWeightingsForExpressions[expressionId][d] = 0.0f;

		// get drive weightings for this expression:
		for (int i=0; i<4; i++) {
			int driveId = g.GetByte();
			if (driveId<0 || driveId>=NUMDRIVES)
				continue;

			float driveWeight = g.GetSignedFloat();
			myDriveWeightingsForExpressions[expressionId][driveId]
				= driveWeight;
		}
	}
}


const int BLINKRATE = 32;			// probability (rnd(n) that a creature
									// will blink this frame

// ------------------------------------------------------------------------
// Function:    Update
// Class:       ExpressiveFaculty
// Description: 
// ------------------------------------------------------------------------
void ExpressiveFaculty::Update() {
	// Blinking, smiling etc:
    int iBlinkRate = (BLINKRATE + 1) - 
		Map::FastFloatToInteger((myCreature.GetCreatureReference().GetDriveLevel(SLEEPINESS) / 8.0f));

    // Set eyes closed if sleeping or blinking or dead, else eyes are open:
	int newEyeState = 
		!(myCreature.GetCreatureReference().Life()->GetWhetherAlert() || myCreature.GetCreatureReference().Life()->GetWhetherZombie()) || 
		!Rnd(iBlinkRate) ? 0 : 1;
    ASSERT(newEyeState==0 || newEyeState==1);
	myCreature.GetCreatureReference().SetEyeState(newEyeState);


	// Change expressions (was updated only slowly):
	int newExpressionId = CalculateExpressionFromDrives();
    if (newExpressionId != myCreature.GetCreatureReference().GetFacialExpression()) {

		// change expressions via Normal:
		newExpressionId = 
			myCreature.GetCreatureReference().GetFacialExpression()!=EXPR_NORMAL ?
            EXPR_NORMAL : newExpressionId;

        myCreature.GetCreatureReference().SetFacialExpression(newExpressionId);
/*		if (newExpressionId==EXPR_ILL)
			myCreature.GetCreatureReference().ExecuteScriptForEvent(SCRIPT_ILL_FACE, myCreature,
			INTEGERZERO, INTEGERZERO);*/
    }
}


// ------------------------------------------------------------------------
// Function:    CalculateExpressionFromDrives
// Class:       ExpressiveFaculty
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int ExpressiveFaculty::CalculateExpressionFromDrives() 
{
	int bestExpressionIdSoFar = EXPR_NORMAL;
	float bestRecommendationSoFar = -999.0f;

	for (int e=EXPR_NORMAL; e<EXPR_COUNT; e++) 
	{
		float recommendationForThisExpression = 0.0f;
		for (int d=0; d<NUMDRIVES; d++) 
		{
			recommendationForThisExpression +=
				myDriveWeightingsForExpressions[e][d] *
				(myCreature.GetCreatureReference().GetDriveLevel(d)-0.5f);
		}
		recommendationForThisExpression *= myExpressionWeightings[e];

		if (recommendationForThisExpression>bestRecommendationSoFar) 
		{
			bestRecommendationSoFar = recommendationForThisExpression;
			bestExpressionIdSoFar = e;
		}
	}
    return bestExpressionIdSoFar;
}



// ------------------------------------------------------------------------
// Function:    Write
// Class:       ExpressiveFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool ExpressiveFaculty::Write(CreaturesArchive &archive) const {
	base::Write( archive );
	for (int i=0; i<EXPR_COUNT; i++) {
		archive << myExpressionWeightings[i];
		for (int o=0; o<NUMDRIVES; o++) {
		    archive << myDriveWeightingsForExpressions[i][o];
		}
	}
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       ExpressiveFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool ExpressiveFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		if(!base::Read( archive ))
			return false;

		for (int i=0; i<EXPR_COUNT; i++) {
			archive >> myExpressionWeightings[i];
			for (int o=0; o<NUMDRIVES; o++) {
				archive >> myDriveWeightingsForExpressions[i][o];
			}
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}
