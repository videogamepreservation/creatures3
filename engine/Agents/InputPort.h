// -------------------------------------------------------------------------
// Filename:    InputPort.h
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
#ifndef INPUTPORT_H
#define INPUTPORT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Port.h"

class OutputPort;

class InputPort	: public Port
{
	CREATURES_DECLARE_SERIAL( InputPort )
public:
	InputPort() {};
	InputPort( AgentHandle& owner, const char* name, const char* desc,
		const Vector2D& relativePosition, int msgid );
	virtual ~InputPort();
	void ConnectToOutputPort( OutputPort* source );
	void DisconnectFromOutputPort();

	OutputPort* GetConnectedPort()
	{
		return myConnectedPort;
	}
	int GetMessageNumber() const
	{ 
		return myMessageNumber; 
	}

	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);

	int GetExcitementLevel() 
	{ 
		return myExcitementLevel; 
	}
	
	void Excite(int toThis) 
	{ 
		myExcitementLevel = toThis; 
	}
	
	void Relax(int byThis) 
	{ 
		if (myExcitementLevel > 0) 
			myExcitementLevel -= byThis; 
		else
			myExcitementLevel = 0;
	}

private:
	// message # should be sent to this script upon signal
	int			myMessageNumber;
	int			myExcitementLevel;
	OutputPort*	myConnectedPort;
};

#endif // INPUTPORT_H
