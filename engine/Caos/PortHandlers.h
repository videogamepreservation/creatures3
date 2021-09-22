// -------------------------------------------------------------------------
// Filename:    PortHandlers.h
// Class:       PortHandlers
// Purpose:     Routines to implement agent input and output ports
// Description:
//
// Usage:
//
// History:
// 03Feb99  BenC	Initial version
// -------------------------------------------------------------------------


#ifndef PORTHANDLERS_H
#define PORTHANDLERS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
#include "../Agents/AgentHandle.h"

class CAOSMachine;

class PortHandlers
{
public:
	// Commands
	static void Command_PRT( CAOSMachine& vm );

	static void Command_ECON( CAOSMachine& vm );

	// RValues
	static int IntegerRV_PRT( CAOSMachine& vm );
	static void StringRV_PRT( CAOSMachine& vm, std::string& str );
	static AgentHandle AgentRV_PRT( CAOSMachine& vm );

private:
	static void SubCommand_PRT_INEW( CAOSMachine& vm );
	static void SubCommand_PRT_IZAP( CAOSMachine& vm );
	static void SubCommand_PRT_ONEW( CAOSMachine& vm );
	static void SubCommand_PRT_OZAP( CAOSMachine& vm );
	static void SubCommand_PRT_JOIN( CAOSMachine& vm );
	static void SubCommand_PRT_SEND( CAOSMachine& vm );
	static void SubCommand_PRT_BANG( CAOSMachine& vm );
	static void SubCommand_PRT_KRAK( CAOSMachine& vm );

	static int SubIntegerRV_PRT_FROM( CAOSMachine& vm );
	static int SubIntegerRV_PRT_ITOT( CAOSMachine& vm );
	static int SubIntegerRV_PRT_OTOT( CAOSMachine& vm );

	static void SubStringRV_PRT_NAME( CAOSMachine& vm, std::string& str );

	static AgentHandle SubAgentRV_PRT_FRMA( CAOSMachine& vm );
};



#endif	// PORTHANDLERS_H


