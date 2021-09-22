// -------------------------------------------------------------------
//
// Lobe.cpp: implementation of the Lobe class.
//
// --------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Lobe.h"
#include "BrainIO.h"
#include "../Genome.h"



CREATURES_IMPLEMENT_SERIAL(Lobe)


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Lobe
// Description: 
// ------------------------------------------------------------------------
Lobe::Lobe()
{
	myWinningNeuronId = 0;
	myNeuronInput = NULL;
}

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Lobe
// Description: 
// Arguments:   Genome &genome = 
// ------------------------------------------------------------------------
Lobe::Lobe(Genome &genome)
{
	myWinningNeuronId = 0;
	myNeuronInput = NULL;
	// GENOME READING:

	// update and ID:
	myToken = genome.GetToken();
	myUpdateAtTime = genome.GetInt();

	// info for vat tool and size of lobe:
	myX = genome.GetInt();
	myY = genome.GetInt();
	myWidth = genome.GetByte();
	myHeight= genome.GetByte();

	// for vat tool:
	myColour[0] = genome.GetByte();
	myColour[1] = genome.GetByte();
	myColour[2] = genome.GetByte();

	// winner takes all?
	/*myNeuronsAreWinnerTakesAll = */genome.GetBool();

	// for biochemistry:
	myTissueId = genome.GetByte();

	myRunInitRuleAlwaysFlag = genome.GetCodon(0,1) > 0 ? true : false;

	// dummy slots:
	genome.GetByte();
	genome.GetInt();
	genome.GetToken();

	// init SV-Rules:
	myInitRule.InitFromGenome(genome);
	myUpdateRule.InitFromGenome(genome);

	strcpy(myName, Ezinekot(myToken));

	// error if too small or too big:
	int noOfNeurons = myWidth * myHeight;
	if (noOfNeurons==0)
		throw(GenomeInitFailedException(4, (char*)*this));
	if (noOfNeurons>MAX_NEURONS_PER_LOBE)
		throw(GenomeInitFailedException(5, (char*)*this));

	// init neurons:
	myNeuronInput = new float[noOfNeurons];
	for (int i=0; i<noOfNeurons; i++)
	{
		myNeuronInput[i] = 0.0f;

		Neuron* n = new Neuron();
		n->idInList = i;
		myNeurons.push_back(n);
	}

	// init spare neuron variables to first neuron (default)
	mySpareNeuronVariables = &(myNeurons[0]->states);
}


// ------------------------------------------------------------------------
// Function:    Initialise
// Class:       Lobe
// Description: 
// ------------------------------------------------------------------------
void Lobe::Initialise()
{
	if(myInitialised)
		return;

	myInitialised = true;

	for (int i=0; i<myNeurons.size(); i++)
	{
		Neuron* n = myNeurons[i];
		_ASSERT(n->idInList == i);

		if( myRunInitRuleAlwaysFlag )
			n->ClearStates();
		else
		{
			myInitRule.ProcessGivenVariables(
				SVRule::invalidVariables, SVRule::invalidVariables, n->states,
				SVRule::invalidVariables, i,i
			);
		}
	}
}


// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Lobe
// Description: 
// Arguments:   std::istream &in = 
// ------------------------------------------------------------------------
Lobe::Lobe(std::istream &in) {
	// DESCRIPTION READING:

	// update and ID:
	ReadDesc(&myIdInList, in);
	ReadDesc(&myWinningNeuronId, in);
	ReadDesc(&myToken, in);
	ReadDesc(&myUpdateAtTime, in);

	// info for vat tool and size of lobe:
	ReadDesc(&myX, in);
	ReadDesc(&myY, in);
	ReadDesc(&myWidth, in);
	ReadDesc(&myHeight, in);

	// for vat tool:
	ReadDesc(&myColour[0], in);
	ReadDesc(&myColour[1], in);
	ReadDesc(&myColour[2], in);

	// for biochemistry:
	ReadDesc(&myTissueId, in);

	// init SV-Rules:
	myUpdateRule.InitFromDesc(in);



	// INITIALIZATION:
	strcpy(myName, Ezinekot(myToken));
	// error if too small or too big:
	int noOfNeurons = myWidth * myHeight;
	if (noOfNeurons==0)
		throw(GenomeInitFailedException(4, (char*)*this));
	if (noOfNeurons>MAX_NEURONS_PER_LOBE)
		throw(GenomeInitFailedException(5, (char*)*this));

	// init neurons:
	myNeuronInput = new float[noOfNeurons];
	for (int i=0; i<noOfNeurons; i++) {
		ReadDesc(&myNeuronInput[i], in);

		Neuron* n = new Neuron();
		ReadDesc(&n->idInList, in);
		for(int s = 0; s != NUM_SVRULE_VARIABLES; s++)
			ReadDesc(&n->states[s], in);

		myNeurons.push_back(n);
	}

	// init spare neuron variables to first neuron (default)
	mySpareNeuronVariables = &(myNeurons[0]->states);
}


// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       Lobe
// Description: 
// ------------------------------------------------------------------------
Lobe::~Lobe()
{
	for (Neurons::iterator n=myNeurons.begin(); n!=myNeurons.end(); n++)
	{
		delete *n;
	}

	if (myNeuronInput)
	{
		delete [] myNeuronInput;
		myNeuronInput = NULL;
	}
}


// ------------------------------------------------------------------------
// Function:    DoUpdate
// Class:       Lobe
// Description: 
// ------------------------------------------------------------------------
void Lobe::DoUpdate()
{
	if (myUpdateAtTime==0) return;

	// use dummy else have problems with first neuron being winning neuron
	Neuron dummySpare;
	mySpareNeuronVariables = &dummySpare.states; 
	
	myWinningNeuronId = 0;

	for (int i=0; i<myNeurons.size(); i++)
	{
		Neuron& n = *(myNeurons[i]);

		SVRule::invalidVariables[0] = myNeuronInput[i];	// (for input lobes)
		myNeuronInput[i] = 0.0f;						// reset to build up until next processed.

		int returnCode;
		bool shouldFlagThisNeuronAsSpare = false;
		
		if( myRunInitRuleAlwaysFlag )
		{
			returnCode = myInitRule.ProcessGivenVariables(
				SVRule::invalidVariables, SVRule::invalidVariables, n.states,
				*mySpareNeuronVariables, n.idInList, n.idInList
			);
			if (returnCode==SVRule::setSpareNeuronToCurrent)
				shouldFlagThisNeuronAsSpare = true;
		}

		returnCode = myUpdateRule.ProcessGivenVariables(
			SVRule::invalidVariables, SVRule::invalidVariables, n.states,
			*mySpareNeuronVariables, n.idInList, n.idInList
		);
		if (returnCode==SVRule::setSpareNeuronToCurrent)
			shouldFlagThisNeuronAsSpare = true;

		if (shouldFlagThisNeuronAsSpare)
		{
			mySpareNeuronVariables=&(n.states);
			myWinningNeuronId = i;
		}
	}

	// make sure dummy is not winner
	if(mySpareNeuronVariables == &dummySpare.states)
	{
		mySpareNeuronVariables = &myNeurons[0]->states;
		myWinningNeuronId = 0;
	}

}

// ------------------------------------------------------------------------
// Function:    ClearActivity
// Class:       Lobe
// Description: 
// ------------------------------------------------------------------------
void Lobe::ClearActivity()
{
	for (int i=0; i<myNeurons.size(); i++) 
	{
		myNeurons[i]->states[0] = 0.0f;
	}
}


// ------------------------------------------------------------------------
// Function:    DoUpdateFromDesc
// Class:       Lobe
// Description: 
// Arguments:   std::istream &in = 
// ------------------------------------------------------------------------
void Lobe::DoUpdateFromDesc(std::istream &in)
{
	// update and ID:
	in.seekg(4, std::ios::cur);
	ReadDesc(&myWinningNeuronId, in);
	in.seekg(40+(7*SVRule::length), std::ios::cur);
	int noOfNeurons = myWidth * myHeight;
	for (int i=0; i<noOfNeurons; i++) 
		DoUpdateNeuronFromDesc(i, in);
}


// ------------------------------------------------------------------------
// Function:    DoUpdateNeuronFromDesc
// Class:       Lobe
// Description: 
// Arguments:   int i = 
//              std::istream &in = 
// ------------------------------------------------------------------------
void Lobe::DoUpdateNeuronFromDesc(int i, std::istream &in) {
	Neuron *n = myNeurons[i];
	// 4 for myNeuronInput, 4 for myIdInList gives 8:
	in.seekg(8, std::ios::cur);
	in.read((char*)(n->states), sizeof(SVRuleVariables));
/*	for(int s = 0; s != NUM_SVRULE_VARIABLES; s++)
		ReadDesc(&n->states[s], in);*/
}


// ------------------------------------------------------------------------
// Function:    GetWhichNeuronWon
// Class:       Lobe
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int Lobe::GetWhichNeuronWon() {
	return myWinningNeuronId;
}

// ------------------------------------------------------------------------
// Function:    GetNeuronState
// Class:       Lobe
// Description: 
// Arguments:   int whichNeuron = 
//              int whichState = 
// Returns:     float = 
// ------------------------------------------------------------------------
float Lobe::GetNeuronState(int whichNeuron, int whichState)
{
	float* n = GetNeuronStatePointer(whichNeuron, whichState);
	return n==NULL ? 0.0f : *n;
}

// ------------------------------------------------------------------------
// Function:    GetNeuronStatePointer
// Class:       Lobe
// Description: 
// Arguments:   int whichNeuron = 
//              int whichState = 
// Returns:     float* = 
// ------------------------------------------------------------------------
float* Lobe::GetNeuronStatePointer(int whichNeuron, int whichState)
{
	if (whichNeuron<0 || whichNeuron>=myNeurons.size())
		return NULL;
	if (whichState<0 || whichState>=NUM_SVRULE_VARIABLES)
		return NULL;

	return &(myNeurons[whichNeuron]->states[whichState]);
}


// Increase the neuron input:
// ------------------------------------------------------------------------
// Function:    SetNeuronInput
// Class:       Lobe
// Description: 
// Arguments:   int whichNeuron = 
//              float toWhat = 
// ------------------------------------------------------------------------
void Lobe::SetNeuronInput(int whichNeuron, float toWhat)
{
	if (whichNeuron<0 || whichNeuron>=myNeurons.size())
		return;

	myNeuronInput[whichNeuron] += toWhat;
}


// ------------------------------------------------------------------------
// Function:    ClearNeuronActivity
// Class:       Lobe
// Description: 
// Arguments:   int whichNeuron = 
// ------------------------------------------------------------------------
void Lobe::ClearNeuronActivity(int whichNeuron)
{
	if (whichNeuron<0 || whichNeuron>=myNeurons.size())
		return;

	myNeurons[whichNeuron]->states[STATE_VAR] = 0.0f;
}

// ------------------------------------------------------------------------
// Function:    SetLobeWideInput
// Class:       Lobe
// Description: 
// Arguments:   float toWhat = 
// ------------------------------------------------------------------------
void Lobe::SetLobeWideInput(float toWhat)
{
	for (int i=0; i<myNeurons.size(); i++)
	{
		SetNeuronInput(i, toWhat);
	}
}


// ------------------------------------------------------------------------
// Function:    TraceDebugInformation
// Class:       Lobe
// Description: 
// ------------------------------------------------------------------------
void Lobe::TraceDebugInformation()
{
	for (Neurons::iterator n=myNeurons.begin(); n!=myNeurons.end(); n++)
	{
		for (int i=0; i<1/*NUM_SVRULE_VARIABLES*/; i++)
		{
			OutputFormattedDebugString("LOBE %s: state %d: %f\n", (char*)*this, i, (*n)->states[i]);
		}
	}
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
// Class:       Lobe
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Lobe::Write(CreaturesArchive &archive) const
{
	base::Write( archive );

	archive << myWinningNeuronId;
	archive << myToken;
	archive << myTissueId;
	archive << (uint32)(myNeurons.size());
	for (int i=0; i<myNeurons.size(); i++)
	{
		archive << myNeurons[i]->idInList;
		for (int j=0; j<NUM_SVRULE_VARIABLES; j++)
		{
			archive.WriteFloatRefTarget( myNeurons[i]->states[j] );
		}
	}
	archive << myX << myY << myWidth << myHeight;
	archive << myColour[0] << myColour[1] << myColour[2];
	archive.Write( myName, 5 );
	int neuronCount = myWidth * myHeight;
	{
		for( int i = 0; i < neuronCount; ++i )
			archive << myNeuronInput[i];
	}
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
// Class:       Lobe
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Lobe::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!base::Read( archive ))
			return false;

		archive >> myWinningNeuronId;
		archive >> myToken;
		archive >> myTissueId;

		uint32 n;
		archive >> n;
		myNeurons.resize(n);
		for (int i=0; i<n; i++)
		{
			myNeurons[i] = new Neuron();
			archive >> myNeurons[i]->idInList;
			for (int j=0; j<NUM_SVRULE_VARIABLES; j++)
			{
				archive.ReadFloatRefTarget( myNeurons[i]->states[j] );
			}
		}
		archive >> myX >> myY >> myWidth >> myHeight;
		archive >> myColour[0] >> myColour[1] >> myColour[2];
		archive.Read( myName, 5 );
		int neuronCount = myWidth * myHeight;
		myNeuronInput = new float[ neuronCount ];
		{
			for( int i = 0; i < neuronCount; ++i )
				archive >> myNeuronInput[i];
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
// Function:    SetNeuronState
// Class:       Lobe
// Description: 
// Arguments:   const int neuron = 
//              const int state = 
//              const float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Lobe::SetNeuronState(const int neuron, const int state, const float value)
{
	if(neuron < 0 || neuron > myNeurons.size()-1 || 
		state < 0 || state > NUM_SVRULE_VARIABLES-1 ||
		value < -1 || value > 1)
		return false;

	myNeurons[neuron]->states[state] = value;
	return true;
}

// ------------------------------------------------------------------------
// Function:    SetSVFloat
// Class:       Lobe
// Description: 
// Arguments:   int entryNo = 
//              float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Lobe::SetSVFloat(int entryNo, float value)
{
	return myUpdateRule.SetFloat(entryNo, value);
}

// ------------------------------------------------------------------------
// Function:    DumpLobe
// Class:       Lobe
// Description: 
// Arguments:   std::ostream& out = 
// ------------------------------------------------------------------------
void Lobe::DumpLobe(std::ostream& out)
{
	WriteDesc(&myIdInList , out);
	WriteDesc(&myWinningNeuronId, out);
	
	// init spare neuron variables to first neuron (default)
	//mySpareNeuronVariables = &(myNeurons[0]->states); - build manually

	WriteDesc(&myToken, out);
	WriteDesc(&myUpdateAtTime, out);

	WriteDesc(&myX, out);
	WriteDesc(&myY, out);
	WriteDesc(&myWidth, out);
	WriteDesc(&myHeight, out);
	WriteDesc(&myColour[0], out);
	WriteDesc(&myColour[1], out);
	WriteDesc(&myColour[2], out);

	WriteDesc(&myTissueId, out);

	myUpdateRule.Dump(out);
	
	int noNeurons = myNeurons.size();
	for(int n = 0; n != noNeurons; n++)
		DumpNeuron(n, out);
};


// ------------------------------------------------------------------------
// Function:    DumpNeuron
// Class:       Lobe
// Description: 
// Arguments:   int n = 
//              std::ostream& out = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Lobe::DumpNeuron(int n, std::ostream& out)
{
	if(n < 0 || n > myNeurons.size()-1)
		return false;

	WriteDesc(&myNeuronInput[n], out);
	WriteDesc(&myNeurons[n]->idInList, out);
	out.write((char*)(myNeurons[n]->states), sizeof(SVRuleVariables));
	return true;
};

// ------------------------------------------------------------------------
// Function:    DumpSize
// Class:       Lobe
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int Lobe::DumpSize()
{
	return 40+(7*SVRule::length)+(9*myNeurons.size());
};

