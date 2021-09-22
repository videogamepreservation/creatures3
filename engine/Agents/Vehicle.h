// -------------------------------------------------------------------------
// Filename:    Vehicle.h
// Class:       Vehicle
// Purpose:     An agent which can contain other agents
// Description:
// A specialization of CompoundAgent. The main addition is the Cab
// rectangle, which is used to contain other agents.
// The movement routines are extended to move passengers whenever the
// vehicle moves.
//
// Authors: BenC, Francis, Robert
// -------------------------------------------------------------------------

#ifndef VEHICLE_H
#define VEHICLE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CompoundAgent.h"
#include <vector>

class Vehicle : public CompoundAgent
{
	typedef CompoundAgent base;

	CREATURES_DECLARE_SERIAL( Vehicle )

public:

	Vehicle();							// serialisation constr
	Vehicle(	int family, int genus, int species,uint32 id,
		FilePath const& gallery,	
			int numimages,
			int baseimage,
			int plane);			// Vehicles' galleries should not be clones
	virtual ~Vehicle();
	virtual void Trash();

	// ---------------------------------------------------------------------
	// Method:      MoveTo
	// Arguments:   x,y - new position (world coordinates)
	// Returns:     None
	// Description:	See CompoundAgent::MoveTo. Additional code added by
	//				Vehicle::MoveTo also moves any agents riding within the
	//				vehicle.
	// ---------------------------------------------------------------------
	virtual void MoveTo( float x, float y );

	virtual void Update();			// called every tick

	virtual void ChangeCameraShyStatus(bool shy);
	
	void SetRelativeCabin(const Box& r)			// set Cab rectangle
	{
		_ASSERT(!myGarbaged);
		myRelativeCabin = r; 
	}

	void GetRelativeCabin(Box& r) 
	{
		_ASSERT(!myGarbaged);
		r = myRelativeCabin;
	}

	void GetCabinExtent(Box& r)
	{
		_ASSERT(!myGarbaged);
		_ASSERT( myParts.size() >= 1 ); // need part0
		r = myRelativeCabin;	
		r.left += myPositionVector.x;
		r.right += myPositionVector.x;
		r.top += myPositionVector.y;
		r.bottom += myPositionVector.y;
	}

	void SetCabinCapacity(int capacity){myCabinCapacity = capacity;}
	int GetCabinCapacity(){return myCabinCapacity ;}

	void SetCabinRoom(int room){myCabinRoom = room;}
	int GetCabinRoom(){return myCabinRoom ;}

	// ---------------------------------------------------------------------
	// Method:      GetCabinPlane
	// Arguments:   None
	// Returns:     Drawing plane of cab
	// Description:	Returns drawing plane used for carried agents.
	// ---------------------------------------------------------------------
	int GetCabinPlane();
	void SetRelativeCabinPlane(int plane) 
	{
		_ASSERT(!myGarbaged);
		myRelativeCabinPlane = plane;
		SetPlane(0,GetPlane(0));
	}

	int GetRelativeCabinPlane() 
	{
		_ASSERT(!myGarbaged);
		return myRelativeCabinPlane; 
	}

	virtual int32 GetPlaneForCarriedAgent();

	void GetPassengers( int f, int g, int s, int rect_to_use );
	void GetPassenger( AgentHandle& a );
	void DropPassengers( int f, int g, int s );
	void DropPassenger( AgentHandle& a );

	// override from Agent class - returns just one carried object
	virtual AgentHandle GetCarried();

	std::vector<AgentHandle>& GetCarriedAgents() 
	{
		_ASSERT(!myGarbaged);
		return myCarriedAgents; 
	}

	void BreakLinksToOtherAgents();

	// ----------------------------------------------------------------------
	// Method:		Write
	// Arguments:	archive - archive being written to
	// Returns:		true if successful
	// Description:	Overridable function - writes details to archive,
	//				taking serialisation into account
	// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;


	// ----------------------------------------------------------------------
	// Method:		Read
	// Arguments:	archive - archive being read from
	// Returns:		true if successful
	// Description:	Overridable function - reads detail of class from archive
	// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);

	virtual void SetPlane(int plane, int part = 0);

	virtual void ChangePhysicalPlane(int plane);

protected:
	void Init();						// Basic initialisation used by constructors

	// helper for movement functions - moves any carried agents
	void ShiftPassengers( const Vector2D& relshift );

	
	Box myRelativeCabin;		// walls & floor of occupiable region, relative to part0

	int myRelativeCabinPlane;
	int myCabinCapacity;
	int myCabinRoom;


	std::vector<AgentHandle> myCarriedAgents;			// Agents being carried


	virtual bool CanStartCarrying(AgentHandle& a);
	virtual void StartCarrying(AgentHandle& a);
	virtual void StopCarrying(AgentHandle& a);
};


#endif // VEHICLE_H
