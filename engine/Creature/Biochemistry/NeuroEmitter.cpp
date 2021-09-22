// NeuroEmitter.cpp: implementation of the NeuroEmitter class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "NeuroEmitter.h"

float NeuroEmitter::myDefaultNeuronInput = 1.0f;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NeuroEmitter::NeuroEmitter() {
//	static float defaultNeuronInput = 1.0f;

	bioTickRate = bioTick = 0.0f;
	for (int i=0; i<noOfNeuronalInputs; i++) {
		myNeuronalInputs[i] = &myDefaultNeuronInput;
	}
	for (int o=0; o<noOfChemicalEmissions; o++) {
		myChemicalEmissions[o].chemicalId = 0;
		myChemicalEmissions[o].amount = 0.0f;
	}
}


bool NeuroEmitter::Write(CreaturesArchive &archive) const {

	archive << bioTickRate;
	archive << bioTick;

	for (int i=0; i<noOfNeuronalInputs; i++) {
		if( myNeuronalInputs[i] == &myDefaultNeuronInput )
			archive.WriteFloatRef( 0 );
		else
			archive.WriteFloatRef( myNeuronalInputs[i] );
	}
	for (int o=0; o<noOfChemicalEmissions; o++) {
		archive << myChemicalEmissions[o].chemicalId;
		archive << myChemicalEmissions[o].amount;
	}

	return true;
}

bool NeuroEmitter::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		archive >> bioTickRate;
		archive >> bioTick;

		for (int i=0; i<noOfNeuronalInputs; i++) {
			archive.ReadFloatRef( myNeuronalInputs[i] );
			if( !myNeuronalInputs[i]  )
				myNeuronalInputs[i] = &myDefaultNeuronInput;

		}
		for (int o=0; o<noOfChemicalEmissions; o++) {
			archive >> myChemicalEmissions[o].chemicalId;
			archive >> myChemicalEmissions[o].amount;
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

