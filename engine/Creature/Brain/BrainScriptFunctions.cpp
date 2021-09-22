#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../CreatureConstants.h"
#include "../../Agents/AgentConstants.h"
#include "../../C2eServices.h"



bool DoesThisScriptRequireAnItObject(int event) {
	return	event==AC_ACTIVATE1 ||
			event==AC_ACTIVATE2 || event==AC_DEACTIVATE || event==AC_APPROACH ||
			event==AC_RETREAT || event==AC_EAT || event==AC_HIT ||
			event==AC_GET;
}

bool alreadyFetched;
int myScriptMappings[NUMACTIONS];
int myAgentScriptMappings[NUMACTIONS];
std::map<int,int> myNeuronMappings;


inline void EnsureGot()
{
	for (int i=0; i<NUMACTIONS; ++i)
	{
		myScriptMappings[i] = atoi(theCatalogue.Get("Action Script To Neuron Mappings", i));
		myNeuronMappings[myScriptMappings[i]] = i;
		myAgentScriptMappings[i] = atoi(theCatalogue.Get("Decision Offsets to Expected Agent Script", i));
	}
	alreadyFetched = true;
}

int InitBrainMappingsFromCatalogues()
{
	alreadyFetched = false;
	int i;
	for( i=0; i<NUMACTIONS; ++i)
	{
		myScriptMappings[i] = -1;
	}
	myNeuronMappings.clear();
	EnsureGot();
	return i;
}

int GetExpectedAgentScriptFromDecisionOffset(int s)
{
	_ASSERT( s >=0 && s < NUMACTIONS );
	if (s<0 || s>=NUMACTIONS)
		return -1;
	return myAgentScriptMappings[s];	
}

int GetScriptOffsetFromNeuronId(int s) {
	_ASSERT( s >=0 && s < NUMACTIONS );
	if (s<0 || s>=NUMACTIONS)
		return -1;
	return myScriptMappings[s];
}

int GetNeuronIdFromScriptOffset(int s) {
	if (myNeuronMappings.find(s) != myNeuronMappings.end())
		return myNeuronMappings[s];
	return -1;
}

bool IsThisAnIveBeenScript(int event) {
	return	event==SCRIPTDEACTIVATE || event==SCRIPTACTIVATE1 ||
			event==SCRIPTACTIVATE2 || event==SCRIPTHIT || event==SCRIPTPICKUP ||
			event==SCRIPTDROP || event==SCRIPTEAT;
}

