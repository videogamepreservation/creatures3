// -------------------------------------------------------------------------
// Filename:    InputPort.cpp
// Class:       InputPort
// Purpose:     Accepts incoming signals from a connected OutputPort and
//				runs the appropriate script on the agent.
// Description:
//
// An InputPort may be connected to zero or one OutputPorts. When the
// OutputPort sends a signal, the InputPort deals with it by executing
// a script upon the agent it belongs to.
// Each InputPort is responsible for the connection between itself and
// an OutputPort - ie only the InputPort side may create or destroy
// connections.
//
// Authors:	BenC, Robert
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "InputPort.h"
#include "OutputPort.h"
#include "Agent.h"


CREATURES_IMPLEMENT_SERIAL( InputPort )


InputPort::InputPort( AgentHandle& owner, const char* name, const char* desc,
	const Vector2D& relativePosition, int msgid )
	:Port( owner,name,desc,relativePosition )
{
	myMessageNumber = msgid;
	myConnectedPort = NULL;
	myExcitementLevel = 0;
}


InputPort::~InputPort()
{
	if( myConnectedPort )
		DisconnectFromOutputPort();
}


void InputPort::ConnectToOutputPort( OutputPort* source )
{
	if( myConnectedPort )
		DisconnectFromOutputPort();

	myConnectedPort = source;
	source->AddListener( this );
}

void InputPort::DisconnectFromOutputPort()
{
	if( myConnectedPort )
	{
		myConnectedPort->RemoveListener( this );
		myConnectedPort = NULL;
	}
}



//virtual
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool InputPort::Write(CreaturesArchive &ar) const
{
	if( !Port::Write( ar ) )
		return false;
	ar << myMessageNumber;
	ar << myConnectedPort;
	ar << myExcitementLevel;
	return true;
}


//virtual
bool InputPort::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();
	if(version >= 3)
	{
		if( !Port::Read( ar ) )
			return false;
		ar >> myMessageNumber;
		ar >> myConnectedPort;
		ar >> myExcitementLevel;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}
