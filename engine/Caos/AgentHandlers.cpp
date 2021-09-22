// -------------------------------------------------------------------------
// Filename:    AgentHandlers.cpp
// Class:       AgentHandlers
// Purpose:     Routines to implement agent-related commands/values in CAOS.
// Description:
//
// Usage:
//
// History:
// 15Dec98  BenC	Initial version
// 21Jan99	Alima	Made changes to make sure that that the AgentManager
//					creates all agents and handles all calls to the 
//					agentIDMap.a
//
// TODO: Fix up evil std::vector usage in ANMS
//
// -------------------------------------------------------------------------

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>
#ifndef C2E_OLD_CPP_LIB
#include <sstream>
#endif

#include <strstream>

#include "AgentHandlers.h"
#include "../App.h"
#include "../World.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../AgentManager.h"
#include "../Creature/Creature.h"
#include "../Display/ErrorMessageHandler.h"
#include "../Display/MainCamera.h"
#include "Lexer.h"
#include "../Agents/Vehicle.h"
#include "../Creature/voice.h"

#ifndef _WIN32
#include "../unix/FileFuncs.h"
#endif

void AgentHandlers::Command_VOIS( CAOSMachine& vm)
{
	std::string str;
	vm.FetchStringRV(str);
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().GetVoice().ReadData(str);
}

void AgentHandlers::Command_VOIC( CAOSMachine& vm )
{
	int genus, gender, age;
	genus = vm.FetchIntegerRV();
	gender = vm.FetchIntegerRV();
	age = vm.FetchIntegerRV();
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().GetVoice().ReadData(genus,gender,age);
}

void AgentHandlers::StringRV_VOIS( CAOSMachine& vm, std::string& str )
{
	vm.ValidateTarg();
	str = vm.GetTarg().GetAgentReference().GetVoice().GetCurrentVoice();
}

void AgentHandlers::Command_SEZZ( CAOSMachine& vm )
{
	std::string str;
	vm.FetchStringRV(str);
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().SpeakSentence(str);
}

void AgentHandlers::SubCommand_NEW_SIMP( CAOSMachine& vm )
{
	std::string gallery;
	std::string filename;
	int imagecount, firstimage, plane;
	int family, genus, species;

	family = vm.FetchIntegerRV();
	genus = vm.FetchIntegerRV();
	species = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	imagecount = vm.FetchIntegerRV();	// Number of images.
	firstimage = vm.FetchIntegerRV();	// Image number.
	plane = vm.FetchIntegerRV();	// Plane.

	AgentHandle agent(theAgentManager.CreateSimpleAgent(family, genus, species,
		FilePath(gallery + ".s16", IMAGES_DIR),
		imagecount, firstimage, plane));
	vm.SetTarg( agent );
}

void AgentHandlers::SubCommand_NEW_VHCL( CAOSMachine& vm )
{
	std::string gallery;
	int imagecount, firstimage, plane;
	
	int family = vm.FetchIntegerRV();
	int genus = vm.FetchIntegerRV();
	int species = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	imagecount = vm.FetchIntegerRV();	// Number of images.
	firstimage = vm.FetchIntegerRV();	// Image number.
	plane = vm.FetchIntegerRV();	// Plane.

	AgentHandle v = theAgentManager.CreateVehicle(
		family, genus, species,
		FilePath( gallery + ".s16", IMAGES_DIR ),
		imagecount,
		firstimage,
		plane);

	// default to doing poses on the zero part
	vm.SetPart( 0 );
	vm.SetTarg( v );
}

void AgentHandlers::SubCommand_NEW_COMP( CAOSMachine& vm )
{
	std::string gallery;

	int imagecount, firstimage, plane;
	int family = vm.FetchIntegerRV();
	int genus = vm.FetchIntegerRV();
	int species = vm.FetchIntegerRV();
	vm.FetchStringRV( gallery );
	imagecount = vm.FetchIntegerRV();	// Number of images.
	firstimage = vm.FetchIntegerRV();	// Image number.
	plane = vm.FetchIntegerRV();	// Plane number.

	AgentHandle agent = theAgentManager.CreateCompoundAgent
		(family, genus, species,
		FilePath(gallery + ".s16", IMAGES_DIR),
		imagecount,
		firstimage,
		plane);

	// default to doing poses on the zero part
	vm.SetPart( 0 );
	vm.SetTarg( agent );
}

void AgentHandlers::SubCommand_NEW_CREA( CAOSMachine& vm )
{
	int max=0;
	int sizeInMB =0;

	AgentHandle agent; 

	int family = vm.FetchIntegerRV();

	AgentHandle gene = vm.FetchAgentRV();
	int gene_index = vm.FetchIntegerRV();

	int sex = vm.FetchIntegerRV();
	int variant = vm.FetchIntegerRV();

	if (!gene.IsValid())
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );	

	if (gene_index < 1 ||
		gene.GetAgentReference().GetGenomeStore().MonikerAsString(gene_index).empty())
	{
		vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	
	}
	
	agent = theAgentManager.CreateCreature(family, gene, gene_index, sex, variant);

	vm.SetTarg( agent );

	while (!vm.GetTarg().GetCreatureReference().AreYourBodyPartsFullyFormed())
	{
		if(!vm.GetTarg().GetCreatureReference().FormBodyParts())
			vm.ThrowRunError( CAOSMachine::sidFailedToCreateCreature );
	}

	theAgentManager.AddCreatureToUpdateList(vm.GetTarg());
	
}



void AgentHandlers::Command_NEW( CAOSMachine& vm )
{
	const int NEW_SUBCOUNT=4;

	static CommandHandler HandlerTable[ NEW_SUBCOUNT ] =
	{
		SubCommand_NEW_SIMP,
		SubCommand_NEW_COMP,
		SubCommand_NEW_VHCL,
		SubCommand_NEW_CREA,
	};

	int subcmd;

	subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );
}



void AgentHandlers::Command_KILL( CAOSMachine& vm )
{
	AgentHandle agent(vm.FetchAgentRV());
	if (agent.IsInvalid())
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	bool suicide = (vm.GetOwner() == agent);

	// We can't zap the hand
	if( agent != thePointer)
	{
		if (suicide)
		{
			agent.GetAgentReference().YouAreNowDoomed();
			vm.StopScriptExecuting();
		}
		else
			theAgentManager.KillAgent(agent);
	}
}


void AgentHandlers::Command_BASE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int newBase = vm.FetchIntegerRV();
	if (!vm.GetTarg().GetAgentReference().SetBaseImage(newBase, vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidBaseOrPart, newBase, vm.GetPart() );
}

int AgentHandlers::IntegerRV_BASE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetBaseImage(vm.GetPart());
}

void AgentHandlers::Command_POSE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int newPose = vm.FetchIntegerRV();
	if (!vm.GetTarg().GetAgentReference().ShowPose( newPose, vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPoseOrPart, newPose, vm.GetPart(),
						  vm.GetTarg().GetAgentReference().GetBaseImage(vm.GetPart()));
}

void AgentHandlers::Command_ANIM( CAOSMachine& vm )
{
	int count;
	const unsigned char* p;

	vm.ValidateTarg();
	count = vm.FetchInteger();
	p = (const unsigned char*)vm.FetchRawData( count, 1 );

	if (!vm.GetTarg().GetAgentReference().SetAnim( p, count, vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidAnimOrPart, vm.GetPart(),
						  vm.GetTarg().GetAgentReference().GetBaseImage(vm.GetPart()));
}

void AgentHandlers::Command_FRAT( CAOSMachine& vm )
{
	int rate;
	vm.ValidateTarg();
	rate = vm.FetchIntegerRV();
	if (rate<1 || rate > 255)
		vm.ThrowRunError( CAOSMachine::sidFrameRateOutOfRange, rate );
	if (!vm.GetTarg().GetAgentReference().ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );
	vm.GetTarg().GetAgentReference().SetFrameRate( rate, vm.GetPart() );
}

void AgentHandlers::Command_ANMS( CAOSMachine& vm )
{
	std::vector<uint8> animString;
	
	vm.ValidateTarg();
	std::string str;
	vm.FetchStringRV(str);

	Lexer l;
	l.Attach(str.c_str());

	int toktype,val;
	toktype = l.NextItem();
	if ( toktype != Lexer::itemInteger )
	{
		vm.ThrowRunError( CAOSMachine::sidInvalidStringForAnms, str.c_str() );
	}

	do 
	{
		switch (toktype)
		{
		case Lexer::itemInteger:
			val = l.GetIntegerValue();
			if ((val < 0) || (val > 255))
			{
				vm.ThrowRunError( CAOSMachine::sidOutOfRangeForAnms );
			}
			animString.push_back((uint8)val);
			break;
		default:
			vm.ThrowRunError( CAOSMachine::sidInvalidStringForAnms, str.c_str() );
		}
		toktype = l.NextItem();
	} while ( toktype != Lexer::itemFinished );
	
	//Parse was successful, so let's handle the SetAnim process :)

	// TODO: Do this code another way.
	// THIS IS EVIL! EVIL EVIL EVIL!
	// 
	// EEEEEEEEVVVVVVVIIIIIIIILLLLLL!!!
	//
	// YOU SHOULDN'T USE A VECTOR IN THIS WAY!
	// (That said, it'll probably work under most implementations :-)
	// -BenC
	const uint8* p = &(animString[0]);

	if (!vm.GetTarg().GetAgentReference().SetAnim( p, animString.size(), vm.GetPart() ))
	{
		vm.ThrowRunError( CAOSMachine::sidInvalidAnimStringOrPart, str.c_str(), vm.GetPart(),
						  vm.GetTarg().GetAgentReference().GetBaseImage(vm.GetPart()));
	}
}

void AgentHandlers::Command_OVER( CAOSMachine& vm )
{
	vm.ValidateTarg();
	if (!vm.GetTarg().GetAgentReference().ValidatePart( vm.GetPart() ))
		vm.ThrowRunError( CAOSMachine::sidInvalidPart, vm.GetPart() );
	if( vm.GetTarg().GetAgentReference().AnimOver( vm.GetPart() ) )
		vm.UnBlock();
	else
		vm.Block();
}



void AgentHandlers::Command_MVSF( CAOSMachine& vm )
{
	vm.ValidateTarg();
	float preferredX = vm.FetchFloatRV();
	float preferredY = vm.FetchFloatRV();
	Agent& agentref = vm.GetTarg().GetAgentReference();	
	int status = agentref.GetMovementStatus();

	if (status != Agent::AUTONOMOUS)
		vm.ThrowRunError( CAOSMachine::sidInvalidMVSF );
		
	bool ok = agentref.MoveToSafePlace(Vector2D(preferredX,preferredY));
	if (!ok)
		vm.ThrowRunError( CAOSMachine::sidFailedToFindSafeLocation,
			preferredX,preferredY);

	// it's now in a valid position (agents when they're
	// constructed aren't)
	agentref.MarkValidPositionNow();
}


void AgentHandlers::Command_MVTO( CAOSMachine& vm )
{
	MoveAndFloatHelper(vm, false);
}

void AgentHandlers::Command_FLTO( CAOSMachine& vm )
{
	MoveAndFloatHelper(vm, true);
}

// Helper for FLTO and MVTO
// static
void AgentHandlers::MoveAndFloatHelper(CAOSMachine& vm, bool floater)
{
	float x,y;
	Vector2D position;
	vm.ValidateTarg();

	Agent& agentref = vm.GetTarg().GetAgentReference();

	if (floater)
	{
		Vector2D zero(0.0f, 0.0f);
		agentref.FloatRelativeToWorld(zero);
		x = vm.FetchFloatRV() + zero.x;
		y = vm.FetchFloatRV() + zero.y;
	}
	else
	{
		x = vm.FetchFloatRV();
		y = vm.FetchFloatRV();
	}

	if (agentref.GetMovementStatus() == Agent::FLOATING)
	{
		agentref.FloatTo( Vector2D(x, y) );
		agentref.MarkValidPositionNow();
	}
	else
	{
		if (vm.GetTarg().IsCreature())
			vm.ThrowRunError( CAOSMachine::sidOperationNotAllowedOnCreature );

		bool ok = agentref.TestMoveTo(x,y,false);
		if (!ok) 
		{
			if (agentref.GetMovementStatus() == Agent::INVEHICLE)
				vm.ThrowRunError( CAOSMachine::sidInvalidVehiclePosition, x, y);
			else
				vm.ThrowRunError( CAOSMachine::sidInvalidMapPosition2, x, y );
		}
		agentref.MoveTo( x,y );
	
		// it's now in a valid position (agents when they're
		// constructed aren't)
		agentref.MarkValidPositionNow();
	}
}


void AgentHandlers::Command_MVBY( CAOSMachine& vm )
{
	vm.ValidateTarg();
	float xd = vm.FetchFloatRV();
	float yd = vm.FetchFloatRV();

	Agent& agentref = vm.GetTarg().GetAgentReference();
	bool ok = agentref.TestMoveBy(xd,yd,false);
	if (!ok) 
	{
		float x = agentref.GetPosition().x + xd;
		float y = agentref.GetPosition().y + yd;
		if (agentref.GetMovementStatus() == Agent::INVEHICLE)
			vm.ThrowRunError( CAOSMachine::sidInvalidVehiclePosition, x, y);
		else
			vm.ThrowRunError( CAOSMachine::sidInvalidMapPosition2, x, y );
	}
	agentref.MoveBy( xd,yd );
}



void AgentHandlers::Command_PERM( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int permiability = vm.FetchIntegerRV();
	bool ok = vm.GetTarg().GetAgentReference().SetPermiability(permiability);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidInvalidMapPosition1, permiability );
	}
}



void AgentHandlers::Command_TICK( CAOSMachine& vm )
{
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().SetTimerRate( vm.FetchIntegerRV() );
}

void AgentHandlers::Command_MESG( CAOSMachine& vm )
{
	const int MESG_SUBCOUNT=2;
	static CommandHandler HandlerTable[ MESG_SUBCOUNT ] =
	{
		SubCommand_MESG_WRIT,		// WRIT
		SubCommand_MESG_WRT,		// WRT+
	};
	int subcmd;

	subcmd = vm.FetchOp();
	(HandlerTable[ subcmd ])( vm );
}

void AgentHandlers::SubCommand_MESG_WRIT( CAOSMachine& vm )
{
	AgentHandle to(vm.FetchAgentRV());
	if( to == NULLHANDLE )
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	int msgid = vm.FetchIntegerRV();

	theApp.GetWorld().WriteMessage( vm.GetOwner(), to, msgid, INTEGERZERO, INTEGERZERO, 0 );
}


void AgentHandlers::SubCommand_MESG_WRT( CAOSMachine& vm )
{
	AgentHandle to;
	int msgid;
	CAOSVar p1,p2;
	int delay;

	to = vm.FetchAgentRV();
	if( to == NULLHANDLE )
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	msgid = vm.FetchIntegerRV();
	p1 = vm.FetchGenericRV();
	p2 = vm.FetchGenericRV();
	delay = vm.FetchIntegerRV();

	theApp.GetWorld().WriteMessage( vm.GetOwner(), to, msgid, p1, p2, delay );
}

void AgentHandlers::Command_PART( CAOSMachine& vm )
{
	vm.SetPart( vm.FetchIntegerRV() );
}

void AgentHandlers::Command_GPAS( CAOSMachine& vm )
{
	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();
	int rect_to_use = vm.FetchIntegerRV();

	vm.ValidateTarg();

	if (vm.GetTarg().IsVehicle()) 
		vm.GetTarg().GetVehicleReference().GetPassengers( f,g,s, rect_to_use );
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
}

void AgentHandlers::Command_DPAS( CAOSMachine& vm )
{
	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();

	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		vm.GetTarg().GetVehicleReference().DropPassengers( f,g,s );
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
}

void AgentHandlers::Command_CABN( CAOSMachine& vm )
{
	Box r;
	r.left = vm.FetchIntegerRV();
	r.top = vm.FetchIntegerRV();
	r.right = vm.FetchIntegerRV();
	r.bottom = vm.FetchIntegerRV();

	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		vm.GetTarg().GetVehicleReference().SetRelativeCabin( r );
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
}

int AgentHandlers::IntegerRV_CABL( CAOSMachine& vm )
{
	Box r;
	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
	{
		vm.GetTarg().GetVehicleReference().GetRelativeCabin(r);
		return (int)(r.left);
	}
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
	return -1;
}

int AgentHandlers::IntegerRV_CABR( CAOSMachine& vm )
{
	Box r;
	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
	{
		vm.GetTarg().GetVehicleReference().GetRelativeCabin(r);
		return (int)(r.right);
	}
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
	return -1;
}

int AgentHandlers::IntegerRV_CABB( CAOSMachine& vm )
{
	Box r;
	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
	{
		vm.GetTarg().GetVehicleReference().GetRelativeCabin(r);
		return (int)(r.bottom);
	}
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
	return -1;
}

int AgentHandlers::IntegerRV_CABT( CAOSMachine& vm )
{
	Box r;
	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
	{
		vm.GetTarg().GetVehicleReference().GetRelativeCabin(r);
		return (int)(r.top);
	}
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
	return -1;
}

void AgentHandlers::Command_CABP( CAOSMachine& vm )
{
	int plane = vm.FetchIntegerRV();

	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		vm.GetTarg().GetVehicleReference().SetRelativeCabinPlane( plane );
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
}

int AgentHandlers::IntegerRV_CABP( CAOSMachine& vm )
{
	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		return vm.GetTarg().GetVehicleReference().GetRelativeCabinPlane();
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);

	return 0;
}

void AgentHandlers::Command_SPAS( CAOSMachine& vm )
{
	AgentHandle vehicle = vm.FetchAgentRV();
	AgentHandle pass = vm.FetchAgentRV();
	if (vehicle.IsVehicle())
		vehicle.GetVehicleReference().GetPassenger(pass);
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
}

void AgentHandlers::Command_RPAS( CAOSMachine& vm )
{
	AgentHandle vehicle = vm.FetchAgentRV();
	AgentHandle pass = vm.FetchAgentRV();

	if (vehicle.IsVehicle())
		vehicle.GetVehicleReference().DropPassenger(pass);
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);
}

void AgentHandlers::Command_EPAS( CAOSMachine& vm )
{
	vm.ValidateOwner();
	if (!vm.GetTarg().IsVehicle())
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);

	Vehicle& v = vm.GetTarg().GetVehicleReference();
	int ipnext = vm.FetchInteger();	// get address of op _after_ the NEXT
	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();
	Classifier c(f, g, s);

	std::vector<AgentHandle>& agents = v.GetCarriedAgents();

	// push IDs of all the matching agents onto the stack, then a count
	int n = agents.size();
	for( int i = 0; i < agents.size() ; ++i )
	{
		if (agents[i].IsValid())
		{
			if (agents[i].GetAgentReference().GetClassifier().GenericMatchForWildCard(c))
				vm.PushHandle( agents[i] );
			else
				n--;
		}
		else
			n--;
	}
	vm.Push( n );
	// store address of first command in body of block
	vm.Push( vm.GetIP() );

	// jump to the NEXT instruction
	vm.SetIP( ipnext );
}

///////////////////////////////////

CAOSVar& AgentHandlers::Variable_OVnn( CAOSMachine& vm )
{
	int idx;
	vm.ValidateTarg();
	idx = vm.GetSecondaryOp() - CAOSDescription::varOV00;
	// ADD BOUNDS CHECKING HERE!!!
	return vm.GetTarg().GetAgentReference().GetReferenceToVariable( idx );
}

CAOSVar& AgentHandlers::Variable_MVnn( CAOSMachine& vm )
{
	int idx;
	vm.ValidateOwner();
	idx = vm.GetSecondaryOp() - CAOSDescription::varMV00;
	// ADD BOUNDS CHECKING HERE!!!
	return vm.GetOwner().GetAgentReference().GetReferenceToVariable( idx );
}

CAOSVar& AgentHandlers::Variable_AVAR( CAOSMachine& vm )
{
	int idx;
	AgentHandle a;
	a = vm.FetchAgentRV();
	idx = vm.FetchIntegerRV();
	if (a.IsInvalid())
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );
	return a.GetAgentReference().GetReferenceToVariable( idx );
}

CAOSVar& AgentHandlers::Variable_VELX( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetReferenceToVelocityX();
}

CAOSVar& AgentHandlers::Variable_VELY( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetReferenceToVelocityY();
}


void AgentHandlers::Command_VELO( CAOSMachine& vm )
{
	vm.ValidateTarg();
	float vx = vm.FetchFloatRV();
	float vy = vm.FetchFloatRV();
	vm.GetTarg().GetAgentReference().SetVelocity(Vector2D(vx,vy));
}


int AgentHandlers::IntegerRV_FALL( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return !(vm.GetTarg().GetAgentReference().IsStopped());
}

int AgentHandlers::IntegerRV_ELAS( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetElasticity();
}


int AgentHandlers::IntegerRV_TMVT( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	if (vm.GetTarg().IsCreature())
		vm.ThrowRunError( CAOSMachine::sidOperationNotAllowedOnCreature );
	float x = vm.FetchFloatRV();
	float y = vm.FetchFloatRV();
	return vm.GetTarg().GetAgentReference().TestMoveTo(x, y, true);
}

int AgentHandlers::IntegerRV_TMVB( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	float dx = vm.FetchFloatRV();
	float dy = vm.FetchFloatRV();
	return vm.GetTarg().GetAgentReference().TestMoveBy(dx, dy, true);
}


int AgentHandlers::IntegerRV_TMVF( CAOSMachine& vm ) 
{
	float x = vm.FetchFloatRV();
	float y = vm.FetchFloatRV();
	return vm.GetCreatureTarg().TestMoveFootTo(x, y, true);
}

int AgentHandlers::IntegerRV_AERO( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetAeroDynamicPercentage();
}

int AgentHandlers::IntegerRV_PERM( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetPermiability();
}


float AgentHandlers::FloatRV_ACCG( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetGravitationalAcceleration();
}

float AgentHandlers::FloatRV_FLTX( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetFloatingVector().x;
}

float AgentHandlers::FloatRV_FLTY( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetFloatingVector().y;
}


int AgentHandlers::IntegerRV_FRIC( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetFrictionPercentage();
}


int AgentHandlers::IntegerRV_ATTR( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetAttributes();
}


int AgentHandlers::IntegerRV_IMSK( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetInputMask();
}

int AgentHandlers::IntegerRV_WDTH( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return Map::FastFloatToInteger(vm.GetTarg().GetAgentReference().GetWidth());
}



float AgentHandlers::FloatRV_OBST( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	int direction = vm.FetchIntegerRV();
	return vm.GetTarg().GetAgentReference().GetDistanceToObstacle(direction);
}

int AgentHandlers::IntegerRV_WALL( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetLastWallHit();
}

int AgentHandlers::IntegerRV_MOVS( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetMovementStatus();
}


int AgentHandlers::IntegerRV_HGHT( CAOSMachine& vm ) 
{
	vm.ValidateTarg();
	return Map::FastFloatToInteger(vm.GetTarg().GetAgentReference().GetHeight());
}


int AgentHandlers::IntegerRV_FMLY( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetClassifier().Family();
}

int AgentHandlers::IntegerRV_GNUS( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetClassifier().Genus();
}

int AgentHandlers::IntegerRV_SPCS( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetClassifier().Species();
}

int AgentHandlers::IntegerRV_UNID( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetUniqueID();
}

AgentHandle AgentHandlers::AgentRV_AGNT( CAOSMachine& vm )
{
	int unid = vm.FetchIntegerRV();
	return vm.GetAgentFromID( unid );
}

int AgentHandlers::IntegerRV_TOUC( CAOSMachine& vm )
{
	AgentHandle agent1;
	AgentHandle agent2;

	agent1 = vm.FetchAgentRV();
	agent2 = vm.FetchAgentRV();

	// if not valid agents, say not touching
	if (agent1 == NULLHANDLE || agent2 == NULLHANDLE)
		return 0;

	return agent1.GetAgentReference().CanTouch( agent2 ) ? 1 : 0;
}

int AgentHandlers::IntegerRV_SEEE( CAOSMachine& vm )
{
	AgentHandle agent1;
	AgentHandle agent2;

	agent1 = vm.FetchAgentRV();
	agent2 = vm.FetchAgentRV();

	// if not valid agents, say not seeing
	if (agent1 == NULLHANDLE || agent2 == NULLHANDLE)
		return 0;

	return agent1.GetAgentReference().CanSee( agent2 ) ? 1 : 0;
}

int AgentHandlers::IntegerRV_TOTL( CAOSMachine& vm )
{
	int count=0;
	int f,g,s;
	f = vm.FetchIntegerRV();
	g = vm.FetchIntegerRV();
	s = vm.FetchIntegerRV();

	AgentList aList;

	Classifier classifier(f,g,s);

	theAgentManager.FindByFGS(aList,classifier);
	return aList.size();
}

int AgentHandlers::IntegerRV_CLAC( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int clac,d;
	vm.GetTarg().GetAgentReference().GetClickActions(clac,d,d,d,d);
	return clac;
}

int AgentHandlers::IntegerRV_CLIK( CAOSMachine& vm )
{
	int i = vm.FetchIntegerRV();
	vm.ValidateTarg();
	if ((i < 0) || (i > 3))
		vm.ThrowRunError( CAOSMachine::sidInvalidCLIKRequest );
	int c1,c2,c3,c4,d;
	vm.GetTarg().GetAgentReference().GetClickActions(d,c1,c2,c3,c4);
	if (d != -2)
		return -2;
	switch (i)
	{
	case 0: return c4 + 1;
	case 1: return c1;
	case 2: return c2;
	case 3: return c3;
	}
	return -2;
}

int AgentHandlers::IntegerRV_TICK( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetTimerRate();
}

void AgentHandlers::Command_CLAC( CAOSMachine& vm )
{
	AgentHandle a;
	vm.ValidateTarg();
	int i = vm.FetchIntegerRV();

	a = vm.GetTarg();

	a.GetAgentReference().SetDefaultClickAction(i);
}

void AgentHandlers::Command_CLIK( CAOSMachine& vm )
{
	AgentHandle a;
	vm.ValidateTarg();
	int i1 = vm.FetchIntegerRV();
	int i2 = vm.FetchIntegerRV();
	int i3 = vm.FetchIntegerRV();

	a = vm.GetTarg();

	if (i1 == -1 && i2 == -1 && i3 == -1)
		a.GetAgentReference().SetDefaultClickAction(-1);
	else
		a.GetAgentReference().SetDefaultClickActionCycle(i1,i2,i3);
}


void AgentHandlers::Command_ATTR( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int i = vm.FetchIntegerRV();
	bool ok = vm.GetTarg().GetAgentReference().SetAttributes(i);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidInvalidMapPosition2,
			vm.GetTarg().GetAgentReference().GetPosition().x,
			vm.GetTarg().GetAgentReference().GetPosition().y);
	}
}


void AgentHandlers::Command_FRIC( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int i = vm.FetchIntegerRV();
	vm.GetTarg().GetAgentReference().SetFrictionPercentage(i);
}

void AgentHandlers::Command_IMSK( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int i = vm.FetchIntegerRV();
	vm.GetTarg().GetAgentReference().SetInputMask(i);
}


void AgentHandlers::Command_ACCG( CAOSMachine& vm )
{
	vm.ValidateTarg();
	float f = vm.FetchFloatRV();
	vm.GetTarg().GetAgentReference().SetGravitationalAcceleration(f);
}

void AgentHandlers::Command_ELAS( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int i = vm.FetchIntegerRV();
	vm.GetTarg().GetAgentReference().SetElasticity(i);
}

void AgentHandlers::Command_AERO( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int i = vm.FetchIntegerRV();
	vm.GetTarg().GetAgentReference().SetAeroDynamicPercentage(i);
}



void AgentHandlers::Command_GALL( CAOSMachine& vm )
{
	vm.ValidateTarg();
	std::string filename;

	vm.FetchStringRV(filename);

	int baseindex = vm.FetchIntegerRV();

	// check whether the file exists in C16 or S16 format and only swap it in if it does
	FilePath path( filename + ".c16", IMAGES_DIR );

	bool found = true;
#ifndef _WIN32
	if(!FileExists(path.GetFullPath().c_str()))
	{
		// report the error if there is no s16
		FilePath path(filename + ".s16", IMAGES_DIR);

		if(!FileExists(path.GetFullPath().c_str()))
		{
			ErrorMessageHandler::Show(std::string("display_engine"),
									1,
									std::string("AgentHandlers::Command_GALL"),
									path.GetFullPath().c_str());
			found = false;
		}
	}
#else
	if(GetFileAttributes(path.GetFullPath().c_str()) == -1)
	{
		// report the error if there is no s16
		FilePath path(filename + ".s16", IMAGES_DIR);

		if(GetFileAttributes(path.GetFullPath().c_str()) == -1)
		{
			ErrorMessageHandler::Show(std::string("display_engine"),
									1,
									std::string("AgentHandlers::Command_GALL"),
									path.GetFullPath().c_str());
			found = false;
		}
	}
#endif
	
	if(found)
	{
		vm.GetTarg().GetAgentReference().SetGallery(FilePath( filename + ".s16", IMAGES_DIR ), baseindex, vm.GetPart());
	}
}


/* void AgentHandlers::StringRV_GALL( CAOSMachine& vm, std::string& str)
{
	vm.ValidateTarg();
	str = vm.GetTarg().GetAgentReference().GetGalleryName(vm.GetPart());
}*/


int AgentHandlers::IntegerRV_POSE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return( vm.GetTarg().GetAgentReference().GetPose( vm.GetPart() ) );
}



float AgentHandlers::FloatRV_POSX( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetCentre().x;
}



float AgentHandlers::FloatRV_POSY( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetCentre().y;
}


float AgentHandlers::FloatRV_DISQ( CAOSMachine& vm )
{
	vm.ValidateTarg();
	AgentHandle from = vm.FetchAgentRV();
	Vector2D a = vm.GetTarg().GetAgentReference().GetCentre();
	Vector2D b = from.GetAgentReference().GetCentre();
	Vector2D delta = b - a;
	return delta.SquareLength();
}

float AgentHandlers::FloatRV_POSL( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetPosition().x;
}


float AgentHandlers::FloatRV_POSR( CAOSMachine& vm )
{
	Box extent;
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().GetAgentExtent(extent);
	return extent.right;
}


float AgentHandlers::FloatRV_POST( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetPosition().y;
}


float AgentHandlers::FloatRV_POSB( CAOSMachine& vm )
{
	Box extent;
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().GetAgentExtent(extent);
	return extent.bottom;
}


AgentHandle AgentHandlers::AgentRV_PNTR( CAOSMachine& vm )
{
	return thePointer;
}

AgentHandle AgentHandlers::AgentRV_NULL( CAOSMachine& vm )
{
	return NULLHANDLE;
}

AgentHandle AgentHandlers::AgentRV_NORN( CAOSMachine& vm )
{
	return theApp.GetWorld().GetSelectedCreature();
}

AgentHandle AgentHandlers::AgentRV_HELD( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetCarried();
}

AgentHandle AgentHandlers::AgentRV_CARR( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetCarrier();
}


void AgentHandlers::Command_STAR( CAOSMachine& vm )
{
	vm.ValidateOwner();

	int i;

	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();

	std::list< AgentHandle > agents;
	std::list< AgentHandle >::iterator it;

	theAgentManager.FindBySightAndFGS( vm.GetOwner(), agents, Classifier(f,g,s) );

	if( agents.size() > 0 )
	{
		i = rand() % agents.size();

		// ugh
		it = agents.begin();
		while( i-- )
			++it;

		vm.SetTarg( *it );
	}
	else	// no match
		vm.SetTarg( NULLHANDLE );
}


void AgentHandlers::Command_TTAR( CAOSMachine& vm )
{
	vm.ValidateOwner();

	int i;

	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();

	std::list< AgentHandle > agents;
	std::list< AgentHandle >::iterator it;

	Box r;
	vm.GetOwner().GetAgentReference().GetAgentExtent(r);

	theAgentManager.FindByAreaAndFGS( agents, Classifier(f,g,s), r );

	if( agents.size() > 0 )
	{
		i = rand() % agents.size();

		// ugh
		it = agents.begin();
		while( i-- )
			++it;

		vm.SetTarg( *it );
	}
	else	// no match
		vm.SetTarg( NULLHANDLE );
}

void AgentHandlers::Command_RTAR( CAOSMachine& vm )
{
	int f,g,s;
	int i;
	std::list< AgentHandle > agents;
	std::list< AgentHandle >::iterator it;

	f = vm.FetchIntegerRV();
	g = vm.FetchIntegerRV();
	s = vm.FetchIntegerRV();
	Classifier c( f, g, s );

	theAgentManager.FindByFGS( agents, c );

	if( agents.size() > 0 )
	{
		i = rand() % agents.size();

		// ugh
		it = agents.begin();
		while( i-- )
			++it;

		vm.SetTarg( *it );
	}
	else	// no match
		vm.SetTarg( NULLHANDLE );
}

void AgentHandlers::Command_PLNE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int plane = vm.FetchIntegerRV();
	vm.GetTarg().GetAgentReference().SetNormalPlane(plane, vm.GetPart());
}

void AgentHandlers::Command_RNGE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	float range = vm.FetchFloatRV();
	bool ok = vm.GetTarg().GetAgentReference().SetGeneralRange(range);
	if (!ok)
		vm.ThrowRunError( CAOSMachine::sidInvalidRange );
}

void AgentHandlers::Command_PUPT( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int pose  = vm.FetchIntegerRV() ; 

	int x= vm.FetchIntegerRV();
	int y= vm.FetchIntegerRV();

	if (!vm.GetTarg().GetAgentReference().AddAPointWhereIPickAgentsUp(pose,x,y))
		vm.ThrowRunError( CAOSMachine::sidInvalidPoseForPuptPuhl );		
}


void AgentHandlers::Command_PUHL( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int pose  = vm.FetchIntegerRV(); 

	int x= vm.FetchIntegerRV();
	int y= vm.FetchIntegerRV();

	if (!vm.GetTarg().GetAgentReference().AddAPointWhereYouCanPickMeUp(pose,x,y))
		vm.ThrowRunError( CAOSMachine::sidInvalidPoseForPuptPuhl );		
}

float AgentHandlers::FloatRV_RELX( CAOSMachine& vm )
{
	AgentHandle agent1 = vm.FetchAgentRV();
	if( agent1 == NULLHANDLE )
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	AgentHandle agent2 = vm.FetchAgentRV();
	if( agent2 == NULLHANDLE )
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	return agent2.GetAgentReference().GetCentre().x - agent1.GetAgentReference().GetCentre().x;
}

float AgentHandlers::FloatRV_RELY( CAOSMachine& vm )
{
	AgentHandle agent1 = vm.FetchAgentRV();
	if( agent1 == NULLHANDLE )
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	AgentHandle agent2 = vm.FetchAgentRV();
	if( agent2 == NULLHANDLE )
		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );

	return agent2.GetAgentReference().GetCentre().y - agent1.GetAgentReference().GetCentre().y;
}

int AgentHandlers::IntegerRV_PLNE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetPlane(vm.GetPart());
}

int AgentHandlers::IntegerRV_PUPT( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int pose  = vm.FetchIntegerRV();
	int xory = vm.FetchIntegerRV();

	Vector2D point = vm.GetTarg().GetAgentReference().GetMapCoordOfWhereIPickUpAgentsFrom(pose, true);

	if(xory == 1)
		return (int)(point.x);
	else
		return (int)(point.y);

}

int AgentHandlers::IntegerRV_PUHL( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int pose  = vm.FetchIntegerRV();
	int xory = vm.FetchIntegerRV();

	Vector2D point = vm.GetTarg().GetAgentReference().GetMyOffsetOfWhereIAmPickedUpFrom(pose);

	if(xory == 1)
		return (int)(point.x);
	else
		return (int)(point.y);

}

float AgentHandlers::FloatRV_RNGE( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetGeneralRange();
}



int AgentHandlers::IntegerRV_BHVR( CAOSMachine& vm )
{
	vm.ValidateTarg();
	return vm.GetTarg().GetAgentReference().GetCreaturePermissions();
}

void AgentHandlers::Command_BHVR( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int bhvr = vm.FetchIntegerRV();
	if (!vm.GetTarg().GetAgentReference().SetCreaturePermissions(bhvr))
		vm.ThrowRunError( CAOSMachine::sidBehaviourWithoutScript, bhvr );
}

void AgentHandlers::Command_MOUS(CAOSMachine& vm)
{
	int definition  = vm.FetchIntegerRV();

	if (thePointer.IsValid())
	{
		thePointer.GetPointerAgentReference().DefineMouseClicks(definition);
	}
}

void AgentHandlers::Command_PURE(CAOSMachine& vm)
{
	int value = vm.FetchIntegerRV();

	if (thePointer.IsValid())
	{
		thePointer.GetPointerAgentReference().SetScanInputEvents((value == 1) ? false : true);
	}
}

int AgentHandlers::IntegerRV_PURE(CAOSMachine& vm)
{
	if (thePointer.IsValid())
	{
		bool bValue = thePointer.GetPointerAgentReference().GetScanInputEvents();
		return bValue ? 0 : 1;
	}
	else
		return -1;
}


AgentHandle AgentHandlers::AgentRV_HOTS(CAOSMachine& vm)
{
	AgentHandle a;
	if (thePointer.IsValid())
		a = thePointer.GetPointerAgentReference().IsTouching(0, 0);
	return a;
}


AgentHandle AgentHandlers::AgentRV_TWIN(CAOSMachine& vm)
{
	AgentHandle original = vm.FetchAgentRV();

	int agent_null = vm.FetchIntegerRV();
	CreaturesArchive::AgentArchiveStyle style =
		(agent_null == 1) ? CreaturesArchive::ONE_AGENT_ONLY_NULL_OTHERS
			: CreaturesArchive::ONE_AGENT_ONLY_COPY_POINTER;
	if (original.IsInvalid())
		vm.ThrowInvalidAgentHandle(CAOSMachine::sidInvalidAgent);

	// disconnect all links to other agents
	original.GetAgentReference().BreakLinksToOtherAgents();

#ifndef C2E_OLD_CPP_LIB
	// serialise an agent to memory, and back in again
	std::stringstream memory_out;
	{
		CreaturesArchive archive( memory_out, CreaturesArchive::Mode::Save );
		archive.SetAgentArchiveStyle(style);
		archive << original;
	}

	std::string serialised_agent = memory_out.str();
	const char* data = serialised_agent.data();
	int data_size = memory_out.tellp();

	std::strstream memory_in(const_cast<char *>(data), data_size);
	AgentHandle clone;
	{
		CreaturesArchive archive( memory_in, CreaturesArchive::Mode::Load );
		archive.SetAgentArchiveStyle(style);
		archive.SetCloningACreature(true);
		archive >> clone;
	}
#else
	// TODO: maybe just serialise out to a temp file?

	#warning "fix me!"
	ASSERT( false );

	AgentHandle clone;
#endif

	// clone genome store (duplicating the *.gen files referenced by the agent)
	clone.GetAgentReference().GetGenomeStore().ClonedEntirely();

	// if a creature, now we know its moniker, make a skeleton
	if (clone.IsCreature())
		clone.GetCreatureReference().RemakeSkeletonAfterSerialisation();

	// make ourselves a new unid, and sort out the lists
	theAgentManager.RegisterClone(clone);
	
	return clone;
}


void AgentHandlers::Command_PAUS(CAOSMachine& vm)
{
	int value = vm.FetchIntegerRV();
	vm.ValidateTarg();
	vm.GetTarg().GetAgentReference().SetRunning((value == 1) ? false : true);

}

int AgentHandlers::IntegerRV_PAUS(CAOSMachine& vm)
{
	vm.ValidateTarg();

	bool running = vm.GetTarg().GetAgentReference().IsRunning();
	return running ? 0 : 1;
}

int AgentHandlers::IntegerRV_VISI(CAOSMachine& vm)
{
	vm.ValidateTarg();

	bool visible = vm.GetTarg().GetAgentReference().Visibility(	vm.FetchIntegerRV());
	return visible ? 1 : 0;
}


int AgentHandlers::IntegerRV_TYPE(CAOSMachine& vm)
{
	CAOSVar  var = vm.FetchGenericRV();
	int ret = var.GetType();
	if (ret == CAOSVar::typeAgent)
	{
		AgentHandle agent = var.GetAgent();
		if (agent.IsInvalid())
			ret = -1;
		else if (agent.IsCreature())
			ret = 7;
		else if (agent.IsVehicle())
			ret = 6;
		else if (agent.IsCompoundAgent())
			ret = 5;
		else if (agent.IsPointerAgent())
			ret = 4;
		else if (agent.IsSimpleAgent())
			ret = 3;
		else
		{
			_ASSERT(false);
			// Unknown agent type?!?!?!?
			ret = -2;
		}
	}
	return ret;
}


void AgentHandlers::Command_STPT(CAOSMachine& vm)
{
	vm.ValidateTarg();
	CAOSMachine& cm = vm.GetTarg().GetAgentReference().GetVirtualMachine();
	cm.StopScriptExecuting();
}

AgentHandle AgentHandlers::AgentRV_NCLS(CAOSMachine& vm)
{
	AgentHandle prev = vm.FetchAgentRV();
	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();
	Classifier c(f, g, s);

	AgentHandle next = theAgentManager.FindNextAgent(prev, c);
	return next;
}

AgentHandle AgentHandlers::AgentRV_PCLS(CAOSMachine& vm)
{
	AgentHandle next = vm.FetchAgentRV();
	int f = vm.FetchIntegerRV();
	int g = vm.FetchIntegerRV();
	int s = vm.FetchIntegerRV();
	Classifier c(f, g, s);

	AgentHandle prev = theAgentManager.FindPreviousAgent(next, c);
	return prev;
}

void AgentHandlers::Command_FREL( CAOSMachine& vm )
{
	AgentHandle with = vm.FetchAgentRV();
	vm.GetTarg().GetAgentReference().AttachFloatingWith(with);
}

void AgentHandlers::Command_SHOW( CAOSMachine& vm )
{
	vm.ValidateTarg();

	int visibility = vm.FetchIntegerRV();
	if(visibility == 0)
		vm.GetTarg().GetAgentReference().Hide();
	else
		vm.GetTarg().GetAgentReference().Show();
}

int AgentHandlers::IntegerRV_MOWS( CAOSMachine& vm )
{
	AgentList aList;
	Classifier classifier(2, 6, 1);

	theAgentManager.FindByFGS(aList, classifier);

	if (aList.size() < 14 )
		return 1;
	else 
		return 0;
}


void AgentHandlers::Command_NEWC( CAOSMachine& vm )
{

	AgentHandle agent;

	// first time around create the creature
	if(!vm.IsBlocking())
	{
		int family = vm.FetchIntegerRV();

		AgentHandle gene = vm.FetchAgentRV();
		int gene_index = vm.FetchIntegerRV();

		int sex = vm.FetchIntegerRV();
		int variant = vm.FetchIntegerRV();

		if (!gene.IsValid())
			vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );	

		if (gene_index < 1 ||
			gene.GetAgentReference().GetGenomeStore().MonikerAsString(gene_index).empty())
		{
			vm.ThrowRunError( CAOSMachine::sidInvalidGeneVariable );	
		}
		
		agent = theAgentManager.CreateCreature(family, gene, gene_index, sex, variant);

		vm.SetTarg( agent );
		vm.Block();
	}
	else
	{
		vm.ValidateTarg();

		// continue to load body parts
		if(vm.GetTarg().GetCreatureReference().AreYourBodyPartsFullyFormed())
		{
			theAgentManager.AddCreatureToUpdateList(vm.GetTarg());
			vm.UnBlock();
		}
		else
		{
			if(!vm.GetTarg().GetCreatureReference().FormBodyParts())
				vm.ThrowRunError( CAOSMachine::sidFailedToCreateCreature );
		}
	}
}


void AgentHandlers::Command_CABW(CAOSMachine& vm)
{
	int width  = vm.FetchIntegerRV();

	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		vm.GetTarg().GetVehicleReference().SetCabinCapacity( width );
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);

}

int AgentHandlers::IntegerRV_CABW(CAOSMachine& vm)
{
	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		return vm.GetTarg().GetVehicleReference().GetCabinCapacity( );
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);

	return 0;

}

int AgentHandlers::IntegerRV_MIRA(CAOSMachine& vm)
{
	vm.ValidateTarg();
	return 	vm.GetTarg().GetAgentReference().IsMirrored();
}

void AgentHandlers::Command_MIRA(CAOSMachine& vm)
{
	int mirror  = vm.FetchIntegerRV();

	vm.ValidateTarg();

	if(mirror == 0)
	vm.GetTarg().GetAgentReference().DrawMirrored(false);
	else
	vm.GetTarg().GetAgentReference().DrawMirrored(true);
}


void AgentHandlers::Command_CABV(CAOSMachine& vm)
{
	int room_id = vm.FetchIntegerRV();

	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		vm.GetTarg().GetVehicleReference().SetCabinRoom( room_id );
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);

}

int AgentHandlers::IntegerRV_CABV(CAOSMachine& vm)
{
	vm.ValidateTarg();
	if (vm.GetTarg().IsVehicle()) 
		return vm.GetTarg().GetVehicleReference().GetCabinRoom();
	else
		vm.ThrowRunError(CAOSMachine::sidNotAVehicle);

	return -1;
}


void AgentHandlers::StringRV_HAND( CAOSMachine& vm, std::string& str )
{
	str = thePointer.GetPointerAgentReference().GetName();
}



void AgentHandlers::Command_HAND(CAOSMachine& vm)
{
	std::string str;
	vm.FetchStringRV(str);

	if(thePointer.GetPointerAgentReference().GetName() != str)
		thePointer.GetPointerAgentReference().SetName(str);
}
