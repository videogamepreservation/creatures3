#ifndef GAME_INTERFACE_H
#define GAME_INTERFACE_H

typedef class GameInterface CGameInterface;

#include "clientside.h"
#include <string>

class GameInterface
{
public:
	GameInterface();
	void SetDynConnect(bool bDynConnect) { m_bDynConnect = bDynConnect; };
	bool Connect();
	void Disconnect();
	bool Inject(std::string Macro, std::string & ReplyBuffer, bool bAddExecute = true);

	// -------------------------------------------------------------------------
	// Method:		Connected
	// Arguments:	none
	// Returns:		whether the interface has is connected to creatures
	// -------------------------------------------------------------------------
	bool Connected(){return myConnected;};

	// -------------------------------------------------------------------------
	// Method:		Inject
	// Arguments:	macro string, reply
	// Returns:		creatures reply
	// Description:	sends a macro to creatures
	// -------------------------------------------------------------------------
	bool Inject(char *macro, char **outReply);

	// -------------------------------------------------------------------------
	// Method:		GetBufferSize
	// Arguments:	none
	// Returns:		myClients buffer size
	// -------------------------------------------------------------------------
	long GetBufferSize(){return myClient.GetBufferSize();};

private:
	ClientSide myClient;
	bool bMidInjection; // ASSERT if windows messaging should cause inject code to reenter before finishing
	bool myConnected;
	bool m_bDynConnect;
};

extern GameInterface theGameInterface;

#endif	//#ifndef GAME_INTERFACE_H
