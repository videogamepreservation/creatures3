// -------------------------------------------------------------------------
// Filename:    OutputPort.cpp
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
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "OutputPort.h"
#include "InputPort.h"
#include "../Message.h"
#include "../App.h"
#include "../World.h"

CREATURES_IMPLEMENT_SERIAL( OutputPort )


OutputPort::OutputPort( AgentHandle& owner, const char* name, const char* description,
	const Vector2D& relativePosition ) : Port( owner, name, description, relativePosition)
{
}

OutputPort::~OutputPort()
{
	if( !myListeners.empty() )
		KillAllConnections();
}




void OutputPort::RemoveListener( InputPort* inport )
{
	std::list< InputPort* >::iterator it;

#ifdef _DEBUG
	// paranoia - make sure inport appears once (and only once)
	int i=0;
	for( it = myListeners.begin(); it != myListeners.end(); ++it )
	{
		if( *it == inport )
			++i;
	}
	ASSERT( i==1 );
#endif // _DEBUG


	for( it = myListeners.begin(); it != myListeners.end(); ++it )
	{
		if( *it == inport )
		{
			myListeners.erase( it );
			return;
		}
	}
}


void OutputPort::Signal( CAOSVar& data )
{
	CAOSVar p2;
	std::list< InputPort* >::iterator it;
	for( it = myListeners.begin(); it != myListeners.end(); ++it )
	{
		(*it)->Excite(200);
		theApp.GetWorld().WriteMessage( GetOwner(),
			(*it)->GetOwner(),
			(*it)->GetMessageNumber(),
			data, p2, 0);
	}
}


void OutputPort::KillAllConnections()
{
	// Go through all connected inputs and tell them
	// to kill the connection.
	while( !myListeners.empty() )
		myListeners.front()->DisconnectFromOutputPort();
}



//virtual
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool OutputPort::Write(CreaturesArchive &ar) const
{
	std::list< InputPort* >::const_iterator it;

	if( !Port::Write( ar ) )
		return false;

	ar << (int)myListeners.size();
	for( it = myListeners.begin(); it != myListeners.end(); ++it )
		ar << (*it);

	return true;
}

//virtual
bool OutputPort::Read(CreaturesArchive &ar)
{

	int count;
	int i;
	InputPort* inp;
	
	int32 version = ar.GetFileVersion();
	if(version >= 3)
	{

		myListeners.clear();

		if( !Port::Read( ar ) )
			return false;

		ar >> count;

		// reserve() not supported by VC stl implementation?
	//	myListeners.reserve( count );
		for( i=0; i<count; ++i )
		{
			ar >> inp;
			myListeners.push_back( inp );
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}
