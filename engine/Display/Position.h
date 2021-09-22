#ifndef		POSITION_H
#define		POSITION_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"
class CreaturesArchive;

// at some point I should overhaul this
class Position 
{
public:

	Position():
	  myX(0),
		myY(0)
	{;}

	Position(int32 x,int32 y)
		:myX(x)
		,myY(y)
	{;}

	void operator+=(const Position& p)
	{
		myX+=p.myX; 
		myY+=p.myY;
	}

	void operator-=(const Position& p)
	{
		myX-=p.myX; 
		myY-=p.myY;
	}

	bool operator!=(const Position& p)
	{
		return !((myX == p.myX) && (myY == p.myY));
	}

	bool operator==(const Position& p)
	{
		return ((myX == p.myX) && (myY == p.myY));
	}
// ----------------------------------------------------------------------
// Method:      Get/SetX or Y 
// Arguments:   None/position - the x or y position in which you wish the
//				entity to reside
//
// Returns:     The x or y position that the entity exists in/None
//
//				This is here because the current version of C2e
//				doesn't know about positions by deals in bare x,y coords.
//			
// ----------------------------------------------------------------------
	int32 GetX() const
	{
		return myX;
	}

	int32 GetY() const
	{
		return myY;
	}

	void SetX(int32 x)
	{
	myX = x;
	}

	void SetY(int32 y)
	{
		myY = y;
	}

	void AdjustX(int32 x)
	{
	myX += x;
	}

	void AdjustY(int32 y)
	{
	myY += y;
	}

	void Reset()
	{
		myX = myY = 0;
	}
	friend CreaturesArchive &operator<<( CreaturesArchive &archive, const Position &position );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, Position &position );
/*
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
	virtual bool Read(CreaturesArchive &archive);*/

private:
	int32	myX;
	int32	myY;
};

#endif		// POSITION_H