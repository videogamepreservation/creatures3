// Dendrite.h: interface for the Dendrite class.
//
//////////////////////////////////////////////////////////////////////

#ifndef Dendrite_H
#define Dendrite_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BrainConstants.h"

class SVRule;
struct Neuron;
class Tract;

#include <vector>
struct Dendrite;
typedef std::vector<Dendrite*> Dendrites;
typedef std::vector<Dendrite*>::iterator DendritesIterator;

struct Dendrite
{
	int idInList;

	Neuron* srcNeuron;
	Neuron* dstNeuron;

	SVRuleVariables weights;

	Dendrite(Neuron* s, Neuron* d);
	Dendrite();

	void InitByRule(SVRule& initRule, Tract* myOwner);
	void ClearWeights();
};

#endif//Dendrite_H
