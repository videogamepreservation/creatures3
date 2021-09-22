#ifndef HISTORY_HANDLERS_H
#define HISTORY_HANDLERS_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "CAOSMachine.h"

#include <string>

class LifeEvent;
class CreatureHistory;

class HistoryHandlers
{
public:
	static void Command_HIST( CAOSMachine& vm );
	static int IntegerRV_HIST( CAOSMachine& vm );
	static void StringRV_HIST( CAOSMachine& vm, std::string& str );
	static int IntegerRV_OOWW( CAOSMachine& vm );

private:
	static void SubCommand_HIST_EVNT( CAOSMachine& vm );
	static void SubCommand_HIST_UTXT( CAOSMachine& vm );
	static void SubCommand_HIST_NAME( CAOSMachine& vm );
	static void SubCommand_HIST_WIPE( CAOSMachine& vm );
	static void SubCommand_HIST_FOTO( CAOSMachine& vm );

	static int IntegerRV_HIST_COUN( CAOSMachine& vm );
	static int IntegerRV_HIST_TYPE( CAOSMachine& vm );
	static int IntegerRV_HIST_WTIK( CAOSMachine& vm );
	static int IntegerRV_HIST_TAGE( CAOSMachine& vm );
	static int IntegerRV_HIST_RTIM( CAOSMachine& vm );
	static int IntegerRV_HIST_CAGE( CAOSMachine& vm );
	static int IntegerRV_HIST_GEND( CAOSMachine& vm );
	static int IntegerRV_HIST_GNUS( CAOSMachine& vm );
	static int IntegerRV_HIST_VARI( CAOSMachine& vm );
	static int IntegerRV_HIST_FIND( CAOSMachine& vm );
	static int IntegerRV_HIST_FINR( CAOSMachine& vm );
	static int IntegerRV_HIST_SEAN( CAOSMachine& vm );
	static int IntegerRV_HIST_TIME( CAOSMachine& vm );
	static int IntegerRV_HIST_YEAR( CAOSMachine& vm );
	static int IntegerRV_HIST_DATE( CAOSMachine& vm );
	static int IntegerRV_HIST_MUTE( CAOSMachine& vm );
	static int IntegerRV_HIST_CROS( CAOSMachine& vm );

	static void StringRV_HIST_MON1( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_MON2( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_UTXT( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_WNAM( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_WUID( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_NAME( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_NEXT( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_PREV( CAOSMachine& vm, std::string& str );
	static void StringRV_HIST_FOTO( CAOSMachine& vm, std::string& str );

	static LifeEvent& LifeEventHelper( CAOSMachine& vm );
	static CreatureHistory& CreatureHistoryHelper( CAOSMachine& vm );
};

#endif //HISTORY_HANDLERS_H
