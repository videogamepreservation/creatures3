// -------------------------------------------------------------------------
// Filename:    Vehicle.cpp
// Class:       Vehicle
// Purpose:     An agent which can contain other agents
// Description:
// A specialization of CompoundAgent. The main addition is the Cab
// rectangle, which is used to contain other agents.
// The movement routines are extended to move passengers whenever the
// vehicle moves.
//
// Usage:
//
//
// History:
// ???					Initial version
// Jan99		BenC	Serious revision and code pruning for C2E.
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Vehicle.h"
#include "../World.h"
#include "../App.h"
#include "../AgentManager.h"
#include "../Display/MainCamera.h"
#include "../Creature/Creature.h"

CREATURES_IMPLEMENT_SERIAL( Vehicle)

void Vehicle::ChangeCameraShyStatus(bool shy)
{
	base::ChangeCameraShyStatus(shy);
	for(int i=0;i<myCarriedAgents.size(); i++)
	{
		myCarriedAgents[i].GetAgentReference().ChangeCameraShyStatus(shy);
	}
}

// Basic initialisation used by constructors
void Vehicle::Init()
{
	// The default cabin is the dimensions of part 0
	myRelativeCabin.left = 0.0f;
	myRelativeCabin.top = 0.0f;
	myRelativeCabin.right = myCurrentWidth;
	myRelativeCabin.bottom = myCurrentHeight;

	myAttributes = attrGreedyCabin;
	myRelativeCabinPlane = 1;
	myDefaultClickAction = ACTIVATE1;
	myCabinCapacity = 0;
	myCabinRoom = -1;

	myCarriedAgent = NULLHANDLE;			// never carries anything
}


// Serialisation & subclass constr
Vehicle::Vehicle()
{
	_ASSERT(!myGarbaged);

//	Init();
	myAgentType = AgentHandle::agentNormal | AgentHandle::agentCompound | AgentHandle::agentVehicle;
}


Vehicle::Vehicle(	int family, int genus, int species,uint32 id,
				 FilePath const& gallery,
				 int numimages,
			int baseimage,
			int plane)
	: base(family, genus, species, id, gallery,numimages,baseimage,plane)
{
	try
	{
		if (!myFailedConstructionException.empty())
			return;
		Init();
		if (!myFailedConstructionException.empty())
			return;
		myAgentType = AgentHandle::agentNormal | AgentHandle::agentCompound | AgentHandle::agentVehicle;
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0011: Unknown exception caught in vehicle constructor";
	}
}

Vehicle::~Vehicle()
{
	// Empty.
}


void Vehicle::SetPlane(int plane, int part)
{
	base::SetPlane(plane,part);
	if (part == 0)
	{
		int n = myCarriedAgents.size();
		for( int i = 0; i < n; ++i )
		{
			AgentHandle a = myCarriedAgents[i];
			if (a.IsInvalid())
				continue;
			_ASSERT(a.GetAgentReference().GetCarrier() == mySelf);
			if (myCabinCapacity == 0)
				a.GetAgentReference().ChangePhysicalPlane( GetPlaneForCarriedAgent());
			else
				a.GetAgentReference().ChangePhysicalPlane( GetPlaneForCarriedAgent() + i);
		}
	}
}

void Vehicle::ChangePhysicalPlane(int plane)
{
	base::ChangePhysicalPlane(plane);

	int n = myCarriedAgents.size();
	for( int i = 0; i < n; ++i )
	{
		AgentHandle a = myCarriedAgents[i];
		if (a.IsInvalid())
			continue;
		_ASSERT(a.GetAgentReference().GetCarrier() == mySelf);

		if (myCabinCapacity == 0)
			a.GetAgentReference().ChangePhysicalPlane( GetPlaneForCarriedAgent());
		else
			a.GetAgentReference().ChangePhysicalPlane( GetPlaneForCarriedAgent() + i);
	}

}

// Return the plot plane an object should occupy if it is to be carried by vehicle
int Vehicle::GetCabinPlane()
{
	_ASSERT(!myGarbaged);

	return GetPlane(0) + myRelativeCabinPlane;
}

void Vehicle::GetPassengers( int f, int g, int s, int rect_to_use )
{
	_ASSERT(!myGarbaged);

	Box r;
	if (rect_to_use == 1)
		GetCabinExtent(r);
	else if (rect_to_use == 0)
		GetAgentExtent(r);
	else
	{
		// should only use 0 or 1
		ASSERT(false);
		GetAgentExtent(r);
	}


	std::list<AgentHandle> agents;
	theAgentManager.FindByAreaAndFGS( agents, Classifier(f,g,s), r );

	if( agents.size() > 0 )
	{
		for( std::list< AgentHandle >::iterator it = agents.begin(); it != agents.end(); ++it )
		{
			if( (*it).GetAgentReference().GetCarrier().IsInvalid() && (*it) != mySelf )
				GetPassenger(*it);
		}
	}
}

void Vehicle::GetPassenger( AgentHandle& a )
{
	_ASSERT(!myGarbaged);

	if (a.IsInvalid())
		return;

	theApp.GetWorld().WriteMessage( mySelf, a, STOP_HOLD_HANDS,
		INTEGERZERO, INTEGERZERO, 0);
	theApp.GetWorld().WriteMessage( mySelf, a, PICKUP,
		INTEGERZERO, INTEGERZERO, 0);
}

void Vehicle::DropPassengers( int f, int g, int s )
{
	_ASSERT(!myGarbaged);

	Classifier c( f,g,s );
	AgentHandle a;
	int n = myCarriedAgents.size();
	for(int i = 0; i < n; ++i)
	{
		if (i >= myCarriedAgents.size())
			// The carried list can be shrunk under our feet
			break;
		a = myCarriedAgents[i];
		if (a.IsInvalid())
			continue;
		ASSERT(a.IsValid());
		if (a.IsValid() && 
			a.GetAgentReference().GetClassifier().
				GenericMatchForWildCard(c, TRUE, TRUE, TRUE, FALSE))
		{
			DropPassenger(a);
			// This will shrink the list by one, so the next item
			// to check is the index before - So we reduce the index
			// by one to make sure we don't miss a thing.
			--i;
		}
	}
}

void Vehicle::DropPassenger( AgentHandle& a )
{
	_ASSERT(!myGarbaged);

	if (a.IsInvalid())
		return;

	// Force the drop
	 a.GetAgentReference().TryToBeDropped
		(mySelf, INTEGERZERO, INTEGERZERO, true);
}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool Vehicle::Write(CreaturesArchive &archive) const
{
	_ASSERT(!myGarbaged);

	base::Write(archive);

	archive << myRelativeCabin << myRelativeCabinPlane;

	archive << myCarriedAgents;
	archive << myCabinCapacity;
	archive << myCabinRoom;

	return false;
}

bool Vehicle::Read(CreaturesArchive &archive)
{
	_ASSERT(!myGarbaged);

	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{

		if(!base::Read(archive))
			return false;
			
		archive >> myRelativeCabin >> myRelativeCabinPlane;

		archive >> myCarriedAgents;
		archive >> myCabinCapacity;
		archive >> myCabinRoom;

	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


void Vehicle::MoveTo( float x, float y )
{
	_ASSERT(!myGarbaged);
	Vector2D oldpos = myPositionVector;
	base::MoveTo(x,y);

	if( myPositionVector != oldpos )
		ShiftPassengers( myPositionVector - oldpos );
}

void Vehicle::ShiftPassengers( const Vector2D& relshift )
{
	_ASSERT(!myGarbaged);

	int n = myCarriedAgents.size();
	for( int i = 0; i < n; ++i )
	{
		AgentHandle a = myCarriedAgents[i];
		if (a.IsInvalid())
			continue;
		_ASSERT(a.GetAgentReference().GetCarrier() == mySelf);
		a.GetAgentReference().MoveBy( relshift.x, relshift.y );
	}
}



bool Vehicle::CanStartCarrying(AgentHandle& a)
{
	if (myCabinCapacity != 0)
	{
		if (myCarriedAgents.size() == myCabinCapacity)
			return false;
	}
	Agent& agentref = a.GetAgentReference();
	float agentw = agentref.GetWidth();
	float agenth = agentref.GetHeight();
	float cabinw = myRelativeCabin.Width();
	float cabinh = myRelativeCabin.Height();
	if ((agentw < (cabinw-1.0f)) && (agenth < (cabinh-1.0f)))
		// The agent can fit wholly in my cabin
		return true;
	else
		return false;
}


void Vehicle::StartCarrying(AgentHandle& a)
{
	//
	// We know that "a" can safely fit into the vehicle
	//

	Agent& agentref = a.GetAgentReference();

	// Create parent-child relationships
	agentref.SetCarrier(mySelf);
	myCarriedAgents.push_back(a);

	// Agent is now being carried by this vehicle
	agentref.SetMovementStatus(INVEHICLE);

	// Allow the newly carried agent to start falling if need be
	agentref.SetVelocity(ZERO_VECTOR);

	Box cabin;
	GetCabinExtent(cabin);
	Box agentbox;
	agentref.GetAgentExtent(agentbox);
	Vector2D agentCentre(agentref.GetCentre());

	if ((agentbox.left >= (cabin.left+1.0f)) && 
		(agentbox.right <= (cabin.right-1.0f)) &&
		(agentbox.top >= (cabin.top+1.0f)) &&
		(agentbox.bottom <= (cabin.bottom-1.0f))) 
	{
		// Agent is wholly inside the cabin, do nothing
	}
	else if ((agentCentre.x > cabin.left) && 
			 (agentCentre.x < cabin.right) &&
			 (agentCentre.y > cabin.top) &&
			 (agentCentre.y < cabin.bottom))
	{
		// Agent centre lies inside the cabin, so snap it into the vehicle
		float x, y;
		if (agentbox.left < (cabin.left+1.0f))
			x = cabin.left+1.0f;
		else if (agentbox.right > (cabin.right-1.0f))
			x = cabin.right-1.0f-agentbox.Width();
		else 
			x = agentbox.left;

		if (agentbox.top < (cabin.top+1.0f))
			y = cabin.top+1.0f;
		else if (agentbox.bottom > (cabin.bottom-1.0f))
			y = cabin.bottom-1.0f-agentbox.Height();
		else
			y = agentbox.top;
		agentref.MoveTo(x,y);
	}
	else 
	{
		// Centre the agent on the floor of the vehicle
		float cabinw = cabin.Width();
		float cabinh = cabin.Height();
		float diffw = (cabinw - agentbox.Width())/2.0f;
		agentref.MoveTo
			(cabin.left+diffw, cabin.bottom-1.0f-agentbox.Height());
	}

	// Place the carried agent into the appropriate carry plane
	int32 plane = GetPlaneForCarriedAgent();
	if (plane != -1) 
	{
		if (myCabinCapacity == 0)
			agentref.ChangePhysicalPlane(plane);
		else
			agentref.ChangePhysicalPlane(plane+myCarriedAgents.size()-1);
	}

	// Make the picked-up agent have the same camera-shy status as the
	// carrier 
	agentref.ChangeCameraShyStatus((myAttributes & attrCameraShy) == attrCameraShy);
	
	// Inform the new carrier that it has gained a carried agent
	ExecuteScriptForEvent(SCRIPGOTCARRIEDAGENT, a, INTEGERZERO, INTEGERZERO);
}


void Vehicle::StopCarrying(AgentHandle& a)
{
	_ASSERT(!myGarbaged);

	Agent& agentref = a.GetAgentReference();

	// Break the parent-child relationship
	agentref.SetCarrier(NULLHANDLE);
	int n = myCarriedAgents.size();
	int carried = -1;
	for (int i = 0; i < n; ++i)
	{
		if (i >= myCarriedAgents.size())
			// The carried list can be shrunk under our feet
			break;
		if (a == myCarriedAgents[i])
		{
			carried = i;
			break;
		}
	}
	_ASSERT(carried != -1);
	myCarriedAgents.erase(myCarriedAgents.begin() + carried);

	// Restore the agent to its default movement status
	if(agentref.TestAttributes(attrFloatable))
		agentref.SetMovementStatus(FLOATING);
	else
		agentref.SetMovementStatus(AUTONOMOUS);

	// Restore the agent to its default plane
	agentref.ChangePhysicalPlane(agentref.GetNormalPlane(0));

	// Restore the agent to its default camera-shy status
	agentref.ChangeCameraShyStatus(agentref.TestAttributes(attrCameraShy) == attrCameraShy);

	if  (
			((agentref.TestAttributes(attrSufferCollisions) & 
				Agent::attrSufferCollisions) == 
			Agent::attrSufferCollisions) &&
			(agentref.GetMovementStatus() == AUTONOMOUS)
		)
	{
		agentref.SetVelocity(agentref.GetVelocity()+GetVelocity());
	}
	else 
	{
		// Stop dead
		agentref.SetVelocity(ZERO_VECTOR);
	}

	// Inform the old carrier that it has lost its carried agent
	ExecuteScriptForEvent(SCRIPTLOSTCARRIEDAGENT, a, INTEGERZERO, INTEGERZERO);
}



void Vehicle::Trash()
{
	_ASSERT(!myGarbaged);
	BreakLinksToOtherAgents();

	// This must be last line in the function
	base::Trash();
}

int32 Vehicle::GetPlaneForCarriedAgent()
{
	_ASSERT(!myGarbaged);
	return GetCabinPlane();
}

inline AgentHandle Vehicle::GetCarried()
{
	_ASSERT(!myGarbaged);

	if (myCarriedAgents.size() > 0)
		return myCarriedAgents[Rnd(0, myCarriedAgents.size() - 1)];
	else
		return NULLHANDLE;
}

void Vehicle::Update()			// called every tick
{
	_ASSERT(!myGarbaged);

	base::Update();
	if( theMainView.IsMapDisplayed() )
	{
		EntityImage *entityImage = myParts[0]->GetEntity();
		Box rect;
		GetCabinExtent( rect );
		int left = Map::FastFloatToInteger(rect.left);
		int right = Map::FastFloatToInteger(rect.right);
		int top = Map::FastFloatToInteger(rect.top);
		int bottom = Map::FastFloatToInteger(rect.bottom);		
		entityImage->DrawLine( left, top, right, top, 255, 255, 255 );
		entityImage->DrawLine( left, bottom, right, bottom, 255, 255, 255 );
		entityImage->DrawLine( left, top, left, bottom, 255, 255, 255 );
		entityImage->DrawLine( right, top, right, bottom, 255, 255, 255 );
		myResetLines = true;
	}
}

void Vehicle::BreakLinksToOtherAgents()
{
	_ASSERT(!myGarbaged);

	// Make sure we're not carrying anything
	while (myCarriedAgents.size() > 0)
	{
		if ( myCarriedAgents[0].IsValid() )
		{
			myCarriedAgents[0].GetAgentReference().TryToBeDropped
				(mySelf, INTEGERZERO, INTEGERZERO, true);
		}
		else
		{
			myCarriedAgents.erase(myCarriedAgents.begin());
		}
	}

	// This must be the last line in the function
	base::BreakLinksToOtherAgents();
}
