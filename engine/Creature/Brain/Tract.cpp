// Tract.cpp: implementation of the Tract class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Tract.h"
#include "BrainIO.h"
#include "../Genome.h"


CREATURES_IMPLEMENT_SERIAL(Tract)

// Offset into Brain.catalogue to access "Migration Parameters" fields
enum Migration_Parameters
{
	MAX_MIGRATIONS_PER_TRACT = 0,
	DENDRITE_STRENGTH_VAR
};

//////////////////////////////////////////////////////////////////////
// Construction/dstruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
Tract::Tract()
{
	// INITIALIZATION:
	myMaxMigrations = atoi( theCatalogue.Get( "Migration Parameters", MAX_MIGRATIONS_PER_TRACT ) );
	myDendriteStrengthSVIndex = atoi( theCatalogue.Get( "Migration Parameters", DENDRITE_STRENGTH_VAR ) );
}

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Tract
// Description: 
// Arguments:   Genome &genome = 
//              Lobes& lobes = 
// ------------------------------------------------------------------------
Tract::Tract(Genome &genome, Lobes& lobes)
{
	// INITIALIZATION:
	myMaxMigrations = atoi( theCatalogue.Get( "Migration Parameters", MAX_MIGRATIONS_PER_TRACT ) );
	myDendriteStrengthSVIndex = atoi( theCatalogue.Get( "Migration Parameters", DENDRITE_STRENGTH_VAR ) );

	myUpdateAtTime = genome.GetInt();

	// get tract attachment details, source:
	TOKEN srcLobeToken = genome.GetToken();
	mySrc.neuronRangeToUse.min = genome.GetInt();
	mySrc.neuronRangeToUse.max = genome.GetInt();
	mySrc.noOfDendritesPerNeuronOnEachPass = genome.GetInt();

	// and destination:
	TOKEN dstLobeToken = genome.GetToken();
	myDst.neuronRangeToUse.min = genome.GetInt();
	myDst.neuronRangeToUse.max = genome.GetInt();
	myDst.noOfDendritesPerNeuronOnEachPass = genome.GetInt();

	// flags
	myDendritesAreRandomlyConnectedAndMigrate = genome.GetBool();
	myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound = genome.GetBool();

	// Indices of neural state variables to be used to control migration of this tract
	mySrc.neuralGrowthFactorStateVariableIndex = genome.GetCodon(0,NUM_SVRULE_VARIABLES-1);
	myDst.neuralGrowthFactorStateVariableIndex = genome.GetCodon(0,NUM_SVRULE_VARIABLES-1);

	myRunInitRuleAlwaysFlag = genome.GetCodon(0,1) > 0 ? true : false;

	/* may be determined in genome at some point...
	myReward.SetChemicalIndex( genome.GetCodon(0,NUMCHEM-1) );
	myReward.SetRate( genome.GetCodon(0,255) );
	myReward.SetThreshold( genome.GetCodon(0,255) );
	myPunishment.SetChemicalIndex( genome.GetCodon(0,NUMCHEM-1) );
	myPunishment.SetRate( genome.GetCodon(0,255) );
	myPunishment.SetThreshold( genome.GetCodon(0,255) );
	*/
	
	// dummy slots:
	genome.GetByte();
	genome.GetToken();

	// SV-Rules for dendrites:
	myInitRule.InitFromGenome(genome);
	myUpdateRule.InitFromGenome(genome);


	// POST GENOME STUFF:
	// define tract name:
	std::string srcName = Ezinekot(srcLobeToken);
	std::string dstName = Ezinekot(dstLobeToken);
	myName = srcName + "->" +dstName;


	// attach to lobes:
	mySrc.lobe = myDst.lobe = NULL;
	int i;
	for ( i=0; i<lobes.size(); i++)
	{
		Lobe* lobe = lobes[i];
		if (lobe->GetToken() == srcLobeToken)
			mySrc.lobe = lobe;
		if (lobe->GetToken() == dstLobeToken)
			myDst.lobe = lobe;
	}
	if (mySrc.lobe==NULL || myDst.lobe==NULL)
		throw(GenomeInitFailedException(6, myName.c_str()));

	InitNeuronLists();

	// error if either neuron list is empty:
	if (mySrc.neurons.size()==0 || myDst.neurons.size()==0)
		throw(GenomeInitFailedException(7, myName.c_str()));

	// error if both connections are unconstrained:
	if (mySrc.noOfDendritesPerNeuronOnEachPass==0 && myDst.noOfDendritesPerNeuronOnEachPass==0)
		throw(GenomeInitFailedException(8, myName.c_str()));

	int listId = 0;
	// dendrite initialization:
	if (myDendritesAreRandomlyConnectedAndMigrate)
	{
		// migration initialization:

		// error if both connections are constrained:
		if (mySrc.noOfDendritesPerNeuronOnEachPass>0 && myDst.noOfDendritesPerNeuronOnEachPass>0)
			throw(GenomeInitFailedException(9, myName.c_str()));

		if (mySrc.noOfDendritesPerNeuronOnEachPass==0)
		{
			// make sure we don't try to connect more dendrites to a dest neuron than there are source neurons:
			if (myDst.noOfDendritesPerNeuronOnEachPass >= mySrc.lobe->GetNoOfNeurons())
				myDst.noOfDendritesPerNeuronOnEachPass = mySrc.lobe->GetNoOfNeurons();

			// source unconstrained (=> dest constrained):
			// go through all the destination neurons and attach them to source neurons:
			for (Neurons::iterator d=myDst.neurons.begin(); d!=myDst.neurons.end(); d++)
			{
				int n = myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound ?
					Rnd(1,myDst.noOfDendritesPerNeuronOnEachPass):
					myDst.noOfDendritesPerNeuronOnEachPass;
				for (i=0; i<n; i++)
				{
					Dendrite* dendrite = new Dendrite();
					dendrite->idInList = listId;
					listId++;
					dendrite->dstNeuron = *d;
					do {
						dendrite->srcNeuron = mySrc.neurons[Rnd(mySrc.neurons.size()-1)];
					} while (DoesDendriteExistFromTo(dendrite->srcNeuron, dendrite->srcNeuron));
					myDendrites.push_back(dendrite);
				}
			}
		}
		else
		{
			// make sure we don't try to connect more dendrites to a source neuron than there are dest neurons:
			if (mySrc.noOfDendritesPerNeuronOnEachPass >= myDst.lobe->GetNoOfNeurons())
				mySrc.noOfDendritesPerNeuronOnEachPass = myDst.lobe->GetNoOfNeurons();

			// destination unconstrained (=> source constrained):
			// go through all the source neurons and attach them to destination neurons:
			for (Neurons::iterator s=mySrc.neurons.begin(); s!=mySrc.neurons.end(); s++)
			{
				int n = myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound ?
					Rnd(1,mySrc.noOfDendritesPerNeuronOnEachPass):
					mySrc.noOfDendritesPerNeuronOnEachPass;
				for (i=0; i<n; i++)
				{
					Dendrite* dendrite = new Dendrite();
					dendrite->idInList = listId;
					listId++;
					dendrite->srcNeuron = *s;
					do {
						dendrite->dstNeuron = myDst.neurons[Rnd(myDst.neurons.size()-1)];
					} while (DoesDendriteExistFromTo(dendrite->srcNeuron, dendrite->srcNeuron));
					myDendrites.push_back(dendrite);
				}
			}
		}
	}
	else
	{
		if (mySrc.noOfDendritesPerNeuronOnEachPass==0 || myDst.noOfDendritesPerNeuronOnEachPass==0)
			throw(GenomeInitFailedException(10, myName.c_str()));

		Neurons::iterator s = mySrc.neurons.begin();
		Neurons::iterator d = myDst.neurons.begin();

		i=0; do {
			Dendrite* dendrite = new Dendrite(*s,*d);
			dendrite->idInList = listId;
			listId++;
			myDendrites.push_back(dendrite);	// check doesn't exist already?

			i++;
			if (i%mySrc.noOfDendritesPerNeuronOnEachPass==0)
			{
				if (++d==myDst.neurons.end())
					d = myDst.neurons.begin();
			}
			if (i%myDst.noOfDendritesPerNeuronOnEachPass==0)
			{
				if (++s==mySrc.neurons.end())
					s = mySrc.neurons.begin();
			}
		} while (i<MAX_DENDRITES_PER_TRACT &&
			(d!=myDst.neurons.begin() || s!=mySrc.neurons.begin()) );
	}
	if (myDendrites.size()>=MAX_DENDRITES_PER_TRACT)
	{
	//	OutputFormattedDebugString("Reached maximum no of dendrites in tract %s.\n",(const char*)*this);
	}

	// This brain component supports the SV Opcodes relating to reward and punishment
	mySupportReinforcementFlag = true;
	STtoLTRate = 0;
}



// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Tract
// Description: 
// Arguments:   std::istream &in = 
//              Lobes& lobes = 
// ------------------------------------------------------------------------
Tract::Tract(std::istream &in, Lobes& lobes)
{
	// INITIALIZATION FROM DESCRIPTION:
	myMaxMigrations = atoi( theCatalogue.Get( "Migration Parameters", MAX_MIGRATIONS_PER_TRACT ) );
	myDendriteStrengthSVIndex = atoi( theCatalogue.Get( "Migration Parameters", DENDRITE_STRENGTH_VAR ) );

	ReadDesc(&myIdInList, in);

	ReadDesc(&myUpdateAtTime, in);

	int l;
	// get tract attachment details, source:
	ReadDesc(&l, in);
	mySrc.lobe = lobes[l];
	ReadDesc(&mySrc.neuronRangeToUse.min, in);
	ReadDesc(&mySrc.neuronRangeToUse.max, in);
	in.get((char &)mySrc.noOfDendritesPerNeuronOnEachPass);

	// and destination:
	ReadDesc(&l, in);
	myDst.lobe = lobes[l];
	ReadDesc(&myDst.neuronRangeToUse.min, in);
	ReadDesc(&myDst.neuronRangeToUse.max, in);
	in.get((char &)myDst.noOfDendritesPerNeuronOnEachPass);

	// flags
	ReadDesc(&myDendritesAreRandomlyConnectedAndMigrate, in);
	ReadDesc(&myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound, in);


	// SV-Rules for dendrites:
	myInitRule.InitFromDesc(in);
	myUpdateRule.InitFromDesc(in);

	// define tract name:
	myName = (char*)(mySrc.lobe);
	myName += "->";
	myName += (char*)(myDst.lobe);

	InitNeuronLists();

	// error if either neuron list is empty:
	if (mySrc.neurons.size()==0 || myDst.neurons.size()==0)
		throw(GenomeInitFailedException(7, myName.c_str()));

	// error if both connections are unconstrained:
	if (mySrc.noOfDendritesPerNeuronOnEachPass==0 && myDst.noOfDendritesPerNeuronOnEachPass==0)
		throw(GenomeInitFailedException(8, myName.c_str()));

	int noDendrites;
	ReadDesc(&noDendrites, in);
	for (int i=0; i<noDendrites; i++)
	{
		Dendrite* dendrite = new Dendrite();
		ReadDesc(&dendrite->idInList, in);
		int d;
		ReadDesc(&d, in);
		dendrite->srcNeuron = mySrc.lobe->GetNeuron(d);
		ReadDesc(&d, in);
		dendrite->dstNeuron = myDst.lobe->GetNeuron(d);
		for(int w = 0; w != NUM_SVRULE_VARIABLES; w++)
			ReadDesc(&dendrite->weights[w], in);

		myDendrites.push_back(dendrite);
	}
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
Tract::~Tract()
{
	for (Dendrites::iterator d=myDendrites.begin(); d!=myDendrites.end(); d++)
	{
		delete *d;
	}
}




// ------------------------------------------------------------------------
// Function:    Initialise
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
void Tract::Initialise()
{
	if(myInitialised)
		return;

	myInitialised = true;
	// init all dendrites from SV-Rule
	for (Dendrites::iterator d=myDendrites.begin(); d!=myDendrites.end(); d++)
	{
		myRunInitRuleAlwaysFlag ?
			(*d)->ClearWeights() :
			(*d)->InitByRule(myInitRule, this);
	}
}




// ------------------------------------------------------------------------
// Function:    GetNoOfDendritesConnectedTo
// Class:       Tract
// Description: 
// Arguments:   Neuron* neuron = 
//              bool checkSrcNotDst = 
// Returns:     int = 
// ------------------------------------------------------------------------
int Tract::GetNoOfDendritesConnectedTo(Neuron* neuron, bool checkSrcNotDst)
{
	int noOfDendritesSoFar = 0;
	for (Dendrites::iterator d=myDendrites.begin(); d!=myDendrites.end(); d++)
	{
		if (checkSrcNotDst ? ((*d)->srcNeuron==neuron):((*d)->dstNeuron==neuron))
		{
			noOfDendritesSoFar++;
		}
	}
	return noOfDendritesSoFar;
}

// ------------------------------------------------------------------------
// Function:    InitNeuronLists
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
void Tract::InitNeuronLists()
{
	// initialize neuron lists for dendrite initialisation (and migration) purposes:
	int i;
	for (i=mySrc.neuronRangeToUse.min; i<=mySrc.neuronRangeToUse.max && i<mySrc.lobe->GetNoOfNeurons(); i++)
	{
		mySrc.neurons.push_back(mySrc.lobe->GetNeuron(i));
	}
	for (i=myDst.neuronRangeToUse.min; i<=myDst.neuronRangeToUse.max && i<myDst.lobe->GetNoOfNeurons(); i++)
	{
		myDst.neurons.push_back(myDst.lobe->GetNeuron(i));
	}
}

// ------------------------------------------------------------------------
// Function:    DoesDendriteExistFromTo
// Class:       Tract
// Description: 
// Arguments:   Neuron* srcNeuron = 
//              Neuron* dstNeuron = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Tract::DoesDendriteExistFromTo(Neuron* srcNeuron, Neuron* dstNeuron)
{
	for (Dendrites::iterator d=myDendrites.begin(); d!=myDendrites.end(); d++)
	{
		if ((*d)->srcNeuron==srcNeuron && (*d)->dstNeuron==dstNeuron)
			return true;
	}
	return false;
}

// ------------------------------------------------------------------------
// Function:    DoUpdate
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
void Tract::DoUpdate()
{
	if (myUpdateAtTime==0) return;

	// Migrate dendrites in the WeakDendrites list
	// before any of the dendrites are processed.
	if (myDendritesAreRandomlyConnectedAndMigrate)
	{
		MigrateWeakDendrites();
	}
	
	// Firing Rules:
	for (int k=0; k<myDendrites.size(); k++)
	{
		Dendrite& d = *(myDendrites[k]);

		if( myRunInitRuleAlwaysFlag )
		{
			myInitRule.ProcessGivenVariables(
				d.srcNeuron->states, d.weights, d.dstNeuron->states,
				mySrc.lobe->GetSpareNeuronVariables(),
				d.srcNeuron->idInList, d.dstNeuron->idInList, (BrainComponent*) this
			);
		}

		myUpdateRule.ProcessGivenVariables(
			d.srcNeuron->states, d.weights, d.dstNeuron->states,
			mySrc.lobe->GetSpareNeuronVariables(),
			d.srcNeuron->idInList, d.dstNeuron->idInList, (BrainComponent*) this
		);

		// Perform reward and punishment reinforcement if necessary
		ProcessRewardAndPunishment( d );

		if (myDendritesAreRandomlyConnectedAndMigrate)
		{
			// Store weakest dendrites for next update cycle
			UpdateWeakDendritesList( d );
		}
	}
}

// Clearing activity for the dendrites involves setting the STW to the LTW values.
// This allows instincts to be burned in with high LTW->STW convergence rates. 
// ------------------------------------------------------------------------
// Function:    ClearActivity
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
void Tract::ClearActivity()
{
	for (int i=0; i<myDendrites.size(); i++)
	{
		myDendrites[i]->weights[WEIGHT_SHORTTERM_VAR] = myDendrites[i]->weights[WEIGHT_LONGTERM_VAR];
	}
}

// Keep the list of weakest dendrites updated
// Insert this dendrite into the list depending on its strength - in ascending strength, i.e. weakest first.
// Keep the maximum length of the list to myMaxMigrations
// ------------------------------------------------------------------------
// Function:    UpdateWeakDendritesList
// Class:       Tract
// Description: 
// Arguments:   Dendrite &d  = 
// ------------------------------------------------------------------------
void Tract::UpdateWeakDendritesList( Dendrite &d )
{

	if( myMaxMigrations == 0 )
	{
		// return if not looking for weak dendrites
		return;
	}

	// Catch the most frequent case (i.e. the list is full and this dendrite has a higher
	// strength than (or equal to) the strongest dendrite in the list) and return straight away.
	int numWeakDendrites = myWeakDendrites.size();
	if( numWeakDendrites == myMaxMigrations )
	{
		if( d.weights[myDendriteStrengthSVIndex] >= myWeakDendrites[ numWeakDendrites-1 ]->weights[myDendriteStrengthSVIndex] )
		{
			return;
		}
	}

	DendritesIterator dendIter;
	// Otherwise search the list to find where this dendrite would be placed
	for( dendIter = myWeakDendrites.begin(); dendIter < myWeakDendrites.end(); dendIter++ )
	{

		if( d.weights[myDendriteStrengthSVIndex] < (*dendIter)->weights[myDendriteStrengthSVIndex] )
		{
			// insert the dendrite before this iterator
			myWeakDendrites.insert( dendIter, &d );

			// Keep the maximum length of the list to myMaxMigrations
			if( myWeakDendrites.size() > myMaxMigrations )
			{
				myWeakDendrites.pop_back();
				ASSERT(myWeakDendrites.size() <= myMaxMigrations);
			}
			return;
		}
	}
	
	// As far as I can tell this dendrite should be appended to the list (which is not full)
	myWeakDendrites.push_back( &d );

	// Keep the maximum length of the list to myMaxMigrations
	if( myWeakDendrites.size() > myMaxMigrations )
	{
		myWeakDendrites.pop_back();
	}

	ASSERT(myWeakDendrites.size() <= myMaxMigrations);
	return;
}

// Attempt to perform migration for weak dendrites of this tract
// This functions finds a single destination neuron and then finds
// myMaxMigrations source neurons that should be migrated
// towards. If there are no dendrites between these neurons then
// the weakest dendrites of the tract are recruited to make these
// connections.
// sourceStateIndex determines the state variable that is used to select source neurons to migrate from
// destStateIndex determines the state variable that is used to select destination neurons to migrate to
// ------------------------------------------------------------------------
// Function:    MigrateWeakDendrites
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
void Tract::MigrateWeakDendrites()
{

	int numMigratingDendrites = myWeakDendrites.size();
	if( numMigratingDendrites )
	{

		int sourceStateIndex = mySrc.neuralGrowthFactorStateVariableIndex;
		int destStateIndex = myDst.neuralGrowthFactorStateVariableIndex;

		Neuron *highestSrcNGFNeuron = NULL;
		Neuron *highestDstNGFNeuron = NULL;

		ASSERT(mySrc.neurons.size() > 0);
		ASSERT(myDst.neurons.size() > 0);

		/////////////////////////////////
		// Look for destination neuron with highest 'migration' chemical
		/////////////////////////////////

		float highestDstNGF = -1.0;
		// Search for the highest destination neuron
		for( NeuronsIterator destNeurons = myDst.neurons.begin(); destNeurons<myDst.neurons.end(); destNeurons++ )
		{
			if( (*destNeurons)->states[destStateIndex] > highestDstNGF )
			{
				highestDstNGFNeuron = *destNeurons;
				highestDstNGF = (*destNeurons)->states[destStateIndex];
			}
		}

		// No suitable destination neuron has been found
		if( highestDstNGF <= 0.0f )
		{
			myWeakDendrites.clear();
			return;
		}

		/////////////////////////////////
		// Look for source neurons with highest 'migration' chemical
		/////////////////////////////////
		Neurons migrateFrom = FindNNeuronsWithHighestGivenState( mySrc.neurons, sourceStateIndex, numMigratingDendrites );

		for( NeuronsIterator migrateFromIter = migrateFrom.begin(); migrateFromIter < migrateFrom.end(); migrateFromIter++ )
		{
			// only migrate dendrites whose state levels are nonzero
			// i.e. make sure that migration does not occur when levels are zero or less
			if( (*migrateFromIter)->states[sourceStateIndex] > 0.0f )
			{
				AttemptMigration( highestDstNGFNeuron, *migrateFromIter, sourceStateIndex );
			}
		}

	} // if there are migatable dendrites

	// Empty my list of weak dendrites so the list can be remade during the tract update
	myWeakDendrites.clear();
}

// Search the list of neurons (searchNeurons) comparing neurons based on the state variable
// indexed by stateIndex. Sort the top maxListSize Neurons and store them in resultNeuronsPtr
// ------------------------------------------------------------------------
// Function:    FindNNeuronsWithHighestGivenState
// Class:       Tract
// Description: 
// Arguments:   Neurons searchNeurons = 
//              int stateIndex = 
//              int maxListSize  = 
// Returns:     Neurons = 
// ------------------------------------------------------------------------
Neurons Tract::FindNNeuronsWithHighestGivenState( Neurons searchNeurons, int stateIndex, int maxListSize )
{

	Neurons resultNeurons;

	if( maxListSize == 0 )
	{
		return resultNeurons;
	}

	// Seed the list of migratable source neurons with the first source neuron
	NeuronsIterator resultNeuronsIter = resultNeurons.begin(); 
	NeuronsIterator searchNeuronsIter = searchNeurons.begin();
	resultNeurons.push_back( *searchNeuronsIter );
	searchNeuronsIter++;

	// Search the source neurons for the ones with higher migration chemical
	for( ; searchNeuronsIter<searchNeurons.end(); searchNeuronsIter++ )
	{
		
		// Don't sort neurons which have no chance of getting into the resultNeurons list
		// If the list is full, see if the last entry (lowest state level) is lower than
		// the neuron being tested - if so skip this neuron
		int numResultNeurons = resultNeurons.size();
		if( numResultNeurons == maxListSize )
		{
			if( (*searchNeuronsIter)->states[stateIndex] <= resultNeurons[ numResultNeurons -1 ]->states[stateIndex] )
			{
				continue;
			}
		}

		// This neuron should be inserted into the list; search the list to find where it should be placed
		for( resultNeuronsIter = resultNeurons.begin(); resultNeuronsIter < resultNeurons.end(); resultNeuronsIter++ )
		{
			if( (*searchNeuronsIter)->states[stateIndex] > (*resultNeuronsIter)->states[stateIndex] )
			{

				// insert the neuron before this iterator
				resultNeurons.insert( resultNeuronsIter , *searchNeuronsIter );
				// stop searching the resultNeurons list
				break;

			}
		}

		if( resultNeuronsIter == resultNeurons.end() )
		{
			// This neuron should be appended to the end of the list
			resultNeurons.push_back( *searchNeuronsIter );				
		}

		// Keep the maximum length of the list to 'maxListSize'
		if( resultNeurons.size() > maxListSize )
		{
			resultNeurons.pop_back();
			ASSERT(resultNeurons.size() <= maxListSize);
		}

	} // for each search neuron

	return resultNeurons;
}

// AttemptMigration tries to create a connection between the dest and source neurons
// This function uses the myWeakDendrites list as a source of the dendrites to 
// use to make the connections. sourceStateIndex specifies the index of the state
// variable in the source neuron that is used to judge whether a dendrite can
// be migrated. A dendrite can connect to a source neuron if the dendrite's strength
// (i.e. variable myDendriteStrengthSVIndex) is less than the value of source neuron's
// state variable specified by sourceStateIndex.
// ------------------------------------------------------------------------
// Function:    AttemptMigration
// Class:       Tract
// Description: 
// Arguments:   Neuron* dest = 
//              Neuron *source = 
//              int sourceStateIndex  = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Tract::AttemptMigration( Neuron* dest, Neuron *source, int sourceStateIndex )
{
	Dendrite* wiredUpDendrite;
	wiredUpDendrite = GetDendriteIfExistingFromTo(source, dest);

	if( !wiredUpDendrite )
	{
		// find the best dendrite to migrate - one with strength just less 
		// than the state level within the source neuron
		
		// Start with least weak dendrite
		DendritesIterator looseEnd = myWeakDendrites.begin();
		while( looseEnd != myWeakDendrites.end() )
		{
			if( (*looseEnd)->weights[myDendriteStrengthSVIndex] < source->states[sourceStateIndex] )
			{
				// Found a dendrite that can be migrated - rewire it
				wiredUpDendrite = *looseEnd;
				wiredUpDendrite->srcNeuron = source;
				wiredUpDendrite->dstNeuron = dest;
				myWeakDendrites.erase(looseEnd);

				if( !myRunInitRuleAlwaysFlag )
				{
					// Reinitialise this dendrite because it has moved
					wiredUpDendrite->InitByRule(myInitRule, this);
				}
				else
				{
					// default initialisation is to clear all weights
					wiredUpDendrite->ClearWeights();
				}

				return true;
			}
			looseEnd++;
		}
	}

	if( wiredUpDendrite )
	{
		// A dendrite already exists
		return true;
	}
	else
	{
		// No weak enough dendrite can be found
		return false;
	}
}

// ------------------------------------------------------------------------
// Function:    GetDendriteIfExistingFromTo
// Class:       Tract
// Description: 
// Arguments:   Neuron* srcNeuron = 
//              Neuron* dstNeuron = 
// Returns:     Dendrite* = 
// ------------------------------------------------------------------------
Dendrite* Tract::GetDendriteIfExistingFromTo(Neuron* srcNeuron, Neuron* dstNeuron)
{
	for (Dendrites::iterator d=myDendrites.begin(); d!=myDendrites.end(); d++)
	{
		if ((*d)->srcNeuron==srcNeuron && (*d)->dstNeuron==dstNeuron)
			return *d;
	}
	return NULL;
}

// ------------------------------------------------------------------------
// Function:    DoUpdateFromDesc
// Class:       Tract
// Description: 
// Arguments:   std::istream &in = 
// ------------------------------------------------------------------------
void Tract::DoUpdateFromDesc(std::istream &in)
{
	in.seekg(36+(7*(SVRule::length*2)), std::ios::cur);

	int noDendrites;
	ReadDesc(&noDendrites, in);
	for (int i=0; i<noDendrites; i++) 
		DoUpdateDendriteFromDesc(i, in);

}


// ------------------------------------------------------------------------
// Function:    DoUpdateDendriteFromDesc
// Class:       Tract
// Description: 
// Arguments:   int i = 
//              std::istream &in = 
// ------------------------------------------------------------------------
void Tract::DoUpdateDendriteFromDesc(int i, std::istream &in)
{
	Dendrite *dendrite = myDendrites[i];
	ReadDesc(&(dendrite->idInList), in);
	int d;
	ReadDesc(&d, in);
	dendrite->srcNeuron = mySrc.lobe->GetNeuron(d);
	ReadDesc(&d, in);
	dendrite->dstNeuron = myDst.lobe->GetNeuron(d);

	in.read((char*)dendrite->weights, sizeof(SVRuleVariables));
}


// ------------------------------------------------------------------------
// Function:    TraceDebugInformation
// Class:       Tract
// Description: 
// ------------------------------------------------------------------------
void Tract::TraceDebugInformation()
{
	for (Dendrites::iterator d=myDendrites.begin(); d!=myDendrites.end(); d++)
	{
		for (int i=0; i<NUM_SVRULE_VARIABLES; i++)
		{
			OutputFormattedDebugString("TRACT %s: state %d: %f",
				myName.c_str(), i, (*d)->weights[i]);
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
// Class:       Tract
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Tract::Write(CreaturesArchive &archive) const
{
	base::Write( archive );


	archive << myName << mySrc.lobe << mySrc.neuronRangeToUse.min << 
		mySrc.neuronRangeToUse.max << mySrc.noOfDendritesPerNeuronOnEachPass << 
		mySrc.neuralGrowthFactorStateVariableIndex;
	archive << myDst.lobe << myDst.neuronRangeToUse.min << 
		myDst.neuronRangeToUse.max << myDst.noOfDendritesPerNeuronOnEachPass << 
		myDst.neuralGrowthFactorStateVariableIndex;
	
	archive << (uint32)myDendrites.size();
	int i,j;
	for ( i=0; i<myDendrites.size(); i++)
	{
		for ( j=0; j<NUM_SVRULE_VARIABLES; j++)
		{
			archive << myDendrites[i]->weights[j];
		}
		archive << myDendrites[i]->idInList;
		archive << myDendrites[i]->srcNeuron->idInList;
		archive << myDendrites[i]->dstNeuron->idInList;
	}


	archive << (uint32)myWeakDendrites.size();
	for (i=0; i<myWeakDendrites.size(); i++ )
	{
		// Only store the id of the dendrite
		archive << myDendrites[i]->idInList;
	}


	archive << myDendritesAreRandomlyConnectedAndMigrate;
	archive << myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound;

	myInitRule.Write(archive);
	myUpdateRule.Write(archive);

	myReward.Write(archive);
	myPunishment.Write(archive);

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
// Class:       Tract
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Tract::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		if(!base::Read( archive ))
			return false;

		archive >> myName >> mySrc.lobe >> mySrc.neuronRangeToUse.min >>
			mySrc.neuronRangeToUse.max >> mySrc.noOfDendritesPerNeuronOnEachPass >>
			mySrc.neuralGrowthFactorStateVariableIndex;
		archive >> myDst.lobe >> myDst.neuronRangeToUse.min >>
			myDst.neuronRangeToUse.max >> myDst.noOfDendritesPerNeuronOnEachPass >>
			myDst.neuralGrowthFactorStateVariableIndex;
		InitNeuronLists();

		uint32 n;
		archive >> n;
		myDendrites.resize(n);
		int i;
		for ( i=0; i<n; i++)
		{
			myDendrites[i] = new Dendrite();
			for (int j=0; j<NUM_SVRULE_VARIABLES; j++)
			{
				archive >> myDendrites[i]->weights[j];
			}
			archive >> myDendrites[i]->idInList;
			int id;
			archive >> id;
			myDendrites[i]->srcNeuron = mySrc.lobe->GetNeuron(id);
			archive >> id;
			myDendrites[i]->dstNeuron = myDst.lobe->GetNeuron(id);
		}


		archive >> n;
		myWeakDendrites.resize(n);
		for (i=0; i<n; i++)
		{
			int id;
			archive >> id;
			int j;
			for( j=0; j<myDendrites.size(); j++ )
			{
				if( myDendrites[j]->idInList == id )
				{
					myWeakDendrites[i] = myDendrites[j];
					break;
				}
			}
		}

		archive >> myDendritesAreRandomlyConnectedAndMigrate;
		archive >> myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound;

		myInitRule.Read(archive);
		myUpdateRule.Read(archive);

		myReward.Read(archive);
		myPunishment.Read(archive);

	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

// ------------------------------------------------------------------------
// Function:    SetDendriteWeight
// Class:       Tract
// Description: 
// Arguments:   const int dendrite = 
//              const int weight = 
//              const float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Tract::SetDendriteWeight( const int dendrite, const int weight, const float value)
{
	if(dendrite < 0 || dendrite > myDendrites.size()-1 || 
		weight < 0 || weight > NUM_SVRULE_VARIABLES-1 ||
		value < -1 || value > 1)
		return false;

	myDendrites[dendrite]->weights[weight] = value;
	return true;
}

// ------------------------------------------------------------------------
// Function:    SetSVFloat
// Class:       Tract
// Description: 
// Arguments:   int entryNo = 
//              float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Tract::SetSVFloat(int entryNo, float value)
{
	return myUpdateRule.SetFloat(entryNo, value);
}

// ------------------------------------------------------------------------
// Function:    DumpTract
// Class:       Tract
// Description: 
// Arguments:   std::ostream& out = 
// ------------------------------------------------------------------------
void Tract::DumpTract(std::ostream& out)
{
	WriteDesc(&myIdInList, out);
	WriteDesc(&myUpdateAtTime, out);

	WriteDesc(mySrc.lobe->GetPointerToIdInList(), out);
	WriteDesc(&mySrc.neuronRangeToUse.min, out); 
	WriteDesc(&mySrc.neuronRangeToUse.min, out);
	out.put((char &)mySrc.noOfDendritesPerNeuronOnEachPass);
	//	Neurons neurons - can connect manually;

	WriteDesc(myDst.lobe->GetPointerToIdInList(), out);
	WriteDesc(&myDst.neuronRangeToUse.min, out);
	WriteDesc(&myDst.neuronRangeToUse.min, out);
	out.put((char &)myDst.noOfDendritesPerNeuronOnEachPass);
	//	Neurons neurons - can connect manually;

	WriteDesc(&myDendritesAreRandomlyConnectedAndMigrate, out);
	WriteDesc(&myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound, out);

	myInitRule.Dump(out);
	myUpdateRule.Dump(out);

	int noDendrites = myDendrites.size();
	WriteDesc(&noDendrites, out);
	for(int d = 0; d != noDendrites; d++)
		DumpDendrite(d, out);

};



// ------------------------------------------------------------------------
// Function:    DumpDendrite
// Class:       Tract
// Description: 
// Arguments:   int d = 
//              std::ostream& out = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool Tract::DumpDendrite(int d, std::ostream& out)
{
	if(d < 0 || d > myDendrites.size()-1)
		return false;

	WriteDesc(&myDendrites[d]->idInList, out);
	WriteDesc(&myDendrites[d]->srcNeuron->idInList, out);
	WriteDesc(&myDendrites[d]->dstNeuron->idInList, out);

	out.write((char*)(myDendrites[d]->weights), sizeof(SVRuleVariables));

	return true;
};


// ------------------------------------------------------------------------
// Function:    DumpSize
// Class:       Tract
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int Tract::DumpSize()
{
	return 36+(7*(SVRule::length*2))+4+(16*myDendrites.size());
};



/////////////////////////
// Reward/Punishment data and functions
//	This mechanism increases or decreases weights of dendrites connected to a 
//	winning neuron. When reinforcement takes place and the amount of reinforcement
//	depends on biochemical levels and parameters that are set by SVRules.
//
//	Reward/Punishment is a hard-coded mechanism that can be configured
//	and switched on from the SVRules of a dendrite - dsb
/////////////////////////
// ------------------------------------------------------------------------
// Function:    ProcessRewardAndPunishment
// Class:       Tract
// Description: 
// Arguments:   Dendrite &d  = 
// ------------------------------------------------------------------------
void Tract::ProcessRewardAndPunishment( Dendrite &d )
{
	if( !(myReward.IsSupported()||myPunishment.IsSupported()) )
	{
		return;
	}

	// check is destination neuron is winner, if not then return;
	if( d.dstNeuron->states[OUTPUT_VAR] == 0.0f )
	{
		return;
	}

	if( myReward.IsSupported() )
	{
		myReward.ReinforceAVariable( myPointerToChemicals[myReward.GetChemicalIndex()], d.weights[WEIGHT_SHORTTERM_VAR] );
	}	

	if( myPunishment.IsSupported() )
	{
		myPunishment.ReinforceAVariable( myPointerToChemicals[myPunishment.GetChemicalIndex()], d.weights[WEIGHT_SHORTTERM_VAR] );
	}
}

void Tract::ReinforcementDetails::ReinforceAVariable( float levelOfReinforcement, float &variableToReinforce )
{
	if( levelOfReinforcement > myThreshold )
	{
		float reinforcementModifier = levelOfReinforcement - myThreshold;
		variableToReinforce = BoundIntoMinusOnePlusOne( variableToReinforce + (myRate * reinforcementModifier) );
	}
}

bool Tract::ReinforcementDetails::Write(CreaturesArchive &archive) const
{
	archive << myDendritesSupportReinforcement;
	archive << myThreshold;
	archive << myRate;
	archive << myChemicalIndex;
	return true;
}

bool Tract::ReinforcementDetails::Read(CreaturesArchive &archive)
{
	archive >> myDendritesSupportReinforcement;
	archive >> myThreshold;
	archive >> myRate;
	archive >> myChemicalIndex;
	return true;
}
