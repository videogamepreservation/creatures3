// -------------------------------------------------------------------------
// Filename:    CompoundHandlers.h
// Class:       CompoundHandlers
// Purpose:     Routines to manipulate compound-agent parts in CAOS
// Description:
//
// Usage:
//
// History:
// 11Feb98  BenC	Initial version
// -------------------------------------------------------------------------

#ifndef COMPOUNDHANDLERS_H
#define COMPOUNDHANDLERS_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif



class CAOSMachine;

class CompoundHandlers
{
public:
	// Commands
	static void Command_PAT( CAOSMachine& vm );
	static void Command_PTXT( CAOSMachine& vm );
	static void StringRV_PTXT( CAOSMachine& vm, std::string& str );
	static void Command_SCAM( CAOSMachine& vm );
	static void Command_FCUS( CAOSMachine& vm );
	static void Command_GRPL( CAOSMachine& vm );
	static void Command_GRPV( CAOSMachine& vm );
	static void Command_FRMT( CAOSMachine& vm );
	static int IntegerRV_NPGS( CAOSMachine& vm );
	static int IntegerRV_PAGE( CAOSMachine& vm );
	static void Command_PAGE( CAOSMachine& vm );

private:
	static void SubCommand_PAT_DULL( CAOSMachine& vm );
	static void SubCommand_PAT_BUTT( CAOSMachine& vm );
	static void SubCommand_PAT_TEXT( CAOSMachine& vm );
	static void SubCommand_PAT_FIXD( CAOSMachine& vm );
	static void SubCommand_PAT_CMRA( CAOSMachine& vm );
	static void SubCommand_PAT_GRPH( CAOSMachine& vm );
	static void SubCommand_PAT_KILL( CAOSMachine& vm );
};


#endif	// COMPOUNDHANDLERS_H

