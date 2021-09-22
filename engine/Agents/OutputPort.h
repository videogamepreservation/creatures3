// -------------------------------------------------------------------------
// Filename:    OutputPort.h
// Class:       OutputPort
// Purpose:     A way to send signals to InputPorts
// Description:
//
// Each OutputPort has a list of 'listening' InputPorts. A signal is sent
// out to all listeners using the Signal() function.
// The InputPort - OutputPort connection is owned and maintained by the
// InputPort side. If the OutputPort needs to manipulate the connection
// in any way it must ask the InputPort to do all the dirty work
// (see KillAllConnections for an example)
//
// Authors:	BenC, Robert
// -------------------------------------------------------------------------
#ifndef OUTPUTPORT_H
#define OUTPUTPORT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Port.h"
#include <list>

class InputPort;
class CAOSVar;

class OutputPort : public Port
{
	CREATURES_DECLARE_SERIAL( OutputPort )
public:
	OutputPort() {};
	OutputPort( AgentHandle& owner, const char* name, const char* description,
		const Vector2D& relativePosition );
	virtual ~OutputPort();
	void AddListener( InputPort* inport )
	{
		myListeners.push_back( inport );
	}
	void RemoveListener( InputPort* inport );
	void Signal( CAOSVar& data );
	void KillAllConnections();

	const std::list< InputPort* >& GetListeners() const { return myListeners; }

	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);
private:
	std::list< InputPort* > myListeners;
};

#endif // OUTPUTPORT_H
