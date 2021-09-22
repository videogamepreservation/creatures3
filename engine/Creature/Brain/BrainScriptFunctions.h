#ifndef BrainScriptFunctions_H
#define BrainScriptFunctions_H

int GetNeuronIdFromScriptOffset(int s);
int GetScriptOffsetFromNeuronId(int s);
int GetExpectedAgentScriptFromDecisionOffset(int s);

bool DoesThisScriptRequireAnItObject(int event);

bool IsThisAnIveBeenScript(int event);
int InitBrainMappingsFromCatalogues();


#endif//BrainScriptFunctions_H