////////////////////////////////////////////////////////////////////////////////
// Filename:	Map.cpp
// Class:		Map
//
// Description: Stores all data relevant to the map and provides member 
//				functions for adding, removing and retrieving information about 
//				meta-rooms and rooms. It also provides for line of sight
//				testing; agent/creature collision testing; agent/creature 
//				location testing; agent/creature physics; serialization.
//
// Author:		Robert Dickson
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Map.h"
#include "../C2eServices.h"
#include "../App.h"
#include "../World.h"


CREATURES_IMPLEMENT_SERIAL(Map)
CREATURES_IMPLEMENT_SERIAL(Door)
CREATURES_IMPLEMENT_SERIAL(Room)
CREATURES_IMPLEMENT_SERIAL(MetaRoom)


// 
// Constructor
//
Map::Map(void) 
{
	Initialise();
}


//
// Destructor
//
Map::~Map(void)
{
	Cleanup();
}



// ---------------------------------------------------------------------------
// Function:	Reset
// Description:	Resets the map to its initial, empty state
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
void Map::Reset(void)
{
	Cleanup();
	Initialise();
}


// ---------------------------------------------------------------------------
// Function:	Initialise
// Description:	Initialises the map by setting member variables to their
//				default state
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
void Map::Initialise(void)
{
	int i;

	myRoomIDCollection.clear();
	myMetaRoomIDCollection.clear();
	myMetaRoomIndexBase = 0;
	myRoomIndexBase = 0;
	myDoorCollection.clear();
	myConstructedDoorCollection.clear();
	myLinkCollection.clear();
	myRoomCount = 0;
	myMaxRoomID = -1;
	for (i=0; i<MAX_ROOMS; ++i)
	{
		myRoomCollection[i] = NULL;
		myAlreadyProcessedList[i] = 0;
	}
	myAlreadyProcessedTruthValue = 0;
	myMetaRoomCount = 0;
	myMetaRoomMaxID = -1;
	myMetaRoomID = -1;
	for (i=0; i<MAX_META_ROOMS; ++i)
		myMetaRoomCollection[i] = NULL;
	myIntegerCollectionGrid = NULL;
	myMapWidth = 100.0f;
	myMapHeight = 100.0f;
	myGridSquareSize = 100.0f;
	myMetaRoomMaxCoordinates = Vector2D(-1.0f, -1.0f);
	mySquareCountX = -1;
	mySquareCountY = -1;
	myNextCAProperty = 0;
	BuildCache();
}


// ---------------------------------------------------------------------------
// Function:	Cleanup
// Description:	Frees all heap-allocated memory
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
void Map::Cleanup(void)
{
	DoorIterator doorIterator;
	DoorIterator doorIteratorEnd;
	Door* door;
	Room* room;
	MetaRoom* metaRoom;
	int i;

	// Clean up door memory
	doorIterator = myDoorCollection.begin(); 
	doorIteratorEnd = myDoorCollection.end();
	while (doorIterator != doorIteratorEnd) 
	{
		door = *doorIterator;
		delete door;
		++doorIterator;
	}
	// Clean up room memory
	for (i=0; i<=myMaxRoomID; ++i) 
	{
		room = myRoomCollection[i];
		if (room != NULL)
			delete room;
	}
	// Clean up meta-room memory
	for (i=0; i<=myMetaRoomMaxID; ++i) 
	{
		metaRoom = myMetaRoomCollection[i];
		if (metaRoom != NULL)
			delete metaRoom;
	}
	// Destroy the grid if it exists
	if (myIntegerCollectionGrid != NULL)
		delete []myIntegerCollectionGrid;
	// Clear the link collection
	if (myLinkCollection.size() > 0)
	{
		while (myLinkCollection.size() > 0)
		{
			delete *(myLinkCollection.begin());
			myLinkCollection.erase(myLinkCollection.begin());
		}
	}
}



//
//
// Low-level maths
//
//

// ---------------------------------------------------------------------------
// Function:	DoesPathIntersectLine
// Description:	Tests if a path intersects with a line
// Arguments:	startPath - start of the path (in)
//				endPath - end of the path (in)
//				deltaPath - delta of the path (in)
//				startLine - start of the line (in)
//				endLine - end of the line (in)
//				deltaLine - delta of the line (in)
//				intersectionPath - intersection point along the path (out)
//              intersectionLine - intersection point along the line (out)
// Returns:		true if there is an intersection; false otherwise
// ---------------------------------------------------------------------------
bool Map::DoesPathIntersectLine
	(const Vector2D& startPath,
	 const Vector2D& endPath, 
	 const Vector2D& deltaPath, 
	 const Vector2D& startLine, 
	 const Vector2D& endLine, 
	 const Vector2D& deltaLine,
	 Vector2D& intersectionPath, 
	 Vector2D& intersectionLine)
{
	register float denominator = 
		deltaPath.x*deltaLine.y - deltaPath.y*deltaLine.x;

	if (denominator == 0.0f)
		// Collinear, no intersection
		return false;

	Vector2D diff = startPath - startLine;

	register float parameterLine = 
		(deltaPath.x*diff.y - deltaPath.y*diff.x) / denominator;
	if ((parameterLine < 0.0f) || (parameterLine > 1.0f))
		// The path never crosses the line, no intersection
		return false;

	register float parameterPath = 
		(deltaLine.x*diff.y - deltaLine.y*diff.x) / denominator;
	if (parameterPath < 0.0f)
		// Line is behind the path, no intersection
		return false;
	
	float pathLength = deltaPath.Length();

	// Get the (smallest) angle between the path and the line
	float th = acos(deltaPath.DotProduct(deltaLine)/
		(pathLength*deltaLine.Length()));
	if (th > HALFPI)
		th = PI - th; 
	// Get the world tolerance as the tolerance along the path projected onto 
	// the line normal. This ensures that the intersection is always a fixed 
	// distance away from the line.
	float sinth = sin(th);
	float toleranceWorld;
	if (sinth == 0.0f)
		toleranceWorld = TOLERANCE;
	else
		toleranceWorld = TOLERANCE/sinth;
	// Get the tolerance as a parameter in the range [0,1]
	float toleranceParametric = toleranceWorld/pathLength;
	// To prevent rounding issues we need to:
	//    1) scan ahead by the tolerance
	//    2) rewind the intersection point backwards along the path by 
	//       the tolerance
	if (parameterPath > 1.0f + toleranceParametric)
		// Path (plus the tolerance) stops short of the line, no intersection
		return false;
	if (parameterPath > 1.0f)
		parameterPath = 1.0f - toleranceParametric;
	else
		parameterPath -= toleranceParametric;
	if (parameterPath < 0.0f)
		parameterPath = 0.0f;
	// Calculate the two intersection points
	intersectionPath = startPath + deltaPath*parameterPath;
	intersectionLine = startLine + deltaLine*parameterLine;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	DoesPathCrossOrTouchLine
// Description:	Tests if a path crosses or touches a line
// Arguments:	startPath - start of the path (in)
//				endPath - end of the path (in)
//				deltaPath - delta of the path (in)
//				startLine - start of the line (in)
//				endLine - end of the line (in)
//				deltaLine - delta of the line (in)
//				intersection - intersection point (out)
// Returns:		true if there is an intersection; false otherwise
// ---------------------------------------------------------------------------
bool Map::DoesPathCrossOrTouchLine
	(const Vector2D& startPath,
	 const Vector2D& endPath, 
	 const Vector2D& deltaPath, 
	 const Vector2D& startLine, 
	 const Vector2D& endLine, 
	 const Vector2D& deltaLine,
	 Vector2D& intersection)
{
	register float denominator = 
		deltaPath.x*deltaLine.y - deltaPath.y*deltaLine.x;

	if (denominator == 0.0f)
		// Collinear, no intersection
		return false;

	Vector2D diff = startPath - startLine;

	register float parameterLine = 
		(deltaPath.x*diff.y - deltaPath.y*diff.x) / denominator;
	if ((parameterLine < 0.0f) || (parameterLine > 1.0f))
		// No intersection
		return false;

	register float parameterPath = 
		(deltaLine.x*diff.y - deltaLine.y*diff.x) / denominator;
	if ((parameterPath < 0.0f) || (parameterPath > 1.0f))
		// No intersection
		return false;
	// Calculate the intersection point
	intersection = startPath + deltaPath*parameterPath;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetLineBoundingBox
// Description:	Gets the bounding box of a line segment
// Arguments:	start - start of the line segment (in)
//				end - end of the line segment (in)
//				positionMin - minimum point of the bounding box (out)
//				positionMax - maximum point of the bounding box (out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::GetLineBoundingBox
	(const Vector2D& start, 
	 const Vector2D& end, 
	 Vector2D& positionMin, 
	 Vector2D& positionMax)
{
	// Deal with X coordinate
	if (start.x < end.x) 
	{
		positionMin.x = start.x;
		positionMax.x = end.x;
	}
	else 
	{
		positionMin.x = end.x;
		positionMax.x = start.x;
	}
	// Deal with Y coordinate
	if (start.y < end.y) 
	{
		positionMin.y = start.y;
		positionMax.y = end.y;
	}
	else 
	{
		positionMin.y = end.y;
		positionMax.y = start.y;
	}
}


// ---------------------------------------------------------------------------
// Function:	IsPointOnLine
// Description:	Tests if a point lies close to a line segment
// Arguments:	position - point to test (in)
//				start - start of the line segment (in)
//				end - end of the line segment (in)
//				delta - delta of the line segment (in)
//				tolerance - acceptable tolerance (in)
// Returns:		true if on line segment; false otherwise
// ---------------------------------------------------------------------------
bool Map::IsPointOnLine
	(const Vector2D& position, 
	 const Vector2D& start, 
	 const Vector2D& end, 
	 const Vector2D& delta, 
	 const float tolerance) 
{
	float m, y;
	Vector2D positionMin, positionMax;

	GetLineBoundingBox(start, end, positionMin, positionMax);
	if (delta.x != 0.0f) 
	{
		// Non-vertical line
		// Check that x is in range
		if ((position.x < positionMin.x) || 
		    (position.x > positionMax.x))
			return false;
		// Check that y coordinate is close enough
		m = delta.y/delta.x;
		y = start.y + m*(position.x - start.x);
		return (fabsf(y-position.y) <= tolerance);
	}
	else 
	{
		// Vertical line
		if (position.x != start.x)
			return false;
		if (position.y < (positionMin.y - tolerance))
			return false;
		if (position.y > (positionMax.y + tolerance))
			return false;
		return true;
	}
}


// ---------------------------------------------------------------------------
// Function:	IsPointBelowCeilingDoors
// Description:	Tests if a point lies below a room's ceiling doors
// Arguments:	position - point to test (in)
//				room - room to test against (in)
// Returns:		true if below
// ---------------------------------------------------------------------------
bool Map::IsPointBelowCeilingDoors
	(const Vector2D& position, Room* room)
{
	DoorIterator doorIterator = room->ceilingCollection.begin();
	int count = room->ceilingCollection.size();
	Door* door;
	float dummy;

	while (count-- > 0) 
	{
		door = *(doorIterator++);
		if ((position.x < door->start.x) || (position.x > door->end.x))
			continue;
		return IsPointBelowLine(position, door->start, door->delta, dummy);
	}
	_ASSERT(false);
	return false;
}


// ---------------------------------------------------------------------------
// Function:	IsPointAboveFloorDoors
// Description:	Tests if a point lies above a room's floor doors
// Arguments:	position - point to test (in)
//				room - room to test against (in)
// Returns:		true if above
// ---------------------------------------------------------------------------
bool Map::IsPointAboveFloorDoors
	(const Vector2D& position, Room* room)
{
	DoorIterator doorIterator = room->floorCollection.begin();
	int count = room->floorCollection.size();
	Door* door;
	float dummy;

	while (count-- > 0) 
	{
		door = *(doorIterator++);
		if ((position.x < door->start.x) || (position.x > door->end.x))
			continue;
		return IsPointAboveLine(position, door->start, door->delta, dummy);
	}
	_ASSERT(false);
	return false;
}


// ---------------------------------------------------------------------------
// Function:	GetGridIndexesForPoint
// Description:	Gets the X and Y grid indexes for an arbitrary point
// Arguments:	position - point to get indexes for (in)
//				squareX - X grid index (out) 
//				squareY - Y grid index (out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::GetGridIndexesForPoint
	(const Vector2D& position, int& squareX, int& squareY) 
{
	if (position.x < myGridSquareSize)
		squareX = 0;
	else if (position.x >= myMapWidth-1.0f)
		squareX = mySquareCountX-1;
	else
		squareX = FastFloatToInteger((position.x/myGridSquareSize)-0.5f);
	if (position.y < myGridSquareSize)
		squareY = 0;
	else if (position.y >= myMapHeight-1.0f)
		squareY = mySquareCountY-1;
	else
		squareY = FastFloatToInteger((position.y/myGridSquareSize)-0.5f);
}


// ---------------------------------------------------------------------------
// Function:	GetRoomInformationForPoint
// Description:	Gets room information for a given point
// Arguments:	position - point to get information about (in)
//				roomIDList - array of room ID numbers (out)
//				roomList - array of room pointers (out)
//				roomCount - how many rooms contain the point (out) 
//				roomID - the "primary" room ID of those found (out)
// Returns:		true if good input, false otherwise
// Notes:		A point can be in a maximum of four rooms when it lies on a 
//				vertex. The primary ID is the leftmost/topmost.
// ---------------------------------------------------------------------------
bool Map::GetRoomInformationForPoint
	(const Vector2D& position, 
	 int roomIDList[4], 
	 Room* roomList[4],
	 int& roomCount,
	 int& roomID)
{
	int xSquare, ySquare;

	// Get which grid square the point lies in
	GetGridIndexesForPoint(position, xSquare, ySquare);
	// Get the index of the grid square
	int index = ySquare*mySquareCountX + xSquare;
	roomCount = 0;
	// Get number of rooms that cross this grid square
	int count = myIntegerCollectionGrid[index].size();
	if (count == 0)
		return false;
	roomID = INT_MAX;
	IntegerIterator integerIterator = myIntegerCollectionGrid[index].begin();
	int id;
	Room* room;
	while (count-- > 0) 
	{
		id = *(integerIterator++);
		room = myRoomCollection[id];
		if ((position.x >= room->positionMin.x) && 
			(position.x <= room->positionMax.x)) 
		{			
			// Check against the ceiling
			if (IsPointBelowCeilingDoors(position, room))
			{
				// Check against the floor
				if (IsPointAboveFloorDoors(position, room))
				{
					roomIDList[roomCount] = id;
					roomList[roomCount] = room;
					++roomCount;
				}	
			}
		}
	}
	if (roomCount == 0)
		return false;
	else if (roomCount == 1) 
	{
		roomID = roomIDList[0];
		return true;
	}

	// Find the room that is leftmost and topmost
	float minAngle = FLT_MAX;
	float angle;
	Vector2D v;
	int i;
	for (i=0; i<roomCount; ++i) 
	{
		room = roomList[i];
		v = room->centre - position;
		if (v == ZERO_VECTOR) 
		{
			roomID = roomIDList[i];
			return true;
		}
		v = v.Normalise();
		angle = acos(-v.y);
		if (v.x > 0.0f) 
			// clockwise
			angle = 2.0f*PI - angle; 
		if (angle < minAngle) 
		{
			minAngle = angle;
			roomID = roomIDList[i];
		}
	}
	return true;
}



// ---------------------------------------------------------------------------
// Function:	IsPointInsideRoom
// Description:	Tests if a point lies wholly within a room
// Arguments:	position - point to test (in)
//				room - room to test against (in)
// Returns:		true if inside the room
// ---------------------------------------------------------------------------
bool Map::IsPointInsideRoom(const Vector2D& position, Room* room)
{
	float xMin, xMax;
	float yBelow, yAbove;
	float below, above;
	xMin = room->positionMin.x + 0.5f;
	xMax = room->positionMax.x - 0.5f;
	if ((position.x > xMin) && (position.x < xMax)) 
	{			
		// Check against the ceiling
		below = IsPointBelowLine(position, room->startCeiling, 
			room->deltaCeiling, yBelow);
		if (below && (yBelow > 0.5f)) 
		{
			// Check against the floor
			above = IsPointAboveLine(position, room->startFloor, 
				room->deltaFloor, yAbove);
			if (above && (yAbove > 0.5f)) 
				return true;
		}
	}
	return false;
}


// ---------------------------------------------------------------------------
// Function:	IsRegionInsideSameRoom
// Description:	Tests if a region of the map lies within the same room
// Arguments:	positionMin - min point of region (in)
//				positionMax - max point of region (in)
// Returns:		true if inside same room
// Notes:		myTempRoomCollection stores the rooms
//				myTempRoomCount stores the room count
// ---------------------------------------------------------------------------
bool Map::IsRegionInsideSameRoom
	(const Vector2D& positionMin, 
	 const Vector2D& positionMax)
{
	int i;
	bool found = false;
	Room* room;
	// Determine which of the rooms (if any) that the minimum point is in
	for (i=0; i<myTempRoomCount; ++i)
	{
		room = myTempRoomCollection[i];
		if (IsPointInsideRoom(positionMin, room))
		{
			found = true;
			break;
		}
	}
	if (!found)
		return false;

	// Check that the other three corners of the region are wholly inside
	// the same room as the minimum point
	if (!IsPointInsideRoom(positionMax, room))
		return false;

	Vector2D temp;
	temp.x = positionMin.x;
	temp.y = positionMax.y;

	if (!IsPointInsideRoom(temp, room))
		return false;

	temp.x = positionMax.x;
	temp.y = positionMin.y;

	if (!IsPointInsideRoom(temp, room))
		return false;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetRoomsOverlappingRegion
// Description:	Gets the set of rooms that overlap a region of the map
// Arguments:	xSquareMin, ySquareMin - min grid square of region (in)
//				xSquareMax, ySquareMax - max grid square of region (in)
// Returns:		true if inside same room
// Notes:		myTempRoomCollection stores the rooms
//				myTempRoomCount stores the room count
// ---------------------------------------------------------------------------
void Map::GetRoomsOverlappingRegion
	(const int xSquareMin,
	 const int ySquareMin,
	 const int xSquareMax,
	 const int ySquareMax)
{
	int xSquare, ySquare;
	int i, index, count, roomID;
	IntegerIterator idIterator;
	
	// This is an extremely fast way of knowing if we've seen a room ID
	// before
	++myAlreadyProcessedTruthValue;
	if (myAlreadyProcessedTruthValue == 0)
	{
		for (i=0; i<MAX_ROOMS; ++i)
			myAlreadyProcessedList[i] = 0;
		myAlreadyProcessedTruthValue = 1;
	}

	myTempRoomCount = 0;
	for (xSquare=xSquareMin; xSquare<=xSquareMax; ++xSquare) 
	{	
		for (ySquare=ySquareMin; ySquare<=ySquareMax; ++ySquare) 
		{
			index = ySquare*mySquareCountX + xSquare;
			idIterator = myIntegerCollectionGrid[index].begin();
			count = myIntegerCollectionGrid[index].size();
			while (count-- > 0)
			{
				roomID = *(idIterator++);
				if (myAlreadyProcessedList[roomID] == myAlreadyProcessedTruthValue)
					// Already seen this room so ignore it
					continue;
				// Flag that we've seen this room ID
				myAlreadyProcessedList[roomID] = myAlreadyProcessedTruthValue;
				myTempRoomCollection[myTempRoomCount++] = myRoomCollection[roomID];
			}
		}
	}
}


// ---------------------------------------------------------------------------
// Function:	SearchForAdjacentRoom
// Description:	Searches for an adjacent room in a given direction
// Arguments:	position - the position to test (in)
//				direction - the direction to test (in)
//				roomID - the adjacent room ID or -1 if none (out)
// Returns:		true if position is valid, false if not
// ---------------------------------------------------------------------------
bool Map::SearchForAdjacentRoom
	(const Vector2D& position, 
	 const int direction,
	 int& roomID)
{
	int roomIDList[4];
	Room* roomList[4];
	int roomCount;
	int startID;
	int id;
	Room* room;
	IntegerCollection neighbours;
	IntegerIterator integerIterator;
	IntegerIterator integerIteratorEnd;
	float xmin = FLT_MAX;
	float ymin = FLT_MAX;

	bool ok = GetRoomInformationForPoint(position, roomIDList, roomList,
		roomCount, startID);
	if (!ok)
		return false;

	_ASSERT((direction >= 0) && (direction <= 3));

	roomID = -1;
	// Get neighbours
	GetNeighbours(startID, direction, neighbours);
	integerIterator = neighbours.begin();
	integerIteratorEnd = neighbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		if ((direction == DIRECTION_UP) || (direction == DIRECTION_DOWN))
		{
			if ((position.x >= room->startCeiling.x) && 
				(position.x <= room->endCeiling.x))
			{
				if (room->startCeiling.x < xmin) 
				{
					xmin = room->startCeiling.x;
					roomID = id;
				}
			} 
		}
		else if (direction == DIRECTION_LEFT)
		{
			if ((position.y >= room->endCeiling.y) && 
				(position.y <= room->endFloor.y))
			{
				if (room->endCeiling.y < ymin) 
				{
					ymin = room->endCeiling.y;
					roomID = id;
				}
			} 
		}
		else if (direction == DIRECTION_RIGHT)
		{
			if ((position.y >= room->startCeiling.y) && 
				(position.y <= room->startFloor.y))
			{
				if (room->startCeiling.y < ymin) 
				{
					ymin = room->startCeiling.y;
					roomID = id;
				}
			} 
		}
		++integerIterator;
	}
	return true;
}





// ---------------------------------------------------------------------------
// Function:	CalculateSlideAccelerationAndSlideVelocity
// Description:	Projects velocity and gravity onto a slope 
// Arguments:	velocity - velocity vector (in)
//				slope - slope vector (in) 
//				gravity - gravitational acceleration (in)
//				slideAcceleration - projection of gravity (out)
//				slideVelocity - projection of velocity (out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateSlideAccelerationAndSlideVelocity
	(const Vector2D& velocity, 
	 const Vector2D& slope, 
	 const Vector2D& gravity,
	 Vector2D& slideAcceleration, 
	 Vector2D& slideVelocity)
{
	// Get a unit vector of the slope
	Vector2D slopeUnit(slope.Normalise());
	// Project the gravitational acceleration onto the slope
	slideAcceleration = slopeUnit*(slopeUnit.DotProduct(gravity));
	// Project the velocity vector onto the slope
	slideVelocity = slopeUnit*(slopeUnit.DotProduct(velocity));
}


// ---------------------------------------------------------------------------
// Function:	CalculateCollisionVelocityAndElapsedTime
// Description:	um...
// Arguments:	velocity - velocity before move (in)
//				acceleration - acceleration during the move (in) 
//				path - trajectory of the move (in)
//				deltaTravelled - how far actually travelled (in)
//				velocityCollision - velocity at collision point (out)
//				timeElapsed - how much time used up by the move as a number
//					in the range [0,1] (out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateCollisionVelocityAndElapsedTime
	(const Vector2D& velocity, 
	 const Vector2D& acceleration, 
	 const Vector2D& path, 
	 const Vector2D& deltaTravelled, 
	 Vector2D& velocityCollision, 
	 float& timeElapsed)
{
	//
	// Work out the x collision velocity
	//

	if (acceleration.x == 0.0f) 
	{
		velocityCollision.x = velocity.x;
	}
	else 
	{

		float temp = velocity.x*velocity.x+2.0f*acceleration.x*deltaTravelled.x;
		_ASSERT(temp >= 0.0f);
		float vx = sqrtf(temp);
		if (path.x < 0.0f)
			vx = -vx;
		velocityCollision.x = vx;
	}

	//
	// Work out y collision velocity
	//

	if (acceleration.y == 0.0f) 
	{
		velocityCollision.y = velocity.y;
	}
	else 
	{
		float temp = velocity.y*velocity.y+2.0f*acceleration.y*deltaTravelled.y;
		_ASSERT(temp >= 0.0f);
		float vy = sqrtf(temp);
		if (path.y < 0.0f)
			vy = -vy;
		velocityCollision.y = vy;
	}

	//
	// Work out the elapsed time 
	//

	if ((acceleration.x == 0.0f) && (acceleration.y == 0.0f)) 
	{
		if (velocity.x != 0.0f)
			timeElapsed = deltaTravelled.x/velocity.x;
		else if (velocity.y != 0.0f)
			timeElapsed = deltaTravelled.y/velocity.y;
		else 
			timeElapsed = 0.0f;
	}
	else if (acceleration.x == 0.0f) 
		timeElapsed = (velocityCollision.y - velocity.y) / acceleration.y;
	else 
		timeElapsed = (velocityCollision.x - velocity.x) / acceleration.x;
}



// ---------------------------------------------------------------------------
// Function:	CalculateWallFromDoorAndPath
// Description:	Gets the wall type of a blocking door
// Arguments:	door - the blocking door (in)
//				path - the trajectory that was blocked (in)
// Returns:		The wall type
// ---------------------------------------------------------------------------
int Map::CalculateWallFromDoorAndPath(Door* door, const Vector2D& path) 
{
	if (door->doorType == DOOR_CEILING_FLOOR) 
	{
		if ((door->delta).CrossProduct(path) >= 0.0f)
			return DIRECTION_FLOOR;
		else
			return DIRECTION_CEILING;
	}
	else 
	{
		if (path.x >= 0.0f) 
			return DIRECTION_RIGHT;
		else
			return DIRECTION_LEFT;
	}
}



// ---------------------------------------------------------------------------
// Function:	GetReflectionFromVertex
// Description:	Reflects a vector off the blocking doors that meet at a vertex
// Arguments:	positionVertex - position of the vertex (in)
//				v - incoming vector (in)
//				useEdgeDelta - true to use "edgeDelta" parameter (in)
//				edgeDelta - agents striking a spike along an edge of 
//					their diamond are best reflected about that edge. This
//					gives the delta of the edge vector.
//				minDoorPermiability - minimum door permiability (in)
//				vReflection - the reflected vector (out)
//				slidingDoor - the door easiest to slide along (out)
//				spike - true if collided with a spike (out)
// Returns:		None	
// ---------------------------------------------------------------------------
void Map::GetReflectionFromVertex
	(const Vector2D& positionVertex, 
	 const Vector2D& v,
	 const bool useEdgeDelta,
	 const Vector2D& edgeDelta,
	 const int minDoorPermiability, 
	 Vector2D& vReflection,
	 Door& slidingDoor,
	 bool& spike)
{
	Vector2D mirror;
	int roomIDList[4];
	Room* roomList[4];
	int roomCount;
	int i, j, count, roomID;
	Room* room;
	DoorIterator doorIterator, doorIteratorEnd;
	Door* door;
	Door* leftDoor, *rightDoor, *parallelDoor;
	bool found;
	bool gotLeft, gotRight;
	int parallelCount;
	float angle;
	Vector2D vAnti;
	Vector2D vAntiUnit;
	Vector2D parallel;
	struct unitVector2D {Vector2D v; Door* door;} unitVectors[4];
	float leftAngle, rightAngle;
	Vector2D left, right;
	float ax, ay, bx, by, z;
	int dummy1;
	int doorCount;
	Door* d1, *d2;
	bool ok;

	_ASSERT((v.x != 0.0f) || (v.y != 0.0f));

	count = 0;
	// Get the rooms that contain the vertex
	ok = GetRoomInformationForPoint(positionVertex, roomIDList, roomList,
		roomCount, dummy1);
	_ASSERT(ok);

	for (i=0; i<roomCount; ++i) 
	{
		roomID = roomIDList[i];
		room = myRoomCollection[roomID];
		doorIterator = room->doorCollection.begin();
		doorIteratorEnd = room->doorCollection.end();
		while (doorIterator != doorIteratorEnd) 
		{
			door = *(doorIterator++);
			if (door->permiability >= minDoorPermiability) 
				// Ignore doors that aren't blocking
				continue;
			
			found = false;
			for (j=0; j<count; ++j) 
			{
				if (unitVectors[j].door == door) 
				{
					found = true;
				}
			}
			if (found)
				// Ignore doors that we've seen already
				continue;

			// Store a unit vector heading away from the vertex
			if (door->start == positionVertex) 
			{
				unitVectors[count].v = (door->delta).Normalise();
				unitVectors[count].door = door;
				++count;
			}
			else if (door->end == positionVertex) 
			{
				unitVectors[count].v = (-(door->delta)).Normalise();
				unitVectors[count].door = door;
				++count;
			}
		}
	}

	_ASSERT(count != 0);

	vAnti = -v;

	if (count == 1) 
	{
		// We've hit a "spike"
		slidingDoor = *(unitVectors[0].door);
		if (useEdgeDelta)
			vReflection = v.Reflect(edgeDelta);
		else
		{
			Vector2D delta(-slidingDoor.delta.y, slidingDoor.delta.x);
			vReflection = v.Reflect(delta);
		}
		spike = true;
		return;
	}

	// Get a unit vector of the incoming vector heading away from the vertex
	vAntiUnit = vAnti.Normalise();
	gotLeft = false;
	gotRight = false;
	leftAngle = FLT_MAX;
	rightAngle = FLT_MAX;
	parallelCount = 0;
	
	ax = vAntiUnit.x;
	ay = vAntiUnit.y;
	for (i=0; i<count; ++i) 
	{
		bx = unitVectors[i].v.x;
		by = unitVectors[i].v.y;
		z = ax*by-bx*ay;
		angle = acos(ax*bx + ay*by);
		if (z == 0.0f) 
		{
			// Parallel, neither left nor right
			++parallelCount;
			parallel = unitVectors[i].v;
			parallelDoor = unitVectors[i].door;
		}
		else if (z>0.0f) 
		{
			// Clockwise, right
			if (angle < rightAngle) 
			{
				gotRight = true;
				rightAngle = angle;
				right = unitVectors[i].v;
				rightDoor = unitVectors[i].door;
			}
		}
		else 
		{
			// Anti-clockwise, left
			if (angle < leftAngle) 
			{
				gotLeft = true;
				leftAngle = angle;
				left = unitVectors[i].v;
				leftDoor = unitVectors[i].door;
			}
		}
	}

	// Work out what the mirror should be
	if (gotLeft && !gotRight) 
	{
		if (parallelCount) 
		{
			doorCount = 2;
			d1 = parallelDoor;
			d2 = leftDoor;
			mirror = parallel - left;
		}
		else 
		{
			doorCount = 1;
			slidingDoor = *leftDoor;
			mirror = left;
		}
	}
	else if (!gotLeft && gotRight) 
	{
		if (parallelCount) 
		{
			doorCount = 2;
			d1 = parallelDoor;
			d2 = rightDoor;
			mirror = parallel - right;
		}
		else 
		{
			doorCount = 1;
			slidingDoor = *rightDoor;
			mirror = right;
		}
	}
	else if (gotLeft && gotRight) 
	{
		doorCount = 2;
		d1 = rightDoor;
		d2 = leftDoor;
		mirror = right - left;		
	}
	else 
	{
		doorCount = 1;
		slidingDoor = *parallelDoor;
		mirror = parallel;
	}

	_ASSERT((mirror.x != 0.0f) || (mirror.y != 0.0f));

	// Reflect the original vector off the mirror
	vReflection = v.Reflect(mirror);

	// Work out the door easiest to slide along
	if (doorCount == 2) 
	{
		if (d1->doorType != DOOR_CEILING_FLOOR) 
		{
			slidingDoor = *d2;
		}
		else if (d2->doorType != DOOR_CEILING_FLOOR) 
		{
			slidingDoor = *d1;
		}
		else 
		{
			if (v.x >= 0.0f) 
			{
				// Choose right hand floor/ceiling
				if (d1->end.x > d2->end.x)
					slidingDoor = *d1;
				else
					slidingDoor = *d2;
			}
			else 
			{
				// Choose left hand floor/ceiling
				if (d1->end.x < d2->end.x)
					slidingDoor = *d1;
				else
					slidingDoor = *d2;
			}
		}
	}
	spike = false;
}


//
//
// Line of sight testing
//
//


// ---------------------------------------------------------------------------
// Function:	DoesPathReachRoomBoundary
// Description:	Tests if a path crosses or touches a room's boundary
// Arguments:	room - pointer to room (in)
//				start - start of path (in)
//				end - end of path (in)
//				delta - delta of path (in)
//				minDoorPermiability - minimum door permiability (in)
//				reached - true if path reaches the boundary (out)
//				positionReached - where the path touches the boundary (out)
//				doorReached - which door the path reached, or NULL if path
//					passes through a vertex (out)
//				blocked - true if the path is blocked (out)
//				throughVertex - true if the path goes through a vertex (out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::DoesPathReachRoomBoundary
	(Room* room, 
	 const Vector2D& start, 
	 const Vector2D& end, 
	 const Vector2D& delta, 
	 const int minDoorPermiability, 
	 bool& reached, 
	 Vector2D& positionReached, 
	 Door*& doorReached, 
	 bool& blocked, 
	 bool& throughVertex)
{
	Door* door;
	bool intersect, blockedTemp, throughVertexTemp;
	Vector2D position, positionTemp;

	reached = false;
	DoorIterator doorIterator = room->doorCollection.begin();
	int count = room->doorCollection.size();
	while (count-- > 0) 
	{
		door = *(doorIterator++);
		// Determine if the path reaches this door
		intersect = DoesPathCrossOrTouchLine(start, end, delta, door->start, 
			door->end, door->delta, position);
		if (!intersect) 
			// No intersection, move on to the next door
			continue;
		if (position == start) 
			// The start of the path lies on the door so ignore it
			// and move on to the next door
			continue;
		// We've reached the room boundary
		reached = true;
		// Determine if the door blocks the path
		if (door->permiability < minDoorPermiability)
			blockedTemp = true;
		else
			blockedTemp = false;
		// Determine if the path passed through a vertex
		if ((position == door->start) ||
			(position == door->end))
			throughVertexTemp = true;
		else
			throughVertexTemp = false;

		if (blockedTemp || !throughVertexTemp) 
		{
			positionReached = position;
			if (throughVertexTemp)
				// Path reaches multiple doors, not just one
				doorReached = NULL;
			else
				doorReached = door;
			blocked = blockedTemp;
			throughVertex = throughVertexTemp;
			return;
		}

		// Get to here if the path passed through a vertex but wasn't 
		// blocked. We must scan all remaining doors to see if any of them DO 
		// block it.
		
		// Store the vertex position
		positionTemp = position;
	}

	if (reached) 
	{
		// Passed though a vertex but wasn't blocked
		positionReached = positionTemp;
		doorReached = NULL;
		blocked = false;
		throughVertex = true;
	}
}


// ---------------------------------------------------------------------------
// Function:	DoesPathReachSurroundingRoomBoundary
// Description:	Tests if a path crosses or touches any of the room boundaries 
//				surrounding a point
// Arguments:	start - start of path (in)
//				end - end of path (in)
//				delta - delta of path (in)
//				minDoorPermiability - minimum door permiability (in)
//				reached - true if path reaches a boundary (out)
//				positionReached - where the path touches the boundary (out)
//				doorReached - which door the path reached (out)
//				roomIDReached - which of the nearby rooms contained the 
//					crossed boundary (out)
//				blocked - true if the path is blocked (out)
//				throughVertex - true if the path goes through a vertex (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::DoesPathReachSurroundingRoomBoundary
	(const Vector2D& start, 
	 const Vector2D& end, 
	 const Vector2D& delta, 
	 const int minDoorPermiability, 
	 bool& reached, 
	 Vector2D& positionReached, 
	 Door*& doorReached, 
	 int& roomIDReached, 
	 bool& blocked, 
	 bool& throughVertex)
{
	int i, roomID, idTemp, dummy1;
	Door* door;
	bool reachedRoom, blockedTemp, throughVertexTemp;
	int roomIDList[4], roomCount;
	Room* roomList[4];
	Vector2D position, positionTemp;

	reached = false;
	// Get the rooms that contain the start point
	if (!GetRoomInformationForPoint(start, roomIDList, roomList,
		roomCount, dummy1))
		// The start point is invalid
		return false;

	for (i=0; i<roomCount; ++i) 
	{
		// Get the room's ID
		roomID = roomIDList[i];
		// Determine if the path reaches this room
		DoesPathReachRoomBoundary(roomList[i], start, end, 
			delta, minDoorPermiability, reachedRoom, position, door, 
			blockedTemp, throughVertexTemp);
		if (!reachedRoom)
			// Move on to next room
			continue;

		if (blockedTemp || !throughVertexTemp) 
		{
			reached = true;
			positionReached = position;
			if (throughVertexTemp) 
				// Path reaches multiple doors, not just one
				doorReached = NULL;
			else 
				doorReached = door;
			roomIDReached = roomID;
			blocked = blockedTemp;
			throughVertex = throughVertexTemp;
			return true;
		}
	
	    // Get to here if the path passed through a vertex but wasn't 
		// blocked. We must scan all remaining rooms to see if any of them DO 
		// block it.

		reached = true;
		// Store the vertex position
		positionTemp = position;
		// Store the room ID
		idTemp = roomID;
	}
	if (reached) 
	{
		// Passed though a vertex but wasn't blocked
		positionReached = positionTemp;
		doorReached = NULL;
		roomIDReached = idTemp;
		blocked = false;
		throughVertex = true;
	}
	return true;
}


// ---------------------------------------------------------------------------
// Function:	CanPointSeePoint
// Description:	Determines if one point can "see" another one
// Arguments:	start - first point (in)
//				end - second point (in)
//				minDoorPermiability - minimum door permiability (in)
//				canSee - true if point can see the other point (out)
// Returns:		true if good input, false otherwise
// --------------------------------------------------------------------------
bool Map::CanPointSeePoint
	(const Vector2D& start, 
	 const Vector2D& end, 
	 const int minDoorPermiability, 
	 bool& canSee)
{
	// Check that start point lies on the map
	if ((start.x < 0.0f) || (start.x >= myMapWidth))
		return false;
	if ((start.y < 0.0f) || (start.y >= myMapHeight))
		return false;

	bool blocked, throughVertex, reached;
	Door* door;
	int previousRoomID, reachedRoomID, roomID;
	Vector2D position, temp, tempDelta, delta = end-start;
	
	if (!DoesPathReachSurroundingRoomBoundary(start, end, delta, 
		minDoorPermiability, reached, temp, door, previousRoomID, blocked, 
		throughVertex))
		// First point is invalid
		return false;

	if (!reached) 
	{
		canSee = true;
		return true;
	}
	if (blocked) 
	{
		canSee = false;
		return true;
	}

	tempDelta = end-temp;

	while (tempDelta.SquareLength() >= 0.01f) 
	{
		if (throughVertex) 
		{
			(void)DoesPathReachSurroundingRoomBoundary(temp, end, tempDelta, 
				minDoorPermiability, reached, position, door, 
				reachedRoomID, blocked, throughVertex);
			previousRoomID = reachedRoomID;
		}
 		else 
		{
			// Pass in to neighbouring room
			roomID = GetAlternativeParent(door, previousRoomID);
			DoesPathReachRoomBoundary(myRoomCollection[roomID], temp, end, 
				tempDelta, minDoorPermiability, reached, position, door, 
				blocked, throughVertex);
			previousRoomID = roomID;
		}
		if (!reached) 
		{
			canSee = true;
			return true;
		}
		if (blocked) 
		{
			canSee = false;
			return true;
		}
		
		temp = position;
		tempDelta = end-position;
	}
	canSee = true;
	return true;
}


//
//
// Location validity testing
//
//


// ---------------------------------------------------------------------------
// Function:	FindSafeAgentLocation
// Description:	Finds a safe place on the map to place an agent, given a
//					preferred location
// Arguments:	positionPreferred - top-left corner of agent (in)
//				width - width of agent (in)
//				height - height of agent (in)
//				minDoorPermiability - minimum door permiability (in)
//				positionSafe - a safe place to locate the agent (out)
// Returns:		If a safe place could be found
// ---------------------------------------------------------------------------
bool Map::FindSafeAgentLocation
	(const Vector2D& positionPreferred,
	 const float width, 
	 const float height,
	 const int minDoorPermiability,
	 Vector2D& positionSafe)
{
	static const float unit = 5.0f;
	static const int count = 100;
	positionSafe = positionPreferred;
	float xmin;
	float xmax;
	int i, j;

	for (i=0; i<count; ++i) 
	{
		positionSafe.y -= unit;
		if (IsAgentLocationValidInRoomSystem(positionSafe, width, height, 
			minDoorPermiability))
			return true;
	}
	xmin = positionPreferred.x - unit;
	xmax = positionPreferred.x + unit;
	for (i=0; i<count; ++i)
	{
		positionSafe.x = xmin;
		positionSafe.y = positionPreferred.y;
		for (j=0; j<count; ++j) 
		{
			positionSafe.y -= unit;
			if (IsAgentLocationValidInRoomSystem(positionSafe, width, height, 
				minDoorPermiability))
				return true;
		}
		positionSafe.x = xmax;
		positionSafe.y = positionPreferred.y;
		for (j=0; j<count; ++j) 
		{
			positionSafe.y -= unit;
			if (IsAgentLocationValidInRoomSystem(positionSafe, width, height, 
				minDoorPermiability))
				return true;
		}
		xmin -= unit;
		xmax += unit;
	}

	positionSafe = positionPreferred;
	for (i=0; i<count; ++i) 
	{
		positionSafe.y += unit;
		if (IsAgentLocationValidInRoomSystem(positionSafe, width, height, 
			minDoorPermiability))
			return true;
	}
	xmin = positionPreferred.x - unit;
	xmax = positionPreferred.x + unit;
	for (i=0; i<count; ++i)
	{
		positionSafe.x = xmin;
		positionSafe.y = positionPreferred.y;
		for (j=0; j<count; ++j) 
		{
			positionSafe.y += unit;
			if (IsAgentLocationValidInRoomSystem(positionSafe, width, height, 
				minDoorPermiability))
				return true;
		}
		positionSafe.x = xmax;
		positionSafe.y = positionPreferred.y;
		for (j=0; j<count; ++j) 
		{
			positionSafe.y += unit;
			if (IsAgentLocationValidInRoomSystem(positionSafe, width, height, 
				minDoorPermiability))
				return true;
		}
		xmin -= unit;
		xmax += unit;
	}
	return false;
}




// ---------------------------------------------------------------------------
// Function:	FindSafeCreatureLocation
// Description:	Finds a safe place on the map to place a creature, given a
//					preferred location
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				currentDownFoot - which foot is currently down; 0 is
//					left, 1 is right (in)
//				minDoorPermiability - minimum door permiability (in)
//				downFootSafe - a safe place to locate the creature (out)
// Returns:		If a safe place could be found
// ---------------------------------------------------------------------------
bool Map::FindSafeCreatureLocation
	(const Vector2D& footLeft, 
	 const Vector2D& footRight, 
	 const float yMin,
	 const int currentDownFoot,
	 const int minDoorPermiability,
	 Vector2D& downFootSafe)
{
	static const float unit = 5.0f;
	static const int count = 100;
	float xminLeft;
	float xminRight;
	float xmaxLeft;
	float xmaxRight;
	int i, j;
	Vector2D left, right;
	float y;

	left = footLeft;
	right = footRight;
	y = yMin;
	for (i=0; i<count; ++i) 
	{
		left.y -= unit;
		right.y -= unit;
		y -= unit;
		if (IsCreatureLocationValidInRoomSystem(left, right, y, 
			minDoorPermiability))
		{
			if (currentDownFoot == 0)
				downFootSafe = left;
			else	
				downFootSafe = right;
			return true;
		}
	}
	xminLeft = footLeft.x - unit;
	xmaxLeft = footLeft.x + unit;
	xminRight = footRight.x - unit;
	xmaxRight = footRight.x + unit;
	for (i=0; i<count; ++i)
	{
		left.x = xminLeft;
		left.y = footLeft.y;
		right.x = xminRight;
		right.y = footRight.y;
		y = yMin;
		for (j=0; j<count; ++j) 
		{
			left.y -= unit;
			right.y -= unit;
			y -= unit;
			if (IsCreatureLocationValidInRoomSystem(left, right, y, 
				minDoorPermiability))
			{
				if (currentDownFoot == 0)
					downFootSafe = left;
				else	
					downFootSafe = right;
				return true;
			}
		}
		left.x = xmaxLeft;
		left.y = footLeft.y;
		right.x = xmaxRight;
		right.y = footRight.y;
		y = yMin;
		for (j=0; j<count; ++j) 
		{
			left.y -= unit;
			right.y -= unit;
			y -= unit;
			if (IsCreatureLocationValidInRoomSystem(left, right, y, 
				minDoorPermiability))
			{
				if (currentDownFoot == 0)
					downFootSafe = left;
				else	
					downFootSafe = right;
				return true;
			}
		}
		xminLeft -= unit;
		xmaxLeft += unit;
		xminRight -= unit;
		xmaxRight += unit;
	}
	

	left = footLeft;
	right = footRight;
	y = yMin;
	for (i=0; i<count; ++i) 
	{
		left.y += unit;
		right.y += unit;
		y += unit;
		if (IsCreatureLocationValidInRoomSystem(left, right, y, 
			minDoorPermiability))
		{
			if (currentDownFoot == 0)
				downFootSafe = left;
			else	
				downFootSafe = right;
			return true;
		}
	}
	xminLeft = footLeft.x - unit;
	xmaxLeft = footLeft.x + unit;
	xminRight = footRight.x - unit;
	xmaxRight = footRight.x + unit;
	for (i=0; i<count; ++i)
	{
		left.x = xminLeft;
		left.y = footLeft.y;
		right.x = xminRight;
		right.y = footRight.y;
		y = yMin;
		for (j=0; j<count; ++j) 
		{
			left.y += unit;
			right.y += unit;
			y += unit;
			if (IsCreatureLocationValidInRoomSystem(left, right, y, 
				minDoorPermiability))
			{
				if (currentDownFoot == 0)
					downFootSafe = left;
				else	
					downFootSafe = right;
				return true;
			}
		}
		left.x = xmaxLeft;
		left.y = footLeft.y;
		right.x = xmaxRight;
		right.y = footRight.y;
		y = yMin;
		for (j=0; j<count; ++j) 
		{
			left.y += unit;
			right.y += unit;
			y += unit;
			if (IsCreatureLocationValidInRoomSystem(left, right, y, 
				minDoorPermiability))
			{
				if (currentDownFoot == 0)
					downFootSafe = left;
				else	
					downFootSafe = right;
				return true;
			}
		}
		xminLeft -= unit;
		xmaxLeft += unit;
		xminRight -= unit;
		xmaxRight += unit;
	}
	return false;
}



// ---------------------------------------------------------------------------
// Function:	ArePointsInRectangle
// Description:	Determines if a set of points are inside a rectangle
// Arguments:	points - the set of points (in)
//				pointCount - the number of points (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
// Returns:		true if all points are inside the rectangle
// --------------------------------------------------------------------------
bool Map::ArePointsInRectangle
	(Vector2D points[4],
	 const int pointCount,
	 const float left,
	 const float right,
	 const float top,
	 const float bottom)
{
	float x, y;
	for (int i=0; i<pointCount; ++i)
	{
		x = points[i].x;
		y = points[i].y;
		if ((x > left) && (x < right) && (y > top) && (y < bottom))
			continue;
		return false;
	}
	return true;
}


// ---------------------------------------------------------------------------
// Function:	AreLinesValidInRoomSystem
// Description:	Determines if a set of lines are wholly contained within the
//				room system
// Arguments:	starts - the set of starting points (in)
//				ends - the set of ending points (in)
//				positionMin - the minimum x and y of all the lines (in)
//				positionMax - the maximum x and y of all the lines (in)
//				minDoorPermiability - minimum door permiability (in)
//				lineCount - how many lines (in)
// Returns:		true if all lines are valid
// --------------------------------------------------------------------------
bool Map::AreLinesValidInRoomSystem
	(Vector2D starts[4], 
	 Vector2D ends[4],
	 const Vector2D& positionMin,
	 const Vector2D& positionMax,
	 const int minDoorPermiability,
	 const int lineCount)
{
	int xSquareMin, ySquareMin;
	int xSquareMax, ySquareMax;
	Room* room;
	DoorIterator doorIterator;
	int count;
	Door* door;
	int dummy1[4];
	Room* dummy2[4];
	int dummy3;
	int dummy4;
	int i, j;
	Vector2D dummy;
	Vector2D deltas[4];

	// Check that at least one of the points is inside a room
	if (!GetRoomInformationForPoint(starts[1], dummy1, dummy2, dummy3,
		dummy4))
		return false;
	
	// Get the minimum and maximum grid squares covered
	GetGridIndexesForPoint(positionMin, xSquareMin, ySquareMin);
	GetGridIndexesForPoint(positionMax, xSquareMax, ySquareMax);

	for (i=0; i<lineCount; ++i) 
	{
		deltas[i] = ends[i] - starts[i];
	}

	GetRoomsOverlappingRegion(xSquareMin, ySquareMin, xSquareMax, ySquareMax);
		
	for (i=0; i<myTempRoomCount; ++i)
	{
		room = myTempRoomCollection[i];
		doorIterator = room->doorCollection.begin();
		count = room->doorCollection.size();
		while (count-- > 0) 
		{
			door = *(doorIterator++);
			if (door->permiability >= minDoorPermiability) 
			{
				// This door is non-blocking, so ignore it
				continue;
			}

			for (j=0; j<lineCount; ++j) 
			{
				if (DoesPathCrossOrTouchLine(starts[j], ends[j], 
					deltas[j], door->start, door->end, door->delta, 
					dummy))
					return false;
			}
		}
	}
	return true;
}


// ---------------------------------------------------------------------------
// Function:	IsAgentLocationValidInRoomSystem
// Description:	Determines if an agent location is valid within the room 
//				system
// Arguments:	position - top-left corner of agent (in)
//				width - width of agent (in)
//				height - height of agent (in)
//				minDoorPermiability - minimum door permiability (in)
// Returns:		true if agent location valid; false otherwise
// ---------------------------------------------------------------------------
bool Map::IsAgentLocationValidInRoomSystem
	(const Vector2D& position, 
	 const float width, 
	 const float height,
	 const int minDoorPermiability)
{
	Vector2D positionMin;
	Vector2D positionMax;
	Vector2D starts[4];
	Vector2D ends[4];
	float w = (width-1.0f);
	float h = (height-1.0f);
	float hw = w/2.0f;
	float hh = h/2.0f;
	float xmin = position.x;
	float ymin = position.y;
	float xmax = position.x + w;
	float ymax = position.y + h;
	float xmid = xmin + hw;
	float ymid = ymin + hh;
	Vector2D top(xmid, ymin);
	Vector2D bottom(xmid, ymax);
	Vector2D left(xmin, ymid);
	Vector2D right(xmax, ymid);

	positionMin.x = xmin;
	positionMin.y = ymin;
	positionMax.x = xmax;
	positionMax.y = ymax;

	starts[0] = left;
	ends[0] = top;
	starts[1] = top;
	ends[1] = right;
	starts[2] = right;
	ends[2] = bottom;
	starts[3] = bottom;
	ends[3] = left;

	return AreLinesValidInRoomSystem(starts, ends, positionMin, positionMax, 
		minDoorPermiability, 4);
}


// ---------------------------------------------------------------------------
// Function:	IsAgentLocationValidInRectangle
// Description:	Determines if an agent location is valid within a rectangle
// Arguments:	position - top-left corner of agent (in)
//				width - width of agent (in)
//				height - height of agent (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
// Returns:		true if agent location valid; false otherwise
// ---------------------------------------------------------------------------
bool Map::IsAgentLocationValidInRectangle
	(const Vector2D& position, 
	 const float width, 
	 const float height,
	 const float left,
	 const float right,
	 const float top,
	 const float bottom)
{
	Vector2D points[4];
	float w = (width-1.0f);
	float h = (height-1.0f);
	float hw = w/2.0f;
	float hh = h/2.0f;
	float xmin = position.x;
	float ymin = position.y;
	float xmax = position.x + w;
	float ymax = position.y + h;
	float xmid = xmin + hw;
	float ymid = ymin + hh;
	points[0] = Vector2D(xmid, ymin);
	points[1] = Vector2D(xmid, ymax);
	points[2] = Vector2D(xmin, ymid);
	points[3] = Vector2D(xmax, ymid);
	return ArePointsInRectangle(points, 4, left, right, top, bottom);
}


// ---------------------------------------------------------------------------
// Function:	GetCreaturePoints
// Description:	Gets the set of edges/points used to model a Creature
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				starts - the set of starting points (out)
//				ends - the set of ending points (out)
//				points - the four points representing the Creature (out)
//				positionMin - the minimum x and y of the set (out)
//				positionMax - the maximum x and y of the set (out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::GetCreaturePoints
	(const Vector2D& footLeft, 
	 const Vector2D& footRight, 
	 const float yMin,
	 Vector2D starts[3], 
	 Vector2D ends[3], 
	 Vector2D points[4],
	 Vector2D& positionMin, 
	 Vector2D& positionMax)
{
	float xMin, xMax, yMax;

	if (footLeft.x < footRight.x)
	{
		xMin = footLeft.x;
		xMax = footRight.x;
	}
	else 
	{
		xMax = footLeft.x;
		xMin = footRight.x;
	}

	if (footLeft.y < footRight.y)
		yMax = footRight.y;
	else
		yMax = footLeft.y;

	positionMin.x = xMin;
	positionMin.y = yMin;
	positionMax.x = xMax;
	positionMax.y = yMax;

	points[0] = footLeft;
	points[1] = footRight;
	points[2].x = footLeft.x;
	points[2].y = yMin;
	points[3].x = footRight.x;
	points[3].y = yMin;
	starts[0] = points[0];
	starts[1] = points[1]; 
	starts[2] = points[2];
	ends[0] = points[2];
	ends[1] = points[3];
	ends[2] = points[3];
}


// ---------------------------------------------------------------------------
// Function:	IsCreatureLocationValidInRoomSystem
// Description:	Determines if a creature location is valid within the room 
//				system
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				minDoorPermiability - minimum door permiability (in)
// Returns:		true if location valid; false otherwise
// ---------------------------------------------------------------------------
bool Map::IsCreatureLocationValidInRoomSystem
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const int minDoorPermiability)
{
	Vector2D positionMin, positionMax;
	Vector2D starts[3], ends[3], points[4];

	GetCreaturePoints(footLeft, footRight, yMin, starts, ends, points, 
		positionMin, positionMax);
	return AreLinesValidInRoomSystem(starts, ends, positionMin,
		positionMax, minDoorPermiability, 3);
}


// ---------------------------------------------------------------------------
// Function:	IsCreatureLocationValidInRectangle
// Description:	Determines if a creature location is valid within a rectangle
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
// Returns:		true if location valid; false otherwise
// ---------------------------------------------------------------------------
bool Map::IsCreatureLocationValidInRectangle
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const float left,
	 const float right,
	 const float top,
	 const float bottom)
{
	Vector2D dummy1, dummy2;
	Vector2D starts[3], ends[3];
	Vector2D points[4];

	GetCreaturePoints(footLeft, footRight, yMin, starts, ends, points, 
		dummy1, dummy2);
	return ArePointsInRectangle(points, 4, left, right, top, bottom);
}


//
//
// Low-level collision testing
//
//


// ---------------------------------------------------------------------------
// Function:	IsAgentPathBlockedByRectangle
// Description: Determines if a rectangle presents an obstacle to the path of
//				an agent
// Arguments:	position - top-left corner of agent (in)
//				width - agent width (in)
//				height - agent height (in)
//				path - the path of the agent (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
//				positionCollision - the position of the collision (out)
//				deltaCollision - delta from the agent to the collision (out)
//				deltaWall - the delta of the blocking wall (out)
//				wall - which of the four walls we collided with (out) 
//				minSquaredDistance - distance to collision (out)
// Returns:		true if agent path is blocked, false otherwise
// ---------------------------------------------------------------------------
bool Map::IsAgentPathBlockedByRectangle
	(const Vector2D& position, 
	 const float width, 
	 const float height, 
	 const Vector2D& path, 
	 const float left,
	 const float right,
	 const float top,
	 const float bottom,
	 Vector2D& positionCollision, 
	 Vector2D& deltaCollision,
	 Vector2D& deltaWall, 
	 int& wall,
	 float& minSquaredDistance)
{
	// Test for no path
	if ((path.x == 0.0f) && (path.y == 0.0f))
		return false;

	// Calculate the four corners of the rectangle
	Vector2D rectangleLeftTop(left, top);
	Vector2D rectangleRightTop(right, top);
	Vector2D rectangleLeftBottom(left, bottom);
	Vector2D rectangleRightBottom(right, bottom);

	// Calculate the four points of the agent's diamond
	Vector2D agentTop(position);
	Vector2D agentBottom(position);
	Vector2D agentLeft(position);
	Vector2D agentRight(position);
	float w = width-1.0f;
	float h = height-1.0f;
	float halfWidth = w/2.0f;
	float halfHeight = h/2.0f;
	agentLeft.y += halfHeight;
	agentRight.x += w;
	agentRight.y += halfHeight;
	agentBottom.x += halfWidth;
	agentBottom.y += h;
	agentTop.x += halfWidth;

	// Calculate which walls we could possibly collide with
	int directions[2];
	int count;
	if (path.x < 0.0f) 
	{
		if (path.y < 0.0f)
		{
			directions[0] = DIRECTION_LEFT;
			directions[1] = DIRECTION_UP;
			count = 2;
		}
		else if (path.y > 0.0f) 
		{
			directions[0] = DIRECTION_LEFT;
			directions[1] = DIRECTION_DOWN;
			count = 2;
		}
		else
		{
			directions[0] = DIRECTION_LEFT;
			count = 1;
		}
	}
	else if (path.x > 0.0f) 
	{
		if (path.y < 0.0f)
		{
			directions[0] = DIRECTION_RIGHT;
			directions[1] = DIRECTION_UP;
			count = 2;
		}
		else if (path.y > 0.0f) 
		{
			directions[0] = DIRECTION_RIGHT;
			directions[1] = DIRECTION_DOWN;
			count = 2;
		}
		else 
		{
			directions[0] = DIRECTION_RIGHT;
			count = 1;
		}
	}
	else if (path.y < 0.0f)
	{
		directions[0] = DIRECTION_UP;
		count = 1;
	}
	else 
	{
		directions[0] = DIRECTION_DOWN;
		count = 1;
	}

	Vector2D positionPath;
	Vector2D positionLine;
	bool intersect;
	int i;
	Vector2D edgeStart, edgeEnd, edgeDelta;
	Vector2D agentPoint; 
	Vector2D agentPointProjected;
	float distance;

	bool blocked = false;
	minSquaredDistance = FLT_MAX;
	for (i=0; i<count; ++i) 
	{
		switch (directions[i])
		{
		case DIRECTION_UP:
		{
			agentPoint = agentTop;
			edgeStart = rectangleLeftTop;
			edgeEnd = rectangleRightTop;
			break;
		}
		case DIRECTION_DOWN: 
		{
			agentPoint = agentBottom;
			edgeStart = rectangleLeftBottom;
			edgeEnd = rectangleRightBottom;
			break;
		}
		case DIRECTION_LEFT:
		{
			agentPoint = agentLeft;
			edgeStart = rectangleLeftTop;
			edgeEnd = rectangleLeftBottom;
			break;
		}
		case DIRECTION_RIGHT:
		{
			agentPoint = agentRight;
			edgeStart = rectangleRightTop;
			edgeEnd = rectangleRightBottom;
			break;
		}
		} // switch
		agentPointProjected = agentPoint + path;
		edgeDelta = edgeEnd - edgeStart;

		// See if the path collides with the wall
		intersect = DoesPathIntersectLine(agentPoint, agentPointProjected, path, 
			edgeStart, edgeEnd, edgeDelta, positionPath, positionLine);
		if (intersect) 
		{
			distance = agentPoint.SquareDistanceTo(positionPath);
			if (distance < minSquaredDistance) 
			{
				blocked = true;
				minSquaredDistance = distance;
				deltaCollision = positionPath-agentPoint;
				positionCollision = positionPath;
				deltaWall = edgeDelta;
				wall = directions[i];
			}
		}
	}
	return blocked;
}


// ---------------------------------------------------------------------------
// Function:	IsCreaturePathBlockedByRectangle
// Description: Determines if a rectangle presents an obstacle to the path of
//				a creature
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				path - the path of the creature (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
//				positionCollision - the position of the collision (out)
//				deltaCollision - delta to the collision (out)
//				deltaWall - the delta of the blocking wall (out)
//				wall - which of the four walls we collided with (out)
//				floorCollision - true if one of the feet hit the floor (out)
//				foot - which foot hit the floor; left 0, right 1 (out)
//				minSquaredDistance - distance to collision (out)
// Returns:		true if creature path is blocked, false otherwise
// ---------------------------------------------------------------------------
bool Map::IsCreaturePathBlockedByRectangle
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const Vector2D& path, 
	 const float left,
	 const float right,
	 const float top,
	 const float bottom,
	 Vector2D& positionCollision, 
	 Vector2D& deltaCollision,
	 Vector2D& deltaWall,
	 int& wall,
	 bool& floorCollision, 
	 int& foot,
	 float& minSquaredDistance)
{
	// Test for no path
	if ((path.x == 0.0f) && (path.y == 0.0f))
		return false;

	// Calculate the four corners of the rectangle
	Vector2D rectangleLeftTop(left, top);
	Vector2D rectangleRightTop(right, top);
	Vector2D rectangleLeftBottom(left, bottom);
	Vector2D rectangleRightBottom(right, bottom);

	// Calculate the four points defining the creature
	int i;
	Vector2D points[4];
	points[0] = footLeft;
	points[1] = footRight;
	points[2].x = footLeft.x;
	points[2].y = yMin;
	points[3].x = footRight.x;
	points[3].y = yMin;

	// Project the four points through the path
	Vector2D pointsProjected[4];
	for (i=0; i<4; ++i)
		pointsProjected[i] = points[i] + path;

	// Calculate which walls we could possibly collide with
	int directions[2];
	int count;
	if (path.x < 0.0f) 
	{
		if (path.y < 0.0f)
		{
			directions[0] = DIRECTION_LEFT;
			directions[1] = DIRECTION_UP;
			count = 2;
		}
		else if (path.y > 0.0f) 
		{
			directions[0] = DIRECTION_LEFT;
			directions[1] = DIRECTION_DOWN;
			count = 2;
		}
		else
		{
			directions[0] = DIRECTION_LEFT;
			count = 1;
		}
	}
	else if (path.x > 0.0f) 
	{
		if (path.y < 0.0f)
		{
			directions[0] = DIRECTION_RIGHT;
			directions[1] = DIRECTION_UP;
			count = 2;
		}
		else if (path.y > 0.0f) 
		{
			directions[0] = DIRECTION_RIGHT;
			directions[1] = DIRECTION_DOWN;
			count = 2;
		}
		else 
		{
			directions[0] = DIRECTION_RIGHT;
			count = 1;
		}
	}
	else if (path.y < 0.0f)
	{
		directions[0] = DIRECTION_UP;
		count = 1;
	}
	else 
	{
		directions[0] = DIRECTION_DOWN;
		count = 1;
	}

	int direction;
	Vector2D edgeStart, edgeEnd, edgeDelta;
	Vector2D p, pProjected;
	Vector2D positionPath, positionLine;
	float dist;
	int j;
	bool down;
	bool blocked = false;
	minSquaredDistance = FLT_MAX;
	for (i=0; i<count; ++i) 
	{
		direction = directions[i];
		switch (direction) 
		{
		case DIRECTION_LEFT:
			edgeStart = rectangleLeftTop;
			edgeEnd = rectangleLeftBottom;
			down = false;
			break;
		case DIRECTION_RIGHT:
			edgeStart = rectangleRightTop;
			edgeEnd = rectangleRightBottom;
			down = false;
			break;
		case DIRECTION_UP:
			edgeStart = rectangleLeftTop;
			edgeEnd = rectangleRightTop;
			down = false;
			break;
		case DIRECTION_DOWN:
			edgeStart = rectangleLeftBottom;
			edgeEnd = rectangleRightBottom;
			down = true;
			break;
		}
		edgeDelta = edgeEnd - edgeStart;
		for (j=0; j<4; ++j)
		{
			p = points[j];
			pProjected = pointsProjected[j];
			if (DoesPathIntersectLine(p, pProjected, path, edgeStart, edgeEnd, 
					edgeDelta, positionPath, positionLine))
			{
				dist = p.SquareDistanceTo(positionPath);
				if (down && ((j==0) || (j==1)))
				{
					// One of the feet has collided with the floor
					// Feet test has priority, hence <= not <
					if (dist <= minSquaredDistance) 
					{
						blocked = true;
						minSquaredDistance = dist;
						positionCollision = positionPath;
						deltaCollision = positionPath-p;
						deltaWall = edgeDelta;
						floorCollision = true;
						foot = j;
						wall = direction;
					}
				}
				else
				{
					if (dist < minSquaredDistance) 
					{
						blocked = true;
						minSquaredDistance = dist;
						positionCollision = positionPath;
						deltaCollision = positionPath-p;
						deltaWall = edgeDelta;
						floorCollision = false;
						wall = direction;
					}
				}
			}
		}
	}
	return blocked;
}


// ---------------------------------------------------------------------------
// Function:	IsAgentPathBlockedByRoom
// Description: Determines if an agent's path is blocked by a room boundary
// Arguments:	room - room to test against (in) 
//				pathBoxMin - minimum point of path bounding box (in)
//				pathBoxMax - maximum point of path bounding box (in)
//				startN, endN - three points describing the three exposed
//					vertices and two exposed edges of the agent diamond (in)
//				edge1Delta - the delta of the first edge (in)
//				edge2Delta - the delta of the second edge (in)
//				path - the path of the agent (in)
//				pathReversed - the negative of the path vector (in)
//				minDoorPermiability - minimum door permiability (in)
//				positionCollision - the position of the collision (out)
//				deltaCollision - delta from the agent to the collision (out)
//				doorCollision - the door that the path collided with (out)
//				hitVertex - true if the collision point is a door vertex (out)
//				positionVertex - the position of the blocking vertex (out)
//				edge - true if an agent edge strikes the collision point, or
//					false if an agent vertex strikes (out)
//				edgeDelta - the delta of the edge which struck, this is
//					always returned even if edge is false (out)
//				minSquaredDistance - min distance to collision (in/out)
// Returns:		true if agent path is blocked by a room, false otherwise
// ---------------------------------------------------------------------------
bool Map::IsAgentPathBlockedByRoom
	(Room* room,
	 const Vector2D& pathBoxMin,
	 const Vector2D& pathBoxMax,
	 const Vector2D& start1, 
	 const Vector2D& end1, 
	 const Vector2D& start2,
	 const Vector2D& end2, 
	 const Vector2D& start3, 
	 const Vector2D& end3,
	 const Vector2D& edge1Delta, 
	 const Vector2D& edge2Delta, 
	 const Vector2D& path, 
	 const Vector2D& pathReversed,
	 const int minDoorPermiability, 
	 Vector2D& positionCollision, 
	 Vector2D& deltaCollision, 
	 Door*& doorCollision, 
	 bool& hitVertex,
	 Vector2D& positionVertex,
	 bool& edge, 
	 Vector2D& edgeDelta,
	 float& minSquaredDistance)
{
	Door* door;
	bool intersect;
	float distance;
	Vector2D positionPath;
	Vector2D startDoorProjected;
	Vector2D endDoorProjected;
	Vector2D startDoor, endDoor, deltaDoor;
	Vector2D positionLine;
	bool blocked = false;
	DoorIterator doorIterator = room->doorCollection.begin();
	int count = room->doorCollection.size();

	while (count-- > 0) 
	{
		door = *(doorIterator++);

		if (!((door->positionMin.x < (pathBoxMax.x+0.5f)) &&
			  (door->positionMax.x > (pathBoxMin.x-0.5f)) && 
			  (door->positionMin.y < (pathBoxMax.y+0.5f)) &&
			  (door->positionMax.y > (pathBoxMin.y-0.5f)) &&
			  (door->permiability < minDoorPermiability)))
			// This door is non-blocking or non-intersecting so ignore it
			// (this stops us doing so many intersections)
			continue;

		// Get the door information
		startDoor = door->start;
		endDoor = door->end;
		deltaDoor = door->delta;
		
		// Project the start and end of the door backwards along the path
		startDoorProjected = startDoor + pathReversed;
		endDoorProjected = endDoor + pathReversed;

		// 
		// See if any of the three agent vertices strike the door
		// 

		if (DoesPathIntersectLine(start1, end1, path, startDoor, endDoor, 
			deltaDoor, positionPath, positionLine))
		{
			distance = start1.SquareDistanceTo(positionPath);
			if (distance < minSquaredDistance) 
			{
				blocked = true;
				hitVertex = false;
				edge = false;
				edgeDelta = edge1Delta;
				minSquaredDistance = distance;
				positionCollision = positionPath;
				doorCollision = door;
				deltaCollision = positionPath-start1;
			}
		}

		if (DoesPathIntersectLine(start2, end2, path, startDoor, endDoor, 
			deltaDoor, positionPath, positionLine))
		{
			distance = start2.SquareDistanceTo(positionPath);
			if (distance < minSquaredDistance) 
			{
				blocked = true;
				hitVertex = false;					
				edge = false;
				edgeDelta = edge1Delta;
				minSquaredDistance = distance;
				positionCollision = positionPath;
				doorCollision = door;
				deltaCollision = positionPath-start2;
			}
		}
	
		if (DoesPathIntersectLine(start3, end3, path, startDoor, endDoor, 
			deltaDoor, positionPath, positionLine))
		{
			distance = start3.SquareDistanceTo(positionPath);
			if (distance < minSquaredDistance) 
			{
				blocked = true;
				hitVertex = false;					
				edge = false;
				edgeDelta = edge2Delta;
				minSquaredDistance = distance;
				positionCollision = positionPath;
				doorCollision = door;
				deltaCollision = positionPath-start3;
			}
		}
	
		//
		// See if the start of the door blocks the agent
		//

		intersect = DoesPathIntersectLine(startDoor, startDoorProjected, 
			pathReversed, start1, start2, edge1Delta, positionPath, 
			positionLine);
		if (intersect)
		{
			distance = startDoor.SquareDistanceTo(positionPath);
			// Hit vertex has priority, hence <= not <
			if (distance <= minSquaredDistance) 
			{				
				blocked = true;
				hitVertex = true;
				positionVertex = startDoor;
				if (positionLine == start1)
				{
					edgeDelta = edge1Delta;
					edge = false;
				}
				else if (positionLine == start2)
				{
					edge = false;
					edgeDelta = edge1Delta;
				}
				else 
				{
					edgeDelta = edge1Delta;
					edge = true;
				}
				minSquaredDistance = distance;
				positionCollision = startDoor+(positionLine-positionPath);
				doorCollision = NULL;
				deltaCollision = startDoor-positionPath;
			}
		}

		if (!intersect) 
		{
			if (DoesPathIntersectLine(startDoor, startDoorProjected, 
					pathReversed, start2, start3, edge2Delta, positionPath, 
					positionLine))
			{
				distance = startDoor.SquareDistanceTo(positionPath);
				// Hit vertex has priority, hence <= not <
				if (distance <= minSquaredDistance) 
				{
					blocked = true;
					hitVertex = true;	
					positionVertex = startDoor;
					if (positionLine == start2)
					{
						edge = false;
						edgeDelta = edge2Delta;
					}
					else if (positionLine == start3)
					{
						edgeDelta = edge2Delta;
						edge = false;						
					}
					else 
					{
						edgeDelta = edge2Delta;
						edge = true;
						
					}
					minSquaredDistance = distance;
					positionCollision = startDoor+(positionLine-positionPath);
					doorCollision = NULL;
					deltaCollision = startDoor-positionPath;
				}
			}
		}
		
		//
		// See if the end of the door blocks the agent
		//

		intersect = DoesPathIntersectLine(endDoor, endDoorProjected, 
			pathReversed, start1, start2, edge1Delta, positionPath, 
			positionLine);
		if (intersect) 
		{
			distance = endDoor.SquareDistanceTo(positionPath);
			// Hit vertex has priority, hence <= not <
			if (distance <= minSquaredDistance) 
			{
				blocked = true;
				hitVertex = true;	
				positionVertex = endDoor;
				if (positionLine == start1)
				{
					edgeDelta = edge1Delta;
					edge = false;		
				}
				else if (positionLine == start2)
				{
					edgeDelta = edge1Delta;
					edge = false;
				}
				else 
				{
					edgeDelta = edge1Delta;
					edge = true;	
				}
				minSquaredDistance = distance;
				positionCollision = endDoor+(positionLine-positionPath);
				doorCollision = NULL;
				deltaCollision = endDoor-positionPath;
			}
		}

		if (!intersect) 
		{
			if (DoesPathIntersectLine(endDoor, endDoorProjected, pathReversed,
					start2, start3, edge2Delta, positionPath, positionLine))
			{
				distance = endDoor.SquareDistanceTo(positionPath);
				// Hit vertex has priority, hence <= not <
				if (distance <= minSquaredDistance) 
				{
					blocked = true;
					hitVertex = true;		
					positionVertex = endDoor;
					if (positionLine == start2)
					{
						edge = false;
						edgeDelta = edge2Delta;
					}
					else if (positionLine == start3)
					{
						edgeDelta = edge2Delta;
						edge = false;
					}
					else 
					{
						edgeDelta = edge2Delta;
						edge = true;
					}
					minSquaredDistance = distance;
					positionCollision = endDoor+(positionLine-positionPath);
					doorCollision = NULL;
					deltaCollision = endDoor-positionPath;
				}
			}
		}
	}
	return blocked;
}




// ---------------------------------------------------------------------------
// Function:	IsCreaturePathBlockedByRoom
// Description: Determines if a room boundary presents an obstacle to the 
//				path of a creature
// Arguments:	room - room to test against (in) 
//				pathBoxMin - minimum point of path bounding box (in)
//				pathBoxMax - maximum point of path bounding box (in)
//				footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				points - points defining the creature (in)
//				starts - creature edge starting points (in)
//				ends - creature edge ending points (in)
//				deltas - creature edge deltas (in)
//				path - the path of the creature (in)
//				pathReversed - the negative of the path vector (in)
//				minDoorPermiability - minimum door permiability (in)
//				positionCollision - the position of the collision (out)
//				deltaCollision - delta to the collision (out)
//				doorCollision - the door that the path collided with (out)
//				hitVertex - true if the collision point is a door vertex (out)
//				positionVertex - the position of the blocking vertex (out)
//				floorCollision - true if one of the feet hit the floor (out)
//				foot - which foot hit the floor; left 0, right 1 (out)
//				minSquaredDistance - distance to collision (out)
// Returns:		true if creature path is blocked, false otherwise
// ---------------------------------------------------------------------------
bool Map::IsCreaturePathBlockedByRoom
	(Room* room,
	 const Vector2D& pathBoxMin,
	 const Vector2D& pathBoxMax,
	 const Vector2D& footLeft,
	 const Vector2D& footRight,
	 Vector2D points[4],
 	 Vector2D starts[3], 
	 Vector2D ends[3], 
	 Vector2D deltas[3],
	 const Vector2D& path,
	 const Vector2D& pathReversed,
	 const int minDoorPermiability,
	 Vector2D& positionCollision, 
	 Vector2D& deltaCollision, 
	 Door*& doorCollision, 
	 bool& hitVertex,
	 Vector2D& positionVertex,
	 bool& floorCollision, 
	 int& foot,
	 float& minSquaredDistance)
{
	Door* door;
	bool intersect;
	float distance;
	Vector2D positionPath;
	Vector2D startDoorProjected;
	Vector2D endDoorProjected;
	Vector2D startDoor, endDoor, deltaDoor;
	Vector2D positionLine;
	int wall, i;
	Vector2D start, end, delta;

	bool blocked = false;
	DoorIterator doorIterator = room->doorCollection.begin();
	int count = room->doorCollection.size();

	while (count-- > 0) 
	{
		door = *(doorIterator++);

		if (!((door->positionMin.x < (pathBoxMax.x+0.5f)) &&
			  (door->positionMax.x > (pathBoxMin.x-0.5f)) && 
			  (door->positionMin.y < (pathBoxMax.y+0.5f)) &&
			  (door->positionMax.y > (pathBoxMin.y-0.5f)) &&
			  (door->permiability < minDoorPermiability)))
			// This door is non-blocking or non-intersecting so ignore it
			// (this stops us doing so many intersections)
			continue;

		// Which type of wall is this door?
		wall = CalculateWallFromDoorAndPath(door, path);

		// Get the door information
		startDoor = door->start;
		endDoor = door->end;
		deltaDoor = door->delta;
		
		// 
		// See if any of the creature vertices are blocked by the door
		//

		for (i=0; i<4; ++i) 
		{
			start = points[i];
			intersect = DoesPathIntersectLine(start, start+path, path, 
				startDoor, endDoor, deltaDoor, positionPath, positionLine);
			if (intersect) 
			{
				distance = start.SquareDistanceTo(positionPath);
				if ((wall == DIRECTION_FLOOR) && ((i==0) || (i==1)))
				{
					// Feet test has priority, hence <= not <
					if (distance <= minSquaredDistance) 
					{
						blocked = true;
						minSquaredDistance = distance;
						positionCollision = positionPath;
						deltaCollision = positionPath-start;
						doorCollision = door;
						hitVertex = false;
						floorCollision = true;
						foot = i;
					}
				}
				else 
				{
					if (distance < minSquaredDistance) 
					{
						blocked = true;
						minSquaredDistance = distance;
						positionCollision = positionPath;
						deltaCollision = positionPath-start;
						doorCollision = door;
						hitVertex = false;
						floorCollision = false;
					}
				}
			}
		}

		// Project the start and end of the door backwards along the path
		startDoorProjected = startDoor + pathReversed;
		endDoorProjected = endDoor + pathReversed;

		for (i=0; i<3; ++i)
		{
			start = starts[i];
			end = ends[i];
			delta = deltas[i];

			intersect = DoesPathIntersectLine(startDoor, startDoorProjected,
				pathReversed, start, end, delta, positionPath, positionLine);
			if (intersect)
			{
				distance = startDoor.SquareDistanceTo(positionPath);
				int value;
				if (positionLine == footLeft)
					value = 0;
				else if (positionLine == footRight)
					value = 1;
				else
					value = -1;
				if ((wall == DIRECTION_FLOOR) && 
					((value == 0) || (value == 1)))
				{
					// Feet test has priority, hence <= not <
					if (distance <= minSquaredDistance) 
					{
						blocked = true;
						minSquaredDistance = distance;
						positionCollision = startDoor+(positionLine-positionPath);
						deltaCollision = startDoor-positionPath;
						doorCollision = NULL;
						hitVertex = true;
						positionVertex = startDoor;
						floorCollision = true;
						foot = value;
					}
				}
				else
				{
					if (distance < minSquaredDistance)
					{
						blocked = true;
						minSquaredDistance = distance;
						positionCollision = startDoor+(positionLine-positionPath);
						deltaCollision = startDoor-positionPath;
						doorCollision = NULL;
						hitVertex = true;
						positionVertex = startDoor;
						floorCollision = false;
					}
				}
			}

			intersect = DoesPathIntersectLine(endDoor, endDoorProjected,
				pathReversed, start, end, delta, positionPath, positionLine);
			if (intersect)
			{
				distance = endDoor.SquareDistanceTo(positionPath);
				int value;
				if (positionLine == footLeft)
					value = 0;
				else if (positionLine == footRight)
					value = 1;
				else
					value = -1;
				if ((wall == DIRECTION_FLOOR) && 
					((value == 0) || (value == 1)))
				{
					// Feet test has priority, hence <= not <
					if (distance <= minSquaredDistance) 
					{
						blocked = true;
						minSquaredDistance = distance;
						positionCollision = endDoor+(positionLine-positionPath);
						deltaCollision = endDoor-positionPath;
						doorCollision = NULL;
						hitVertex = true;
						positionVertex = endDoor;
						floorCollision = true;
						foot = value;
					}
				}
				else
				{
					if (distance < minSquaredDistance)
					{
						blocked = true;
						minSquaredDistance = distance;
						positionCollision = endDoor+(positionLine-positionPath);
						deltaCollision = endDoor-positionPath;
						doorCollision = NULL;
						hitVertex = true;
						positionVertex = endDoor;
						floorCollision = false;
					}
				}
			}
		}
	}
	return blocked;
}


// ---------------------------------------------------------------------------
// Function:	IsAgentPathBlockedByRoomSystem
// Description: Determines if the room system presents an obstacle to the 
//				path of an agent
// Arguments:	position - top-left corner of agent (in)
//				width - agent width (in)
//				height - agent height (in)
//				path - the path of the agent (in)
//				minDoorPermiability - minimum door permiability (in)
//				positionCollision - the position of the collision (out)
//				deltaCollision - delta from the agent to the collision (out)
//				doorCollision - the door that the path collided with (out)
//				hitVertex - true if the collision point is a door vertex (out)
//				positionVertex - the position of the blocking vertex (out)
//				edge - true if an agent edge strikes the collision point, or
//					false if an agent vertex strikes (out)
//				edgeDelta - the delta of the edge which struck, this is
//					always returned even if edge is false (out)
//				minSquaredDistance - distance to collision (out)
// Returns:		true if agent path is blocked, false otherwise
// ---------------------------------------------------------------------------
bool Map::IsAgentPathBlockedByRoomSystem
	(const Vector2D& position,
	 const float width,
	 const float height, 
	 const Vector2D& path, 
	 const int minDoorPermiability, 
	 Vector2D& positionCollision, 
	 Vector2D& deltaCollision, 
	 Door*& doorCollision, 
	 bool& hitVertex, 
	 Vector2D& positionVertex, 
	 bool& edge, 
	 Vector2D& edgeDelta,
	 float& minSquaredDistance)
{	
	//
	// Calculate the three exposed vertices of the agent diamond and project
	// them through the path
	//

	float w = width-1.0f;
	float h = height-1.0f;
	float halfWidth = w/2.0f;
	float halfHeight = h/2.0f;
	float gradientAgent = h/w;
	float gradientTrajectory;
	if (path.x != 0.0f) 
	{
		gradientTrajectory = path.y/path.x;
	}
	Vector2D start1(position);
	Vector2D start2(position);
	Vector2D start3(position);
	if (path.x == 0.0f) 
	{
		if (path.y > 0.0f) 
		{
			// Left
			start1.y += halfHeight;
			// Bottom
			start2.x += halfWidth;
			start2.y += h;
			// Right
			start3.x += w;
			start3.y += halfHeight;
		}
		else 
		{
			// Left
			start1.y += halfHeight;
			// Top
			start2.x += halfWidth;
			// Right
			start3.x += w;
			start3.y += halfHeight;			
		}
	}
	else 
	{
		if ((gradientTrajectory >= -gradientAgent) && 
			(gradientTrajectory <= gradientAgent)) 
		{
			if (path.x > 0.0f) 
			{
				// Top
				start1.x += halfWidth;
				// Right
				start2.x += w;
				start2.y += halfHeight;	
				// Bottom
				start3.x += halfWidth;
				start3.y += h;
			}
			else 
			{
				// Top
				start1.x += halfWidth;
				// Left
				start2.y += halfHeight;
				// Bottom
				start3.x += halfWidth;
				start3.y += h;
			}
		}
		else if (path.y > 0.0f) 
		{
			// Left
			start1.y += halfHeight;
			// Bottom
			start2.x += halfWidth;
			start2.y += h;
			// Right
			start3.x += w;
			start3.y += halfHeight;
		}
		else 
		{
			// Left
			start1.y += halfHeight;
			// Top
			start2.x += halfWidth;
			// Right
			start3.x += w;
			start3.y += halfHeight;
		}
	} 
	// Project the three exposed vertices through the path
	Vector2D end1(start1 + path);
	Vector2D end2(start2 + path);
	Vector2D end3(start3 + path);

	//
	// Get the bounding box of the path
	//

	Vector2D pathBoxMin;
	Vector2D pathBoxMax;
	if (path.x > 0.0f)
	{
		pathBoxMin.x = position.x;
		pathBoxMax.x = position.x + path.x + w;
	}
	else
	{
		pathBoxMin.x = position.x + path.x;
		pathBoxMax.x = position.x + w;
	}
	if (path.y > 0.0f)
	{
		pathBoxMin.y = position.y;
		pathBoxMax.y = position.y + path.y + h;
	}
	else
	{
		pathBoxMin.y = position.y + path.y;
		pathBoxMax.y = position.y + h;
	}

	// Which grid squares does the path cover?
	int xSquareMin, ySquareMin, xSquareMax, ySquareMax;
	GetGridIndexesForPoint(pathBoxMin, xSquareMin, ySquareMin);
	GetGridIndexesForPoint(pathBoxMax, xSquareMax, ySquareMax);

	// Which rooms does the path cover?
	GetRoomsOverlappingRegion(xSquareMin, ySquareMin, xSquareMax, ySquareMax);

	// See if the path extent is inside the same room, if so, no collision
	if (IsRegionInsideSameRoom(pathBoxMin, pathBoxMax))
		return false;

	Room* room;
	bool blocked;
	int i;
	Vector2D edge1delta(start2 - start1);
	Vector2D edge2delta(start3 - start2);
	minSquaredDistance = FLT_MAX;
	Vector2D pathReversed(-path);
	bool collision = false;
	for (i=0; i<myTempRoomCount; ++i)
	{
		room = myTempRoomCollection[i];
		if ((room->positionMin.x < (pathBoxMax.x+0.5f)) &&
			(room->positionMax.x > (pathBoxMin.x-0.5f)) && 
			(room->positionMin.y < (pathBoxMax.y+0.5f)) &&
			(room->positionMax.y > (pathBoxMin.y-0.5f)) &&
			(room->permiability < minDoorPermiability))
		{
			blocked = IsAgentPathBlockedByRoom(room, pathBoxMin, pathBoxMax,
				 start1, end1, start2, end2, start3, end3, edge1delta, 
				 edge2delta, path, pathReversed, minDoorPermiability, 
				 positionCollision, deltaCollision, doorCollision, hitVertex, 
				 positionVertex, edge, edgeDelta, minSquaredDistance);
			if (blocked) 
				collision = true;
		}
	}
	return collision;
}


// ---------------------------------------------------------------------------
// Function:	IsCreaturePathBlockedByRoomSystem
// Description: Determines if the room system presents an obstacle to the 
//				path of a creature
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				path - the path of the creature (in)
//				minDoorPermiability - minimum door permiability (in)
//				positionCollision - the position of the collision (out)
//				deltaCollision - delta to the collision (out)
//				doorCollision - the door that the path collided with (out)
//				hitVertex - true if the collision point is a door vertex (out)
//				positionVertex - the position of the blocking vertex (out)
//				floorCollision - true if one of the feet hit the floor (out)
//				foot - which foot hit the floor; left 0, right 1 (out)
//				minSquaredDistance - distance to collision (out)
// Returns:		true if creature path is blocked, false otherwise
// ---------------------------------------------------------------------------
bool Map::IsCreaturePathBlockedByRoomSystem
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const Vector2D& path, 
	 const int minDoorPermiability,
	 Vector2D& positionCollision, 
	 Vector2D& deltaCollision, 
	 Door*& doorCollision, 
	 bool& hitVertex,
	 Vector2D& positionVertex,
	 bool& floorCollision, 
	 int& foot,
	 float& minSquaredDistance)
{
	// Get the points/edges defining the creature
	Vector2D pMin, pMax;
	Vector2D starts[3];
	Vector2D ends[3];
	Vector2D deltas[3];
	Vector2D points[4];
	GetCreaturePoints(footLeft, footRight, yMin, starts, ends, points, 
		pMin, pMax);
	int i;
	for (i=0; i<3; ++i) 
		deltas[i] = ends[i] - starts[i];

	//
	// Get the bounding box of the path
	//

	Vector2D pathBoxMin, pathBoxMax;
	if (path.x > 0.0f)
	{
		pathBoxMin.x = pMin.x;
		pathBoxMax.x = pMax.x + path.x;
	}
	else 
	{
		pathBoxMin.x = pMin.x + path.x;
		pathBoxMax.x = pMax.x;
	}
	if (path.y > 0.0f)
	{
		pathBoxMin.y = pMin.y;
		pathBoxMax.y = pMax.y + path.y;
	}
	else 
	{
		pathBoxMin.y = pMin.y + path.y;
		pathBoxMax.y = pMax.y;
	}	

	// Which grid squares does the path cover?
	int xSquareMin, ySquareMin, xSquareMax, ySquareMax;
	GetGridIndexesForPoint(pathBoxMin, xSquareMin, ySquareMin);
	GetGridIndexesForPoint(pathBoxMax, xSquareMax, ySquareMax);

	// Which rooms does the path cover?
	GetRoomsOverlappingRegion(xSquareMin, ySquareMin, xSquareMax, ySquareMax);

	// See if the path extent is inside the same room, if so, no collision
	if (IsRegionInsideSameRoom(pathBoxMin, pathBoxMax))
		return false;

	Room* room;
	bool blocked;
	minSquaredDistance = FLT_MAX;
	Vector2D pathReversed(-path);
	bool collision = false;
	for (i=0; i<myTempRoomCount; ++i)
	{
		room = myTempRoomCollection[i];
		if ((room->positionMin.x < (pathBoxMax.x+0.5f)) &&
			(room->positionMax.x > (pathBoxMin.x-0.5f)) && 
			(room->positionMin.y < (pathBoxMax.y+0.5f)) &&
			(room->positionMax.y > (pathBoxMin.y-0.5f)) &&
			(room->permiability < minDoorPermiability))
		{
			blocked = IsCreaturePathBlockedByRoom(room, pathBoxMin, pathBoxMax,
				 footLeft, footRight, points, starts, ends, deltas, path,
				 pathReversed, minDoorPermiability, positionCollision, 
				 deltaCollision, doorCollision, hitVertex, positionVertex, 
				 floorCollision, foot, minSquaredDistance);
			if (blocked) 
				collision = true;
		}
	}
	return collision;
}
	


//
//
// High-level collision testing
//
//


// ---------------------------------------------------------------------------
// Function:	TestForAgentCollisionInRectangle
// Description: Tests to see if an agent would collide with a rectangle in a 
//				given direction, a given distance away
// Arguments:	position - top-left corner of agent (in)
//				width - agent width (in)
//				height - agent height (in)
//				direction - which direction to test, one of DIRECTION_LEFT, 
//					DIRECTION_RIGHT, DIRECTION_UP or DIRECTION_DOWN (in)
//				distanceTest - how far to look ahead (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
//				collision - if there is a collision (out) 
//				distanceCollision - distance to collision (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::TestForAgentCollisionInRectangle
	(const Vector2D& position, 
	 const float width, 
	 const float height, 
	 const int direction, 
	 const float distanceTest, 
	 const float left,
	 const float right,
	 const float top,
	 const float bottom, 
	 bool& collision, 
	 float& distanceCollision)
{
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	float minSquaredDistance;
	Vector2D wallDelta;
	int wall;
 
	if ((width <= 1.0f) || (height <= 1.0f))
		return false;
	if (distanceTest < 1.0f)
		return false;
	switch (direction) 
	{
	case DIRECTION_LEFT: path = Vector2D(-distanceTest, 0.0f); break;
	case DIRECTION_RIGHT: path = Vector2D(distanceTest, 0.0f); break;
	case DIRECTION_UP: path = Vector2D(0.0f, -distanceTest); break;
	case DIRECTION_DOWN: path = Vector2D(0.0f, distanceTest); break;
	default: return false;
	} // switch

	collision = IsAgentPathBlockedByRectangle(position, width, height, 
		path, left, right, top, bottom, positionCollision, deltaCollision, 
		wallDelta, wall, minSquaredDistance);
	if (collision)
		distanceCollision = sqrtf(minSquaredDistance);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	TestForCreatureCollisionInRectangle
// Description: Tests to see if a creature would collide with a rectangle in a 
//				given direction, a given distance away
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				direction - which direction to test, one of DIRECTION_LEFT, 
//					DIRECTION_RIGHT, DIRECTION_UP or DIRECTION_DOWN (in)
//				distanceTest - how far to look ahead (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
//				collision - if there is a collision (out) 
//				distanceCollision - distance to collision (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::TestForCreatureCollisionInRectangle
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const int direction, 
	 const float distanceTest,
	 const float left,
	 const float right,
	 const float top,
	 const float bottom,
	 bool& collision, 
	 float& distanceCollision)
{
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	float minSquaredDistance;
	bool floorCollision;
	int foot;
	int wall;
	Vector2D wallDelta;
 
	if (distanceTest < 1.0f)
		return false;
	switch (direction) 
	{
	case DIRECTION_LEFT: path = Vector2D(-distanceTest, 0.0f); break;
	case DIRECTION_RIGHT: path = Vector2D(distanceTest, 0.0f); break;
	case DIRECTION_UP: path = Vector2D(0.0f, -distanceTest); break;
	case DIRECTION_DOWN: path = Vector2D(0.0f, distanceTest); break;
	default: return false;
	} // switch

	collision = IsCreaturePathBlockedByRectangle(footLeft, footRight, yMin, 
		path, left, right, top, bottom, positionCollision, deltaCollision, 
		wallDelta, wall, floorCollision, foot, minSquaredDistance);
	if (collision)
		distanceCollision = sqrtf(minSquaredDistance);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	TestForAgentCollisionInRoomSystem
// Description: Tests to see if an agent would collide with a wall in a given
//					 direction, a given distance away
// Arguments:	position - top-left corner of agent (in)
//				width - width of agent (in)
//				height - height of agent (in)
//				direction - which direction to test, one of DIRECTION_LEFT, 
//					DIRECTION_RIGHT, DIRECTION_UP or DIRECTION_DOWN (in)
//				distanceTest - how far from agent to test (in)
//				minDoorPermiability - minimum door permiability (in)
//				collision - is there a collision? (out)
//				distanceCollision - how far to the collision (out)
// Returns:		true if good input data, false otherwise
// --------------------------------------------------------------------------
bool Map::TestForAgentCollisionInRoomSystem
	(const Vector2D& position, 
	 const float width, 
	 const float height, 
	 const int direction, 
	 const float distanceTest, 
	 const int minDoorPermiability, 
	 bool& collision, 
	 float& distanceCollision)
{
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	bool vertex;
	float minSquaredDistance;
	Door* doorCollision;
	bool edge;
	Vector2D edgeDelta;
	Vector2D positionVertex;
 
	if ((width <= 1.0f) || (height <= 1.0f))
		return false;
	if (distanceTest < 1.0f)
		return false;
	switch (direction) 
	{
	case DIRECTION_LEFT: path = Vector2D(-distanceTest, 0.0f); break;
	case DIRECTION_RIGHT: path = Vector2D(distanceTest, 0.0f); break;
	case DIRECTION_UP: path = Vector2D(0.0f, -distanceTest); break;
	case DIRECTION_DOWN: path = Vector2D(0.0f, distanceTest); break;
	default: return false;
	} // switch
	if ((minDoorPermiability < IMPERMIABLE) || 
		(minDoorPermiability > PERMIABLE))
		return false;

	collision = IsAgentPathBlockedByRoomSystem(position, width, height, 
		path, minDoorPermiability, positionCollision, deltaCollision, 
		doorCollision, vertex, positionVertex, edge, edgeDelta, 
		minSquaredDistance);
	if (collision)
		distanceCollision = sqrtf(minSquaredDistance);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	TestForCreatureCollisionInRoomSystem
// Description: Tests to see if a creature would collide with a wall in a
//					given direction, a given distance away
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				direction - which direction to test, one of DIRECTION_LEFT, 
//					DIRECTION_RIGHT, DIRECTION_UP or DIRECTION_DOWN (in)
//				distanceTest - how far from creature to test (in)
//				minDoorPermiability - minimum door permiability (in)
//				collision - is there a collision? (out)
//				distanceCollision - how far to the collision (out)
// Returns:		true if good input data, false otherwise
// --------------------------------------------------------------------------
bool Map::TestForCreatureCollisionInRoomSystem
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const int direction, 
	 const float distanceTest, 
	 const int minDoorPermiability, 
	 bool& collision, 
	 float& distanceCollision)
{
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	bool hitVertex;
	float minSquaredDistance;
	Door* doorCollision;
	Vector2D positionVertex;
	bool floorCollision;
	int foot;
 
	if (distanceTest < 1.0f)
		return false;
	switch (direction) 
	{
	case DIRECTION_LEFT: path = Vector2D(-distanceTest, 0.0f); break;
	case DIRECTION_RIGHT: path = Vector2D(distanceTest, 0.0f); break;
	case DIRECTION_UP: path = Vector2D(0.0f, -distanceTest); break;
	case DIRECTION_DOWN: path = Vector2D(0.0f, distanceTest); break;
	default: return false;
	} // switch
	if ((minDoorPermiability < IMPERMIABLE) || 
		(minDoorPermiability > PERMIABLE))
		return false;

	collision = IsCreaturePathBlockedByRoomSystem(footLeft,
		footRight, yMin, path, minDoorPermiability, positionCollision, 
		deltaCollision, doorCollision, hitVertex, positionVertex, 
		floorCollision, foot, minSquaredDistance);
	if (collision)
		distanceCollision = sqrtf(minSquaredDistance);
	return true;
}



//
//
// Movement Routines
//
//


// ---------------------------------------------------------------------------
// Function:	MoveAgentInsideRectangle
// Description: Calculates the new position and velocity of an agent moving 
//					within a rectangle
// Arguments:	width - width of agent (in)
//				height - height of agent (in)
//				applyPhysics - true to apply elasticity, friction, gravity
//					and aerodynamics, false otherwise (in)
//				collisionFactor - percentage decrease in velocity due to
//					collision (in) 
//				aeroDynamicFactor - percentage decrease in velocity due to
//					media resistance (in) 
//				frictionFactor - percentage decrease in velocity due to 
//					friction (in)
//				gravity - how much y velocity increases per time slice (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
//				position - top-left corner of agent (in/out)
//				velocity - velocity of agent (in/out)
//				collision - was there a collision? (out)
//				wall - the wall the agent hit (out)
//				stopped - is agent now at rest? (out)
//				velocityCollision - velocity at point of collision (out)
// Returns:		None
// --------------------------------------------------------------------------
void Map::MoveAgentInsideRectangle
	(const float width, 
	 const float height,
	 const bool applyPhysics,
	 const float collisionFactor, 
	 const float aeroDynamicFactor,
	 const float frictionFactor,
	 const float gravity, 
	 const float left,
	 const float right,
	 const float top,
	 const float bottom,
	 Vector2D& position, 
	 Vector2D& velocity, 
	 bool& collision, 
	 int& wall, 
	 bool& stopped,
	 Vector2D& velocityCollision)
{	
	Vector2D velocityEnd;
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	float timeElapsed;
	float minSquaredDistance;
	bool blocked;
	bool moved;
	Vector2D wallDelta;
	Vector2D acceleration;
	int wallCollision;
	Vector2D vc;
	float timeTotal = 0.0f;
	float timeRemaining = 1.0f;
	bool sliding = false;
	Vector2D gravityAcceleration(0.0f, gravity);
	collision = false;
	stopped = false;
	int times = 10;
	while (times-- > 0)
	{
		// Calculate the final velocity, trajectory and acceleration
		if (applyPhysics && !sliding)
		{
			velocityEnd = velocity;
			velocityEnd.y += gravity * timeRemaining;
			velocityEnd *= aeroDynamicFactor;
			path = (velocity + velocityEnd) / 2.0f;
			path *= timeRemaining;
			acceleration = gravityAcceleration;	
		}
		else
		{
			velocityEnd = velocity;
			path = velocity * timeRemaining;
			acceleration = ZERO_VECTOR;
		}
	
		// See if any of the four walls block the trajectory
		blocked = IsAgentPathBlockedByRectangle(position, width, height, path, 
			left, right, top, bottom, positionCollision, deltaCollision, 
			wallDelta, wallCollision, minSquaredDistance);

		if (!blocked) 
		{
			position += path;
			velocity = velocityEnd;
			// Check integrity
			_ASSERT(IsAgentLocationValidInRectangle(position, width, height, 
				left, right, top, bottom));
			return;
		}

		// Move up to the collision point
		position += deltaCollision;
		// Check integrity
		_ASSERT(IsAgentLocationValidInRectangle(position, width, height, 
			left, right, top, bottom));

		if (minSquaredDistance < 0.0001f) 
			moved = false;
		else 
			moved = true;
				
		// Calculate the collision velocity and how much time we've used up
		if (moved) 
		{
			CalculateCollisionVelocityAndElapsedTime(velocity, acceleration,
				path, deltaCollision, vc, timeElapsed);
			_ASSERT((timeElapsed >= 0.0f) && (timeElapsed <= 1.0f));
		}
		else 
		{
			vc = velocity;
			timeElapsed = 0.0f;
		}

		// Work out the new velocity and whether to slide along the floor
		if (applyPhysics)
		{
			if (moved || (wallCollision != DIRECTION_FLOOR))
			{
				collision = true;
				wall = wallCollision;
				velocityCollision = vc;
			}

			if (!moved && (wallCollision == DIRECTION_FLOOR))
			{	
				sliding = true;
				Vector2D dummy;
				CalculateSlideAccelerationAndSlideVelocity(vc, 
					wallDelta, gravityAcceleration, dummy, 
					velocity);
				velocity *= frictionFactor;
				if (velocity.IsNearZero(0.5f))
				{
					velocity = ZERO_VECTOR;
					stopped = true;
					return;
				}
			}
			else 
			{			
				Vector2D velocityReflected = vc.Reflect(wallDelta);
				Vector2D positionStart = positionCollision - vc;
 				Vector2D positionEnd = positionCollision + velocityReflected;
				Vector2D positionMiddle = positionStart + 
					(positionEnd - positionStart)/2.0f;
				Vector2D normal = positionCollision - positionMiddle;
				Vector2D loss = normal * (1.0f-collisionFactor);
				velocity = velocityReflected + loss;
				if (collisionFactor == 0.0f)
				{
					// When elasticity is zero, we want to apply friction
					// the minute we collide
					velocity *= frictionFactor;
					if (velocity.IsNearZero(0.5f))
					{
						velocity = ZERO_VECTOR;
						return;
					}
				}
				sliding = false;
			}
		}
		else 
		{
			// No physics applied
			collision = true;
			wall = wallCollision;
			velocityCollision = vc;
			sliding = false;			
			velocity = vc.Reflect(wallDelta);
		}

		// Update the timers
		timeTotal += timeElapsed;
		timeRemaining = 1.0f - timeTotal;	

		if (timeRemaining <= 0.0f) 
			// We're done with this time slice
			return;
	}
	// Too many loops, bring the agent to rest
	velocity = ZERO_VECTOR;
	stopped = true;
}


// ---------------------------------------------------------------------------
// Function:	MoveCreatureInsideRectangle
// Description: Calculates the new position and velocity of a creature moving 
//					within a rectangle
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				applyPhysics - true to apply elasticity, gravity and 
//					aerodynamics; false otherwise (in)
//				collisionFactor - percentage decrease in velocity due to
//					collision (in) 
//				aeroDynamicFactor - percentage decrease in velocity due to
//					media resistance (in) 
//				gravity - how much y velocity increases per tick (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
//				downFootPosition - position of the down foot (in/out)
//				velocity - velocity of creature (in/out)
//				collision - was there a collision? (out)
//				wall - the wall the creature hit (out)
//				stopped - has creature came to rest on the floor? (out)
//				downFoot - which foot is the down foot; left 0, right 1 (out)			
//				velocityCollision - velocity at point of collision (out)
// Returns:		None
// --------------------------------------------------------------------------
void Map::MoveCreatureInsideRectangle
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const bool applyPhysics,
	 const float collisionFactor,
	 const float aeroDynamicFactor,
	 const float gravity, 
	 const float left,
	 const float right,
	 const float top,
	 const float bottom,
	 Vector2D& downFootPosition,
	 Vector2D& velocity,
	 bool& collision,
	 int& wall,
	 bool& stopped,
	 int& downFoot,
	 Vector2D& velocityCollision)
{
	Vector2D velocityEnd;
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	Vector2D velocityReflected;
	float timeElapsed;
	float minSquaredDistance;
	bool blocked;
	bool floorCollision;
	Vector2D wallDelta;
	Vector2D acceleration;
	int footCollision;
	int wallCollision;
	float timeTotal = 0.0f;
	float timeRemaining = 1.0f;
	Vector2D gravityAcceleration(0.0f, gravity);
	collision = false;
	stopped = false;
	Vector2D footLeftTemp(footLeft);
	Vector2D footRightTemp(footRight);
	float yMinTemp = yMin;
	int times = 10;
	while (times-- > 0)
	{
		// Calculate the final velocity, trajectory and acceleration
		if (applyPhysics)
		{
			velocityEnd = velocity;
			velocityEnd.y += gravity * timeRemaining;
			velocityEnd *= aeroDynamicFactor;
			path = (velocity + velocityEnd) / 2.0f;
			path *= timeRemaining;
			acceleration = gravityAcceleration;	
		}
		else
		{
			velocityEnd = velocity;
			path = velocity * timeRemaining;
			acceleration = ZERO_VECTOR;
		}
	
		// See if any of the four walls block the trajectory
		blocked = IsCreaturePathBlockedByRectangle(footLeftTemp, 
			footRightTemp, yMinTemp, path, left, right, top, bottom, 
			positionCollision, deltaCollision, wallDelta, wallCollision, 
			floorCollision, footCollision, minSquaredDistance);

		if (!blocked)
		{
			downFootPosition += path;
			velocity = velocityEnd;
#ifdef _DEBUG
			footLeftTemp += path;
			footRightTemp += path;
			yMinTemp += path.y;
			_ASSERT(IsCreatureLocationValidInRectangle(footLeftTemp,
				footRightTemp, yMinTemp, left, right, top, bottom));
#endif
			return;
		}

		collision = true;
		wall = wallCollision;

		// Calculate the collision velocity and how much time we've used up
		if (minSquaredDistance < 0.0001f)
		{
			CalculateCollisionVelocityAndElapsedTime(velocity, acceleration,
				path, deltaCollision, velocityCollision, timeElapsed);
			_ASSERT((timeElapsed >= 0.0f) && (timeElapsed <= 1.0f));
		}
		else
		{
			velocityCollision = velocity;
			timeElapsed = 0.0f;
		}

		// Move everything up to the collision point
		downFootPosition += deltaCollision;
		footLeftTemp += deltaCollision;
		footRightTemp += deltaCollision;
		yMinTemp += deltaCollision.y;
		// Check integrity
		_ASSERT(IsCreatureLocationValidInRectangle(footLeftTemp,
			footRightTemp, yMinTemp, left, right, top, bottom));

		if (applyPhysics && floorCollision)
		{
			downFootPosition = positionCollision;
			velocity = ZERO_VECTOR;
			downFoot = footCollision;
			stopped = true;
			return;
		}

		velocityReflected = velocityCollision.Reflect(wallDelta);

		// Work out the new velocity
		if (applyPhysics)
		{
			Vector2D positionStart = positionCollision - velocityCollision;
 			Vector2D positionEnd = positionCollision + velocityReflected;
			Vector2D positionMiddle = positionStart + 
				(positionEnd - positionStart)/2.0f;
			Vector2D normal = positionCollision - positionMiddle;
			Vector2D loss = normal * (1.0f-collisionFactor);
			velocity = velocityReflected + loss;
		}
		else
			velocity = velocityReflected;
		
		// Update the timers
		timeTotal += timeElapsed;
		timeRemaining = 1.0f - timeTotal;	

		if (timeRemaining <= 0.0f) 
			// We're done with this time slice
			return;
	}
	// Got stuck, so start dropping or come to rest
	velocity = ZERO_VECTOR;
}



// ---------------------------------------------------------------------------
// Function:	MoveAgentInsideRoomSystem
// Description: Calculates the new position and velocity of an agent moving 
//					within the room system
// Arguments:	width - width of agent (in)
//				height - height of agent (in)
//				applyPhysics - true to apply elasticity, friction, gravity
//					and aerodynamics; false otherwise (in)
//				minDoorPermiability - minimum door permiability [0 if
//					unblockable] (in) 
//				collisionFactor - percentage decrease in velocity due to
//					collision (in) 
//				aeroDynamicFactor - percentage decrease in velocity due to
//					media resistance (in) 
//				frictionFactor - percentage decrease in velocity due 
//					to friction (in)
//				gravity - how much y velocity increases per tick (in)
//				position - top-left corner of agent (in/out)
//				velocity - velocity of agent (in/out)
//				collision - was there a collision? (out)
//				wall - the wall the agent hit (out)
//				stopped - is agent now at rest? (out)
//				velocityCollision - velocity at point of collision (out)
// Returns:		None
// --------------------------------------------------------------------------
void Map::MoveAgentInsideRoomSystem
	(const float width, 
	 const float height, 
	 const bool applyPhysics,
	 const int minDoorPermiability,
	 const float collisionFactor, 
	 const float aeroDynamicFactor,
	 const float frictionFactor,
	 const float gravity, 
	 Vector2D& position, 
	 Vector2D& velocity,
	 bool& collision, 
	 int& wall, 
	 bool& stopped,
	 Vector2D& velocityCollision)
{
	Vector2D velocityEnd;
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	Vector2D velocityReflected;
	float timeElapsed;
	float minSquaredDistance;
	bool blocked;
	bool hitVertex;
	Door* doorCollision;
	Vector2D slope;
	Vector2D slideAcceleration;
	bool moved;
	Vector2D mirror;
	Door slidingDoor;
	bool edge;
	Door* wallDoor;
	Vector2D edgeDelta;
	Vector2D acceleration;
	bool floorSlidePotential;
	Vector2D positionVertex;
	bool spike;
	int wallCollision;
	Vector2D vc;

#ifdef _DEBUG
	Vector2D tempvelocity = velocity;
	Vector2D tempposition = position;
	// For testing, rewind to here
	velocity = tempvelocity;
	position = tempposition;
#endif

	collision = false;
	stopped = false;
	if (minDoorPermiability == 0) 
	{
		// Collision detection is off
		if (applyPhysics) 
		{
			velocityEnd = velocity;
			velocityEnd.y += gravity;
			path = (velocity + velocityEnd) / 2.0f;
			position += path;
			velocity = velocityEnd;
		}
		else
		{
			// Velocity doesn't change
			position += velocity;
		}
		return;
	}

	float timeTotal = 0.0f;
	float timeRemaining = 1.0f;
	bool sliding = false;
	int times = 10;
	Vector2D gravityAcceleration(0.0f, gravity);

	while (times-- > 0)
	{
		// Calculate the final velocity, trajectory and acceleration
		if (!applyPhysics)
		{
			velocityEnd = velocity;
			path = velocity * timeRemaining;
			acceleration = ZERO_VECTOR;
		}
		else if (!sliding) 
		{
			velocityEnd = velocity;
			velocityEnd.y += gravity * timeRemaining;
			velocityEnd *= aeroDynamicFactor;
			path = (velocity + velocityEnd) / 2.0f;
			path *= timeRemaining;
			acceleration = gravityAcceleration;
		}
		else
		{
			velocityEnd = velocity + (slideAcceleration * timeRemaining);
			path = velocity + slideAcceleration/2.0f;
			path *= timeRemaining;
			acceleration = slideAcceleration;
		}

		// See if any walls get in the way of the path
		blocked = IsAgentPathBlockedByRoomSystem(position, width, height, 
			path, minDoorPermiability, positionCollision, deltaCollision,
			doorCollision, hitVertex, positionVertex, edge, edgeDelta, 
			minSquaredDistance);

//		OutputFormattedDebugString("[%d] Position %f %f Velocity %f %f Path %f %f Blocked %d\n",
//			times, position.x, position.y, velocity.x, velocity.y, path.x, path.y, 
//			blocked);

		if (!blocked) 
		{
			position += path;
			velocity = velocityEnd;
			// Check integrity
			_ASSERT(IsAgentLocationValidInRoomSystem(position, width, height, 
				minDoorPermiability));
			return;
		}

		// Move up to the collision point
		position += deltaCollision;
		// Check integrity
		_ASSERT(IsAgentLocationValidInRoomSystem(position, width, height, 
			minDoorPermiability));

		if (minSquaredDistance < 0.0001f) 
			moved = false;
		else 
			moved = true;
		
		// Calculate the collision velocity and how much time we've used up
		if (moved) 
		{
			CalculateCollisionVelocityAndElapsedTime(velocity, acceleration,
				path, deltaCollision, vc, timeElapsed);
			_ASSERT((timeElapsed >= 0.0f) && (timeElapsed <= 1.0f));
		}
		else
		{
			vc = velocity;
			timeElapsed = 0.0f;
		}

		if (hitVertex && edge) 
		{	
			//
			// An edge of the diamond has hit a vertex
			//		
			floorSlidePotential = false;
			Vector2D temp;
			if (vc.IsNearZero(0.01f))
				temp = path;
			else
				temp = vc;
			GetReflectionFromVertex(positionVertex, temp, true, edgeDelta, 
				minDoorPermiability, velocityReflected, slidingDoor, spike);
			slope = edgeDelta;
			wallDoor = &slidingDoor;
		}
		else 
		{
			floorSlidePotential = true;
			if (hitVertex) 
			{
				// 
				// A point of the diamond has hit a vertex
				//
				Vector2D temp;
				if (vc.IsNearZero(0.01f))
					temp = path;
				else
					temp = vc;
				GetReflectionFromVertex(positionVertex, temp, true, edgeDelta,
					minDoorPermiability, velocityReflected, slidingDoor,
					spike);
				if (spike) 
					slope = edgeDelta;
				else
					slope = slidingDoor.delta;
				wallDoor = &slidingDoor;
			}
			else 
			{
				// 
				// A point of the diamond has hit a wall
				//
				velocityReflected = vc.Reflect(doorCollision->delta);
				slope = doorCollision->delta;
				wallDoor = doorCollision;
				spike = false;
			}
		}

		// Which wall did we hit?
		wallCollision = CalculateWallFromDoorAndPath(wallDoor, path);

		if (applyPhysics)
		{
			if (moved || (wallCollision != DIRECTION_FLOOR)) 
			{
				collision = true;
				wall = wallCollision;
				velocityCollision = vc;
			}

			if ((!moved) && 
				((wallCollision == DIRECTION_FLOOR) || spike)) 
			{	
				sliding = true;
				CalculateSlideAccelerationAndSlideVelocity(vc, slope, 
					gravityAcceleration, slideAcceleration, velocity);
				// Apply friction when sliding along the floor - but not when 
				// sliding along the edge of the diamond. This stops agents 
				// coming to rest on their edges.
				if (floorSlidePotential) 
				{
					velocity *= frictionFactor;
					if (velocity.IsNearZero(1.0f))
					{
						float d = slideAcceleration.Length();
						float s = gravity * (1.0f - frictionFactor);
						if (d <= s) 
						{
							// Friction has brought the agent to rest
							velocity = ZERO_VECTOR;
							stopped = true;
							return;
						}
					}
				}
			}
			else if (spike) 
			{
				// Moved and hit a spike
				velocity = velocityReflected * collisionFactor;
				sliding = false;
			}
			else 
			{
				Vector2D positionStart = positionCollision - vc;
 				Vector2D positionEnd = positionCollision + velocityReflected;
				Vector2D positionMiddle = positionStart + (positionEnd - positionStart)/2.0f;
				Vector2D normal = positionCollision - positionMiddle;
				Vector2D loss = normal * (1.0f-collisionFactor);
				velocity = velocityReflected + loss;
				if (collisionFactor == 0.0f)
				{
					// When elasticity is zero, we want to apply friction
					// the minute we collide
					velocity *= frictionFactor;
					if (velocity.IsNearZero(0.5f))
					{
						velocity = ZERO_VECTOR;
						return;
					}
				}
				sliding = false;
			}
		}
		else 
		{
			// No physics applied
			collision = true;
			wall = wallCollision;
			velocityCollision = vc;
			velocity = velocityReflected;
			sliding = false;
		}

		// Update the timers
		timeTotal += timeElapsed;
		timeRemaining = 1.0f - timeTotal;	

		if (timeRemaining <= 0.0f) 
			// We're done with this time slice
			return;
	}
	// Too many loops, bring the agent to rest
	velocity = ZERO_VECTOR;
	stopped = true;
}


// ---------------------------------------------------------------------------
// Function:	MoveCreatureInsideRoomSystem
// Description: Calculates the new position and velocity of a creature moving 
//					within the room system
// Arguments:	footLeft - position of left foot (in)
//				footRight - position of right foot (in)
//				yMin - y of top of bounding box (in)
//				applyPhysics - true to apply elasticity, gravity and 
//					aerodynamics; false otherwise (in)
//				minDoorPermiability - minimum door permiability [0 if
//					unblockable] (in) 
//				collisionFactor - percentage decrease in velocity due to
//					collision (in) 
//				aeroDynamicFactor - percentage decrease in velocity due to
//					media resistance (in) 
//				gravity - how much y velocity increases per tick (in)
//				downFootPosition - position of the down foot (in/out)
//				velocity - velocity of creature (in/out)
//				collision - was there a collision? (out)
//				wall - the wall the creature hit (out)
//				stopped - has creature came to rest on the floor? (out)
//				downFoot - which foot is the down foot; left 0, right 1 (out)			
//				velocityCollision - velocity at point of collision (out)
// Returns:		None
// --------------------------------------------------------------------------
void Map::MoveCreatureInsideRoomSystem
	(const Vector2D& footLeft,
	 const Vector2D& footRight,
	 const float yMin,
	 const bool applyPhysics,
	 const int minDoorPermiability,
	 const float collisionFactor,
	 const float aeroDynamicFactor,
	 const float gravity, 
	 Vector2D& downFootPosition,
	 Vector2D& velocity,
	 bool& collision,
	 int& wall,
	 bool& stopped,
	 int& downFoot,
	 Vector2D& velocityCollision)
{
	Vector2D velocityEnd;
	Vector2D path;
	Vector2D positionCollision;
	Vector2D deltaCollision;
	Vector2D velocityReflected;
	float timeElapsed;
	float minSquaredDistance;
	bool blocked;
	bool hitVertex;
	Door* doorCollision;
	Door slidingDoor;
	Door* wallDoor;
	Vector2D acceleration;
	Vector2D positionVertex;
	bool floorCollision;
	int footCollision;
	bool moved;
	bool spike;
	
	collision = false;
	stopped = false;
	if (minDoorPermiability == 0) 
	{
		// Collision detection is off
		if (applyPhysics) 
		{
			velocityEnd = velocity;
			velocityEnd.y += gravity;
			path = (velocity + velocityEnd) / 2.0f;
			downFootPosition += path;
			velocity = velocityEnd;
		}
		else
		{
			// Velocity doesn't change
			downFootPosition += velocity;
		}
		return;
	}
	float timeTotal = 0.0f;
	float timeRemaining = 1.0f;
	Vector2D gravityAcceleration(0.0f, gravity);
	Vector2D footLeftTemp = footLeft;
	Vector2D footRightTemp = footRight;
	float yMinTemp = yMin;
	int times = 10;
	while (times-- > 0)
	{
		// Calculate the final velocity, trajectory and acceleration
		if (applyPhysics)
		{
			velocityEnd = velocity;
			velocityEnd.y += gravity * timeRemaining;
			velocityEnd *= aeroDynamicFactor;
			path = (velocity + velocityEnd) / 2.0f;
			path *= timeRemaining;
			acceleration = gravityAcceleration;	
		}
		else
		{
			velocityEnd = velocity;
			path = velocity * timeRemaining;
			acceleration = ZERO_VECTOR;
		}
		
		// See if any walls get in the way of the path
		blocked = IsCreaturePathBlockedByRoomSystem(footLeftTemp, 
			footRightTemp, yMinTemp, path, minDoorPermiability, 
			positionCollision, deltaCollision, doorCollision, hitVertex, 
			positionVertex, floorCollision, footCollision, 
			minSquaredDistance);

		if (!blocked) 
		{
			downFootPosition += path;
			velocity = velocityEnd;
#ifdef _DEBUG
			// Check integrity
			footLeftTemp += path;
			footRightTemp += path;
			yMinTemp += path.y;
			_ASSERT(IsCreatureLocationValidInRoomSystem(footLeftTemp,
				footRightTemp, yMinTemp, minDoorPermiability));
#endif
			return;
		}

		collision = true;

		// Move everything up to the collision point
		downFootPosition += deltaCollision;
		footLeftTemp += deltaCollision;
		footRightTemp += deltaCollision;
		yMinTemp += deltaCollision.y;

		// Check integrity
		_ASSERT(IsCreatureLocationValidInRoomSystem(footLeftTemp,
			footRightTemp, yMinTemp, minDoorPermiability));

		if (minSquaredDistance < 0.0001f) 
			moved = false;
		else 
			moved = true;
		
		// Calculate the collision velocity and how much time we've used up
		if (moved) 
		{
			CalculateCollisionVelocityAndElapsedTime(velocity, acceleration,
				path, deltaCollision, velocityCollision, timeElapsed);
			_ASSERT((timeElapsed >= 0.0f) && (timeElapsed <= 1.0f));
		}
		else
		{
			velocityCollision = velocity;
			timeElapsed = 0.0f;
		}

		if (hitVertex) 
		{
			Vector2D temp;
			if (velocityCollision.IsNearZero(0.01f))
				temp = path;
			else
				temp = velocityCollision;
			Vector2D dummy;
			GetReflectionFromVertex(positionVertex, temp, false, dummy,
				minDoorPermiability,
				velocityReflected, slidingDoor, spike);
			wallDoor = &slidingDoor;
		}
		else 
		{
			velocityReflected = velocityCollision.Reflect(doorCollision->delta);
			wallDoor = doorCollision;
			spike = false;
		}

		if (applyPhysics && floorCollision && !spike)
		{
			downFootPosition = positionCollision;
			velocity = ZERO_VECTOR;
			downFoot = footCollision;
			wall = DIRECTION_FLOOR;
			stopped = true;
			return;
		}

		// Which wall did we hit?
		wall = CalculateWallFromDoorAndPath(wallDoor, path);

		// Work out the new velocity
		if (applyPhysics) 
		{			
			Vector2D positionStart = positionCollision - velocityCollision;
 			Vector2D positionEnd = positionCollision + velocityReflected;
			Vector2D positionMiddle = positionStart + (positionEnd - positionStart)/2.0f;
			Vector2D normal = positionCollision - positionMiddle;
			Vector2D loss = normal * (1.0f-collisionFactor);
			velocity = velocityReflected + loss;
		}
		else
			velocity = velocityReflected;

		// Update the timers
		timeTotal += timeElapsed;
		timeRemaining = 1.0f - timeTotal;	

		if (timeRemaining <= 0.0f) 
			// We're done with this time slice
			return;
	}
	// Got stuck, so start dropping or come to rest
	velocity = ZERO_VECTOR;
}



//
//
// Creature Walking
//
//


// ---------------------------------------------------------------------
// Function:	GetNearbyFloorInformation
// Description:	Gets the rooms which define the floor surrounding a 
//					given point
// Arguments:	position - position to get floor information for (in)
//				floorRooms - the floor rooms (out)
//				floorRoomCount - the number of rooms (out)
// Returns:		None
// ----------------------------------------------------------------------
void Map::GetNearbyFloorInformation
	(const Vector2D& position,
	 Room** floorRooms,
	 int& floorRoomCount)
{
	int id;
	int nid, id1, id2;
	int dummy1;
	Room* startroom;
	Room* leftroom;
	Room* rightroom;
	Room* room1;
	Room* room2;
	int idList[4];
	Room* roomList[4];
	int idCount;

	// Get the room IDs surrounding our point
	bool ok = GetRoomInformationForPoint(position, idList, roomList,
		idCount, dummy1);
	_ASSERT(ok);
	_ASSERT((idCount == 1) || (idCount == 2));

	floorRoomCount = 0;
	if (idCount == 1)
	{
		id = idList[0];
		startroom = myRoomCollection[id];
		floorRooms[floorRoomCount] = startroom;
		++floorRoomCount;
		// Go left from initial room
		nid = startroom->leftFloorRoomID;
		if (nid != -1)
		{
			leftroom = myRoomCollection[nid];
			floorRooms[floorRoomCount] = leftroom;
			++floorRoomCount;
			// Go left
			nid = leftroom->leftFloorRoomID;
			if (nid != -1)
			{
				leftroom = myRoomCollection[nid];
				floorRooms[floorRoomCount] = leftroom;
				++floorRoomCount;
			}
		}
		// Go right from initial room
		nid = startroom->rightFloorRoomID;
		if (nid != -1)
		{
			rightroom = myRoomCollection[nid];
			floorRooms[floorRoomCount] = rightroom;
			++floorRoomCount;
			// Go right
			nid = rightroom->rightFloorRoomID;
			if (nid != -1)
			{
				rightroom = myRoomCollection[nid];
				floorRooms[floorRoomCount] = rightroom;
				++floorRoomCount;
			}
		}
	}
	else
	{
		id1 = idList[0];
		id2 = idList[1];
		room1 = myRoomCollection[id1];
		room2 = myRoomCollection[id2];
		if (room1->startFloor.x < room2->startFloor.x)
		{
			leftroom = room1;
			rightroom = room2;
		}
		else
		{
			leftroom = room2;
			rightroom = room1;
		}
		floorRooms[0] = leftroom;
		floorRooms[1] = rightroom;
		floorRoomCount = 2;
		// Go left from left room
		nid = leftroom->leftFloorRoomID;
		if (nid != -1)
		{
			leftroom = myRoomCollection[nid];
			floorRooms[floorRoomCount] = leftroom;
			++floorRoomCount;
		}
		// Go right from right room
		nid = rightroom->rightFloorRoomID;
		if (nid != -1)
		{
			rightroom = myRoomCollection[nid];
			floorRooms[floorRoomCount] = rightroom;
			++floorRoomCount;
		}
	}
}	


// ---------------------------------------------------------------------
// Function:	IsRegionBlockedByVerticalWall
// Description:	See if any surrounding vertical walls block a region,
//					where the region represents the movement of a 
//					walking creature
// Arguments:	floorRooms - the floor rooms (in)
//				floorRoomCount - the number of rooms (in)
//				minDoorPermiability - minimum door permiability (in) 
//				xMinTest... - the region to test (in)
//				right - the direction of creature movement (in)
//				xDisplacement - how much to recoil (out)
// Returns:		true if blocked, false if not
// ----------------------------------------------------------------------
bool Map::IsRegionBlockedByVerticalWall
	(Room** floorRooms,
	 int floorRoomCount,
	 const int minDoorPermiability,
	 const float xMinTest,
	 const float xMaxTest,
	 const float yMinTest,
	 const float yMaxTest,
	 const bool right,
	 float& xDisplacement)
{
	int i;
	Room* room;
	DoorIterator doorIterator;
	DoorIterator doorIteratorEnd;
	Door* door;
	float dist;

	bool collision = false;
	if (right)
		xDisplacement = -FLT_MAX;
	else
		xDisplacement = FLT_MAX;

	for (i=0; i<floorRoomCount;++i)
	{
		room = floorRooms[i];
		if (right)
		{
			// Only check right-side walls
			doorIterator = room->rightCollection.begin();
			doorIteratorEnd = room->rightCollection.end();
		}
		else
		{
			// Only check left-side walls
			doorIterator = room->leftCollection.begin();
			doorIteratorEnd = room->leftCollection.end();
		}

		while (doorIterator != doorIteratorEnd) 
		{
			door = *(doorIterator++);
			if (door->permiability >= minDoorPermiability)
				// Door is non-blocking, ignore it
				continue;

			// Check if door's x lies within the region
			if ((door->start.x < xMinTest) || (door->start.x > xMaxTest))
				continue;

			// Check if door overlaps the region
			if (((yMinTest >= door->start.y) && (yMinTest <= door->end.y)) ||
				((yMaxTest >= door->start.y) && (yMaxTest <= door->end.y)) ||
				((door->start.y >= yMinTest) && (door->start.y <= yMaxTest)) ||
				((door->end.y >= yMinTest) && (door->end.y <= yMaxTest)))
			{
				collision = true;
				if (right)
				{
					dist = door->start.x - xMaxTest - TOLERANCE;
					if (dist > xDisplacement)
						xDisplacement = dist;
				}
				else
				{
					dist = door->start.x - xMinTest + TOLERANCE;
					if (dist < xDisplacement)
						xDisplacement = dist;
				}
			}
		} 		
	}
	return collision;
}



// ---------------------------------------------------------------------
// Function:	SnapPointsToFloor
// Description:	Snaps two points vertically onto the surrounding floor
// Arguments:	position1/2 - points to be snapped
//				floorRooms - the floor rooms (in)
//				floorRoomCount - the number of rooms (in)
//				minDoorPermiability - minimum door permiability (in)
//				right - true if moving right (in) 
//				snapped1/2 - whether the points could be snapped (out)
//				positionSnap1/2 - the snapped points (out)
//				door1/2 - the doors the points snapped to (out)
//				distance1/2 - the distances to the snap points (out)
// Returns:		None
// ----------------------------------------------------------------------
void Map::SnapPointsToFloor
	(const Vector2D& position1,
	 const Vector2D& position2,
	 Room** floorRooms, 
	 const int floorRoomCount,
	 const int minDoorPermiability,
	 const bool right,
	 bool& snapped1,
	 bool& snapped2,
	 Vector2D& positionSnap1,
	 Vector2D& positionSnap2,
	 Door*& door1,
	 Door*& door2,
	 float& distance1,
	 float& distance2)
{
	int i;
	Room* room;
	DoorIterator doorIterator;
	DoorIterator doorIteratorEnd;
	Door* door;
	float y;
	float yMin1 = FLT_MAX;
	float yMin2 = FLT_MAX;
	float d;
	float x1, x2;
	static const float MAX_STRIDE = 50.0f;

	// Assume neither point snaps
	snapped1 = false;
	snapped2 = false;
	if (right) 
	{
		x1 = -FLT_MAX;
		x2 = -FLT_MAX;
	}
	else
	{
		x1 = FLT_MAX;
		x2 = FLT_MAX;
	}
	for (i=0; i<floorRoomCount;++i)
	{
		room = floorRooms[i];
		doorIterator = room->floorCollection.begin();
		doorIteratorEnd = room->floorCollection.end();
		while (doorIterator != doorIteratorEnd) 
		{
			door = *(doorIterator++);
			if (door->permiability >= minDoorPermiability)
				// This is a non-blocking door
				continue;

			if ((position1.x >= door->start.x) && 
				(position1.x <= door->end.x))
			{
				y = CalculateY(position1, door->start, door->delta);
				d = y - position1.y;
				if ((d < MAX_STRIDE) && (y <= yMin1))
				{
					snapped1 = true;
					yMin1 = y;
					positionSnap1.x = position1.x;
					positionSnap1.y = y-TOLERANCE;
					distance1 = d;	
					if (right)
					{
						if (door->start.x > x1)
						{
							door1 = door;
							x1 = door->start.x;
						}
					}
					else
					{
						if (door->start.x < x1)
						{
							door1 = door;
							x1 = door->start.x;
						}
					}
				}
			}
			
			if ((position2.x >= door->start.x) && 
				(position2.x <= door->end.x))
			{
				y = CalculateY(position2, door->start, door->delta);
				d = y - position2.y;
				if ((d < MAX_STRIDE) && (y <= yMin2))
				{
					snapped2 = true;
					yMin2 = y;
					positionSnap2.x = position2.x;
					positionSnap2.y = y-TOLERANCE;
					distance2 = d;
					if (right)
					{
						if (door->start.x > x2)
						{
							door2 = door;
							x2 = door->start.x;
						}
					}
					else
					{
						if (door->start.x < x2)
						{
							door2 = door;
							x2 = door->start.x;
						}
					}
				}
			}
		}
	}
}


// ---------------------------------------------------------------------
// Function:	SnapFeetToFloor
// Description:	Snaps the two feet onto the surrounding floor
// Arguments:	downFoot - the down foot position (in)
//				upFoot - the up foot position (in)
//				floorRooms - the floor rooms (in)
//				floorRoomCount - the number of rooms (in)
//				minDoorPermiability - minimum door permiability (in) 
//				right - true if moving right (in)
//				downFootNew - the new down foot position (out)
//				door - the door that the new down foot is on (out)
//				footChange - true if up foot is now the down foot (out)
//				fall - true if creature couldn't be snapped to the
//					floor and must start to fall (out)
// Returns:		None
// ----------------------------------------------------------------------
void Map::SnapFeetToFloor
	(const Vector2D& downFoot,
	 const Vector2D& upFoot, 
	 Room** floorRooms, 
	 const int floorRoomCount,
	 const int minDoorPermiability,
	 const bool right,
	 Vector2D& downFootNew,
	 Door*& door,
	 bool& footChange,
	 bool& fall)
{
	float xDisplacement;
	float upDistance, downDistance;
	Vector2D upSnap, downSnap;
	bool upSnapped, downSnapped;
	Vector2D down(downFoot);
	Vector2D up(upFoot);
	Door* downDoor;
	Door* upDoor;
	static const float SHIFT_DISTANCE = 10.0f;
	
	// Snap the two feet positions onto the floor
	SnapPointsToFloor(down, up, floorRooms, floorRoomCount,
		minDoorPermiability, right, downSnapped, upSnapped, downSnap, upSnap,
		downDoor, upDoor, downDistance, upDistance);

	if (!downSnapped && !upSnapped)
	{
		// None of the feet could snap, so just fall
		downFootNew = down;
		door = NULL;
		footChange = false;
		fall = true;
	}
	else if (downSnapped && upSnapped)
	{
		// Both feet snapped, find nearest one to floor
		if (upDistance < downDistance)
		{
			door = upDoor;
			footChange = true;
			downFootNew = upSnap;
		}
		else
		{
			door = downDoor;
			footChange = false;
			downFootNew = downSnap;
		}
		fall = false;
	}
	else
	{
		// One of the feet snapped but the other didn't, so move the
		// snapped foot a little bit towards the non-snapped foot and 
		// try again
		if (downSnapped && !upSnapped)
		{	
			if (down.x < up.x)
				xDisplacement = SHIFT_DISTANCE;
			else
				xDisplacement = -SHIFT_DISTANCE;
		}
		else
		{
			if (down.x < up.x)
				xDisplacement = -SHIFT_DISTANCE;
			else
				xDisplacement = SHIFT_DISTANCE;
		}

		// Shift the feet
		up.x += xDisplacement;
		down.x += xDisplacement;

		// Snap the new feet positions onto the floor
		SnapPointsToFloor(down, up, floorRooms, floorRoomCount,
			minDoorPermiability, right,
			downSnapped, upSnapped, downSnap, upSnap,
			downDoor, upDoor,
			downDistance, upDistance);	

		if (!downSnapped && !upSnapped)
		{
			// None of the feet could snap, so just fall
			door = NULL;
			downFootNew = down;
			footChange = false;
			fall = true;
		}
		else if (downSnapped && upSnapped)
		{
			// Both feet snapped, find nearest one to floor
			if (upDistance < downDistance)
			{
				door = upDoor;
				footChange = true;
				downFootNew = upSnap;
			}
			else
			{
				door = downDoor;
				footChange = false;
				downFootNew = downSnap;
			}
			fall = false;
		}
		else if (downSnapped && !upSnapped)
		{
			door = downDoor;
			footChange = false;
			downFootNew = downSnap;
			fall = false;
		}
		else
		{
			door = upDoor;
			footChange = true;
			downFootNew = upSnap;
			fall = false;
		}
	}
}


// ---------------------------------------------------------------------
// Function:	CalculateGradients
// Description:	Calculates the downhill/uphill gradients
// Arguments:	fall - if the creature will fall (in)
//				right - if the creature was moving right (in)
//				door - the door the down foot snapped to (in)
//				gradientDownhill - downhill gradient (out)
//				gradientUphill - uphill gradient (out)
// Returns:		None
// ----------------------------------------------------------------------
void Map::CalculateGradients
	(const bool fall,
	 const bool right,
	 Door* door,
	 float& gradientDownhill,
	 float& gradientUphill)
{
	if (fall)
	{
		gradientUphill = 0.0f;
		gradientDownhill = 0.0f;
		return;
	}
	float gradient = (door->delta.y)/(door->delta.x);
	if (gradient == 0.0f)
	{
		gradientUphill = 0.0f;
		gradientDownhill = 0.0f;
		return;
	}

	if (gradient > 1.0f)
		gradient = 1.0f;
	else if (gradient < -1.0f)
		gradient = -1.0f;
	if (right)
	{
		if (gradient < 0.0f)
		{
			// Uphill
			gradientDownhill = 0.0f;
			gradientUphill = -gradient;
		}
		else
		{
			// Downhill
			gradientDownhill = gradient;
			gradientUphill = 0.0f;
		}
	}
	else
	{
		if (gradient < 0.0f)
		{
			// Downhill
			gradientDownhill = -gradient;
			gradientUphill = 0.0f;
		}
		else
		{
			// Uphill
			gradientDownhill = 0.0f;
			gradientUphill = gradient;
		}
	}
}



// ---------------------------------------------------------------------
// Function:	TestNewUpFootInRoomSystem
// Description:	Tests the position of the new up foot with respect to
//					the room system
// Arguments:	downFootCurrent - the current down foot position (in)
//				upFootPrevious - the previous up foot position (in)
//				upFootCurrent - the current up foot position (in)
//				xMinCurrent - min x of the current bounding box (in)
//				xMaxCurrent - max x of the current bounding box (in)
//				yMinPrevious - min y of the previous bounding box (in)
//				yMinCurrent - min y of the current bounding box (in)
//				minDoorPermiability - minimum door permiability (in) 
//				directionRight - if the creature is walking right (in)
//				downFootNew - the new down foot position (out)
//				gradientDownhill - downhill gradient (out)
//				gradientUphill - uphill gradient (out)
//				collision - true if there was a wall collision (out)
//				wall - which wall the creature hit (out)
//				footChange - true if up foot is now the down foot (out)
//				fall - true if creature should start to fall (out)
// Returns:		None
// ----------------------------------------------------------------------
void Map::TestNewUpFootInRoomSystem
	(const Vector2D& downFootCurrent,
	 const Vector2D& upFootPrevious, 
	 const Vector2D& upFootCurrent,
	 const float xMinCurrent,
	 const float xMaxCurrent,
	 const float yMinPrevious,
	 const float yMinCurrent,
	 const int minDoorPermiability,
	 const bool directionRight,
	 Vector2D& downFootNew,
	 float& gradientDownhill,
	 float& gradientUphill,
	 bool& collision,
	 int& wall,
	 bool& footChange,
	 bool& fall)
{
	// Work out the minimum and maximum y
	float yMinTest, yMaxTest;
	if (yMinPrevious < yMinCurrent)
		yMinTest = yMinPrevious;
	else
		yMinTest = yMinCurrent;
	if (upFootPrevious.y > upFootCurrent.y)
		yMaxTest = upFootPrevious.y;
	else	
		yMaxTest = upFootCurrent.y;

	// Work out the minimum and maximum x and the direction of movement
	float xMinTest, xMaxTest;
	bool right;
	if (upFootCurrent.x > upFootPrevious.x)
	{
		// Moving right
		xMinTest = upFootPrevious.x;
		xMaxTest = xMaxCurrent;
		right = true;
	}
	else
	{
		// Moving left
		xMinTest = xMinCurrent;
		xMaxTest = upFootPrevious.x;
		right = false;
	}

	Vector2D down(downFootCurrent);
	Vector2D up(upFootCurrent);
	Room* floorRooms[5];
	int floorRoomCount;
	float xDisplacement;

	// Get the floors surrounding the down foot
	GetNearbyFloorInformation(down, floorRooms, floorRoomCount);

	// See if there is a vertical wall in the way
	collision = IsRegionBlockedByVerticalWall(floorRooms, floorRoomCount,
		minDoorPermiability, xMinTest, xMaxTest, yMinTest, yMaxTest, 
		right, xDisplacement);

	if (collision)
	{
		// Shift backwards away from the wall
		down.x += xDisplacement;	
		up.x += xDisplacement;
		if (right)
			wall = DIRECTION_RIGHT;
		else
			wall = DIRECTION_LEFT;
	}


	Door* door;
	// Snap the feet to the floor
	SnapFeetToFloor(down, up, floorRooms, floorRoomCount, 
		minDoorPermiability, right, downFootNew, door, footChange, fall);
	CalculateGradients(fall, directionRight, door, gradientDownhill, 
		gradientUphill);
}


// ---------------------------------------------------------------------
// Function:	TestNewUpFootInRectangle
// Description:	Tests the position of the new up foot with respect to
//					a rectangle
// Arguments:	downFootCurrent - the current down foot position (in)
//				upFootCurrent - the current up foot position (in)
//				xMinCurrent - min x of the current bounding box (in)
//				xMaxCurrent - max x of the current bounding box (in)
//				left - x of left side (in)
//				right - x of right side (in)
//				top - y of top side (in)
//				bottom - y of bottom side (in)
//				downFootNew - the new down foot position (out)
//				collision - true if there was a wall collision (out)
//				wall - which wall the creature hit (out)
//				footChange - true if up foot is now the down foot (out)
// Returns:		None
// ----------------------------------------------------------------------
void Map::TestNewUpFootInRectangle
	(const Vector2D& downFootCurrent,
	 const Vector2D& upFootCurrent,
	 const float xMinCurrent,
	 const float xMaxCurrent,
	 const float left,
	 const float right,
	 const float top,
	 const float bottom,
	 Vector2D& downFootNew,
	 bool& collision,
	 int& wall,
	 bool& footChange)
{
	float gapWest;
	float gapEast;

	if (downFootCurrent.x < upFootCurrent.x)
	{
		gapWest = downFootCurrent.x - xMinCurrent;
		gapEast = xMaxCurrent - upFootCurrent.x; 
	}
	else
	{
		gapWest = upFootCurrent.x - xMinCurrent;
		gapEast = xMaxCurrent - downFootCurrent.x; 
	}

	Vector2D p = upFootCurrent;
	if (upFootCurrent.x < left+gapWest+TOLERANCE)
	{
		collision = true;
		p.x = left+gapWest+TOLERANCE;
		wall = DIRECTION_LEFT;
	}
	else if (upFootCurrent.x > right-gapEast-TOLERANCE)
	{
		collision = true;
		p.x = right-gapEast-TOLERANCE;
		wall = DIRECTION_RIGHT;
	} 
	else 
	{
		collision = false;
	}

	if (upFootCurrent.y > bottom-TOLERANCE)
	{
		footChange = true;
		p.y = bottom-TOLERANCE;
	} 
	else 
	{
		footChange = false;
	}

	if (footChange) 
	{
		downFootNew = p;
	}
	else
	{
		// Shift down foot away from collision if need be
		downFootNew = downFootCurrent;
		downFootNew.x += p.x - upFootCurrent.x;
	}
}



// ---------------------------------------------------------------------
// Function:	ShiftCreatureAlongFloor
// Description:	Shifts a creature left or right along the floor
// Arguments:	downFoot - the current down foot position (in)
//				upFoot - the current up foot position (in)
//				xMin - min x of the current bounding box (in)
//				xMax - max x of the current bounding box (in)
//				yMin - min y of the current bounding box (in)
//				minDoorPermiability - minimum door permiability (in)
//				xShift - the amount to shift the creature by (in) 
//				downFootNew - the new down foot position (out)
//				collision - true if there was a wall collision (out)
//				wall - which wall the creature hit (out)
//				footChange - true if up foot is now the down foot (out)
//				fall - true if creature should start to fall (out)
// Returns:		None
// ----------------------------------------------------------------------
void Map::ShiftCreatureAlongFloor
	(const Vector2D& downFoot,
	 const Vector2D& upFoot, 
	 const float xMin,
	 const float xMax,
	 const float yMin,
	 const int minDoorPermiability,
	 const float xShift,
	 Vector2D& downFootNew,
	 bool& collision,
	 int& wall,
	 bool& footChange,
	 bool& fall)
{
	float xMinTest, xMaxTest;
	float yMinTest, yMaxTest;
	bool right;

	yMinTest = yMin;
	if (upFoot.y > downFoot.y)
		yMaxTest = upFoot.y;
	else
		yMaxTest = downFoot.y;

	float westX;
	float eastX;
	if (downFoot.x < upFoot.x)
	{
		westX = downFoot.x;
		eastX = upFoot.x;
	}
	else
	{
		westX = upFoot.x;
		eastX = downFoot.x;
	}

	if (xShift > 0.0f)
	{
		// Moving right
		xMinTest = westX;
		xMaxTest = xMax + xShift;
		right = true;
	}
	else
	{
		// Moving left
		xMinTest = xMin + xShift;
		xMaxTest = eastX;
		right = false;
	}

	// Get the floors surrounding the down foot
	Room* floorRooms[5];
	int floorRoomCount;
	GetNearbyFloorInformation(downFoot, floorRooms, floorRoomCount);

	// See if there is a vertical wall in the way
	float xDisplacement;
	collision = IsRegionBlockedByVerticalWall(floorRooms, floorRoomCount,
		minDoorPermiability, xMinTest, xMaxTest, yMinTest,
		yMaxTest, right, xDisplacement);

	Vector2D down(downFoot);
	Vector2D up(upFoot);

	// Shift the feet
	down.x += xShift;
	up.x += xShift;
	if (collision)
	{
		// Shift backwards away from the wall
		down.x += xDisplacement;	
		up.x += xDisplacement;
		if (right)
			wall = DIRECTION_RIGHT;
		else
			wall = DIRECTION_LEFT;
	}


	Door* door;
	// Snap the feet to the floor
	SnapFeetToFloor(down, up, floorRooms, floorRoomCount, 
		minDoorPermiability, right, downFootNew, door, footChange, fall);
}



//
//
// Room and Meta-Room Handling
//
// 


// ---------------------------------------------------------------------
// Function:	GetRoomCentre
// Description:	Gets the centre point of a room
// Arguments:	roomID - ID of the room (in)
//				centre - the centre point (out)
// Returns:	true if good input, false otherwise
// ----------------------------------------------------------------------
bool Map::GetRoomCentre
	(const int roomID, Vector2D& centre)
{
	Room* room;
	if (!GetValidRoomPointer(roomID, room))
		return false;
	centre = room->centre;
	return true;
}


// ---------------------------------------------------------------------
// Function:	GetMetaRoomTrack
// Description:	Gets a meta-room's track string
// Arguments:	metaRoomID - ID of the meta-room to get track for (in)
//				track - track string (out)
// Returns:	true if good input, false otherwise
// ----------------------------------------------------------------------
bool Map::GetMetaRoomTrack
	(const int metaRoomID, std::string& track)
{
	MetaRoom* metaRoom;
	bool ok; 

	// Get a pointer to the meta-room
	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	track = metaRoom->track;
	return true;
}


// ---------------------------------------------------------------------
// Function:	SetMetaRoomTrack
// Description:	Sets a meta-room's track string
// Arguments:	metaRoomID - ID of the meta-room to set (in)
//				track - track string (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------
bool Map::SetMetaRoomTrack
	(const int metaRoomID, std::string& track)
{
	MetaRoom* metaRoom;
	bool ok;

	// Get a pointer to the meta-room
	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	metaRoom->track = track;
	return true;
}

// ---------------------------------------------------------------------
// Function:	SetRoomTrack
// Description:	Sets a room's track string
// Arguments:	roomID - ID of the room to set (in)
//				track - track string (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------
bool Map::SetRoomTrack
	(const int roomID, std::string& track)
{
	Room* room;
	bool ok;

	// Get a pointer to the meta-room
	ok = GetValidRoomPointer(roomID, room);
	if (!ok)
		return false;
	room->track = track;
	return true;
}


// ---------------------------------------------------------------------
// Function:	GetRoomTrack
// Description:	Gets a room's track string
// Arguments:	roomID - ID of the room to get track for (in)
//				track - track string (out)
// Returns:		true if good input, false otherwise
// ----------------------------------------------------------------------
bool Map::GetRoomTrack
	(const int roomID, std::string& track)
{
	Room* room;
	bool ok;

	// Get a pointer to the room
	ok = GetValidRoomPointer(roomID, room);
	if (!ok)
		return false;
	track = room->track;
	return true;
}

 
// ---------------------------------------------------------------------------
// Function:	AddMetaRoom
// Description:	Adds a new meta-room to the map
// Arguments:	positionX - top-left x coordinate of meta-room (in)
//				positionY - top-left y coordinate of meta-room (in)
//				width - width of meta-room (in)
//				height - height of meta-room (in)
//				background - initial background (in)
//				metaRoomID - ID of the new meta-room (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::AddMetaRoom
	(const int positionX, const int positionY, const int width, 
	const int height, std::string background, int & metaRoomID)
{
	int i;
	bool slotFound;
	MetaRoom* metaRoom;
	int metaID;
	float w = width;
	float h = height;
	Vector2D position(positionX, positionY);
	Vector2D positionMin = position;
	Vector2D positionMax = position;
	positionMax.x += w - 1.0f;
	positionMax.y += h - 1.0f;

	// Is there an unused ID slot?
	slotFound = false;
	if( myMetaRoomMaxID < myMetaRoomIndexBase - 1)
		myMetaRoomMaxID = myMetaRoomIndexBase - 1;
	for (i=myMetaRoomIndexBase; i<myMetaRoomMaxID; ++i) 
	{
		metaRoom = myMetaRoomCollection[i];
		if (metaRoom == NULL) 
		{
			slotFound = true;
			break;
		}
	}
	// Get the ID to use for the meta-room
	if (!slotFound) 
	{
		++myMetaRoomMaxID;
		metaID = myMetaRoomMaxID;
	}
	else 
	{
		metaID = i;
	}

	// Create a new meta-room
	metaRoom = new MetaRoom;
	metaRoom->metaRoomID = metaID;
	metaRoom->positionMin = positionMin;
	metaRoom->positionMax = positionMax;
	metaRoom->positionDefault = positionMin;
	metaRoom->width = w;
	metaRoom->height = h;	
	metaRoom->background = background;
	metaRoom->backgroundCollection.insert(background);
	// Store the new meta-room
	myMetaRoomCollection[metaID] = metaRoom;
	++myMetaRoomCount;
	// Store the ID in the list of meta-room IDs
	myMetaRoomIDCollection.push_back(metaID);	
	// Update the maximum extents of all the meta-rooms
	if (positionMax.x > myMetaRoomMaxCoordinates.x)
		myMetaRoomMaxCoordinates.x = positionMax.x;
	if (positionMax.y > myMetaRoomMaxCoordinates.y)
		myMetaRoomMaxCoordinates.y = positionMax.y;
	// Pass back the new meta-room ID to the caller
	metaRoomID = metaID;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	RemoveMetaRoom
// Description:	Removes an existing meta-room from the map
// Arguments:	metaRoomID - ID of the meta-room to remove (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::RemoveMetaRoom
	(const int metaRoomID)
{
	MetaRoom* metaRoom;
	MetaRoom* m;
	int roomCount;
	int i;
	float xMax, yMax;
	bool ok;

	// Get a pointer to the meta-room
	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	// Not allowed to remove a meta-room while it still has rooms in it
	roomCount = metaRoom->roomIDCollection.size();
	if (roomCount > 0)
		return false;
	// Remove the meta-room from the meta-room collection
	myMetaRoomCollection[metaRoomID] = NULL;
	myMetaRoomCount--;
	// Remove the ID from the meta-room ID list
	myMetaRoomIDCollection.erase
		(std::find(myMetaRoomIDCollection.begin(), 
				   myMetaRoomIDCollection.end(), 
				   metaRoomID));
	if (myMetaRoomCount == 0) 
	{
		myMetaRoomMaxID = -1;
	}
	else if (metaRoomID == myMetaRoomMaxID) 
	{
		// Scan backwards to find the highest used ID
		for (i=metaRoomID-1; i>=0; i--) 
		{
			if (myMetaRoomCollection[i] != NULL) 
			{
				myMetaRoomMaxID = i;
				break;
			}
		}
	}
	// Scan all remaining meta-rooms to find the new maximum extents
	myMetaRoomMaxCoordinates = Vector2D(-1.0f, -1.0f);
	for (i=0; i<=myMetaRoomMaxID; ++i) 
	{
		m = myMetaRoomCollection[i];
		if (m == NULL)
			continue;
		xMax = m->positionMax.x;
		yMax = m->positionMax.y;
		if (xMax > myMetaRoomMaxCoordinates.x)
			myMetaRoomMaxCoordinates.x = xMax;
		if (yMax > myMetaRoomMaxCoordinates.y)
			myMetaRoomMaxCoordinates.y = yMax;
	}
	// Destroy the meta-room
	delete metaRoom;
	if (metaRoomID == myMetaRoomID)
		myMetaRoomID = -1;
	return true;
}


// ------------------------------------------------------------------------
// Function:	GetMetaRoomCount
// Description:	Returns the number of meta-rooms in the map
// Arguments:	None
// Returns:		Meta-room count
// ------------------------------------------------------------------------
int Map::GetMetaRoomCount(void)
{
	return myMetaRoomCount;
}


// ---------------------------------------------------------------------------
// Function:	SetCurrentMetaRoom
// Description:	Sets the map's current meta-room
// Arguments:	metaRoomID - ID of the meta-room to set (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::SetCurrentMetaRoom
	(const int metaRoomID)
{
	bool ok;
	MetaRoom* metaRoom;

	// Get a pointer to the meta-room
	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	myMetaRoomID = metaRoomID;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetCurrentMetaRoom
// Description:	Gets the map's current meta-room
// Arguments:	None
// Returns:		ID of the meta-room, -1 if none
// ---------------------------------------------------------------------------
int Map::GetCurrentMetaRoom(void)
{
	return myMetaRoomID;
}



// ---------------------------------------------------------------------------
// Function:	GetMetaRoomDefaultCameraLocation
// Description:	Gets a meta-room's default camera location
// Arguments:	metaRoomID - ID of the meta-room (in)
//				positionX/Y - position of default location (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::GetMetaRoomDefaultCameraLocation
	(const int metaRoomID, long& positionX, long& positionY)
{	
	MetaRoom* metaRoom;
	bool ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	positionX = FastFloatToInteger(metaRoom->positionDefault.x);
	positionY = FastFloatToInteger(metaRoom->positionDefault.y);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	SetMetaRoomDefaultCameraLocation
// Description:	Sets a meta-room's default camera location
// Arguments:	metaRoomID - ID of the meta-room (in)
//				position - position of default location (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::SetMetaRoomDefaultCameraLocation
	(const int metaRoomID, const Position& position)
{
	MetaRoom* metaRoom;
	bool ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	metaRoom->positionDefault = Vector2D((int)position.GetX(), (int)position.GetY());
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetDoorCollection
// Description:	Gets the map's list of doors
// Arguments:	None
// Returns:		A reference to the internal door collection
// ---------------------------------------------------------------------------
DoorCollection& Map::GetDoorCollection(void)
{
	return myDoorCollection;
}


// ---------------------------------------------------------------------------
// Function:	SetIndexBases
// Description:	Sets the base IDs for meta-rooms and rooms
// Arguments:	metaRoomBase - meta-room base ID (in)
//				roomBase - room base ID (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::SetIndexBases(const int metaRoomBase, const int roomBase)
{
	if ((metaRoomBase < MAX_META_ROOMS) && (roomBase < MAX_ROOMS))
	{
		myMetaRoomIndexBase = metaRoomBase;
		myRoomIndexBase = roomBase;
		return true;
	}
	else
		return false;
}


// ---------------------------------------------------------------------------
// Function:	CalculateRoomPermiabilityAndDoorage
// Description:	Calculates the minimum permiability and total doorage of a 
//					particular room
// Arguments:	room - room to perform calculation on (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateRoomPermiabilityAndDoorage(Room* room)
{
	int permiability = INT_MAX;
	int perm;
	DoorIterator doorIterator = room->doorCollection.begin();	
	int count = room->doorCollection.size(); 
	Door* door;

	// We now need to recalculate the total doorage for the room
	// in case it has changed

	room->caTotalDoorage = 0;
	while (count-- > 0)
	{
		door = *(doorIterator++);
		perm = door->permiability;
		if (perm < permiability)
			permiability = perm;

		CalculateDoorDoorage(*door);

		if( door->parent1 == room->roomID )
			room->caTotalDoorage += door->doorage1;
		else
			room->caTotalDoorage += door->doorage2;
	}

	count = myLinkCollection.size(); 
	Link* link;
	LinkIterator linkIterator = myLinkCollection.begin();	

	while (count-- > 0 )
	{
		link = *(linkIterator++);
		if ( link->parent1 == room->roomID || link->parent2 ==  room->roomID )
		{
			CalculateDoorDoorage( *link );
			if( link->parent1 == room->roomID )
				room->caTotalDoorage += link->doorage1;
			else
				room->caTotalDoorage += link->doorage2;
		}
	}
	room->permiability = permiability;
}


// ---------------------------------------------------------------------------
// Function:	CalculateFloorsForRoom
// Description:	Calculates which of a room's doors are floors
// Arguments:	room - room to perform calculation on (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateFloorsForRoom(Room* room)
{
	DoorIterator doorIterator;
	Door* door;
	int roomID = room->roomID;

	room->floorCollection.clear();
	for(doorIterator = room->doorCollection.begin();
		 doorIterator != room->doorCollection.end();
		 ++doorIterator)
	{
		door = *doorIterator;
		if ((door->doorType == DOOR_CEILING_FLOOR) &&
			(door->parent1 == roomID))
			room->floorCollection.push_back(door);
	}
}


// ---------------------------------------------------------------------------
// Function:	CalculateCeilingsForRoom
// Description:	Calculates which of a room's doors are ceilings
// Arguments:	room - room to perform calculation on (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateCeilingsForRoom(Room* room)
{
	DoorIterator doorIterator;
	Door* door;
	int roomID = room->roomID;

	room->ceilingCollection.clear();
	for(doorIterator = room->doorCollection.begin();
		 doorIterator != room->doorCollection.end();
		 ++doorIterator)
	{
		door = *doorIterator;
		if ((door->doorType == DOOR_CEILING_FLOOR) &&
			(door->parent2 == roomID))
			room->ceilingCollection.push_back(door);
	}
}




// ---------------------------------------------------------------------------
// Function:	CalculateLeftWallsForRoom
// Description:	Calculates which of a room's doors are left walls
// Arguments:	room - room to perform calculation on (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateLeftWallsForRoom(Room* room)
{
	DoorIterator doorIterator;
	Door* door;
	int roomID = room->roomID;

	room->leftCollection.clear();
	for(doorIterator = room->doorCollection.begin();
		 doorIterator != room->doorCollection.end();
		 ++doorIterator)
	{
		door = *doorIterator;
		if ((door->doorType == DOOR_LEFT_RIGHT) &&
			(door->parent2 == roomID))
			room->leftCollection.push_back(door);
	}
}


// ---------------------------------------------------------------------------
// Function:	CalculateRightWallsForRoom
// Description:	Calculates which of a room's doors are right walls
// Arguments:	room - room to perform calculation on (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateRightWallsForRoom(Room* room)
{
	DoorIterator doorIterator;
	Door* door;
	int roomID = room->roomID;

	room->rightCollection.clear();
	for(doorIterator = room->doorCollection.begin();
		 doorIterator != room->doorCollection.end();
		 ++doorIterator)
	{
		door = *doorIterator;
		if ((door->doorType == DOOR_LEFT_RIGHT) &&
			(door->parent1 == roomID))
			room->rightCollection.push_back(door);
	}
}


// ---------------------------------------------------------------------------
// Function:	CalculateNeighbourInformationForRoom
// Description:	Calculates the left and right floor neighbours and the left 
//					and right navigable doors for a given room
// Arguments:	room - room to perform calculation on (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::CalculateNeighbourInformationForRoom(Room* room)
{
	float maxLeftY = -FLT_MAX;
	float maxRightY = -FLT_MAX;
	DoorIterator doorIterator;
	Door* door;
	int roomID = room->roomID;
	int roomIDLeft = -1;
	int roomIDRight = -1;
	room->leftNavigableDoor = NULL;
	room->rightNavigableDoor = NULL;

	float y;

	for( doorIterator = room->doorCollection.begin();
		 doorIterator != room->doorCollection.end();
		 ++doorIterator )
	{
		door = *doorIterator;
		if ((door->parentCount != 2) ||
			(door->doorType != DOOR_LEFT_RIGHT))
			continue;

		y = door->start.y;

		if ((door->parent1 == roomID) && (y > maxRightY))
		{
			maxRightY = y;
			roomIDRight = door->parent2;
		}
		if ((door->parent2 == roomID) && (y > maxLeftY))
		{
			maxLeftY = y;
			roomIDLeft = door->parent1;
		}

		if ((door->parent1 == roomID) && room->endFloor == 
			myRoomCollection[ door->parent2 ]->startFloor)
			room->rightNavigableDoor = door;
		else if ((door->parent2 == roomID) && room->startFloor ==
			myRoomCollection[ door->parent1 ]->endFloor)
			room->leftNavigableDoor = door;
	}

	room->leftFloorRoomID = roomIDLeft;
	room->rightFloorRoomID = roomIDRight;
}



// ---------------------------------------------------------------------------
// Function:	AddRoom
// Description:	Adds a new room to the map
// Arguments:	metaRoomID - ID of the meta-room (in)
//				xLeft - x coordinate of left wall (in)
//				xRight - x coordinate of right wall (in)
//				yLeftCeiling - y coordinate of left hand ceiling (in)
//				yRightCeiling - y coordinate of right hand ceiling (in)
//				yLeftFloor - y coordinate of left hand floor (in)
//				yRightFloor - y coordinate of right hand floor (in)
//				roomID - ID of the new room (out)
// Returns:		true if good input, false otherwise		
// ---------------------------------------------------------------------------
bool Map::AddRoom
	(const int metaRoomID, const int xLeft, const int xRight,
	 const int yLeftCeiling, const int yRightCeiling, 
	 const int yLeftFloor, const int yRightFloor, int & roomID)
{
	int id, i;
	bool slotFound;
	IntegerIterator integerIterator;
	IntegerIterator integerIteratorEnd;
	DoorIterator doorIterator;
	Room* room;
	float yMin, yMax;
	bool ok;
	MetaRoom* metaRoom;
	Vector2D startCeiling(xLeft, yLeftCeiling);
	Vector2D endCeiling(xRight, yRightCeiling);
	Vector2D startFloor(xLeft, yLeftFloor);
	Vector2D endFloor(xRight, yRightFloor);
	Vector2D deltaFloor = endFloor - startFloor;
	Vector2D deltaCeiling = endCeiling - startCeiling;

	// Check that the meta-room exists
	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	// Is there an unused ID slot?
	slotFound = false;
	if( myMaxRoomID < myRoomIndexBase - 1) myMaxRoomID = myRoomIndexBase - 1;
	for (i=myRoomIndexBase; i<myMaxRoomID; ++i) 
	{
		if (myRoomCollection[i] == NULL) 
		{
			slotFound = true;
			break;
		}
	}
	if (slotFound) 
	{
		id = i;
	}
	else 
	{
		++myMaxRoomID;
		id = myMaxRoomID;
	}
	// Create a new room
	room = new Room;
	room->roomID = id;
	room->metaRoomID = metaRoomID;
	// Add the left hand wall
	AddRoomEdge(metaRoom->roomIDCollection, room, startCeiling, startFloor, 
		DIRECTION_LEFT);
	// Add the right hand wall
	AddRoomEdge(metaRoom->roomIDCollection, room, endCeiling, endFloor, 
		DIRECTION_RIGHT);
	// Add the ceiling 
	AddRoomEdge(metaRoom->roomIDCollection, room, startCeiling, endCeiling, 
		DIRECTION_CEILING);
	room->startCeiling = startCeiling;
	room->endCeiling = endCeiling;
	room->deltaCeiling = deltaCeiling;
	// Add the floor
	AddRoomEdge(metaRoom->roomIDCollection, room, startFloor, endFloor, 
		DIRECTION_FLOOR);
	room->startFloor = startFloor;
	room->endFloor = endFloor;
	room->deltaFloor = deltaFloor;
	// Work out the centre point of the room
	room->centre.x = (startCeiling.x + endCeiling.x) / 2.0f;
	room->centre.y = 
		(startCeiling.y + endCeiling.y + startFloor.y + endFloor.y) / 4.0f;
	// Get the extents of the room
	if (startCeiling.y < endCeiling.y)
		yMin = startCeiling.y;
	else
		yMin = endCeiling.y;
	if (startFloor.y > endFloor.y)
		yMax = startFloor.y;
	else
		yMax = endFloor.y;
	// Store the extents
	room->positionMin = Vector2D(startFloor.x, yMin);
	room->positionMax = Vector2D(endFloor.x, yMax);
	// Copy the newly built doors into the real door collection
	for (doorIterator=myConstructedDoorCollection.begin(); 
		 doorIterator!=myConstructedDoorCollection.end(); ++doorIterator) 
	{
		myDoorCollection.push_back(*doorIterator);
	}

	// Empty the temporary door collection
	myConstructedDoorCollection.clear();
	// Store the new room in the room collection
	myRoomCollection[id] = room;
	++myRoomCount;
	// Store the new ID in the room ID list
	myRoomIDCollection.push_back(id);
	// Store the new ID in the meta-room's room ID list
	metaRoom->roomIDCollection.push_back(id);
	// Add the room to the internal grid
	AddRoomToGrid(room);
	// Pass back the new room ID to the caller
	roomID = id;

	// Initialize room's CA attributes
	room->type = 0;
	for (i = 0; i < CA_PROPERTY_COUNT; ++i)
	{
		room->caValues[i] = 0;
		room->caOldValues[i] = 0;
		room->caOlderValues[i] = 0;
	}
	room->caTempValue = 0;
	room->caTotalDoorage = 0;
	room->caInput = 0;
	room->perimeterLength = room->deltaFloor.Length() +
							room->deltaCeiling.Length() +
							( yLeftFloor - yLeftCeiling ) +
							( yRightFloor - yRightCeiling );

	CalculateRoomPermiabilityAndDoorage(room);
	CalculateNeighbourInformationForRoom(room);
	CalculateFloorsForRoom(room);
	CalculateCeilingsForRoom(room);
	CalculateLeftWallsForRoom(room);
	CalculateRightWallsForRoom(room);


	IntegerCollection& upNeigbours = room->neighbourIDCollections[DIRECTION_UP];
	IntegerCollection& downNeigbours = room->neighbourIDCollections[DIRECTION_DOWN];
	IntegerCollection& leftNeigbours = room->neighbourIDCollections[DIRECTION_LEFT];
	IntegerCollection& rightNeigbours = room->neighbourIDCollections[DIRECTION_RIGHT];

	integerIterator = upNeigbours.begin();
	integerIteratorEnd = upNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateFloorsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}

	integerIterator = downNeigbours.begin();
	integerIteratorEnd = downNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateCeilingsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}

	integerIterator = leftNeigbours.begin();
	integerIteratorEnd = leftNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateNeighbourInformationForRoom(room);
		CalculateRightWallsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}

	integerIterator = rightNeigbours.begin();
	integerIteratorEnd = rightNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateNeighbourInformationForRoom(room);
		CalculateLeftWallsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}

	return true;
}


// ---------------------------------------------------------------------------
// Function:	RemoveRoom
// Description:	Removes an existing room from the map
// Arguments:	roomID - ID of the room to remove (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::RemoveRoom
	(const int roomID)
{
	int i;
	DoorIterator doorIterator, doorIteratorEnd;
	DoorIterator doorToDelete;
	IntegerIterator neighbourToDelete;
	Door* door;
	int otherRoomID;
	Room* otherRoom;	
	Room* room;
	bool ok;
	MetaRoom* metaRoom;
	IntegerIterator integerIterator;
	IntegerIterator integerIteratorEnd;
	int id;

	// Check that the room exists
	ok = GetValidRoomPointer(roomID, room);
	if (!ok)
		return false;

	IntegerCollection& upNeigbours = room->neighbourIDCollections[DIRECTION_UP];
	IntegerCollection& downNeigbours = room->neighbourIDCollections[DIRECTION_DOWN];
	IntegerCollection& leftNeigbours = room->neighbourIDCollections[DIRECTION_LEFT];
	IntegerCollection& rightNeigbours = room->neighbourIDCollections[DIRECTION_RIGHT];

	// Get the parent meta-room
	metaRoom = myMetaRoomCollection[room->metaRoomID];
	doorIterator = room->doorCollection.begin();
	doorIteratorEnd = room->doorCollection.end();
	while (doorIterator != doorIteratorEnd) 
	{
		door = *doorIterator;
		myNavigableDoorCollection.remove(door);		

		if (door->parentCount == 1) 
		{
			// Destroy the door
			doorToDelete = std::find(myDoorCollection.begin(),
				myDoorCollection.end(), door);
			myDoorCollection.erase(doorToDelete);		
			delete door;
		}
		else 
		{
			// Get the room ID that's sharing this door
			otherRoomID = GetAlternativeParent(door, roomID);
			// Get a pointer to the room that's sharing this door
			otherRoom = myRoomCollection[otherRoomID];
			// Make the door unshared
			door->parentCount = 1;
			if (door->parent1 == roomID) 
			{
				door->parent1 = -1;
				door->parent2 = otherRoomID;
			}
			else 
			{
				door->parent1 = otherRoomID;
				door->parent2 = -1;
			}
			door->permiability = IMPERMIABLE;
			// Delete the room ID from the sharing room's neighbour list
			neighbourToDelete = std::find
				(otherRoom->neighbourIDCollection.begin(),
				 otherRoom->neighbourIDCollection.end(),
				 roomID);
			otherRoom->neighbourIDCollection.erase(neighbourToDelete);
			for (i=0; i<4; ++i) 
			{
				neighbourToDelete = std::find
					(otherRoom->neighbourIDCollections[i].begin(),
					otherRoom->neighbourIDCollections[i].end(),
					roomID);
				if (neighbourToDelete != otherRoom->neighbourIDCollections[i].end()) 
					break;
			}
			otherRoom->neighbourIDCollections[i].erase(neighbourToDelete);
			AmalgamateDoors(otherRoom, door);
		}
		++doorIterator;
	}
	// Remove the room from the room collection
	myRoomCollection[roomID] = NULL;
	myRoomCount--;
	// Erase the room ID from the set of room IDs
	myRoomIDCollection.erase
		(std::find(myRoomIDCollection.begin(),
		myRoomIDCollection.end(), 
		roomID));
	// Erase the room ID from the meta-room's set of room IDs
	metaRoom->roomIDCollection.erase
		(std::find(metaRoom->roomIDCollection.begin(),
		metaRoom->roomIDCollection.end(), roomID));
	if (myRoomCount == 0) 
	{
		myMaxRoomID = -1;
	}
	else if (roomID == myMaxRoomID) 
	{
		// Scan backwards to find the highest used ID
		for (i=roomID-1; i>=0; i--) 
		{
			if (myRoomCollection[i] != NULL) 
			{
				myMaxRoomID = i;
				break;
			}
		}
	}
	// Remove the room from the internal grid
	RemoveRoomFromGrid(room);
	// Destroy the room
	delete room;

	integerIterator = upNeigbours.begin();
	integerIteratorEnd = upNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateFloorsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}

	integerIterator = downNeigbours.begin();
	integerIteratorEnd = downNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateCeilingsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}

	integerIterator = leftNeigbours.begin();
	integerIteratorEnd = leftNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateNeighbourInformationForRoom(room);
		CalculateRightWallsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}

	integerIterator = rightNeigbours.begin();
	integerIteratorEnd = rightNeigbours.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		room = myRoomCollection[id];
		CalculateNeighbourInformationForRoom(room);
		CalculateLeftWallsForRoom(room);
		CalculateRoomPermiabilityAndDoorage(room);
		++integerIterator;
	}
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetMetaRoomIDCollection
// Description:	Gets the current set of meta-room IDs
// Arguments:	metaRoomIDCollection - the set of IDs (out)
// Returns:		Number of meta-rooms
// ---------------------------------------------------------------------------
int Map::GetMetaRoomIDCollection(IntegerCollection & metaRoomIDCollection)
{
	metaRoomIDCollection = myMetaRoomIDCollection;
	return myMetaRoomCount;
}


// ---------------------------------------------------------------------------
// Function:	GetRoomIDCollection
// Description:	Gets the current set of room IDs
// Arguments:	roomIDCollection - the set of IDs (out)
// Returns:		Number of rooms
// ---------------------------------------------------------------------------
int Map::GetRoomIDCollection(IntegerCollection & roomIDCollection)
{
	roomIDCollection = myRoomIDCollection;
	return myRoomCount;
}


// ---------------------------------------------------------------------------
// Function:	GetRoomIDCollectionForMetaRoom
// Description:	Gets the set of room IDs that are inside a given meta-room
// Arguments:	metaRoomID - meta-room ID (in)
//				roomIDCollection - the set of IDs (out)
//				roomCount - the number of room IDs (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::GetRoomIDCollectionForMetaRoom
	(const int metaRoomID, IntegerCollection & roomIDCollection, 
	 int & roomCount)
{
	MetaRoom* metaRoom;
	bool ok;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	roomIDCollection = metaRoom->roomIDCollection;
	roomCount = metaRoom->roomIDCollection.size();
	return true;
}


// ---------------------------------------------------------------------------
// Function:	IsMetaRoomIDValid
// Description:	Determines if a meta-room ID is valid
// Arguments:	metaRoomID - ID of the meta-room (in)
// Returns:		true if valid; false otherwise
// ---------------------------------------------------------------------------
bool Map::IsMetaRoomIDValid
	(const int metaRoomID)
{
	MetaRoom* dummy;
	return GetValidMetaRoomPointer(metaRoomID, dummy);
}


// ---------------------------------------------------------------------------
// Function:	IsRoomIDValid
// Description:	Determines if a room ID is valid
// Arguments:	roomID - ID of the room (in)
// Returns:		true if valid; false otherwise
// ---------------------------------------------------------------------------
bool Map::IsRoomIDValid
	(const int roomID)
{
	Room* dummy;
	return GetValidRoomPointer(roomID, dummy);
}


// ---------------------------------------------------------------------------
// Function:	GetMetaRoomIDForPoint
// Description:	Gets the meta-room ID that contains a point
// Arguments:	position - coordinates of point (in)
//				metaRoomID - ID of the meta-room, -1 if none (out) 
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::GetMetaRoomIDForPoint
	(const Vector2D & position, int & metaRoomID)
{
	int id;
	MetaRoom* metaRoom;
	IntegerIterator integerIterator;
	IntegerIterator integerIteratorEnd;

	// Check that position to test lies on the map
	if ((position.x < 0.0f) || (position.x >= myMapWidth))
		return false;
	if ((position.y < 0.0f) || (position.y >= myMapHeight))
		return false;
	integerIterator = myMetaRoomIDCollection.begin();
	integerIteratorEnd = myMetaRoomIDCollection.end();
	while (integerIterator != integerIteratorEnd) 
	{
		id = *integerIterator;
		metaRoom = myMetaRoomCollection[id];
		if (position.x < metaRoom->positionMin.x) 
		{
			++integerIterator;
			continue;
		}
		if (position.y < metaRoom->positionMin.y) 
		{
			++integerIterator;
			continue;
		}
		if (position.x > metaRoom->positionMax.x) 
		{
			++integerIterator;
			continue;
		}
		if (position.y > metaRoom->positionMax.y) 
		{
			++integerIterator;
			continue;
		}
		metaRoomID = id;
		return true;
	}
	metaRoomID = -1;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetRoomIDForPoint
// Description:	Gets the room ID that contains an arbitrary point
// Arguments:	position - coordinates of point (in)
//				roomID - ID of the room
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::GetRoomIDForPoint
	(const Vector2D & position, int & roomID)
{
	int dummy1[4];
	Room* dummy2[4];
	int dummy3;
	bool ok;

	// Check that position to test lies on the map
	if ((position.x < 0.0f) || (position.x >= myMapWidth))
		return false;
	if ((position.y < 0.0f) || (position.y >= myMapHeight))
		return false;
	ok = GetRoomInformationForPoint(position, dummy1, dummy2, dummy3, 
		roomID);
	if (!ok) 
	{
		return false;
	}
	else 
	{
		return true;
	}
}


// ---------------------------------------------------------------------------
// Function:	SetMapDimensions
// Description:	Sets the dimensions of the entire map
// Arguments:	width - new width of the map (in)
//				height - new height of the map (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::SetMapDimensions
	(const int width, const int height)
{	
	float w = width;
	float h = height;
	// Check the dimensions
	if ((w <= 1.0f) || (h <= 1.0f))
		return false;
	// Check that the dimensions are not less than the most extreme coordinates
	// of all the meta-rooms
	if ((w <= myMetaRoomMaxCoordinates.x) || 
		(h <= myMetaRoomMaxCoordinates.y))
		return false;
	myMapWidth = w;
	myMapHeight = h;
	BuildCache();
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetMapDimensions
// Description:	Gets the dimensions of the entire map
// Arguments:	width - width of the map (out)
//				height - height of the map (out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::GetMapDimensions
	(int & width, int & height)
{
	width = FastFloatToInteger(myMapWidth);
	height = FastFloatToInteger(myMapHeight);
}


// ---------------------------------------------------------------------------
// Function:	SetGridSize
// Description:	Sets the size of internal grid squares
// Arguments:	gridSquareSize - size of each grid square (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::SetGridSize(const int gridSquareSize)
{
	float gss = gridSquareSize;

	// Check the grid size
	if (gss <= 1.0f)
		return false;
	myGridSquareSize = gss;
	BuildCache();
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetGridSize
// Description:	Gets the size of internal grid squares
// Arguments:	None
// Returns:		Size of grid square
// ---------------------------------------------------------------------------
int Map::GetGridSize(void)
{
	return FastFloatToInteger(myGridSquareSize);
}


// ---------------------------------------------------------------------------
// Function:	GetMetaRoomLocation
// Description:	Gets the location of a particular meta-room
// Arguments:	metaRoomID - ID of the meta-room (in)
//				positionX - top-left x corner (out)
//				positionY - top-left y corner (out)
//				width - width of the meta-room (out)
//				height - height of the meta-room (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::GetMetaRoomLocation
	(const int metaRoomID, int & positionX, int & positionY, int & width, 
	 int & height)
{	
	MetaRoom* metaRoom;
	bool ok;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	positionX = FastFloatToInteger(metaRoom->positionMin.x);
	positionY = FastFloatToInteger(metaRoom->positionMin.y);
	width = FastFloatToInteger(metaRoom->width);
	height = FastFloatToInteger(metaRoom->height);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetRoomLocation
// Description: Gets the location of a particular room
// Arguments:	roomID - ID of the room (in)
//				xLeft - x coordinate of left wall (out)
//				xRight - x coordinate of right wall (out)
//				yLeftCeiling - y coordinate of left hand ceiling (out)
//				yRightCeiling - y coordinate of right hand ceiling (out)
//				yLeftFloor - y coordinate of left hand floor (out)
//				yRightFloor - y coordinate of right hand floor (out)
// Returns:		true if good input, false otherwise		
// ---------------------------------------------------------------------------
bool Map::GetRoomLocation
	(const int roomID, int & metaRoomID, int & xLeft, int & xRight,
	 int & yLeftCeiling, int & yRightCeiling, int & yLeftFloor, 
	 int & yRightFloor)
{
	bool ok;
	Room* room;

	// Check that the room exists
	ok = GetValidRoomPointer(roomID, room);
	if (!ok)
		return false;
	xLeft = FastFloatToInteger(room->startCeiling.x);
	xRight = FastFloatToInteger(room->endCeiling.x);
	yLeftCeiling = FastFloatToInteger(room->startCeiling.y);
	yRightCeiling = FastFloatToInteger(room->endCeiling.y);
	yLeftFloor = FastFloatToInteger(room->startFloor.y);
	yRightFloor = FastFloatToInteger(room->endFloor.y);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	SetBackgroundCollection
// Description:	Sets the background collection for a given meta-room
// Arguments:	metaRoomID - meta-room ID (in)
//				backgroundCollection - the background collection (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::SetBackgroundCollection
	(const int metaRoomID, StringCollection & backgroundCollection)
{
	MetaRoom* metaRoom;
	bool ok;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	metaRoom->backgroundCollection.clear();
	metaRoom->backgroundCollection = backgroundCollection;
	metaRoom->background = *(metaRoom->backgroundCollection.begin());
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetBackgroundCollection
// Description:	Gets the background collection for a given meta-room
// Arguments:	metaRoomID - meta-room ID (in)
//				backgroundCollection - the background collection (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::GetBackgroundCollection
	(const int metaRoomID, StringCollection & backgroundCollection)
{
	MetaRoom* metaRoom;
	bool ok;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	backgroundCollection = metaRoom->backgroundCollection;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	AddBackground
// Description:	Adds a new background to a given meta-room
// Arguments:	metaRoomID - meta-room ID (in)
//				background - the background filename (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::AddBackground
	(const int metaRoomID, std::string background)
{
	MetaRoom* metaRoom;
	bool ok;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	metaRoom->backgroundCollection.insert(background);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	RemoveBackground
// Description:	Removes a background from a given meta-room
// Arguments:	metaRoomID - meta-room ID (in)
//				background - the background filename (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::RemoveBackground
	(const int metaRoomID, std::string background)
{
	MetaRoom* metaRoom;
	StringIterator backgroundToRemove;

	bool ok;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	backgroundToRemove = metaRoom->backgroundCollection.find(background);
	if (backgroundToRemove == metaRoom->backgroundCollection.end())
		return false;
	if (background == metaRoom->background)
		return false;
	metaRoom->backgroundCollection.erase(backgroundToRemove);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	SetCurrentBackground
// Description:	Sets a meta-room's current background
// Arguments:	metaRoomID - meta-room ID (in)
//				background - the background filename (in)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::SetCurrentBackground
	(const int metaRoomID, std::string background)
{
	MetaRoom* metaRoom;
	bool ok;
	StringIterator backgroundIterator;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	backgroundIterator = metaRoom->backgroundCollection.find(background);
	if (backgroundIterator == metaRoom->backgroundCollection.end())
		return false;
	metaRoom->background = background;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetCurrentBackground
// Description:	Gets a meta-room's current background
// Arguments:	metaRoomID - meta-room ID (in)
//				background - the background filename (out)
// Returns:		true if good input, false otherwise
// ---------------------------------------------------------------------------
bool Map::GetCurrentBackground
	(const int metaRoomID, std::string & background)
{
	MetaRoom* metaRoom;
	bool ok;

	ok = GetValidMetaRoomPointer(metaRoomID, metaRoom);
	if (!ok)
		return false;
	background = metaRoom->background;
	return true;
}


// --------------------------------------------------------------------------
// Function:	GetNeighbours
// Description:	Gets a room's connected neighbours
// Arguments:	roomID - ID of room (in)
//				direction - which wall(s) to get neighbours for, chosen 
//					from DIRECTION_LEFT, DIRECTION_RIGHT, DIRECTION_CEILING,
//					DIRECTION_FLOOR or DIRECTION_ALL (in)
//				neighbourIDCollection - neighbour IDs (out)
// Returns:		true if good input, false otherwise
// --------------------------------------------------------------------------
bool Map::GetNeighbours
	(const int roomID, const int direction, 
	 IntegerCollection & neighbourIDCollection)
{
	Room* room;
	bool ok;

	ok = GetValidRoomPointer(roomID, room);
	if (!ok)
		return false;
	if (direction == DIRECTION_ALL)
		neighbourIDCollection = room->neighbourIDCollection;
	else
		neighbourIDCollection = room->neighbourIDCollections[direction];
	return true;
}


// ---------------------------------------------------------------------------
// Function:	SetDoorPermiability
// Description:	Sets the permiability of a door between two rooms
// Arguments:	roomID1 - ID of first room (in)
//				roomID2 - ID of second room (in)
//				permiability - new permiability of the door (in)
// Returns:		true if good input, false otherwise
// --------------------------------------------------------------------------
bool Map::SetDoorPermiability(const int roomID1, const int roomID2, 
	const int permiability)
{
	Door* door;
	bool ok;
	
	ok = GetDoorBetweenRooms(roomID1, roomID2, door);
	if (!ok)
		return false;
	if ((permiability < IMPERMIABLE) || (permiability > PERMIABLE))
		return false;
	door->permiability = permiability;

	CalculateRoomPermiabilityAndDoorage(myRoomCollection[roomID1]);
	CalculateRoomPermiabilityAndDoorage(myRoomCollection[roomID2]);
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetDoorPermiability
// Description:	Gets the permiability of a door between two rooms
// Arguments:	roomID1 - ID of first room (in)
//				roomID2 - ID of second room (in)
//				permiability - permiability of the door (out)
// Returns:		true if good input, false otherwise
// --------------------------------------------------------------------------
bool Map::GetDoorPermiability(const int roomID1, const int roomID2,
	int & permiability)
{
	Door* door;
	bool ok;
	
	ok = GetDoorBetweenRooms(roomID1, roomID2, door);
	if (!ok)
		return false;
	permiability = door->permiability;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetValidMetaRoomPointer
// Description:	Safely gets a meta-room pointer given an ID
// Arguments:	metaRoomID - ID of the meta-room (in)
//				metaRoom - pointer to the meta-room (out)
// Returns:		true if good input, false otherwise		
// ---------------------------------------------------------------------------
bool Map::GetValidMetaRoomPointer
	(const int metaRoomID, MetaRoom*& metaRoom) 
{
	// Test if the meta-room ID is in range
	if ((metaRoomID < 0) || (metaRoomID >= MAX_META_ROOMS))
		return false;
	// Get the meta-room
	metaRoom = myMetaRoomCollection[metaRoomID];
	if (metaRoom == NULL)
		return false;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	GetValidRoomPointer
// Description:	Safely gets a room pointer given an ID
// Arguments:	roomID - ID of the room (in)
//				room - pointer to the room (out)
// Returns:		true if good input, false otherwise		
// ---------------------------------------------------------------------------
bool Map::GetValidRoomPointer(const int roomID, Room*& room) 
{
	// Test if the room ID is in range
	if ((roomID < 0) || (roomID >= MAX_ROOMS))
		return false;
	// Get the room
	room = myRoomCollection[roomID];
	if (room == NULL)
		return false;
	return true;
}


// ---------------------------------------------------------------------------
// Function:	AddRoomToGrid
// Description:	Adds a room to the internal grid
// Arguments:	room - room pointer of room to be added (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::AddRoomToGrid(Room* room)
{	
	int squareX1, squareY1;
	int squareX2, squareY2;
	int roomID;
	int x, y;
	int index;

	// Get top-left grid square
	GetGridIndexesForPoint(room->positionMin, squareX1, squareY1); 
	// Get bottom-right grid square
	GetGridIndexesForPoint(room->positionMax, squareX2, squareY2);

	roomID = room->roomID;
	// Add this ID to all grid squares in this range
	for (x=squareX1; x<=squareX2; ++x) 
	{
		for (y=squareY1; y<=squareY2; ++y) 
		{
			index = y*mySquareCountX + x;
			myIntegerCollectionGrid[index].push_back(roomID);
		}
	}
}


// ---------------------------------------------------------------------------
// Function:	RemoveRoomFromGrid
// Description:	Removes a room from the internal grid
// Arguments:	room - room pointer of room to be removed (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::RemoveRoomFromGrid(Room* room)
{
	int squareX1, squareY1;
	int squareX2, squareY2;
	int roomID;
	int x, y;
	int index;

	// Get top-left grid square
	GetGridIndexesForPoint(room->positionMin, squareX1, squareY1); 
	// Get bottom-right grid square
	GetGridIndexesForPoint(room->positionMax, squareX2, squareY2);

	roomID = room->roomID;
	// Remove this ID from all grid squares in this range
	for (x=squareX1; x<=squareX2; ++x) 
	{
		for (y=squareY1; y<=squareY2; ++y) 
		{
			index = y*mySquareCountX + x; 
			myIntegerCollectionGrid[index].erase
				(std::find(myIntegerCollectionGrid[index].begin(),
				myIntegerCollectionGrid[index].end(),
				roomID));
		}
	}
}


// ---------------------------------------------------------------------------
// Function:	BuildCache
// Description:	Builds the internal cache
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
void Map::BuildCache(void)
{
	int i;
	float unit;
	Room* room;

	// Delete the old grid if there was one
	if (myIntegerCollectionGrid != NULL)
		delete []myIntegerCollectionGrid;
	unit = myGridSquareSize - 1.0f;
	mySquareCountX = FastFloor((myMapWidth+unit) / myGridSquareSize);
	mySquareCountY = FastFloor((myMapHeight+unit) / myGridSquareSize);
	myIntegerCollectionGrid = new IntegerCollection[mySquareCountX * mySquareCountY];

	for (i=0; i<=myMaxRoomID; ++i) 
	{
		room = myRoomCollection[i];
		if (room != NULL)
			AddRoomToGrid(room);
	}
}


// ---------------------------------------------------------------------------
// Function:	GetAlternativeParent
// Description:	Gets the parent ID that is sharing a door with a known ID
// Arguments:	door - door pointer (in)
//				knownID - known parent ID (in)
// Returns:		The ID of the alternative parent
// ---------------------------------------------------------------------------
int Map::GetAlternativeParent(Door* door, const int knownID)
{
	_ASSERT(door != NULL);
	_ASSERT(door->parentCount == 2);

	if (door->parent1 == knownID)
		return door->parent2;
	else 
		return door->parent1;
}


// ---------------------------------------------------------------------------
// Function:	GetOverlapInformation
// Description:	Determines how an existing door overlaps with a room edge
//				startEdge - start of the room edge (in)
//				endEdge - end of the room edge (in)
//				deltaEdge - delta of the room edge (in)
//				overlapInformation - info on overlapping sections (out)
//				sharedIndex - which of the overlapping sections represents
//					a shared section (out)
// Returns:		Number of overlapping sections
// ---------------------------------------------------------------------------
int Map::GetOverlapInformation
	(Door* door, const Vector2D & startEdge, const Vector2D & endEdge,
	 const Vector2D & deltaEdge, OverlapInformation overlapInformation[3], 
	 int & sharedIndex)
{
	int sections;
	bool doorStartOnEdge, doorEndOnEdge;
	bool edgeStartOnDoor, edgeEndOnDoor;
	bool doorStartEdgeStartSame, doorEndEdgeEndSame;
	bool doorStartEdgeEndSame, doorEndEdgeStartSame;
	Vector2D startDoor, endDoor, deltaDoor;

	startDoor = door->start;
	endDoor = door->end;
	deltaDoor = door->delta;

	doorStartEdgeStartSame = (startDoor == startEdge);
	doorEndEdgeEndSame = (endDoor == endEdge);
	doorStartEdgeEndSame = (startDoor == endEdge);
	doorEndEdgeStartSame = (endDoor == startEdge);

	if (doorStartEdgeEndSame || doorEndEdgeStartSame)
		// Overlapping only at the vertices, so no overlapping sections
		return 0;

	edgeStartOnDoor = IsPointOnLine(startEdge, startDoor, endDoor, 
		deltaDoor, 1.0f);
	edgeEndOnDoor = IsPointOnLine(endEdge, startDoor, endDoor, 
		deltaDoor, 1.0f);
	doorStartOnEdge = IsPointOnLine(startDoor, startEdge, endEdge, 
		deltaEdge, 1.0f); 
	doorEndOnEdge = IsPointOnLine(endDoor, startEdge, endEdge, 
		deltaEdge, 1.0f);
		
	sections = 0;
	if (doorStartEdgeStartSame) 
	{
		if (doorEndEdgeEndSame) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = endDoor;
			overlapInformation[0].shared = true;
			sharedIndex = 0;
			sections = 1;
		}
		else if (edgeEndOnDoor) 
		{
			overlapInformation[0].start = startEdge;
			overlapInformation[0].end = endEdge;
			overlapInformation[0].shared = true;
			sharedIndex = 0;
			overlapInformation[1].start = endEdge;	
			overlapInformation[1].end = endDoor;
			overlapInformation[1].shared = false;
			sections = 2;
		}
		else if (doorEndOnEdge) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = endDoor;
			overlapInformation[0].shared = true;
			sharedIndex = 0;
			sections = 1;
		}
	}
	else if (edgeStartOnDoor) 
	{
		if (doorEndEdgeEndSame) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = startEdge;
			overlapInformation[0].shared = false;
			overlapInformation[1].start = startEdge;
			overlapInformation[1].end = endEdge;
			overlapInformation[1].shared = true;
			sharedIndex = 1;
			sections = 2;
		}
		else if (edgeEndOnDoor) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = startEdge;
			overlapInformation[0].shared = false;
			overlapInformation[1].start = startEdge;
			overlapInformation[1].end = endEdge;
			overlapInformation[1].shared = true;
			sharedIndex = 1;
			overlapInformation[2].start = endEdge;
			overlapInformation[2].end = endDoor;
			overlapInformation[2].shared = false;
			sections = 3;
		}
		else if (doorEndOnEdge) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = startEdge;
			overlapInformation[0].shared = false;
			overlapInformation[1].start = startEdge;
			overlapInformation[1].end = endDoor;
			overlapInformation[1].shared = true;
			sharedIndex = 1;
			sections = 2;
		}
	}
	else if (doorStartOnEdge) 
	{
		if (doorEndEdgeEndSame) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = endDoor;
			overlapInformation[0].shared = true;
			sharedIndex = 0;
			sections = 1;
		}
		else if (edgeEndOnDoor) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = endEdge;
			overlapInformation[0].shared = true;
			sharedIndex = 0;
			overlapInformation[1].start = endEdge;
			overlapInformation[1].end = endDoor;
			overlapInformation[1].shared = false;
			sections = 2;
		}
		else if (doorEndOnEdge) 
		{
			overlapInformation[0].start = startDoor;
			overlapInformation[0].end = endDoor;
			overlapInformation[0].shared = true;
			sharedIndex = 0;
			sections = 1;
		}
	}
	return sections;	
}	


// ---------------------------------------------------------------------------
// Function:	AmalgamateDoors
// Description:	Amalgamates co-linear doors into one big door when a room's
//				neighbour is removed from the map
// Arguments:	room - the room holding the doors to be amalgamated (in)
//				oldSharedDoor - the door that used to be shared between the 
//					room and its deleted neighbour (in/out)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::AmalgamateDoors(Room* room, Door* oldSharedDoor)
{
	DoorIterator doorIterator, tempIterator;
	DoorIterator doorIteratorEnd;
	int doorType;
	Door* door;
	Vector2D start, end;
	bool found;
	bool deleteDoor;
	int roomID;

	roomID = room->roomID;
	doorType = oldSharedDoor->doorType;
	start = oldSharedDoor->start;
	end = oldSharedDoor->end;
	found = false;
	doorIterator = room->doorCollection.begin();
	doorIteratorEnd = room->doorCollection.end();
	while (doorIterator != doorIteratorEnd) 
	{
		door = *doorIterator;
		if ((door == oldSharedDoor) || (door->doorType != doorType) || 
			(door->parentCount != 1)) 
		{
			++doorIterator;
			continue;
		}
		deleteDoor = false;
		if (doorType == DOOR_LEFT_RIGHT) 
		{
			if (door->end.y == start.y) 
			{
				deleteDoor = true;
				start = door->start;
			}
			if (door->start.y == end.y) 
			{
				deleteDoor = true;
				end = door->end;
			}
		}
		else if (doorType == DOOR_CEILING_FLOOR) 
		{
			if (door->end.x == start.x) 
			{
				deleteDoor = true;
				start = door->start;
			}
			if (door->start.x == end.x) 
			{
				deleteDoor = true;
				end = door->end;
			}	
		}
		if (deleteDoor) 
		{
			found = true;
			tempIterator = doorIterator;
			++doorIterator;
			room->doorCollection.erase(tempIterator);
			myDoorCollection.erase(std::find(myDoorCollection.begin(),
				myDoorCollection.end(), door));
			delete door;
		}
		else 
		{
			++doorIterator;
		}
	}
	if (found) 
	{
		// Reconfigure the old shared door
		oldSharedDoor->start = start;
		oldSharedDoor->end = end; 
		oldSharedDoor->delta = end - start;
		oldSharedDoor->start.x = (start.x);
		oldSharedDoor->start.y = (start.y);
		oldSharedDoor->end.x = (end.x);
		oldSharedDoor->end.y = (end.y);
	}
}



// ---------------------------------------------------------------------------
// Function:	AddRoomEdge
// Description: Adds a new room edge (eg the ceiling)
// Arguments:	siblingIDs - the other room IDs in the same meta-room (in)
//				room - the room to add the edge to (in)
//				start - start position of the edge (in)
//				end - end position of the edge (in)
// Returns:		None
// ---------------------------------------------------------------------------
void Map::AddRoomEdge
	(IntegerCollection & siblingIDs, Room* room, const Vector2D & start, 
	 const Vector2D & end, const int edgeType)
{
	Vector2D delta;
	bool overlap;
	bool found;
	Room* exposedRoom;
	Door* exposedDoor;
	Door* newDoor;
	int roomID, exposedRoomID;
	int sections, i;
	int sharedIndex;
	DoorIterator doorIterator;
	DoorIterator doorIteratorEnd;
	DoorIterator doorTemp;
	OverlapInformation overlapInformation[3];
	OverlapInformation overlapDoorInformation;
	OverlapInformation info, infoNext;
	OverlapCollection overlapCollection;
	OverlapIterator overlapIterator;
	OverlapIterator overlapIteratorEnd;
	OverlapIterator firstOverlap, lastOverlap;
	OverlapIterator nextOverlap;
	int edgeTypeOpposite;
	int doorType;
	IntegerIterator roomIDIterator;
	IntegerIterator roomIDIteratorEnd;
	DoorCollection exposedRoomDoorCollection;
	int parent1, parent2;
	bool create;

	// Work out the orientation of the door
	if (edgeType == DIRECTION_LEFT) 
	{
		doorType = DOOR_LEFT_RIGHT;
		edgeTypeOpposite = DIRECTION_RIGHT;
	}
	else if (edgeType == DIRECTION_RIGHT) 
	{
		doorType = DOOR_LEFT_RIGHT;
		edgeTypeOpposite = DIRECTION_LEFT;
	}
	else if (edgeType == DIRECTION_CEILING) 
	{
		doorType = DOOR_CEILING_FLOOR;
		edgeTypeOpposite = DIRECTION_FLOOR;
	}
	else 
	{
		doorType = DOOR_CEILING_FLOOR;
		edgeTypeOpposite = DIRECTION_CEILING;
	}

	delta = end-start;
	roomID = room->roomID;
	overlap = false;
	roomIDIterator = siblingIDs.begin();
	roomIDIteratorEnd = siblingIDs.end();
	// Loop though all the sibling rooms
	while (roomIDIterator != roomIDIteratorEnd) 
	{
		exposedRoomID = *roomIDIterator;
		exposedRoom = myRoomCollection[exposedRoomID];	
		exposedRoomDoorCollection = exposedRoom->doorCollection;
		doorIterator = exposedRoomDoorCollection.begin();
		doorIteratorEnd = exposedRoomDoorCollection.end();
		while (doorIterator != doorIteratorEnd) 
		{
			exposedDoor = *doorIterator;
			if (exposedDoor->parentCount != 1) 
			{
				++doorIterator;
				continue;
			}
			sections = GetOverlapInformation(exposedDoor, start, end, delta, 
				overlapInformation, sharedIndex);
			if (sections == 0) 
			{
				++doorIterator;
				continue;
			}		
			overlap = true;
			overlapDoorInformation = overlapInformation[sharedIndex];
			found = false;
			overlapIterator = overlapCollection.begin();
			overlapIteratorEnd = overlapCollection.end();
			// Keep the overlapping doors ordered left->right, top->down
			while (overlapIterator != overlapIteratorEnd) 
			{
				if ((doorType == DOOR_CEILING_FLOOR) && 
					(overlapIterator->start.x > overlapDoorInformation.start.x)) 
				{
					found = true;
					break;
				}
				else if ((doorType == DOOR_LEFT_RIGHT) && 
						 (overlapIterator->start.y > overlapDoorInformation.start.y)) 
				{
					found = true;
					break;
				}
				++overlapIterator;
			}
			if (found) 
			{
				overlapCollection.insert(overlapIterator, 
					overlapDoorInformation);
			}
			else 
			{
				overlapCollection.insert(overlapIteratorEnd, 
					overlapDoorInformation);
			}

			//
			// Create new doors, reusing the exposed door pointer
			//
			for (i=0; i<sections; ++i) 
			{
				create = (i>0);

				if (overlapInformation[i].shared) 
				{
					if ((edgeType == DIRECTION_LEFT) || (edgeType == DIRECTION_CEILING)) 
					{
						parent1 = exposedRoomID;
						parent2 = roomID;
					}
					else 
					{
						parent1 = roomID;
						parent2 = exposedRoomID;
					}
					if (create)
					{
						newDoor = new Door(doorType, PERMIABLE, 2, parent1, 
							parent2, overlapInformation[i].start, 
							overlapInformation[i].end);
						// Add this door to the new room's set of doors
						room->doorCollection.push_back(newDoor);
						// Add this door to the exposed room's set of doors
						exposedRoom->doorCollection.push_back(newDoor);
					}
					else 
					{
						// Reconfigure the exposed door
						exposedDoor->SetDoorProperties(doorType, PERMIABLE, 2, 
							parent1, parent2, overlapInformation[i].start, 
							overlapInformation[i].end);
						// Add this new door to the new room's set of doors
						room->doorCollection.push_back(exposedDoor);
					}
					// Add neighbour information
					room->neighbourIDCollection.push_back(exposedRoomID);
					room->neighbourIDCollections[edgeType].push_back(exposedRoomID);
					exposedRoom->neighbourIDCollection.push_back(roomID);
					exposedRoom->neighbourIDCollections[edgeTypeOpposite].push_back(roomID);
				}
				else 
				{
					// We've got an unshared door belonging to the exposed room
					if ((edgeType == DIRECTION_LEFT) || (edgeType == DIRECTION_CEILING)) 
					{
						parent1 = exposedRoomID;
						parent2 = -1;
					}
					else 
					{
						parent1 = -1;
						parent2 = exposedRoomID;
					}
					if (create)
					{
						newDoor = new Door(doorType, IMPERMIABLE, 1, parent1, 
							parent2, overlapInformation[i].start, 
							overlapInformation[i].end);
						// Add this door to the exposed room's set of doors
						exposedRoom->doorCollection.push_back(newDoor);
					}
					else 
					{
						exposedDoor->SetDoorProperties(doorType, 
							IMPERMIABLE, 1, parent1, parent2, 
							overlapInformation[i].start, 
							overlapInformation[i].end);
					}
				}

				if (create)
					// Add the new door to the temporary door collection
					myConstructedDoorCollection.push_back(newDoor);
			}
			// Try next door
			++doorIterator;
		}
		// Try next sibling room ID
		++roomIDIterator;
	}

	if ((edgeType == DIRECTION_LEFT) || (edgeType == DIRECTION_CEILING)) 
	{
		parent1 = -1;
		parent2 = roomID;
	}
	else 
	{
		parent1 = roomID;
		parent2 = -1;
	}

	if (!overlap) 
	{
		// The new room edge did not overlap with any existing door in the 
		// meta-room. We need to create a new unshared door.
		newDoor = new Door(doorType, IMPERMIABLE, 1, parent1, parent2, start, end);
		// Add this door to the new room's set of doors
		room->doorCollection.push_back(newDoor);
		// Add the new door to the temporary door collection
		myConstructedDoorCollection.push_back(newDoor);		
	}
	else 
	{
		// The new room edge overlapped with an existing door - we need to 
		// find the parts of the new room edge which were not shared with the 
		// exposed room edge.
		firstOverlap = overlapCollection.begin();
		lastOverlap = overlapCollection.end();
		lastOverlap--;
		info = *firstOverlap;

		if (((doorType == DOOR_LEFT_RIGHT) && 
			 (info.start.y > start.y)) ||
			((doorType == DOOR_CEILING_FLOOR) && 
			 (info.start.x > start.x))) 
		{
			// Create a new door
			
			newDoor = new Door(doorType, IMPERMIABLE, 1, parent1, parent2, start,
				info.start);			
			// Add this door to the new room's set of doors
			room->doorCollection.push_back(newDoor);
			// Add this door to the temporary door collection
			myConstructedDoorCollection.push_back(newDoor);	
		}
		overlapIterator = firstOverlap;
		nextOverlap = firstOverlap;
		++nextOverlap;
		
		while (overlapIterator != lastOverlap) 
		{
			info = *overlapIterator;
			infoNext = *nextOverlap;
			if (((doorType == DOOR_LEFT_RIGHT) && 
				 (info.end.y != infoNext.start.y)) ||
				((doorType == DOOR_CEILING_FLOOR) && 
				 (info.end.x != infoNext.start.x))) 
			{
				// Create a new door
				newDoor = new Door(doorType, IMPERMIABLE, 1, parent1, parent2, 
					info.end, infoNext.start);		
				// Add this door to the new room's set of doors
				room->doorCollection.push_back(newDoor);
				// Add this door to the temporary door collection
				myConstructedDoorCollection.push_back(newDoor);	
			}
			++overlapIterator;
			++nextOverlap;
		}

		info = *lastOverlap;

		if (((doorType == DOOR_LEFT_RIGHT) && 
			 (info.end.y < end.y)) ||
			((doorType == DOOR_CEILING_FLOOR) && 
			 (info.end.x < end.x))) 
		{
			// Create a new door
			newDoor = new Door(doorType, IMPERMIABLE, 1, parent1, parent2, 
				info.end, end);		
			// Add this door to the new room's set of doors
			room->doorCollection.push_back(newDoor);
			// Add this door to the temporary door collection
			myConstructedDoorCollection.push_back(newDoor);
		}
	}
}


// ---------------------------------------------------------------------------
// Function:	GetDoorBetweenRooms
// Description:	Gets a pointer to the door that lies between two rooms
// Arguments:	id1 - ID of first room (in)
//				id2 - ID of second room (in)
//				door - door pointer between the rooms (out)
// Returns:		true if good input, false otherwise	
// ---------------------------------------------------------------------------
bool Map::GetDoorBetweenRooms
	(const int id1, const int id2, Door*& door)
{
	Room* room;
	DoorIterator doorIterator, doorIteratorEnd;
	Door* d;
	int p1, p2;
	bool ok;

	// Test if room is valid
	ok = GetValidRoomPointer(id1, room);
	if (!ok)
		return false;
	doorIterator = room->doorCollection.begin();
	doorIteratorEnd = room->doorCollection.end();
	while (doorIterator != doorIteratorEnd) 
	{
		d = *doorIterator;
		if (d->parentCount == 2) 
		{
			p1 = d->parent1;
			p2 = d->parent2;
			if (((p1 == id1) && (p2 == id2)) ||
				((p1 == id2) && (p2 == id1))) 
			{
				door = d;
				return true;
			}
		}
		++doorIterator;
	}
	return false;
}



//
// 
// Serialization
//
//

static void WriteIntegerCollection
	(CreaturesArchive &ar, const IntegerCollection &integerCollection)
{
	int i;
	ConstantIntegerIterator integerIterator;

	// Write out the size of the collection
	int count = integerCollection.size();
	ar << count;
	for (integerIterator = integerCollection.begin();
		 integerIterator != integerCollection.end(); 
		 ++integerIterator) 
	{	
		i = *integerIterator;
		ar << i;
	}
}



static void WriteDoorCollection
	(CreaturesArchive &ar, const DoorCollection &doorCollection)
{
	ConstantDoorIterator doorIterator;
	Door* door;

	// Write out the size of the collection
	int count = doorCollection.size();
	ar << count;
	for (doorIterator = doorCollection.begin();
		 doorIterator != doorCollection.end(); 
		 ++doorIterator) 
	{	
		door = *doorIterator;
		ar << door;
	}
}


static void WriteStringCollection
	(CreaturesArchive &ar, const StringCollection &stringCollection)
{
	ConstantStringIterator stringIterator;
	std::string s;

	// Write out the size of the collection
	int count = stringCollection.size();
	ar << count;
	for (stringIterator = stringCollection.begin();
		 stringIterator != stringCollection.end(); 
		 ++stringIterator) 
	{	
		s = *stringIterator;
		ar << s;
	}
}

static void ReadIntegerCollection
	(CreaturesArchive &ar, IntegerCollection &integerCollection)
{
	int v, i;

	// Clear the collection
	integerCollection.clear();
	// Read the size of the collection
	int count;
	ar >> count;
	for (i=0; i<count; ++i) 
	{
		ar >> v;
		integerCollection.push_back(v);
	}
}

static void ReadDoorCollection
	(CreaturesArchive &ar, DoorCollection &doorCollection)
{
	Door* door;
	int i;

	// Clear the collection
	doorCollection.clear();
	// Read the size of the collection
	int count;
	ar >> count;
	for (i=0; i<count; ++i) 
	{
		ar >> door;
		doorCollection.push_back(door);
	}
}


static void ReadStringCollection
	(CreaturesArchive &ar, StringCollection &stringCollection)
{
	int i;
	std::string s;

	// Clear the collection
	stringCollection.clear();
	// Read the size of the collection
	int count;
	ar >> count;
	for (i=0; i<count; ++i) 
	{
		ar >> s;
		stringCollection.insert(s);
	}
}


bool Door::Write(CreaturesArchive &ar) const
{
	ar << doorType;
	ar << permiability;
	ar << parentCount;
	ar << parent1;
	ar << parent2;
	ar << start;
	ar << end;
	ar << delta;
	ar << length;
	ar << positionMin;
	ar << positionMax;
	ar << doorage1;
	ar << doorage2;
	return true;
}


bool Door::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();
	
	if(version >= 3)
	{
		ar >> doorType;
		ar >> permiability;
		ar >> parentCount;
		ar >> parent1;
		ar >> parent2;
		ar >> start;
		ar >> end;
		ar >> delta;
		ar >> length;
		ar >> positionMin;
		ar >> positionMax;
		ar >> doorage1;
		ar >> doorage2;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}

bool Link::Write(CreaturesArchive &ar) const
{
	ar << parent1 << parent2 << permiability << length
		<< doorage1 << doorage2;
	return true;
}

bool Link::Read(CreaturesArchive &ar)
{
	ar >> parent1 >> parent2 >> permiability >> length
		>> doorage1 >> doorage2;
	return true;
}

bool Room::Write(CreaturesArchive &ar) const
{
	int i;

	ar << roomID;
	ar << metaRoomID;
	ar << startFloor;
	ar << endFloor;
	ar << deltaFloor;
	ar << startCeiling;
	ar << endCeiling;
	ar << deltaCeiling;
	ar << centre;
	ar << positionMin;
	ar << positionMax;
	ar << permiability;
	::WriteDoorCollection(ar, doorCollection);
	::WriteDoorCollection(ar, floorCollection);
	::WriteDoorCollection(ar, ceilingCollection);
	::WriteDoorCollection(ar, leftCollection);
	::WriteDoorCollection(ar, rightCollection);
	
	for (i=0; i<4; ++i) 
	{
		::WriteIntegerCollection(ar, neighbourIDCollections[i]);
	}
	::WriteIntegerCollection(ar, neighbourIDCollection);
	ar << type;
	for (i=0; i<CA_PROPERTY_COUNT; ++i) 
	{
		ar << caValues[i];
		ar << caOldValues[i];
		ar << caOlderValues[i];
	}
	ar << caTempValue;
	ar << caInput;
	ar << caTotalDoorage;
	ar << perimeterLength;
	ar << track;
	ar << leftFloorRoomID;
	ar << rightFloorRoomID;
	ar << leftNavigableDoor;
	ar << rightNavigableDoor;
	ar << linkCollection;
	return true;
}


bool Room::Read(CreaturesArchive &ar)
{

	int i;

	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		ar >> roomID;
		ar >> metaRoomID;
		ar >> startFloor;
		ar >> endFloor;
		ar >> deltaFloor;
		ar >> startCeiling;
		ar >> endCeiling;
		ar >> deltaCeiling;
		ar >> centre;
		ar >> positionMin;
		ar >> positionMax;
		ar >> permiability;
		::ReadDoorCollection(ar, doorCollection);
		::ReadDoorCollection(ar, floorCollection);
		::ReadDoorCollection(ar, ceilingCollection);
		::ReadDoorCollection(ar, leftCollection);
		::ReadDoorCollection(ar, rightCollection);

		for (i=0; i<4; ++i) 
		{
			::ReadIntegerCollection(ar, neighbourIDCollections[i]);
		}
		::ReadIntegerCollection(ar, neighbourIDCollection);
		ar >> type;
		for (i=0; i<CA_PROPERTY_COUNT; ++i) 
		{
			ar >> caValues[i];
			if( theApp.GetWorld().GetMap().IsCANavigable( i ) ) caValues[i] = 0.0f;
			ar >> caOldValues[i];
			ar >> caOlderValues[i];
		}

		ar >> caTempValue;
		ar >> caInput;
		ar >> caTotalDoorage;
		ar >> perimeterLength;
		ar >> track;
		ar >> leftFloorRoomID;
		ar >> rightFloorRoomID;
		ar >> leftNavigableDoor;
		ar >> rightNavigableDoor;
		ar >> linkCollection;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

bool MetaRoom::Write(CreaturesArchive &ar) const
{
	ar << metaRoomID;
	ar << positionMin;
	ar << positionMax;
	ar << width;
	ar << height;
	::WriteIntegerCollection(ar, roomIDCollection);
	::WriteStringCollection(ar, backgroundCollection);
	ar << background;
	ar << track;
	return true;
}


bool MetaRoom::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		ar >> metaRoomID;
		ar >> positionMin;
		ar >> positionMax;
		ar >> width;
		ar >> height;
		::ReadIntegerCollection(ar, roomIDCollection);
		::ReadStringCollection(ar, backgroundCollection);
		ar >> background;
		ar >> track;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


bool Map::Write(CreaturesArchive &ar) const
{
	int i, j;
	int count;

	ar << myRoomCount;
	ar << myMetaRoomCount;
	ar << myMaxRoomID;
	ar << myMetaRoomMaxID;
	ar << myMetaRoomID;
	ar << myMapWidth;
	ar << myMapHeight;
	ar << myGridSquareSize;
	ar << myMetaRoomMaxCoordinates;
	ar << mySquareCountX;
	ar << mySquareCountY;
	ar << myNextCAProperty;

	for (i=0; i<=myMaxRoomID; ++i) 
	{
		ar << myRoomCollection[i];
	}
	::WriteIntegerCollection(ar, myRoomIDCollection);

	for (i=0; i<=myMetaRoomMaxID; ++i) 
	{
		ar << myMetaRoomCollection[i];
	}
	::WriteIntegerCollection(ar, myMetaRoomIDCollection);	
	::WriteDoorCollection(ar, myDoorCollection);

	count = mySquareCountX*mySquareCountY;
	ar << count;
	for (i=0; i<count; ++i) 
	{
		::WriteIntegerCollection(ar, myIntegerCollectionGrid[i]);
	}

	for (i=0; i<ROOM_TYPE_COUNT; ++i) 
	{
		for (j=0; j<CA_PROPERTY_COUNT; ++j) 
		{
			myCARates[i][j].Write(ar);
		}	
	}	
	ar << myLinkCollection;
	ar << myNavigableDoorCollection;
	return true;
}


bool Map::Read(CreaturesArchive &ar)
{
	int i, j;
	int count;

	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		ar >> myRoomCount;
		ar >> myMetaRoomCount;
		ar >> myMaxRoomID;
		ar >> myMetaRoomMaxID;
		ar >> myMetaRoomID;
		ar >> myMapWidth;
		ar >> myMapHeight;
		ar >> myGridSquareSize;
		ar >> myMetaRoomMaxCoordinates;
		ar >> mySquareCountX;
		ar >> mySquareCountY;
		ar >> myNextCAProperty;

		for (i=0; i<=myMaxRoomID; ++i) 
		{
			ar >> myRoomCollection[i];
		}
		for (i=myMaxRoomID+1; i<MAX_ROOMS; ++i)
			myRoomCollection[i] = NULL;

		::ReadIntegerCollection(ar, myRoomIDCollection);

		for (i=0; i<=myMetaRoomMaxID; ++i) 
		{
			ar >> myMetaRoomCollection[i];
		}
		for (i=myMetaRoomMaxID+1; i<MAX_META_ROOMS; ++i)
			myMetaRoomCollection[i] = NULL;
		::ReadIntegerCollection(ar, myMetaRoomIDCollection);	
		::ReadDoorCollection(ar, myDoorCollection);

		ar >> count;
		ASSERT( count == mySquareCountX*mySquareCountY );
		delete [] myIntegerCollectionGrid;
		myIntegerCollectionGrid = new IntegerCollection[count];
		for (i=0; i<count; ++i) 
		{
			::ReadIntegerCollection(ar, myIntegerCollectionGrid[i]);
		}

		for (i=0; i<ROOM_TYPE_COUNT; ++i) 
		{
			for (j=0; j<CA_PROPERTY_COUNT; ++j) 
			{
				myCARates[i][j].Read(ar);
			}	
		}	
		ar >> myLinkCollection;
		ar >> myNavigableDoorCollection;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}
