// -------------------------------------------------------------------------
// Filename:    PortBundle.cpp
// Class:       PortBundle
// Purpose:     Helper class for agents to manage input and output ports
// Description:
//
// PortBundle has functions to create, destroy and query ports.
// It contains a collection InputPorts and a collection of OutputPorts.
//
// Authors:	BenC, Robert
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "PortBundle.h"
#include "InputPort.h"
#include "OutputPort.h"
#include "../General.h"

CREATURES_IMPLEMENT_SERIAL( PortBundle )

PortBundle::PortBundle()
{
	myOwner = NULLHANDLE;
}

PortBundle::~PortBundle()
{
	DeleteObjects( myInputs.begin(), myInputs.end() );
	DeleteObjects( myOutputs.begin(), myOutputs.end() );
}

void PortBundle::Init( AgentHandle& owner )
{
	myOwner = owner;
}



OutputPort* PortBundle::GetOutputPort( int id )
{
	if( id < myOutputs.size() )
		return myOutputs[id];
	else
		return NULL;
}

InputPort* PortBundle::GetInputPort( int id )
{
	if( id < myInputs.size() )
		return myInputs[id];
	else
		return NULL;
}

void PortBundle::CreateInputPort( int id, const char* name, const char* description,
	const Vector2D& relativePosition, int msgid )
{
	ASSERT( myOwner.IsValid() );

	while( myInputs.size() <= id )
		myInputs.push_back( NULL );

	// kill any existing port with that id
	if( myInputs[id] )
		delete myInputs[id];

	myInputs[id] = new InputPort( myOwner, name, description, relativePosition, msgid );
}

void PortBundle::CreateOutputPort( int id, const char* name, const char* description,
	const Vector2D& relativePosition )
{
	ASSERT( myOwner.IsValid() );

	while( myOutputs.size() <= id )
		myOutputs.push_back( NULL );

	// kill any existing port with that id
	if( myOutputs[id] )
		delete myOutputs[id];

	myOutputs[id] = new OutputPort( myOwner, name, description, relativePosition );
}

void PortBundle::ZapInputPort( int id )
{
	if( myInputs[id] != NULL )
	{
		delete myInputs[id];
		myInputs[id] = NULL;
	}

	// trim trailing empties
	while( myInputs.back() == NULL )
		myInputs.pop_back();
}

void PortBundle::ZapOutputPort( int id )
{
	if( myOutputs[id] != NULL )
	{
		delete myOutputs[id];
		myOutputs[id] = NULL;

	}

	// trim trailing empties
	while( myOutputs.back() == NULL )
		myOutputs.pop_back();
}

// virtual
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool PortBundle::Write(CreaturesArchive &ar) const
{
	ar << myInputs;
	ar << myOutputs;
	return true;
}

// virtual
bool PortBundle::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();
	if(version >= 3)
	{
		ar >> myInputs;
		ar >> myOutputs;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}

void PortBundle::KillAllConnections()
{
	{
		int n = myInputs.size();
		for (int i = 0; i < n; ++i)
			if (myInputs[i])
				myInputs[i]->DisconnectFromOutputPort();
	}
	{
		int n = myOutputs.size();
		for (int i = 0; i < n; ++i)
			if (myOutputs[i])
				myOutputs[i]->KillAllConnections();
	}
}


void PortBundle::Trash()
{
	myOwner = NULLHANDLE;
	{
		int n = myInputs.size();
		for (int i = 0; i < n; ++i)
			if (myInputs[i])
				myInputs[i]->Trash();
	}
	{
		int n = myOutputs.size();
		for (int i = 0; i < n; ++i)
			if (myOutputs[i])
				myOutputs[i]->Trash();
	}
}
