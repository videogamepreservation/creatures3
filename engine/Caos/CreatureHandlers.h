// -------------------------------------------------------------------------
// Filename:    CreatureHandlers.h
// Class:       CreatureHandlers
// Purpose:     Routines to implement creature-related commands/r-values
// Description:
//
// Usage:
//
// History:
// 21Dec98  BenC	Initial version
// -------------------------------------------------------------------------

#ifndef CREATUREHANDLERS_H
#define CREATUREHANDLERS_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>

#include "../Agents/AgentHandle.h"

class CAOSMachine;

class CreatureHandlers
{

public:
	// Commands
	static void Command_DONE( CAOSMachine& vm );
	static void Command_WALK( CAOSMachine& vm );
	static void Command_GAIT( CAOSMachine& vm );
	static void Command_MVFT( CAOSMachine& vm );
	static void Command_DIRN( CAOSMachine& vm );
	static void Command_INJR( CAOSMachine& vm );
	static void Command_APPR( CAOSMachine& vm );
	static void Command_FLEE( CAOSMachine& vm );
	static void Command_TOUC( CAOSMachine& vm );
	static void Command_LTCY( CAOSMachine& vm );
	static void Command_MATE( CAOSMachine& vm );
	static void Command_WEAR( CAOSMachine& vm );
	static void Command_SAYN( CAOSMachine& vm );
	static void Command_BODY( CAOSMachine& vm );
	static void Command_NUDE( CAOSMachine& vm );
	static void Command_AGES( CAOSMachine& vm);
	static void Command_FACE(CAOSMachine& vm );
	static void Command_HAIR( CAOSMachine& vm);

	static void Command_DREA( CAOSMachine& vm );
	static int IntegerRV_DREA( CAOSMachine& vm );

	static void Command_LOCI( CAOSMachine& vm );
	static float FloatRV_LOCI( CAOSMachine& vm );

	static void Command_ASLP( CAOSMachine& vm );
	static int IntegerRV_ASLP( CAOSMachine& vm );

	static void Command_ZOMB( CAOSMachine& vm );
	static int IntegerRV_ZOMB( CAOSMachine& vm );

	static void Command_UNCS( CAOSMachine& vm );
	static int IntegerRV_UNCS( CAOSMachine& vm );

	static int IntegerRV_INS(  CAOSMachine& vm );
	static int IntegerRV_ATTN( CAOSMachine& vm );
	static int IntegerRV_DECN( CAOSMachine& vm );
	static int IntegerRV_BODY( CAOSMachine& vm );
	static int IntegerRV_EXPR( CAOSMachine& vm );
	static int IntegerRV_ORGN( CAOSMachine& vm );

	static void Command_STIM( CAOSMachine& vm );
	static void Command_URGE( CAOSMachine& vm );
	static void Command_SWAY( CAOSMachine& vm );
	static void Command_ORDR( CAOSMachine& vm );
	static void Command_NORN( CAOSMachine& vm );
	static void Command_VOCB( CAOSMachine& vm );
	static void Command_SPNL( CAOSMachine& vm );
	static void Command_NOHH( CAOSMachine& vm);


	enum {SHOU, SIGN, TACT, WRIT};

	static void Command_CHEM( CAOSMachine& vm );
	static void Command_DRIV( CAOSMachine& vm );

	static void Command_BRN( CAOSMachine& vm );
	static void SubCommand_BRN_DMPB( CAOSMachine& vm );
	static void SubCommand_BRN_SETN( CAOSMachine& vm );
	static void SubCommand_BRN_SETD( CAOSMachine& vm );
	static void SubCommand_BRN_SETL( CAOSMachine& vm );
	static void SubCommand_BRN_SETT( CAOSMachine& vm );
	static void SubCommand_BRN_DMPL( CAOSMachine& vm );
	static void SubCommand_BRN_DMPT( CAOSMachine& vm );
	static void SubCommand_BRN_DMPN( CAOSMachine& vm );
	static void SubCommand_BRN_DMPD( CAOSMachine& vm );

	static void Command_GENE( CAOSMachine& vm );
	static void SubCommand_GENE_CROS( CAOSMachine& vm );
	static void SubCommand_GENE_MOVE( CAOSMachine& vm );
	static void SubCommand_GENE_KILL( CAOSMachine& vm );
	static void SubCommand_GENE_LOAD( CAOSMachine& vm );
	static void SubCommand_GENE_CLON( CAOSMachine& vm );

	static void Command_BORN( CAOSMachine& vm );
	static void Command_DEAD( CAOSMachine& vm );
	static void Command_FORF( CAOSMachine& vm );
	static void Command_LIKE( CAOSMachine& vm );

	// Integer r-values
	static int IntegerRV_DEAD( CAOSMachine& vm );
	static int IntegerRV_DIRN( CAOSMachine& vm );
	static int IntegerRV_FACE( CAOSMachine& vm );
	static int IntegerRV_BYIT( CAOSMachine& vm );
	static int IntegerRV_BVAR( CAOSMachine& vm );
	static int IntegerRV_CAGE( CAOSMachine& vm );
	static int IntegerRV_CREA( CAOSMachine& vm );
	static int IntegerRV_DRV( CAOSMachine& vm );
	static int IntegerRV_CATI( CAOSMachine& vm );
	static int IntegerRV_TAGE( CAOSMachine& vm );
	static int IntegerRV_ORGI( CAOSMachine& vm );

	// Float r-values
	static float FloatRV_CHEM( CAOSMachine& vm );
	static float FloatRV_DRIV( CAOSMachine& vm );
	static float FloatRV_DFTX( CAOSMachine& vm );
	static float FloatRV_DFTY( CAOSMachine& vm );
	static float FloatRV_UFTX( CAOSMachine& vm );
	static float FloatRV_UFTY( CAOSMachine& vm );
	static float FloatRV_NOUN( CAOSMachine& vm );
	static float FloatRV_VERB( CAOSMachine& vm );
	static float FloatRV_MTHX( CAOSMachine& vm );
	static float FloatRV_MTHY( CAOSMachine& vm );
	static float FloatRV_ORGF( CAOSMachine& vm );

	// String r-values
	static void StringRV_FACE( CAOSMachine& vm, std::string& str );
	static void StringRV_GTOS( CAOSMachine& vm, std::string& str );
	static void StringRV_CATX( CAOSMachine& vm, std::string& str );

	// Agent r-values
	static AgentHandle AgentRV_IITT( CAOSMachine& vm );
	static AgentHandle AgentRV_MTOC( CAOSMachine& vm );
	static AgentHandle AgentRV_MTOA( CAOSMachine& vm );
	static AgentHandle AgentRV_HHLD( CAOSMachine& vm );


protected:
	static void NavigateDirection(Creature& c, int mapDirection, int caIndex);
};


#endif	// CREATUREHANDLERS_H


