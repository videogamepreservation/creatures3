// Filename:    DisplayHandlers.h
// Class:       DisplayHandlers
// Purpose:     Routines to implement display commands/values in CAOS.
// Description:
//
// Usage:
//
// History: 06Apr99 Alima Created
// -------------------------------------------------------------------------

#ifndef DISPLAYHANDLERS_H
#define DISPLAYHANDLERS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../Display/System.h"
#include "../Agents/AgentHandle.h"
class CAOSMachine;
class Camera;

class DisplayHandlers
{
public:
	// Commands
	static void Command_LINE( CAOSMachine& vm );
	static void Command_CMRA( CAOSMachine& vm );
	static void Command_CMRP( CAOSMachine& vm );
	static void Command_CMRT( CAOSMachine& vm );
	static void Command_DMAP( CAOSMachine& vm );
	static void Command_TRCK( CAOSMachine& vm );
	static void Command_TRAN( CAOSMachine& vm );
	static void Command_ZOOM( CAOSMachine& vm );
	static void Command_TEXT( CAOSMachine& vm );
	static void Command_SNAP( CAOSMachine& vm );
	static void Command_WDOW(CAOSMachine& vm);

	// The Tint Commands :):)
	static void Command_WTNT( CAOSMachine& vm );
	static void Command_TNTW( CAOSMachine& vm );
	static void Command_TINT( CAOSMachine& vm );
	static void Command_FRSH(CAOSMachine& vm);

	// RValues
	static int IntegerRV_WNDT( CAOSMachine& vm );
	static int IntegerRV_WNDB( CAOSMachine& vm );
	static int IntegerRV_WNDL( CAOSMachine& vm );
	static int IntegerRV_WNDR( CAOSMachine& vm );
	static int IntegerRV_WNDW( CAOSMachine& vm );
	static int IntegerRV_WNDH( CAOSMachine& vm );
	static int IntegerRV_CMRX( CAOSMachine& vm );
	static int IntegerRV_CMRY( CAOSMachine& vm );
	static int IntegerRV_LOFT( CAOSMachine& vm );
	static int IntegerRV_SNAX( CAOSMachine& vm );
	static int IntegerRV_WDOW( CAOSMachine& vm );

	static AgentHandle AgentRV_TRCK( CAOSMachine& vm );

	// Helpers
	static void CameraHelper(CAOSMachine& vm, int32 gotox, int32 gotoy, int pan, bool bCentre, Camera* camera);

};

#endif
