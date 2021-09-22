// -------------------------------------------------------------------------
// Filename:    PortBundle.h
// Class:       PortBundle
// Purpose:     Helper class for agents to manage input and output ports
// Description:
//
// PortBundle has functions to create, destroy and query ports.
// It contained a collection InputPorts and a collection of OutputPorts.
//
// Authors:	BenC, Robert
// -------------------------------------------------------------------------

#ifndef PORTBUNDLE_H
#define PORTBUNDLE_H

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "../PersistentObject.h"

#include <vector>

class Agent;

class InputPort;
class OutputPort;


class PortBundle : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( PortBundle )
public:
	PortBundle();
	~PortBundle();

	void Trash();

	void Init( AgentHandle& owner );
	int GetInputPortCount()
	{
		return myInputs.size();
	}

	int GetOutputPortCount()
	{
		return myOutputs.size();
	}

	OutputPort* GetOutputPort( int i );
	InputPort* GetInputPort( int i );

	void CreateInputPort( int id, const char* name, const char* description,
		const Vector2D& relativePosition, int event );
	void CreateOutputPort( int id, const char* name, const char* description,
		const Vector2D& relativePosition );
	void ZapInputPort( int id );
	void ZapOutputPort( int id );

	void KillAllConnections();

	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);

private:
	AgentHandle myOwner;
	std::vector< InputPort* >	myInputs;
	std::vector< OutputPort* >	myOutputs;
};

#endif // PORTBUNDLE_H

