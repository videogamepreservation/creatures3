// -------------------------------------------------------------------------
// Filename:    MapHandlers.cpp
// Class:       MapHandlers
// Purpose:     Routines to implement map-related commands/values in CAOS.
// Description:
//
// Usage:
//
// History:
// -------------------------------------------------------------------------

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>
#ifdef C2E_OLD_CPP_LIB
#include <strstream>
#else
#include <sstream>
#endif
#include "AgentHandlers.h"
#include "../App.h"
#include "../World.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../AgentManager.h"
#include "../Map/Map.h"
#include "MapHandlers.h"
#include "../Display/MainCamera.h"
#include "../Agents/Vehicle.h"


void MapHandlers::Command_DOCA( CAOSMachine& vm )
{
	std::vector<AgentHandle> agentsThatEmitCAs[CA_PROPERTY_COUNT];
	for (AgentListIterator a=theAgentManager.GetAgentIteratorStart(); !theAgentManager.IsEnd(a); a++)
	{
		if ((*a).GetAgentReference().GetCAIncrease()>0.0f)
		{
			int whichCA = (*a).GetAgentReference().GetCAIndex();
			agentsThatEmitCAs[whichCA].push_back(*a);
		}
	}


	int noOfTimesToUpdate = vm.FetchIntegerRV();
	for (int i=0; i<noOfTimesToUpdate; i++)
	{
		for (int j=0; j<CA_PROPERTY_COUNT; j++)
		{
			for (int p=0; p<agentsThatEmitCAs[j].size(); p++)
			{
				agentsThatEmitCAs[theApp.GetWorld().GetMap().GetCAIndex()][p].GetAgentReference().HandleCA();
			}
			theApp.GetWorld().GetMap().UpdateCurrentCAProperty();
		}
	}
}



void MapHandlers::Command_MAPK( CAOSMachine& vm )
{
	theApp.GetWorld().GetMap().Reset();
	theMainView.Refresh();
}


void MapHandlers::Command_DELR( CAOSMachine& vm )
{
	int roomID = vm.FetchIntegerRV();
	bool ok = theApp.GetWorld().GetMap().RemoveRoom(roomID);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToDeleteRoom );
	}
	theMainView.Refresh();
}

void MapHandlers::Command_DELM( CAOSMachine& vm )
{
	int metaRoomID = vm.FetchIntegerRV();
	bool ok = theApp.GetWorld().GetMap().RemoveMetaRoom(metaRoomID);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToDeleteMetaRoom );
	}
	theMainView.Refresh();
}

void MapHandlers::Command_ADDB( CAOSMachine& vm )
{
	int metaRoomID = vm.FetchIntegerRV();
	std::string background;
	vm.FetchStringRV(background);
	bool ok = theApp.GetWorld().GetMap().AddBackground(metaRoomID,background);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToAddBackground );
	}
}


void MapHandlers::Command_META( CAOSMachine& vm )
{
	int metaRoomID = vm.FetchIntegerRV();
	int cameraX = vm.FetchIntegerRV();
	int cameraY = vm.FetchIntegerRV();
	int transitionType = vm.FetchIntegerRV();

	Camera* currentCamera = vm.GetCamera();
	if(currentCamera)
	{
		// now tell the main view to change meta room too
		std::string background;

		int x, y, w, h;
		RECT metaRoomMBR;
		if( !theApp.GetWorld().GetMap().GetMetaRoomLocation
			(metaRoomID, x, y, w, h) )	
		{
			vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
		}

	
		theApp.GetWorld().GetMap().GetCurrentBackground(metaRoomID, background);

		if(!background.empty() )
		{

			metaRoomMBR.left = x;
			metaRoomMBR.right = x+w-1;
			metaRoomMBR.top = y;
			metaRoomMBR.bottom = y+h-1;
			
			if( cameraX < 0 || cameraY < 0)
			{
				long newx, newy;

				if (!theApp.GetWorld().GetMap().
					GetMetaRoomDefaultCameraLocation(metaRoomID, newx, newy))
				{
					vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
				}

				cameraX = newx;
				cameraY = newy;
			}

			// no special transitions allowed
			currentCamera->ChangeMetaRoom(background, metaRoomMBR,
				cameraX, cameraY, 3, 0); // no centring
		}
	}
	else // assume we meant the main camera then
	{

	if( !theApp.GetWorld().SetMetaRoom( metaRoomID, transitionType, cameraX, cameraY ) )
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}


void MapHandlers::Command_BKGD( CAOSMachine& vm )
{
	int metaRoomID;
	std::string background;
	bool ok;
	int transitionType;
	int current;

	metaRoomID = vm.FetchIntegerRV();
	vm.FetchStringRV(background);
	transitionType = vm.FetchIntegerRV();

	Camera* currentCamera = vm.GetCamera();
	if(currentCamera)
	{
		if(ValidateBackground(background,metaRoomID))
		{
			// no fancy transitions here my friends
			currentCamera->SetBackground(background, 0);
		}
	}
	else
	{
		ok = theApp.GetWorld().GetMap().SetCurrentBackground
			(metaRoomID, background);
		if (!ok) {
			vm.ThrowRunError( CAOSMachine::sidFailedToSetBackground );
		}

		current = theApp.GetWorld().GetMap().GetCurrentMetaRoom();
		if (metaRoomID != current) {
			return;
		}
		theMainView.SetBackground(background, transitionType);
	}
}

// maybe this should go in the map when robert has finished with it
// this is for remote cameras
bool MapHandlers::ValidateBackground(const std::string& background,int metaRoomID)
{
	StringCollection backgroundCollection;

	theApp.GetWorld().GetMap().GetBackgroundCollection(metaRoomID, backgroundCollection);
	if(backgroundCollection.find(background) == backgroundCollection.end())
		return false;

	return true;

}


void MapHandlers::Command_MAPD( CAOSMachine& vm )
{
	int w = vm.FetchIntegerRV();
	int h = vm.FetchIntegerRV();	
	theApp.GetWorld().GetMap().SetMapDimensions(w, h);
	theMainView.Refresh();
}


void MapHandlers::Command_DOOR( CAOSMachine& vm )
{
	int id1 = vm.FetchIntegerRV();
	int id2 = vm.FetchIntegerRV();	
	int permiability = vm.FetchIntegerRV();
	bool ok;

	ok = theApp.GetWorld().GetMap().SetDoorPermiability(id1, id2, permiability);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToSetDoorPerm );
	}
	theMainView.Refresh();
}


void MapHandlers::Command_LINK( CAOSMachine& vm )
{
	int id1 = vm.FetchIntegerRV();
	int id2 = vm.FetchIntegerRV();	
	int permiability = vm.FetchIntegerRV();
	bool ok;

	ok = theApp.GetWorld().GetMap().SetLinkPermiability(id1, id2, permiability);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToSetLinkPerm );
	}
	theMainView.Refresh();
}




void MapHandlers::Command_RTYP( CAOSMachine& vm )
{
	int id = vm.FetchIntegerRV();
	int type = vm.FetchIntegerRV();	
	bool ok = theApp.GetWorld().GetMap().SetRoomType(id, type);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToSetRoomType );
	}
}

void MapHandlers::Command_PROP( CAOSMachine& vm )
{
	int id = vm.FetchIntegerRV();
	int caIndex = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();

	bool ok = theApp.GetWorld().GetMap().SetRoomProperty(id, caIndex, value);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToSetRoomProperty );
	}
}

void MapHandlers::Command_RATE( CAOSMachine& vm )
{
	int roomType = vm.FetchIntegerRV();
	int caIndex = vm.FetchIntegerRV();

	float gain = vm.FetchFloatRV();
	float loss = vm.FetchFloatRV();
	float diff = vm.FetchFloatRV();

	bool ok = theApp.GetWorld().GetMap().
		SetCARates( roomType, caIndex, CARates( gain, loss, diff ) );
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToSetCARates );
	}
}

void MapHandlers::StringRV_RATE( CAOSMachine& vm, std::string& str )
{
	int roomType = vm.FetchIntegerRV();
	int caIndex = vm.FetchIntegerRV();

	CARates rates;
	bool ok = theApp.GetWorld().GetMap().GetCARates( roomType, caIndex, rates );

	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToGetCARates );
	}

#ifdef C2E_OLD_CPP_LIB
	char hackbuf[256];
	std::ostrstream stream( hackbuf, sizeof(hackbuf) );
#else
	std::stringstream stream;
#endif
	stream << ' ' << rates.GetGain() << ' ' << rates.GetLoss() << ' ' << rates.GetDiffusion();
	str = stream.str();
}

void MapHandlers::Command_EMIT( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int caIndex = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();
	if(!vm.GetTarg().GetAgentReference().SetEmission( caIndex, value ))
		vm.ThrowRunError( CAOSMachine::sidInvalidEmit );

}

void MapHandlers::Command_CACL( CAOSMachine& vm )
{
	int family = vm.FetchIntegerRV();
	int genus = vm.FetchIntegerRV();
	int species = vm.FetchIntegerRV();
	Classifier c(family,genus,species);
	int caIndex = vm.FetchIntegerRV();
	if (!theAgentManager.UpdateSmellClassifierList(caIndex, &c))
		vm.ThrowRunError( CAOSMachine::sidInvalidCacl );
}

void MapHandlers::Command_ALTR( CAOSMachine& vm )
{
	int roomID = vm.FetchIntegerRV();
	int caIndex = vm.FetchIntegerRV();
	float value = vm.FetchFloatRV();

	if( roomID == -1 )
	{
		vm.ValidateTarg();
		if( !vm.GetTarg().GetAgentReference().GetRoomID( roomID ) )
			vm.ThrowRunError( CAOSMachine::sidFailedToGetRoomID );
	}
	
	bool ok = theApp.GetWorld().GetMap().IsCANavigable(caIndex)?false:
		theApp.GetWorld().GetMap().IncreaseCAProperty( roomID, caIndex, value );
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToIncreaseCAProperty );
	}

}

int MapHandlers::IntegerRV_DOOR( CAOSMachine& vm )
{
	int id1 = vm.FetchIntegerRV();
	int id2 = vm.FetchIntegerRV();	
	int permiability;
	bool ok;

	ok = theApp.GetWorld().GetMap().GetDoorPermiability(id1, id2, permiability);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToGetDoorPerm );
	}
	return permiability;
}

int MapHandlers::IntegerRV_LINK( CAOSMachine& vm )
{
	int id1 = vm.FetchIntegerRV();
	int id2 = vm.FetchIntegerRV();	
	int permiability;
	bool ok;

	ok = theApp.GetWorld().GetMap().GetLinkPermiability(id1, id2, permiability);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToGetLinkPerm );
	}
	return permiability;
}



int MapHandlers::IntegerRV_MAPW( CAOSMachine& vm )
{
	int w, h;
	theApp.GetWorld().GetMap().GetMapDimensions(w, h);
	return w;
}

int MapHandlers::IntegerRV_MAPH( CAOSMachine& vm )
{
	int w, h;
	theApp.GetWorld().GetMap().GetMapDimensions(w, h);
	return h;
}

int MapHandlers::IntegerRV_ADDM( CAOSMachine& vm )
{
	int x, y, w, h;
	bool ok;
	int metaRoomID;
	std::string title;

	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();	
	w = vm.FetchIntegerRV();
	h = vm.FetchIntegerRV();
	vm.FetchStringRV(title);

	ok = theApp.GetWorld().GetMap().AddMetaRoom(x, y, w, h,
		title, metaRoomID);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToAddMetaRoom );
	}
	theMainView.Refresh();
	return metaRoomID;
}


int MapHandlers::IntegerRV_ADDR( CAOSMachine& vm )
{
	int xLeft, xRight, yLeftCeiling, yRightCeiling,
		yLeftFloor, yRightFloor;
	bool ok;
	int roomID;
	int metaRoomID;

	metaRoomID = vm.FetchIntegerRV();
	xLeft = vm.FetchIntegerRV();
	xRight = vm.FetchIntegerRV();	
	yLeftCeiling = vm.FetchIntegerRV();
	yRightCeiling = vm.FetchIntegerRV();
	yLeftFloor = vm.FetchIntegerRV();
	yRightFloor = vm.FetchIntegerRV();

	ok = theApp.GetWorld().GetMap().AddRoom(metaRoomID,
		xLeft, xRight, 
		yLeftCeiling, yRightCeiling, yLeftFloor, yRightFloor,
		roomID);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToAddRoom );
	}
	theMainView.Refresh();
	return roomID;
}


int MapHandlers::IntegerRV_META( CAOSMachine& vm )
{
	// check which camera we are on about...
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		// get the metaroomid from the centre of the room
		int32 x;
		int32 y;
		currentCamera->GetViewCentre(x,y);

		int metaRoomID;
		if(!theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D((int)x,(int)y),metaRoomID))
		{
			vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
		}
		return metaRoomID;
	}

	return theApp.GetWorld().GetMap().GetCurrentMetaRoom();
}

int MapHandlers::IntegerRV_RTYP( CAOSMachine& vm )
{
	int roomID = vm.FetchIntegerRV();
	if( roomID < 0 ) return -1;
	int type;
	if( !theApp.GetWorld().GetMap().GetRoomType( roomID, type ) )
		return -1;
	return type;
}

float MapHandlers::FloatRV_PROP( CAOSMachine& vm )
{
	int roomID = vm.FetchIntegerRV();
	int caIndex = vm.FetchIntegerRV();
	if( roomID < 0 ) return 0;
	float value;
	if( !theApp.GetWorld().GetMap().GetRoomProperty( roomID, caIndex, value ) )
		vm.ThrowRunError( CAOSMachine::sidFailedToGetRoomProperty );

	return value;
}


void MapHandlers::StringRV_BKGD( CAOSMachine& vm, std::string &str)
{
		// check which camera we are on about...
	Camera* currentCamera = vm.GetCamera();

	if(currentCamera)
	{
		str = currentCamera->GetBackgroundName();
		return;
	}

	int metaRoomID = vm.FetchIntegerRV();
	bool ok = theApp.GetWorld().GetMap().GetCurrentBackground(metaRoomID,
		str);
	if (!ok) {
		vm.ThrowRunError( CAOSMachine::sidFailedToGetCurrentBackground );
	}
}

void MapHandlers::StringRV_BKDS( CAOSMachine& vm, std::string& str )
{
	int metaRoomID = vm.FetchIntegerRV();
	StringCollection backgroundCollection;

	if( !theApp.GetWorld().GetMap().
		GetBackgroundCollection(metaRoomID, backgroundCollection) )
		vm.ThrowRunError( CAOSMachine::sidFailedToGetBackgrounds );

#ifdef C2E_OLD_CPP_LIB
	char hackbuf[1024];
	std::ostrstream stream( hackbuf, sizeof(hackbuf) );
#else
	std::stringstream stream;
#endif

	for( StringIterator i = backgroundCollection.begin(); i != backgroundCollection.end(); ++i )
	{
		if( i != backgroundCollection.begin() )
			stream << ',';
		stream << *i;
	}

	str = stream.str();

}

int MapHandlers::IntegerRV_ROOM( CAOSMachine& vm )
{
	AgentHandle agent = vm.FetchAgentRV();
	if (agent.IsInvalid()) {
 		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );
	}
	int roomID = -1;

	// we return -1 if not in a valid room
	if (!agent.GetAgentReference().GetRoomID( roomID ))
		roomID = -1;

	ASSERT(roomID >= -1 && roomID < INT_MAX	);

	return roomID;
}

int MapHandlers::IntegerRV_HIRP( CAOSMachine& vm )
{
	int roomID = vm.FetchIntegerRV();
	int caIndex = vm.FetchIntegerRV();
	int option = vm.FetchIntegerRV();
	if( roomID < 0 )
		return -1;

	int highRoom;
	if( !theApp.GetWorld().GetMap().GetRoomIDWithHighestCA( roomID, caIndex, option != 0, highRoom ) )
	{
		vm.ThrowRunError( CAOSMachine::sidFailedToFindHighestCA );
	}
	return highRoom;
}

int MapHandlers::IntegerRV_LORP( CAOSMachine& vm )
{
	int roomID = vm.FetchIntegerRV();
	int caIndex = vm.FetchIntegerRV();
	int option = vm.FetchIntegerRV();
	if( roomID < 0 )
		return -1;

	int lowRoom;
	if( !theApp.GetWorld().GetMap().GetRoomIDWithLowestCA( roomID, caIndex, option != 0, lowRoom ) )
	{
		vm.ThrowRunError( CAOSMachine::sidFailedToFindLowestCA );
	}
	return lowRoom;
}

float MapHandlers::FloatRV_TORX( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int roomID = vm.FetchIntegerRV();
	if( roomID < 0 ) return 0.0f;
	Vector2D vector;
	theApp.GetWorld().GetMap().GetRoomCentre( roomID, vector );
	return vector.x - vm.GetTarg().GetAgentReference().GetPosition().x;
}

float MapHandlers::FloatRV_TORY( CAOSMachine& vm )
{
	vm.ValidateTarg();
	int roomID = vm.FetchIntegerRV();
	if( roomID < 0 ) return 0.0f;
	Vector2D vector;
	theApp.GetWorld().GetMap().GetRoomCentre( roomID, vector );
	return vector.y - vm.GetTarg().GetAgentReference().GetPosition().y;
}

int MapHandlers::IntegerRV_GRAP( CAOSMachine& vm )
{
	float x = vm.FetchFloatRV();
	float y = vm.FetchFloatRV();

	int roomID;

	if ( !theApp.GetWorld().GetMap().GetRoomIDForPoint(Vector2D(x,y),roomID) )
		roomID = -1;
	return roomID;
}

void MapHandlers::Command_BRMI( CAOSMachine& vm )
{
	int mrbase = vm.FetchIntegerRV();
	int rmbase = vm.FetchIntegerRV();
	theApp.GetWorld().GetMap().SetIndexBases( mrbase, rmbase );
}

int MapHandlers::IntegerRV_GMAP( CAOSMachine& vm )
{
	float x = vm.FetchFloatRV();
	float y = vm.FetchFloatRV();

	int roomID;

	if ( !theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(x,y),roomID) )
		roomID = -1;
	return roomID;
}

int MapHandlers::IntegerRV_GRID( CAOSMachine& vm )
{
	int roomID;
	AgentHandle agent = vm.FetchAgentRV();
	if (agent.IsInvalid()) {
 		vm.ThrowInvalidAgentHandle( CAOSMachine::sidInvalidAgent );
	}
	int direction = vm.FetchIntegerRV();

	// If in vehicle with cabin room, return no adjacent rooms
	Agent& agentRef = agent.GetAgentReference();
	if (agentRef.GetMovementStatus() == Agent::INVEHICLE)
	{
		int cabinRoom = agentRef.GetCarrier().GetVehicleReference().GetCabinRoom();
		if (cabinRoom > -1)
			return -1;
	}

	// Get centre of agent
	Vector2D centre = agentRef.GetCentre();
	bool ok = theApp.GetWorld().GetMap().SearchForAdjacentRoom
		(centre, direction, roomID);
	if (!ok)
		return -1;
	return roomID;
}

void MapHandlers::StringRV_EMID( CAOSMachine& vm, std::string& str )
{
	IntegerCollection metaRoomIDCollection;

	theApp.GetWorld().GetMap().GetMetaRoomIDCollection( metaRoomIDCollection );

#ifdef C2E_OLD_CPP_LIB
	char hackbuf[1024];
	std::ostrstream stream( hackbuf, sizeof(hackbuf) );
#else
	std::stringstream stream;
#endif

	for( IntegerIterator i = metaRoomIDCollection.begin(); i != metaRoomIDCollection.end(); ++i )
		stream << ' ' << *i;

	str = stream.str();
}

void MapHandlers::StringRV_ERID( CAOSMachine& vm, std::string& str )
{
	int metaRoomID = vm.FetchIntegerRV();

	IntegerCollection roomIDCollection;

	int count;

	if( metaRoomID == -1 )
		theApp.GetWorld().GetMap().GetRoomIDCollection( roomIDCollection );
	else
		if( !theApp.GetWorld().GetMap().GetRoomIDCollectionForMetaRoom( metaRoomID, roomIDCollection, count ) )
			vm.ThrowRunError( CAOSMachine::sidFailedToGetRoomIDs );

#ifdef C2E_OLD_CPP_LIB
	char hackbuf[32768];
	std::ostrstream stream( hackbuf, sizeof(hackbuf) );
#else
	std::stringstream stream;
#endif

	for( IntegerIterator i = roomIDCollection.begin(); i != roomIDCollection.end(); ++i )
		stream << ' ' << *i;

	str = stream.str();
}

void MapHandlers::StringRV_RLOC( CAOSMachine& vm, std::string& str )
{
	int roomID = vm.FetchIntegerRV();
	int metaRoomID, xLeft, xRight, yLeftCeiling, yRightCeiling,
		yLeftFloor, yRightFloor;

	if( !theApp.GetWorld().GetMap().GetRoomLocation(roomID, metaRoomID,
		xLeft, xRight, yLeftCeiling, yRightCeiling, yLeftFloor, yRightFloor) )
		vm.ThrowRunError( CAOSMachine::sidFailedToGetRoomLocation );

#ifdef C2E_OLD_CPP_LIB
	char hackbuf[256];
	std::ostrstream stream( hackbuf, sizeof(hackbuf) );
#else
	std::stringstream stream;
#endif

	stream << xLeft << ' ' << xRight << ' ' <<
		yLeftCeiling << ' ' << yRightCeiling << ' ' <<
		yLeftFloor << ' ' << yRightFloor;

	str = stream.str();
}

void MapHandlers::StringRV_MLOC( CAOSMachine& vm, std::string& str )
{
	int metaRoomID = vm.FetchIntegerRV();
	int positionX, positionY, width, height;

	if( !theApp.GetWorld().GetMap().GetMetaRoomLocation( metaRoomID, positionX, positionY, width, height) )
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );

#ifdef C2E_OLD_CPP_LIB
	char hackbuf[256];
	std::ostrstream stream( hackbuf, sizeof(hackbuf) );
#else
	std::stringstream stream;
#endif

	stream << positionX << ' ' << positionY << ' ' << width << ' ' << height;

	str = stream.str();
}
