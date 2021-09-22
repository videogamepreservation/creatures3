// MusicFaculty.cpp: implementation of the MusicFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicFaculty.h"
#include "Creature.h"
#include "../App.h"
#include "../World.h"
#include "LifeFaculty.h"
#include "../Map/Map.h" // For Map::FastFloatToInteger
																		    
// Weights for drive calculations.
const int YYY   = 4;
const int YY    = 2;
const int Y     = 1;
const int N     = -1;
const int NN    = -2;
const int NNN   = -4;

CREATURES_IMPLEMENT_SERIAL(MusicFaculty)

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       MusicFaculty
// Description: 
// ------------------------------------------------------------------------
MusicFaculty::MusicFaculty()
{
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       MusicFaculty
// Description: 
// ------------------------------------------------------------------------
MusicFaculty::~MusicFaculty()
{
}

// ------------------------------------------------------------------------
// Function:    Update
// Class:       MusicFaculty
// Description: 
// ------------------------------------------------------------------------
void MusicFaculty::Update()
{
}

// ------------------------------------------------------------------------
// Function:    SelectableByUser
// Class:       MusicFaculty
// Description: 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool MusicFaculty::SelectableByUser() const
{
	// Game variable grettin used here - yeuch, but can't think of 
	// anything better right now...
	CAOSVar& grettin = theApp.GetWorld().GetGameVar("Grettin");
	if (grettin.GetType() == CAOSVar::typeInteger)
	{
		// If "Grettin" is turned on, everyone is selectable
		if (grettin.GetInteger() == 1)
			return true;
	}

	// Otherwise only Norns are selectable
    return (myCreature.GetCreatureReference().GetClassifier().Genus() == G_NORN);
}

// ------------------------------------------------------------------------
// Function:    Hatching
// Class:       MusicFaculty
// Description: 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool MusicFaculty::Hatching() const
{
    return (myCreature.GetCreatureReference().Life()->GetWhetherDreaming() && myCreature.GetCreatureReference().Life()->GetAge() == AGE_BABY);
}

// ------------------------------------------------------------------------
// Function:    Mood
// Class:       MusicFaculty
// Description: 
// Returns:     float = 
// ------------------------------------------------------------------------
float MusicFaculty::Mood() const
{
	Creature& c = myCreature.GetCreatureReference();

    static int InfluenceOnMood[14] =
	{
        NNN,    //   0 PAIN,
        N,      //   1 HUNGER FOR PROTEIN
        N,      //   2 HUNGER FOR CARB
        N,      //   3 HUNGER FOR FAT
        N,      //   4 COLDNESS, 
        N,      //   5 HOTNESS,
        N,      //   6 TIREDNESS,   
        0,      //   7 SLEEPINESS,
        N,      //   8 LONELINESS,
        N,      //   9 CROWDEDNESS,
        NN,     //  10 FEAR,
        0,      //  11 BOREDOM,
        N,      //  12 ANGER,	
        YYY,    //  13 SEXDRIVE
	};

    if (c.Life()->GetWhetherDead())
        return 0.0;

    int iMood = 0;
	float minMood =0;
	float maxMood = 0;

    for (int i = 0; i < 14; i++)
    {
        int iK = InfluenceOnMood[i];
        iMood += (int)(myCreature.GetCreatureReference().GetDriveLevel(i)*256.0f * -iK);

		
		if (iK > 0)
		{
			minMood += 255 * -iK;
		}
		else
		{
			maxMood += 255 * -iK;
		}
    }

    ASSERT(maxMood > minMood);
    ASSERT(maxMood >= iMood && iMood >= minMood);

    float iRange = maxMood - minMood;
    float iMood2 = iMood - minMood;
    ASSERT(iRange > 0);
    ASSERT(iMood2 >= 0);

    float fResult = (float)1.0 - (((float)iMood2) / ((float)iRange));

    ASSERT(fResult >= 0.0 && fResult <= 1.0);
    return fResult;
}

// ------------------------------------------------------------------------
// Function:    Threat
// Class:       MusicFaculty
// Description: 
// Returns:     float = 
// ------------------------------------------------------------------------
float MusicFaculty::Threat()
{
	return myCreature.GetCreatureReference().GetDriveLevel(FEAR);
}


// ------------------------------------------------------------------------
// Function:    Write
// Class:       MusicFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool MusicFaculty::Write(CreaturesArchive &archive) const
{
	base::Write( archive );
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       MusicFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool MusicFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		if(!base::Read( archive ))
			return false;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}
