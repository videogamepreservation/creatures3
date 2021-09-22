#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Instinct.h"
#include "Brain.h"
#include "BrainScriptFunctions.h"
#include "../Genome.h"

CREATURES_IMPLEMENT_SERIAL(Instinct)


const float REINFORCEMENT_MODIFIER = 0.5f;


// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Instinct
// Description: 
// ------------------------------------------------------------------------
Instinct::Instinct() {}

//////////////////////////// Instinct members ///////////////////////////
// construct by reading data from a gene
// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Instinct
// Description: 
// Arguments:   Genome& g = 
//              Brain* brain = 
// ------------------------------------------------------------------------
Instinct::Instinct(Genome& g, Brain* brain)
{
	myBrain = brain;

	for	(int i=0; i<MAX_INSTINCT_INPUTS; i++)
	{
		int tissueIdOfLobe = g.GetByte()-1;			// 255-1 means invalid
		myInputs[i].neuronId = g.GetByte();
		char* name = brain->GetLobeNameFromTissueId(tissueIdOfLobe);
		myInputs[i].name = name==NULL ? "****" : name;

		if (myInputs[i].name == std::string("decn"))
			myInputs[i].name = "verb";
		if (myInputs[i].name == std::string("attn"))
			myInputs[i].name = "noun";
	}
	int decisionScriptId = g.GetByte();
	myDecisionId = GetNeuronIdFromScriptOffset(decisionScriptId);// required action

	myReinforcement.driveId = g.GetCodon(0,255);	// reinforcer drive
	myReinforcement.amount = g.GetSignedFloat();	// how much reinforcement

	myInstinctTick = 0;
}


// process another phase of this instinct, return true when it's finished:
// ------------------------------------------------------------------------
// Function:    Process
// Class:       Instinct
// Description: 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Instinct::Process()
{
	myBrain->ClearActivity();

	// Init drives and brain inputs:
	for	(int i=0; i<MAX_INSTINCT_INPUTS; i++)
	{
		std::string lobeName = myInputs[i].name;
		int neuronId = myInputs[i].neuronId;

		// remap verb inputs as follows: 
		if (lobeName == std::string("verb"))
			neuronId = GetNeuronIdFromScriptOffset(neuronId);

		if (lobeName == std::string("noun"))
		{
			myBrain->SetInput("visn", myInputs[i].neuronId, 0.1f);	// make sure you can see this object
			myBrain->SetInput("smel", myInputs[i].neuronId, 1.0f);	// and smell it too
		}

		myBrain->SetInput(lobeName.c_str(), myInputs[i].neuronId, 1.0f);
	}


	// try get the creature to do what the instinct suggests:
	myBrain->SetInput("verb", myDecisionId, 1.0f);
	myBrain->UpdateComponents();

	// if we didn't manage to force the creature to do that assume this instinct
	// isn't valid for this brain and don't process it:
	if (myBrain->GetWinningId("decn")!=myDecisionId)
		return true;

	// otherwise send in the reward:
	myBrain->SetInput("resp", myReinforcement.driveId, REINFORCEMENT_MODIFIER*myReinforcement.amount);
	myBrain->UpdateComponents();

	return true;
}


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Write
// Class:       Instinct
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Instinct::Write(CreaturesArchive &archive) const
{

	archive << myBrain;
	for (int i=0; i<MAX_INSTINCT_INPUTS; i++)
	{
		archive << myInputs[i].neuronId;
		archive << myInputs[i].name;
	}
	archive << myDecisionId;			// 'decision' to be taken
	archive << myReinforcement.driveId;
	archive << myReinforcement.amount;
	archive << myInstinctTick;			// where in the instinct-processing we've got to so far
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Read
// Class:       Instinct
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Instinct::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

	//	DWORD version;				// Check version info
	//	archive >> version;
		archive >> myBrain;
		for (int i=0; i<MAX_INSTINCT_INPUTS; i++)
		{
			archive >> myInputs[i].neuronId;
			archive >> myInputs[i].name;
		}
		archive >> myDecisionId;			// 'decision' to be taken
		archive >> myReinforcement.driveId;
		archive >> myReinforcement.amount;
		archive >> myInstinctTick;			// where in the instinct-processing we've got to so far
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}
