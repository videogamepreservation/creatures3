// Filename:    GeneralHandlers.h
// Class:       GeneralHandlers
// Purpose:     Routines to implement general commands/values in CAOS.
// Description:
//
// Usage:
//
// History:
// -------------------------------------------------------------------------


#ifndef GENERALHANDLERS_H
#define GENERALHANDLERS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


class CAOSMachine;

class GeneralHandlers
{
public:
	// 
	static void Command_ADDV( CAOSMachine& vm );
	static void Command_SUBV( CAOSMachine& vm );
	static void Command_MULV( CAOSMachine& vm );
	static void Command_DIVV( CAOSMachine& vm );
	static void Command_MODV( CAOSMachine& vm );
	static void Command_NEGV( CAOSMachine& vm );
	static void Command_ABSV( CAOSMachine& vm );
	static void Command_ANDV( CAOSMachine& vm );
	static void Command_ORRV( CAOSMachine& vm );
	static void Command_SETV( CAOSMachine& vm );
	static void Command_PSWD( CAOSMachine& vm);

	// flow control stuff
	static void Command_DOIF( CAOSMachine& vm );
	static void Command_ELIF( CAOSMachine& vm );
	static void Command_ELSE( CAOSMachine& vm );
	static void Command_ENDI( CAOSMachine& vm );
	static void Command_ENUM( CAOSMachine& vm );
	static void Command_ESEE( CAOSMachine& vm );
	static void Command_ETCH( CAOSMachine& vm );
	static void Command_NEXT( CAOSMachine& vm );
	static void Command_REPS( CAOSMachine& vm );
	static void Command_REPE( CAOSMachine& vm );
	static void Command_LOOP( CAOSMachine& vm );
	static void Command_UNTL( CAOSMachine& vm );
	static void Command_EVER( CAOSMachine& vm );
	static void Command_SUBR( CAOSMachine& vm );
	static void Command_GOTO( CAOSMachine& vm );
	static void Command_GSUB( CAOSMachine& vm );
	static void Command_RETN( CAOSMachine& vm );
	static void Command_INST( CAOSMachine& vm );
	static void Command_SLOW( CAOSMachine& vm );
	static void Command_STOP( CAOSMachine& vm );
	static void Command_WAIT( CAOSMachine& vm );

	static void Command_GIDS( CAOSMachine& vm );
	static void Command_OUTS( CAOSMachine& vm );
	static void Command_OUTX( CAOSMachine& vm );
	static void Command_OUTV( CAOSMachine& vm );
	static void Command_SETS( CAOSMachine& vm );
	static void Command_ADDS( CAOSMachine& vm );
	static void Command_SCRX( CAOSMachine& vm );
	static void Command_SETA( CAOSMachine& vm );
	static void Command_CHAR( CAOSMachine& vm );
	static void Command_DELG( CAOSMachine& vm );
	static void Command_SAVE( CAOSMachine& vm );
	static void Command_LOAD( CAOSMachine& vm );
	static void Command_COPY( CAOSMachine& vm );
	static void Command_DELW( CAOSMachine& vm );
	static void Command_QUIT( CAOSMachine& vm );
	static void Command_WRLD( CAOSMachine& vm );
	static void Command_WPAU( CAOSMachine& vm );
	static void Command_FILE( CAOSMachine& vm );
	static void Command_RGAM( CAOSMachine& vm );
	static void Command_REAF( CAOSMachine& vm );
	static void Command_STRK( CAOSMachine& vm );
	// Integer r-values
	static int IntegerRV_RAND( CAOSMachine& vm );
	static int IntegerRV_KEYD( CAOSMachine& vm );
	static int IntegerRV_MOPX( CAOSMachine& vm );
	static int IntegerRV_MOPY( CAOSMachine& vm );
	static int IntegerRV_LEFT( CAOSMachine& vm );
	static int IntegerRV_RGHT( CAOSMachine& vm );
	static int IntegerRV_UP( CAOSMachine& vm );
	static int IntegerRV_DOWN( CAOSMachine& vm );
	static int IntegerRV_STRL( CAOSMachine& vm );
	static int IntegerRV_CHAR( CAOSMachine& vm );
	static int IntegerRV_TIME( CAOSMachine& vm );
	static int IntegerRV_YEAR( CAOSMachine& vm );
	static int IntegerRV_SEAN( CAOSMachine& vm );
	static int IntegerRV_WTIK( CAOSMachine& vm );
	static int IntegerRV_ETIK( CAOSMachine& vm );
	static int IntegerRV_NWLD( CAOSMachine& vm );
	static int IntegerRV_VMNR( CAOSMachine& vm );
	static int IntegerRV_VMJR( CAOSMachine& vm );
	static int IntegerRV_WPAU( CAOSMachine& vm );
	static int IntegerRV_STOI( CAOSMachine& vm );
	static int IntegerRV_RTIM( CAOSMachine& vm );
	static int IntegerRV_REAN( CAOSMachine& vm );
	static int IntegerRV_REAQ( CAOSMachine& vm );
	static int IntegerRV_DATE( CAOSMachine& vm );
	static int IntegerRV_INNI( CAOSMachine& vm );
	static int IntegerRV_INOK( CAOSMachine& vm );
	static int IntegerRV_DAYT( CAOSMachine& vm );
	static int IntegerRV_MONT( CAOSMachine& vm );
	static int IntegerRV_FTOI( CAOSMachine& vm );
	static int IntegerRV_MUTE( CAOSMachine& vm );
	static int IntegerRV_WOLF( CAOSMachine& vm );
	static int IntegerRV_RACE( CAOSMachine& vm );
	static int IntegerRV_SCOL( CAOSMachine& vm );
	static int IntegerRV_SORQ( CAOSMachine& vm );
	static int IntegerRV_MSEC( CAOSMachine& vm );
	static int IntegerRV_WNTI( CAOSMachine& vm );

	// Float r-values
	static float FloatRV_MOVX( CAOSMachine& vm );
	static float FloatRV_MOVY( CAOSMachine& vm );
	static float FloatRV_SIN( CAOSMachine& vm );	// SIN_
	static float FloatRV_COS( CAOSMachine& vm );	// COS_
	static float FloatRV_TAN( CAOSMachine& vm );	// TAN_
	static float FloatRV_ASIN( CAOSMachine& vm );
	static float FloatRV_ACOS( CAOSMachine& vm );
	static float FloatRV_ATAN( CAOSMachine& vm );
	static float FloatRV_PACE( CAOSMachine& vm );
	static float FloatRV_STOF( CAOSMachine& vm );
	static float FloatRV_INNF( CAOSMachine& vm );
	static float FloatRV_ITOF( CAOSMachine& vm );
	static float FloatRV_SQRT( CAOSMachine& vm );

	// String r-values
	static void StringRV_VTOS( CAOSMachine& vm, std::string& str );
	static void StringRV_SORC( CAOSMachine& vm, std::string& str );
	static void StringRV_READ( CAOSMachine& vm, std::string& str );
	static void StringRV_SUBS( CAOSMachine& vm, std::string& str );
	static void StringRV_BUTY( CAOSMachine& vm, std::string& str );
	static void StringRV_WILD( CAOSMachine& vm, std::string& str );
	static void StringRV_WRLD( CAOSMachine& vm, std::string& str );
	static void StringRV_CAOS( CAOSMachine& vm, std::string& str );
	static void StringRV_WNAM( CAOSMachine& vm, std::string& str );
	static void StringRV_GNAM( CAOSMachine& vm, std::string& str );
	static void StringRV_PSWD( CAOSMachine& vm, std::string& str );
	static void StringRV_WUID( CAOSMachine& vm, std::string& str );
	static void StringRV_RTIF( CAOSMachine& vm, std::string& str );
	static void StringRV_INNL( CAOSMachine& vm, std::string& str );
	static void StringRV_GAMN( CAOSMachine& vm, std::string& str );
	static void StringRV_FVWM( CAOSMachine& vm, std::string& str );

	// Variables
	static CAOSVar& Variable_GAME( CAOSMachine& vm );

private:
	// Subcommands
	static void SubCommand_GIDS_ROOT( CAOSMachine& vm );
	static void SubCommand_GIDS_FMLY( CAOSMachine& vm );
	static void SubCommand_GIDS_GNUS( CAOSMachine& vm );
	static void SubCommand_GIDS_SPCS( CAOSMachine& vm );

	static void SubCommand_FILE_OOPE( CAOSMachine& vm );
	static void SubCommand_FILE_OCLO( CAOSMachine& vm );
	static void SubCommand_FILE_OFLU( CAOSMachine& vm );
	static void SubCommand_FILE_IOPE( CAOSMachine& vm );
	static void SubCommand_FILE_ICLO( CAOSMachine& vm );
	static void SubCommand_FILE_JDEL( CAOSMachine& vm );

	// Helpers
	static void MakeFilenameSafe(std::string& filename);

};



#endif	// GENERALHANDLERS_H


