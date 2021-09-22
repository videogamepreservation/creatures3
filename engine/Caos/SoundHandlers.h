// Filename:    SoundHandlers.h
// Class:       SoundHandlers
// Purpose:     Routines to implement sound commands/values in CAOS.
// Description:
//
// Usage:
//
// History: 31Mar99 Alima Created
// -------------------------------------------------------------------------


#ifndef SOUNDHANDLERS_H
#define SOUNDHANDLERS_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include <string>


class CAOSMachine;

class SoundHandlers
{
public:
	// Commands
	static void Command_RMSC( CAOSMachine& vm );
	static void Command_MMSC( CAOSMachine& vm );
	static void Command_MIDI( CAOSMachine& vm );
	static void Command_RCLR( CAOSMachine& vm );
	static void Command_MCLR( CAOSMachine& vm );
	static void Command_SNDE( CAOSMachine& vm );
	static void Command_SNDQ( CAOSMachine& vm );
	static void Command_SNDC( CAOSMachine& vm );
	static void Command_SNDL( CAOSMachine& vm );
	static void Command_STPC( CAOSMachine& vm );
	static void Command_FADE( CAOSMachine& vm );
	static void Command_VOLM(CAOSMachine& vm );

	// String r-values
	static void StringRV_MMSC( CAOSMachine& vm, std::string& str );
	static void StringRV_RMSC( CAOSMachine& vm, std::string& str );

private:
	// Helpers
	static void CheckSoundFileExists( CAOSMachine& vm, const std::string& file );
};



#endif	// GENERALHANDLERS_H
