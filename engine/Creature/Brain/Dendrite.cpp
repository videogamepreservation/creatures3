// Dendrite.cpp: implementation of the Dendrite class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Dendrite.h"
#include "Neuron.h"
#include "SVRule.h"



// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Dendrite
// Description: 
// Arguments:   Neuron* s = 
//              Neuron* d = 
// ------------------------------------------------------------------------
Dendrite::Dendrite(Neuron* s, Neuron* d)
{
	srcNeuron = s;
	dstNeuron = d;

	for (int i=0; i<NUM_SVRULE_VARIABLES; i++)
	{
		weights[i] = 0.0f;
	}
}

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Dendrite
// Description: 
// ------------------------------------------------------------------------
Dendrite::Dendrite()
{
	srcNeuron = NULL;
	dstNeuron = NULL;

	ClearWeights();
}

// ------------------------------------------------------------------------
// Function:    InitByRule
// Class:       Dendrite
// Description: 
// Arguments:   SVRule& initRule = 
//              Tract* myOwner = 
// ------------------------------------------------------------------------
void Dendrite::InitByRule(SVRule& initRule, Tract* myOwner)
{
	initRule.ProcessGivenVariables(
		SVRule::invalidVariables, weights, SVRule::invalidVariables,
		SVRule::invalidVariables,
		srcNeuron->idInList, dstNeuron->idInList, (BrainComponent*) myOwner
	);
}

// ------------------------------------------------------------------------
// Function:    ClearWeights
// Class:       Dendrite
// Description: 
// ------------------------------------------------------------------------
void Dendrite::ClearWeights()
{
	for( int i=0; i<NUM_SVRULE_VARIABLES; i++ )
	{
		weights[i] = 0.0f;
	}
}
