// -------------------------------------------------------------------------
// Filename:    DebugHandlers.h
// Class:       DebugHandlers
// Purpose:     Routines to implement debug commands in CAOS
// Description:
//
// Usage:
//
// History:
// 24Feb99	BenC	Initial version
// -------------------------------------------------------------------------


#ifndef DEBUGHANDLERS_H
#define DEBUGHANDLERS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <iostream>
#include "../Agents/AgentHandle.h"
#include "../../common/C2eTypes.h"

class CAOSMachine;
class Agent;

class CAOSVar;

class DebugHandlers
{

public:
	// Commands
	static void Command_DBG( CAOSMachine& vm );
	static void Command_MANN( CAOSMachine& vm );
	static void Command_HELP( CAOSMachine& vm );
	static void Command_APRO( CAOSMachine& vm );
	static void Command_MEMX( CAOSMachine& vm );

	// Integer r-values
	static int IntegerRV_PAWS( CAOSMachine& vm );
	static int IntegerRV_CODF( CAOSMachine& vm );
	static int IntegerRV_CODG( CAOSMachine& vm );
	static int IntegerRV_CODS( CAOSMachine& vm );
	static int IntegerRV_CODE( CAOSMachine& vm );
	static int IntegerRV_CODP( CAOSMachine& vm );
	static int IntegerRV_HEAP( CAOSMachine& vm );

	// String r-values
	static void StringRV_DBG( CAOSMachine& vm, std::string& str ); // DBG#
	static void StringRV_DBGA( CAOSMachine& vm, std::string& str );

	// Agent rvalues
	static AgentHandle AgentRV_TACK( CAOSMachine& vm);

	// Helpers
	static void StreamAgent(std::ostream& out, AgentHandle a);
	static void StreamVariable(std::ostream& out, CAOSVar* var);

private:
	// Subcommands
	static void SubCommand_DBG_PAWS( CAOSMachine& vm );
	static void SubCommand_DBG_PLAY( CAOSMachine& vm );
	static void SubCommand_DBG_TOCK( CAOSMachine& vm );
	static void SubCommand_DBG_FLSH( CAOSMachine& vm );
	static void SubCommand_DBG_OUTS( CAOSMachine& vm );
	static void SubCommand_DBG_OUTV( CAOSMachine& vm );
	static void SubCommand_DBG_POLL( CAOSMachine& vm );
	static void SubCommand_DBG_PROF( CAOSMachine& vm );
	static void SubCommand_DBG_CPRO( CAOSMachine& vm );
	static void SubCommand_DBG_HTML( CAOSMachine& vm );
	static void SubCommand_DBG_ASRT( CAOSMachine& vm );
	static void SubCommand_DBG_WTIK( CAOSMachine& vm );
	static void SubCommand_DBG_TACK( CAOSMachine& vm );


#ifdef AGENT_PROFILER
	struct ProfileData
	{
		ProfileData() : updateTotalTime(0), updateTicks(0) {;};

		int64 updateTotalTime;
		int updateTicks;
	};
#endif

};



#endif	// DEBUGHANDLERS_H
