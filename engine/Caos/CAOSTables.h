#ifndef CAOS_TABLES_H
#define CAOS_TABLES_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>

enum {
	idCommandTable = 0,
	idIntegerRVTable,
	idVariableTable,
	idFloatRVTable,
	idStringRVTable,
	idAgentRVTable,
	idSubCommandTable_NEW,
	idSubCommandTable_MESG,
	idSubCommandTable_STIM,
	idSubCommandTable_URGE,
	idSubCommandTable_SWAY,
	idSubCommandTable_ORDR,
	idSubCommandTable_GIDS,
	idSubCommandTable_PRT,
	idSubCommandTable_PAT,
	idSubCommandTable_DBG,
	idSubCommandTable_BRN,
	idSubCommandTable_PRAY,
	idSubIntegerRVTable_PRAY,
	idSubStringRVTable_PRAY,
	idSubCommandTable_GENE,
	idSubCommandTable_FILE,
	idSubCommandTable_HIST,
	idSubIntegerRVTable_HIST,
	idSubStringRVTable_HIST,
	idSubIntegerRVTable_PRT,
	idSubStringRVTable_PRT,
	idSubAgentRVTable_PRT,
	// don't use this in case the serialised number of tables (in the 
	// CAOS tool) differs from the compiled number - it is only used 
	// for setting up the compiled defaults
	DEFAULT_NUMBER_OF_TABLES 
	};

// Add strings correspondingly to CAOSTables.cpp
enum 
{
	categoryNoNeedToDocument = 0, // must be zero (OpSpec constructor defaults to 0)
	categoryAgents,
	categoryBrain,
	categoryCamera,
	categoryCompound,
	categoryCreatures,
	categoryDebug,
	categoryFiles,
	categoryFlow,
	categoryGenetics,
	categoryHistory,
	categoryInput,
	categoryMap,
	categoryMotion,
	categoryPorts,
	categoryResources,
	categoryScripts,
	categorySounds,
	categoryTime, 
	categoryVariables,
	categoryVehicles,
	categoryWorld,

	// not implemented must be last
	categoryNotImplemented,
	categoryMax
};

#endif