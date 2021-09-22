// Brain.cpp: implementation of the Brain class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif
#include "Brain.h"
#include <algorithm>
#include "../Genome.h"

#ifdef _DEBUG
// TODO: should move timers out of window.h...
//#include "../../display/window.h" // high performance timer
//#include <sstream>
#endif // _DEBUG

CREATURES_IMPLEMENT_SERIAL(Brain)



Lobe Brain::ourDummyLobe;



// Offset into Brain.catalogue to access "Brain Parameters" fields
enum Brain_Parameters
{
	INSTINCT_CHEMICAL_NUMBER = 0,
	PREINSTINCT_CHEMICAL_NUMBER
};



// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Brain
// Description: NULLs a few things and finds out which chemicals are relevant
//				to instincts from the catalogue.
// ------------------------------------------------------------------------
Brain::Brain() 
{
	myPointerToChemicals = NULL;
	myInstinctsAreBeingProcessed = false;
	myLastKnowledgeUpdated = 0;
	instinctChemicalNumber = atoi( theCatalogue.Get( "Brain Parameters", INSTINCT_CHEMICAL_NUMBER ) );
	preInstinctChemicalNumber = atoi( theCatalogue.Get( "Brain Parameters", PREINSTINCT_CHEMICAL_NUMBER ) );
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       Brain
// Description: Deletes all the lobes, tracts and instincts.
// ------------------------------------------------------------------------
Brain::~Brain()
{
	int i;

	for ( i=0; i<myBrainComponents.size(); i++)
	{
		delete myBrainComponents[i];
	}
	for	(i=0; i<myInstincts.size(); i++)
	{
		 delete myInstincts[i];
	}
}



// ------------------------------------------------------------------------
// Function:    ReadFromGenome
// Class:       Brain
// Description: Creates lobes, tracts and instincts as appropriate from the genome.
// Arguments:   Genome &genome = C2e genome flagged as a certain age level.
// ------------------------------------------------------------------------
void Brain::ReadFromGenome(Genome &genome)
{
	ASSERT(myPointerToChemicals!=NULL);

	genome.Reset();
	while (genome.GetGeneType(BRAINGENE,G_LOBE,NUMBRAINSUBTYPES))
	{
		try {
			Lobe* lobe = new Lobe(genome);
			
			lobe->SetIdInList(
				myLobes.size()>0 ? myLobes.back()->GetIdInList()+1 : 0
			);
			myLobes.push_back(lobe);
			myBrainComponents.push_back((BrainComponent*)lobe);

			// set up help knowledge vectors
			if(!strcmp(*lobe,"driv"))
			{
				int noNeurons = lobe->GetNoOfNeurons();
				myAssistanceKnowledge.resize(noNeurons);
				for(int n = 0; n != noNeurons; n++) 
				{
					myAssistanceKnowledge[n].decisionId = -1;
					myAssistanceKnowledge[n].attentionId = -1;
				}
			}

		} catch(GenomeInitFailedException&) {
//			OutputFormattedDebugString("Lobe init failed: %s\n",e.what());
		}
		if (myLobes.size()==MAX_LOBES)
		{
			break;
		}
	}

	genome.Reset();
	while (genome.GetGeneType(BRAINGENE,G_TRACT,NUMBRAINSUBTYPES))
	{
		try {
			Tract* tract = new Tract(genome, myLobes);

			tract->SetIdInList(
				myTracts.size()>0 ? myTracts.back()->GetIdInList()+1 : 0
			);
			myTracts.push_back(tract);
			myBrainComponents.push_back((BrainComponent*)tract);
		} catch(GenomeInitFailedException&) {
			//OutputFormattedDebugString("Tract init failed: %s\n",e.what());
		}
		if (myTracts.size()==MAX_TRACTS)
		{
			break;
		}
	}

	// sort brain components according to updateAtTime field:
	std::sort(myBrainComponents.begin(), myBrainComponents.end(), BrainComponent::xIsProcessedBeforeY);

	// register the biochemistry with the brain and init all the new components:
	for (int i=0; i<myBrainComponents.size(); i++)
	{
		myBrainComponents[i]->RegisterBiochemistry(myPointerToChemicals);
		myBrainComponents[i]->Initialise();
	}




	// Read all INSTINCT genes, to pre-wire instinctive/innate/reflexive brain responses.
	// Instincts get stored in Instinct objects and remain pending until creature is next in REM sleep
	// Late-switching genes get stacked up in array and work during subsequent dream periods
	genome.Reset();
	while ((genome.GetGeneType(CREATUREGENE,G_INSTINCT,NUMCREATURESUBTYPES))!=false)
	{
		if (myInstincts.size()<MAX_INSTINCTS)
		{
 			myInstincts.push_back(new Instinct(genome,this));
		}
	}
}


// ------------------------------------------------------------------------
// Function:    GetNoOfInstinctsLeftToProcess
// Class:       Brain
// Description: How many instincts (loaded in from the genome) are still 
//				cued for processing.
// Returns:     int = number
// ------------------------------------------------------------------------
int Brain::GetNoOfInstinctsLeftToProcess()
{
	return myInstincts.size();
}

// ------------------------------------------------------------------------
// Function:    GetWhetherToProcessInstincts
// Class:       Brain
// Description: Are we currently processing instincts?
// Returns:     bool = true for yes, 
// ------------------------------------------------------------------------
bool Brain::GetWhetherToProcessInstincts()
{
	return myInstinctsAreBeingProcessed;
}







// ------------------------------------------------------------------------
// Function:    InitLobeFromDescription
// Class:       Brain
// Description: Init a lobe in the Vat Kit (this code not used in the game itself).
// Arguments:   std::istream &in = game input stream
// Returns:     bool = true for success
// ------------------------------------------------------------------------
bool Brain::InitLobeFromDescription(std::istream &in)
{
	try {
		Lobe* lobe = new Lobe(in);

		static char endDump[9];
		in.read(endDump, 9);
		if (strncmp(endDump+1, "END DUMP", 8))
			throw GenomeInitFailedException(11, (char*)*lobe);

		myBrainComponents.push_back((BrainComponent*)lobe);
		myLobes.push_back(lobe);

		// set up help knowledge vectors
		if(!strcmp(*lobe,"driv"))
		{
			int noNeurons = lobe->GetNoOfNeurons();
			myAssistanceKnowledge.resize(noNeurons);
			for(int n = 0; n != noNeurons; n++) 
			{
				myAssistanceKnowledge[n].decisionId = -1;
				myAssistanceKnowledge[n].attentionId = -1;
			}
		}


	} catch(GenomeInitFailedException&) {
		//	OutputFormattedDebugString("Lobe init failed: %s\n",e.what());
	}
	if (myLobes.size()==MAX_LOBES) {
	}


	// sort brain components according to updateAtTime field:
	std::sort(myBrainComponents.begin(), myBrainComponents.end(), BrainComponent::xIsProcessedBeforeY);


	return true;
}


// ------------------------------------------------------------------------
// Function:    InitTractFromDescription
// Class:       Brain
// Description: Init the brain in the Vat Kit (this code not used in the game itself).
// Arguments:   std::istream &in = game input stream
// Returns:     bool = true for success
// ------------------------------------------------------------------------
bool Brain::InitTractFromDescription(std::istream &in)
{

	try {
		Tract* tract = new Tract(in, myLobes);

		static char endDump[9];
		in.read(endDump, 9);
		if (strncmp(endDump+1, "END DUMP", 8))
			throw GenomeInitFailedException(12, (const char*)*tract);

		myBrainComponents.push_back((BrainComponent*)tract);
		myTracts.push_back(tract);
	} catch(GenomeInitFailedException&) {
//		OutputFormattedDebugString("Tract init failed: %s\n",e.what());
	}

	if (myTracts.size()==MAX_TRACTS) {
	}

	// sort brain components according to updateAtTime field:
	std::sort(myBrainComponents.begin(), myBrainComponents.end(), BrainComponent::xIsProcessedBeforeY);

	return true;
}







// ------------------------------------------------------------------------
// Function:    RegisterBiochemistry
// Class:       Brain
// Description: Give the brain a pointer to the biochemicals so that it can
//				use them to signal instinct processing and reference their values in SV-Rules
//				as need be.
// Arguments:   float* chemicals = an array of 256 floats (chemical concentrations).
// ------------------------------------------------------------------------
void Brain::RegisterBiochemistry(float* chemicals)
{
	myPointerToChemicals = chemicals;

	// register the biochemistry with brain components if we have any yet:
	for (int i=0; i<myBrainComponents.size(); i++)
	{
		myBrainComponents[i]->RegisterBiochemistry(myPointerToChemicals);
	}

}




// ------------------------------------------------------------------------
// Function:    SetWhetherToProcessInstincts
// Class:       Brain
// Description: When flagged the brain processes instincts every tick instead
//				of its normal activity.  When processing instincts two biochemicals
//				are set to indicate to the brain's SV-Rules that all learning should
//				be long-term, not short-term as normal.
// Arguments:   bool b = true for instincts, false for normal processing
// ------------------------------------------------------------------------
void Brain::SetWhetherToProcessInstincts(bool b)
{
	myInstinctsAreBeingProcessed = b;
	if( b )
	{
		// Prepare to process instincts.
		// Firstly warn the brain by using a chemical signal, then update the brain once.
		// This gives dendrites time to do any pre-instinct processing required.
		myPointerToChemicals[preInstinctChemicalNumber] = 1.0;
		myPointerToChemicals[instinctChemicalNumber] = 0.0;
		UpdateComponents();

		// Now set up the brain ready to process instincts on subsequent updates
		myPointerToChemicals[preInstinctChemicalNumber] = 0.0;
		myPointerToChemicals[instinctChemicalNumber] = 1.0;
	}
	else
	{
		myPointerToChemicals[preInstinctChemicalNumber] = 0.0;
		myPointerToChemicals[instinctChemicalNumber] = 0.0;
	}
}



// ------------------------------------------------------------------------
// Function:    ClearActivity
// Class:       Brain
// Description: Zero all neuron activity in the brain (usually so we can proceed
// to processing instincts).
// ------------------------------------------------------------------------
void Brain::ClearActivity()
{
	for (int i=0; i<myLobes.size(); i++)
	{
		myLobes[i]->ClearActivity();
	}

	// If the tracts are cleared then STWs will be set to LTW, i.e. the creature
	// will forget all recently learned weightings stored in STWs.
	// If the tracts are not cleared then, when an instinct is processed all LTWs
	// in the brain are likely to be set to the STW values, i.e. all previous 
	//for (i=0; i<myTracts.size(); i++) {
	//	myTracts[i]->ClearActivity();
	//}
}



// ------------------------------------------------------------------------
// Function:    UpdateComponents
// Class:       Brain
// Description: Update all the lobes and tracts in order.
// ------------------------------------------------------------------------
void Brain::UpdateComponents() 
{
#ifdef _DEBUG
//	int64 stamp = GetHighPerformanceTimeStamp();
#endif
	for (int i=0; i<myBrainComponents.size(); i++)
	{
		myBrainComponents[i]->DoUpdate();
	}
#ifdef _DEBUG
//	int64 stampDelta = GetHighPerformanceTimeStamp() - stamp;
//	std::ostringstream time;
//	time << "SVRules time: " << (int)stampDelta << std::endl;
//	OutputDebugString(time.str().c_str());
#endif
}


// ------------------------------------------------------------------------
// Function:    Update
// Class:       Brain
// Description: Updates all the lobes and tracts in order or, if you are processing
//				instincts, do the next one and find concepts from the brain so that our norn
//				can teach other norns concepts (the linguistic faculty reads this).
// ------------------------------------------------------------------------
void Brain::Update() 
{
	if (!myInstinctsAreBeingProcessed) 
	{
		UpdateComponents();
		return;
	}


	// process instincts
	if (myInstincts.size()!=0) 
	{	
		// process instincts
		//OutputFormattedDebugString("Processing instinct %d.\n", myInstincts.size()-1);
		Instinct *i = myInstincts.back();
		if (i->Process() == true)
		{
			myInstincts.pop_back();
			delete i;					// remove it from memory

/*			if (myInstincts.size() == 0) {
				// No more instincts - update the brain as per usual in the next update
				// NOTE: This is needed if the dendrite SVRules are designed such that
				// they need two updates for (1) STWeights of dendrites to become weighted
				// and then (2) LTW to tend towards the STWeights (resulting in learned instincts)
				myInstinctsAreBeingProcessed = false;
			}*/
			return;						// only process one at a time
		}
	}
	

	// next process help knowledge at same time
	int noNeurons = GetLobeFromTokenString("driv")->GetNoOfNeurons();
	
	if(myLastKnowledgeUpdated>=0 && myLastKnowledgeUpdated<noNeurons)
	{
		ClearActivity();

		int noNounNeurons = GetLobeFromTokenString("noun")->GetNoOfNeurons();
		for(int s = 0; s != noNounNeurons; s++)
			SetInput("noun", s, 0.5f);

		int noVisionNeurons = GetLobeFromTokenString("visn")->GetNoOfNeurons();
		for(int v = 0; v != noVisionNeurons; v++)
			SetInput("visn", v, 0.1f);	

		SetInput("driv", myLastKnowledgeUpdated, 1.0f);
		UpdateComponents();
		myAssistanceKnowledge[myLastKnowledgeUpdated].attentionId = GetWinningId("attn");
		myAssistanceKnowledge[myLastKnowledgeUpdated].decisionId = GetWinningId("decn");
		myAssistanceKnowledge[myLastKnowledgeUpdated].strength = GetLobeFromTokenString("decn")->
			GetNeuronState(myAssistanceKnowledge[myLastKnowledgeUpdated].decisionId, STATE_VAR);
		myLastKnowledgeUpdated++;

		if(myLastKnowledgeUpdated == noNeurons)
		{
			myInstinctsAreBeingProcessed = false;
			myLastKnowledgeUpdated = 0;	// reset for next time instincts are updated
		}

		ClearActivity();
		return; //only process one at a time
	}
}


// ------------------------------------------------------------------------
// Function:    GetLobeNameFromTissueId
// Class:       Brain
// Description: Returns a lobe moniker (a four character string) from the tissue
//				of that lobe.  (Mapping is defined in the genome).
// Arguments:   int tissueId = 0-254
// Returns:     char* = "attn" for example
// ------------------------------------------------------------------------
char* Brain::GetLobeNameFromTissueId(int tissueId)
{
	Lobe* l = GetLobeFromTissueId(tissueId);
	return l==NULL ? NULL : (char*)(*l);
}

// ------------------------------------------------------------------------
// Function:    GetLobeFromTissueId
// Class:       Brain
// Description: Internal brain function to get the pointer to a lobe with a particular
//				lobe tissue ID.  Used by the biochemical functions to attach receptors
//				and emitters to neurons.
// Arguments:   int tissueId = 0-254
// Returns:     Lobe* = "stim" for example
// ------------------------------------------------------------------------
Lobe* Brain::GetLobeFromTissueId(int tissueId)
{
	for (Lobes::iterator l=myLobes.begin(); l!=myLobes.end(); l++)
	{
		if ((*l)->GetTissueId() == tissueId)
		{
			return *l;
		}
	}
	return NULL;						// didn't find this lobe
}

// ------------------------------------------------------------------------
// Function:    GetLocusAddress
// Class:       Brain
// Description: Respond to an enquiry by a biochemical Receptor/Emitter for a locus address.
// Arguments:   int type = receptor or emitter
//              int organ = ORGAN_BRAIN for us
//              int tissue = id of the lobe to access
//              int locus = neuron id
// Returns:     float* = pointer to locus (or NULL if there is no locus for this ID)
// ------------------------------------------------------------------------
float* Brain::GetLocusAddress(int type, int organ, int tissue, int locus) {
	if (organ!=ORGAN_BRAIN)
		return NULL;

	Lobe* lobe = NULL;
	for (Lobes::iterator l=myLobes.begin(); l!=myLobes.end(); l++) {
		if ((*l)->GetTissueId()==tissue)
			lobe = *l;
	}
	if (lobe==NULL) return NULL;
	return lobe->GetNeuronStatePointer(
		locus/noOfVariablesAvailableAsLoci,
		locus%noOfVariablesAvailableAsLoci
	);
}


// ------------------------------------------------------------------------
// Function:    SetInput
// Class:       Brain
// Description: The most used input function to the brain, this sets a neuron input
//				in a lobe to a particular value.
// Arguments:   const char* lobeTokenString = lobe's moniker, e.g. "verb".
//              int whichNeuron = ID of neuron in that lobe, e.g. 3.
//              float toWhat = what value (from 0.0 to 1.0) to set to.
// ------------------------------------------------------------------------
void Brain::SetInput(const char* lobeTokenString, int whichNeuron, float toWhat)
{
	GetLobeFromTokenString(lobeTokenString)->SetNeuronInput(whichNeuron, toWhat);
}

// ------------------------------------------------------------------------
// Function:    SetLobeWideInput
// Class:       Brain
// Description: Just like SetInput only it sets the input of all neurons in a lobe
//				instead of one particular one.
// Arguments:   const char* lobeTokenString = 
//              float toWhat = 
// ------------------------------------------------------------------------
void Brain::SetLobeWideInput(const char* lobeTokenString, float toWhat)
{
	GetLobeFromTokenString(lobeTokenString)->SetLobeWideInput(toWhat);
}


// ------------------------------------------------------------------------
// Function:    GetWinningId
// Class:       Brain
// Description: The output function of the brain.  Returns which neuron was flagged as
//				the winner for the specified lobe, usually either "attn" (attention) or
//				"decn" (decision).
// Arguments:   const char* lobeTokenString = "attn" or "decn"
// Returns:     int = winning neuron ID
// ------------------------------------------------------------------------
int Brain::GetWinningId(const char* lobeTokenString) {
	return GetLobeFromTokenString(lobeTokenString)->GetWhichNeuronWon();
}






// ------------------------------------------------------------------------
// Function:    GetNeuronState
// Class:       Brain
// Description: Used to access neuron states directly.
// Arguments:   const char *lobeTokenString = 
//              const int neuron = 
//              const int state = 
// Returns:     float = 
// ------------------------------------------------------------------------
float Brain::GetNeuronState(const char *lobeTokenString, const int neuron, const int state)
{
	return GetLobeFromTokenString(lobeTokenString)->GetNeuronState(neuron, state);
}


// ------------------------------------------------------------------------
// Function:    SetNeuronState
// Class:       Brain
// Description: Used to set neuron states directly.
// Arguments:   const char* lobeTokenString = 
//              const int neuron = 
//              const int var = 
//              const float value  = 
// ------------------------------------------------------------------------
void Brain::SetNeuronState(const char* lobeTokenString, const int neuron, const int var, const float value )
{
	GetLobeFromTokenString(lobeTokenString)->SetNeuronState(neuron, var, value);
}


// ------------------------------------------------------------------------
// Function:    GetLobeFromTokenString
// Class:       Brain
// Description: 
// Arguments:   const char *lobeTokenString = 
// Returns:     Lobe* = 
// ------------------------------------------------------------------------
Lobe* Brain::GetLobeFromTokenString(const char *lobeTokenString)
{
	for (Lobes::iterator l=myLobes.begin(); l!=myLobes.end(); l++)
	{
		if ((*l)->GetToken() == Tokenize(lobeTokenString))
		{
			return *l;
		}
	}
	return &ourDummyLobe;						// didn't find this lobe
}

// ------------------------------------------------------------------------
// Function:    GetLobeSize
// Class:       Brain
// Description: Returns the number of neurons in the specified lobe (given by moniker).
// Arguments:   const char* lobeTokenString = 
// Returns:     int = 
// ------------------------------------------------------------------------
int Brain::GetLobeSize(const char* lobeTokenString)
{
	return GetLobeFromTokenString(lobeTokenString)->GetNoOfNeurons();
}






// ------------------------------------------------------------------------
// Function:    SetNeuronState
// Class:       Brain
// Description: Set function for the Vat Kit via CAOS:
// Arguments:   const int lobe = 
//              const int neuron = 
//              const int state = 
//              const float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::SetNeuronState(const int lobe, const int neuron, const int state, const float value)
{
	if(lobe < 0 || lobe > myLobes.size()-1)
		return false;

	return myLobes[lobe]->SetNeuronState(neuron, state, value);
}


// ------------------------------------------------------------------------
// Function:    SetDendriteWeight
// Class:       Brain
// Description: Set function for the Vat Kit via CAOS:
// Arguments:   const int tract = 
//              const int dendrite = 
//              const int weight = 
//              const float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::SetDendriteWeight(const int tract, const int dendrite, const int weight, const float value)
{
	if(tract < 0 || tract > myTracts.size()-1)
		return false;

	return myTracts[tract]->SetDendriteWeight(dendrite, weight, value);
}


// ------------------------------------------------------------------------
// Function:    SetLobeSVFloat
// Class:       Brain
// Description: Set function for the Vat Kit via CAOS:
// Arguments:   const int lobe = 
//              const int entryNo = 
//              const float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::SetLobeSVFloat(const int lobe, const int entryNo, const float value)
{
	if(lobe < 0 || lobe > myLobes.size()-1)
		return false;

	return myLobes[lobe]->SetSVFloat(entryNo, value);

}


// ------------------------------------------------------------------------
// Function:    SetTractSVFloat
// Class:       Brain
// Description: Set function for the Vat Kit via CAOS:
// Arguments:   const int tract = 
//              const int entryNo = 
//              const float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::SetTractSVFloat(const int tract, const int entryNo, const float value)
{
	if(tract < 0 || tract > myTracts.size()-1)
		return false;

	return myTracts[tract]->SetSVFloat(entryNo, value);
}










// ------------------------------------------------------------------------
// Function:    GetKnowledge
// Class:       Brain
// Description: Used to get the best known concept for a particular drive.
//				Called by Linguistic faculty for norns teaching each other concepts.
// Arguments:   int drive = 
// Returns:     Brain::KnowledgeAction
// ------------------------------------------------------------------------
Brain::KnowledgeAction Brain::GetKnowledge(int drive)
{
	if(drive < 0 || drive > myAssistanceKnowledge.size()-1)
	{
		KnowledgeAction defaultKA = {-1,-1,0.0f};
		return defaultKA;
	}
	return myAssistanceKnowledge[drive];
}













// ------------------------------------------------------------------------
// Function:    Write
// Class:       Brain
// Description: Serialising out the brain.
// Arguments:   CreaturesArchive &archive = archive to write to.
// Returns:     bool = success if true
// ------------------------------------------------------------------------
bool Brain::Write(CreaturesArchive &archive) const
{
	base::Write( archive );
	archive << myLobes << myTracts;
	archive << myBrainComponents;

	archive << myInstinctsAreBeingProcessed;
	archive << myInstincts;

	archive << myLastKnowledgeUpdated;
	archive << (int)myAssistanceKnowledge.size();
	for(int i = 0; i != myAssistanceKnowledge.size(); i++)
	{
		archive << myAssistanceKnowledge[i].attentionId;
		archive << myAssistanceKnowledge[i].decisionId;
		archive << myAssistanceKnowledge[i].strength;
	}
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       Brain
// Description: Reading in from a serialised brain.
// Arguments:   CreaturesArchive &archive = archive to read from.
// Returns:     bool = success if true
// ------------------------------------------------------------------------
bool Brain::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!base::Read( archive ))
			return false;

		archive >> myLobes >> myTracts;
		archive >> myBrainComponents;

		archive >> myInstinctsAreBeingProcessed;
		archive >> myInstincts;
	
		archive >> myLastKnowledgeUpdated;
		int size;
		archive >> size;
		myAssistanceKnowledge.resize(size);
		for(int i = 0; i != myAssistanceKnowledge.size(); i++)
		{
			archive >> myAssistanceKnowledge[i].attentionId;
			archive >> myAssistanceKnowledge[i].decisionId;
			archive >> myAssistanceKnowledge[i].strength;
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}










// ------------------------------------------------------------------------
// Function:    DumpSpec
// Class:       Brain
// Description: Used in the game to dump out binary details of the brain to
//				the Vat Kit.  This function is called through a CAOS command.
// Arguments:   std::ostream& out = 
// ------------------------------------------------------------------------
void Brain::DumpSpec(std::ostream& out)
{
	out << myLobes.size() << (char)0;
	out << myTracts.size() << (char)0;

	for(int l = 0; l != myLobes.size(); l++)
		out << myLobes[l]->DumpSize() << (char)0;

	for(int t = 0; t != myTracts.size(); t++)
		out << myTracts[t]->DumpSize() << (char)0;

	out << "END DUMP";
};


// ------------------------------------------------------------------------
// Function:    DumpLobe
// Class:       Brain
// Description: Used in the game to dump out binary details of lobes to
//				the Vat Kit.  This function is called through a CAOS command.
// Arguments:   int l = 
//              std::ostream& out = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::DumpLobe(int l, std::ostream& out)
{
	if(l < 0 || l > myLobes.size()-1)
		return false;

	myLobes[l]->DumpLobe(out);
	out << (char)0 << "END DUMP";
	return true;
};

// ------------------------------------------------------------------------
// Function:    DumpTract
// Class:       Brain
// Description: Used in the game to dump out binary details of tracts to
//				the Vat Kit.  This function is called through a CAOS command.
// Arguments:   int t = 
//              std::ostream& out = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::DumpTract(int t, std::ostream& out)
{
	if(t < 0 || t > myTracts.size()-1)
		return false;

	myTracts[t]->DumpTract(out);
	out << (char)0 << "END DUMP";
	return true;
};

// ------------------------------------------------------------------------
// Function:    DumpNeuron
// Class:       Brain
// Description: Used in the game to dump out binary details of neurons to
//				the Vat Kit.  This function is called through a CAOS command.
// Arguments:   int l = 
//              int n = 
//              std::ostream& out = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::DumpNeuron(int l, int n, std::ostream& out)
{
	if(l < 0 || l > myLobes.size()-1)
		return false;

	if(!myLobes[l]->DumpNeuron(n, out))
		return false;

	out << (char)0 << "END DUMP";
	return true;
};

// ------------------------------------------------------------------------
// Function:    DumpDendrite
// Class:       Brain
// Description: Used in the game to dump out binary details of dendrites to
//				the Vat Kit.  This function is called through a CAOS command.
// Arguments:   int t = 
//              int d = 
//              std::ostream& out = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Brain::DumpDendrite(int t, int d, std::ostream& out)
{
	if(t < 0 || t > myLobes.size()-1)
		return false;
	if(!myTracts[t]->DumpDendrite(d, out))
		return false;

	out << (char)0 << "END DUMP";
	return true;
};
