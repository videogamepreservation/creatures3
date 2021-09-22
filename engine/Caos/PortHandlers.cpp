
// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>

#include "PortHandlers.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../AgentManager.h"
#include "../Agents/InputPort.h"
#include "../Agents/OutputPort.h"

// HelperFunction

void AddAgent(AgentHandle agent, std::vector<AgentHandle>& agents)
{
	// first off, ignore if agent is invalid.
	if (agent.IsInvalid())
		return;
	// Next add the agent iff it is not in the list already
	for(int i=0;i<agents.size(); i++)
		if (agents[i] == agent)
			return;
	agents.push_back(agent);
}

void ScanInputs(AgentHandle agent, std::vector<AgentHandle>& agents)
{
	// Here we scan all the input ports of "agent"
	PortBundle& pb = agent.GetAgentReference().GetPorts();
	int ipc = pb.GetInputPortCount();
	for(int i=0; i<ipc; i++)
	{
		InputPort* ip = pb.GetInputPort( i );
		if (ip == NULL)
			continue;
		OutputPort* op = ip->GetConnectedPort();
		if (op == NULL)
			continue;
		AddAgent(op->GetOwner(), agents);
	}
}


void ScanOutputs(AgentHandle agent, std::vector<AgentHandle>& agents)
{
	PortBundle& pb = agent.GetAgentReference().GetPorts();
	int opc = pb.GetOutputPortCount();
	for(int i=0; i<opc; i++)
	{
		OutputPort* op = pb.GetOutputPort( i );
		if (op == NULL)
			continue;
		const std::list< InputPort* >& ips = op->GetListeners();
		std::list< InputPort*>::const_iterator it;
		for(it = ips.begin(); it != ips.end(); it++)
		{
			InputPort* ip = *it;
			if (ip == NULL)
				continue;
			AddAgent(ip->GetOwner(), agents);
		}
	}
}

void ScanAgent(int& scanlocation, std::vector<AgentHandle>& agents)
{
	AgentHandle a = agents[scanlocation];
	scanlocation++;
	// Scan all input ports, reading the agent from them...
	ScanInputs(a,agents);
	// Scan all output ports, adding listener agents
	ScanOutputs(a,agents);
}

void PortHandlers::Command_ECON( CAOSMachine& vm )
{
	// Here we go :):)
	std::vector<AgentHandle> agents;
	int scanned = 0;

	int ipnext = vm.FetchInteger();	// get address of op _after_ the NEXT

	AgentHandle agent = vm.FetchAgentRV();
	if (agent.IsInvalid())
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	agents.push_back(agent);

	do
	{
		ScanAgent(scanned,agents);
	} while (scanned < agents.size());

	// Now push the whole caboodle on the stack and play ENUM :)
	for(int i=0; i< agents.size(); i++)
		vm.PushHandle( agents[i] );
	vm.Push( agents.size() );
	// store address of first command in body of block
	vm.Push( vm.GetIP() );
	// jump to the NEXT instruction
	vm.SetIP( ipnext  );
	

}

int PortHandlers::IntegerRV_PRT( CAOSMachine& vm )
{
	static IntegerRVHandler HandlerTable[] =
	{
		SubIntegerRV_PRT_ITOT,
		SubIntegerRV_PRT_OTOT,
		SubIntegerRV_PRT_FROM
	};

	int subcmd;
	subcmd = vm.FetchOp();
	return (HandlerTable[ subcmd ])( vm );

}

void PortHandlers::StringRV_PRT( CAOSMachine& vm, std::string& str )
{
	static StringRVHandler HandlerTable[] =
	{
		SubStringRV_PRT_NAME
	};
	int subcmd;
	subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm, str );
}

AgentHandle PortHandlers::AgentRV_PRT( CAOSMachine& vm )
{
	static AgentRVHandler HandlerTable[] =
	{
		SubAgentRV_PRT_FRMA
	};
	int subcmd;
	subcmd = vm.FetchOp();
	return (HandlerTable[ subcmd ])( vm );
}

AgentHandle PortHandlers::SubAgentRV_PRT_FRMA( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int input = vm.FetchIntegerRV();
	PortBundle& pb = vm.GetTarg().GetAgentReference().GetPorts();
	InputPort* ip = pb.GetInputPort( input );
	if (ip == NULL )
		return NULLHANDLE;
	OutputPort* op = ip->GetConnectedPort();
	if (op == NULL)
		return NULLHANDLE;
	return op->GetOwner();
}

int PortHandlers::SubIntegerRV_PRT_FROM( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int input = vm.FetchIntegerRV();
	PortBundle& pb = vm.GetTarg().GetAgentReference().GetPorts();
	InputPort* ip = pb.GetInputPort( input );
	if (ip == NULL)
		return -1;
	OutputPort* op = ip->GetConnectedPort();
	if (op == NULL)
		return -2;
	AgentHandle opposite = op->GetOwner();
	if (opposite.IsInvalid())
		return -3;
	// Okay then, we have an output port, let's scan the opposite's portbundle now...
	PortBundle& opb = opposite.GetAgentReference().GetPorts();
	int opc = opb.GetOutputPortCount();
	for (int i=0; i < opc; i++)
		if ( opb.GetOutputPort( i ) == op )
			return i;
	return -4;
}

void PortHandlers::SubStringRV_PRT_NAME( CAOSMachine& vm, std::string& str )
{
	AgentHandle agent = vm.FetchAgentRV();
	bool inport = (vm.FetchIntegerRV() == 0);
	int portnum = vm.FetchIntegerRV();

	if (agent.IsInvalid())
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	// Return "" unless all is well and good in the world :)
	str = "";

	PortBundle& pb = agent.GetAgentReference().GetPorts();
	Port* p = NULL;
	if (inport)
		p = pb.GetInputPort( portnum );
	else
		p = pb.GetOutputPort( portnum );

	if (p == NULL)
		return;
	str = p->GetName();
}

void PortHandlers::SubCommand_PRT_INEW( CAOSMachine& vm )
{
	int id;
	std::string name;
	std::string desc;
	int msgnum;
	int x,y;

	vm.ValidateTarg();
	id = vm.FetchIntegerRV();
	vm.FetchStringRV( name );
	vm.FetchStringRV( desc );
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();
	msgnum = vm.FetchIntegerRV();
	Vector2D relativePosition((float)x, (float)y);

	vm.GetTarg().GetAgentReference().GetPorts().CreateInputPort( id,
		name.c_str(), desc.c_str(), relativePosition, msgnum );
}

void PortHandlers::SubCommand_PRT_ONEW( CAOSMachine& vm )
{
	int id;
	std::string name;
	std::string desc;
	int x,y;

	vm.ValidateTarg();
	id = vm.FetchIntegerRV();
	vm.FetchStringRV( name );
	vm.FetchStringRV( desc );
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	Vector2D relativePosition((float)x, (float)y);
	vm.GetTarg().GetAgentReference().GetPorts().CreateOutputPort( id,
		name.c_str(), desc.c_str(), relativePosition );
}

void PortHandlers::SubCommand_PRT_IZAP( CAOSMachine& vm )
{
	int id;
	vm.ValidateTarg();
	id = vm.FetchIntegerRV();
	vm.GetTarg().GetAgentReference().GetPorts().ZapInputPort( id );
}

void PortHandlers::SubCommand_PRT_OZAP( CAOSMachine& vm )
{
	int id;
	vm.ValidateTarg();
	id = vm.FetchIntegerRV();
	vm.GetTarg().GetAgentReference().GetPorts().ZapOutputPort( id );
}

void PortHandlers::SubCommand_PRT_JOIN( CAOSMachine& vm )
{
	int outputid;
	int inputid;
	AgentHandle src;
	AgentHandle dest;
	OutputPort* outputport=NULL;
	InputPort* inputport=NULL;

	src = vm.FetchAgentRV();
	outputid = vm.FetchIntegerRV();

	if( outputid < src.GetAgentReference().GetPorts().GetOutputPortCount() )
		outputport = src.GetAgentReference().GetPorts().GetOutputPort( outputid );
	if( !outputport )
		vm.ThrowRunError( CAOSMachine::sidInvalidPortID );

	dest = vm.FetchAgentRV();
	inputid = vm.FetchIntegerRV();

	if( inputid < dest.GetAgentReference().GetPorts().GetInputPortCount() )
		inputport = dest.GetAgentReference().GetPorts().GetInputPort( inputid );
	if( !inputport )
		vm.ThrowRunError( CAOSMachine::sidInvalidPortID );

	inputport->ConnectToOutputPort( outputport );
}

void PortHandlers::SubCommand_PRT_SEND( CAOSMachine& vm )
{
	int id;
	
	OutputPort* outputport;

	vm.ValidateTarg();
	id = vm.FetchIntegerRV();
	CAOSVar data = vm.FetchGenericRV();
	outputport = vm.GetTarg().GetAgentReference().GetPorts().GetOutputPort( id );
	if( outputport )
		outputport->Signal( data );
	else 
	{
		// better error message?
		vm.ThrowRunError( CAOSMachine::sidInvalidPortID );
	}
}

void PortHandlers::SubCommand_PRT_BANG( CAOSMachine& vm )
{
	vm.ValidateTarg();

	int bangPower = vm.FetchIntegerRV();

	PortBundle &bundle = vm.GetTarg().GetAgentReference().GetPorts();
	int i;

	for( i = 0; i < bundle.GetInputPortCount(); ++i )
		if( Rnd(1, 100 ) < bangPower && bundle.GetInputPort( i ) )
			bundle.GetInputPort( i )->DisconnectFromOutputPort();

	for( i = 0; i < bundle.GetOutputPortCount(); ++i )
		if( Rnd(1, 100 ) < bangPower && bundle.GetOutputPort( i ) )
			bundle.GetOutputPort( i )->KillAllConnections();
}

void PortHandlers::SubCommand_PRT_KRAK( CAOSMachine& vm )
{
	vm.ValidateTarg();
	
	AgentHandle agent = vm.FetchAgentRV();

	int inorout = vm.FetchIntegerRV();
	int id = vm.FetchIntegerRV();

	PortBundle& bundle = agent.GetAgentReference().GetPorts();

	if (inorout == 0)
	{
		InputPort* input;
		input = bundle.GetInputPort( id );
		if (input)
			input->DisconnectFromOutputPort();
		else
			vm.ThrowRunError( CAOSMachine::sidInvalidPortID );
	}
	else
	{
		OutputPort* output;
		output = bundle.GetOutputPort( id );
		if (output)
			output->KillAllConnections();
		else
			vm.ThrowRunError( CAOSMachine::sidInvalidPortID );
	}

}

void PortHandlers::Command_PRT( CAOSMachine& vm )
{
	const int subcount=8;
	static CommandHandler HandlerTable[ subcount ] =
	{
		SubCommand_PRT_INEW,
		SubCommand_PRT_IZAP,
		SubCommand_PRT_ONEW,
		SubCommand_PRT_OZAP,
		SubCommand_PRT_JOIN,
		SubCommand_PRT_SEND,
		SubCommand_PRT_BANG,
		SubCommand_PRT_KRAK
	};

	int subcmd;

	subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );
}


int PortHandlers::SubIntegerRV_PRT_ITOT( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetPorts().GetInputPortCount();
}


int PortHandlers::SubIntegerRV_PRT_OTOT( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetPorts().GetOutputPortCount();
}


