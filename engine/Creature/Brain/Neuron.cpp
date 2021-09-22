// Neuron.cpp: implementation of the Neuron class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Neuron.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       Neuron
// Description: 
// ------------------------------------------------------------------------
Neuron::Neuron()
{
	ClearStates();
}

// ------------------------------------------------------------------------
// Function:    ClearStates
// Class:       Neuron
// Description: 
// ------------------------------------------------------------------------
void Neuron::ClearStates()
{
	for (int i=0; i<NUM_SVRULE_VARIABLES; i++)
	{
		states[i] = 0.0f;
	}
}