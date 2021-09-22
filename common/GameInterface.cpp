#include "GameInterface.h"
#include "WhichEngine.h"
#include "../engine/C2eServices.h"

// solitary instantiation
GameInterface theGameInterface;


const int theClientBufferSize = 1024*1024;//gtb!temp!

GameInterface::GameInterface()
{
	m_bDynConnect = true;
	bMidInjection = false;
	myConnected = false;
}

bool GameInterface::Connect()
{
	myConnected = myClient.Open( theWhichEngine.ServerName().c_str() );
	return myConnected;
}

void GameInterface::Disconnect()
{
	myClient.Close();
	myConnected = false;
}


bool GameInterface::Inject(std::string Macro, std::string & ReplyBuffer, bool bAddExecute)
{
	ASSERT(!bMidInjection);
	bMidInjection = true;

	if (m_bDynConnect)
	{
		if (!Connect())
		{
			ReplyBuffer = "Initial connection failed";
			bMidInjection = false;
			return false;
		}
	}

	std::string AddExecute = (bAddExecute ? "execute\n" : "") + Macro;
	const char *m = AddExecute.c_str();
	if( !myClient.StartTransaction((unsigned char *)m, AddExecute.size() + 1 ) )
	{
		ReplyBuffer = "Start transaction failed";

		if (m_bDynConnect)
			Disconnect();

		bMidInjection = false;
		return false;
	}

	int code = myClient.GetReturnCode();
	char *result = (char*)myClient.GetResultBuffer();
	ReplyBuffer = std::string(result);
	myClient.EndTransaction();

	if (m_bDynConnect)
		Disconnect();

	bMidInjection = false;
	return (code == 0);
}


// -------------------------------------------------------------------------
// Method:		Dave's version of Inject
// Arguments:	macro string, reply
// Returns:		creatures reply
// Description:	sends a macro to creatures
// -------------------------------------------------------------------------
bool GameInterface::Inject(char *macro, char **outReply)
{
	if(!myConnected)
		return false;
 
	int result = IDRETRY;

	static char Buffer[theClientBufferSize];//gtb!temp!
	if (outReply!=NULL)
		*outReply = Buffer;

	if(!myClient.StartTransaction((unsigned char *)macro, strlen( macro ) + 1 ) ) {
		strcpy(Buffer, "Start transaction failed");
		return false;
	}

	if (myClient.GetReturnCode()) {
		myClient.EndTransaction();
		return false;
	}
	memcpy(Buffer, myClient.GetResultBuffer(), myClient.GetResultSize());//gtb!temp!
	myClient.EndTransaction();

	return true;	
}