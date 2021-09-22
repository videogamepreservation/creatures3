// -------------------------------------------------------------------------
// Filename:    CompoundPart.h
// Class:       CompoundPart
// Purpose:     Base class for compound parts, which form CompoundAgents
// Description:
//
// Usage:
//
//
// History:
// 11Feb99	BenC	Initial version
// -------------------------------------------------------------------------
#ifndef COMPOUNDPART_H
#define COMPOUNDPART_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../Display/EntityImage.h"
#include "../Map/Map.h"

class CompoundAgent;

class CompoundPart: public PersistentObject
{
	CREATURES_DECLARE_SERIAL( CompoundPart )

public:

	enum
	{
		partPlain  = 1,		// CompoundPart
		partUI     = 2,		// UIPart
		partText   = 4,		// UITextPart
		partEdit   = 8,		// UIText
		partGraph  = 16,	// UIGraph
		partCamera = 32,	// CameraPart
		partButton = 64,	// UIButton
	};

	int GetType() { return myType; }

	CompoundPart();
	CompoundPart(FilePath const& gallery, int baseimage, Vector2D& relPos, int relplane, int numImages = 0 );
	CompoundPart( Vector2D& relPos, Vector2D& size, int relplane );
	virtual ~CompoundPart();

	// ---------------------------------------------------------------------
	// Method:      GetRelX, GetRelY, GetRelPlane
	// Arguments:	None
	// Returns:     X, Y or plane, relative to the CompoundAgent
	// Description: Basic accessor functions
	// ---------------------------------------------------------------------
	float GetRelX() const
		{ return myRelativePosition.x; }
	float GetRelY() const
		{ return myRelativePosition.y; }
	int GetRelPlane() const
		{ return myRelPlane; }

	// ---------------------------------------------------------------------
	// Method:		SetRelX, SetRelY, SetRelPlane
	// Arguments:	X, Y or plane, relative to the CompoundAgent
	// Returns:		None
	// Description:	Basic accessor functions
	// ---------------------------------------------------------------------
	void SetRelX( float x )
		{ myRelativePosition.x = x; }
	void SetRelY( float y )
		{ myRelativePosition.y = y; }
	void SetRelPlane( int relplane )
		{ myRelPlane = relplane; }

	// ---------------------------------------------------------------------
	// Method:		GetEntity
	// Arguments:	None
	// Returns:		EntityImage used by part
	// Description:	Basic accessor function
	// ---------------------------------------------------------------------
	EntityImage* GetEntity()
		{ return myEntity; }
	void SetEntity(EntityImage* newEntity)
		{ myEntity = newEntity; }
	
	bool GetPixelPerfectTesting(){return myPixelPerfectHitTestFlag;}
	void SetPixelPerfectTesting(bool flag){myPixelPerfectHitTestFlag = flag;}

	void Show();
	void Hide();
	bool Visibility(int scope);
	void DrawMirrored(bool mirrored);



	// ---------------------------------------------------------------------
	// Method:		SussPosition
	// Arguments:	mainx - x pos of the owning CompoundAgent
	//				mainy - y pos of the owning CompoundAgent
	// Returns:		None
	// Description:	Moves the part to the correct position in the world
	//				given the world position of the owning CompoundAgent.
	// ---------------------------------------------------------------------
	virtual void SussPosition( const Vector2D& position );

	// ---------------------------------------------------------------------
	// Method:		SussPlane
	// Arguments:	mainplane - plot plane of the owning CompoundAgent
	// Returns:		None
	// Description:	Moves the part to the correct plane, with respect
	//				to the owning CompoundAgent.
	// ---------------------------------------------------------------------
	virtual void SussPlane( int mainplane );


	// ---------------------------------------------------------------------
	// Method:		Tick
	// Arguments:	None
	// Returns:		None
	// Description:	Virtual function to update part. Default behaviour is
	//				to just call the EntityImage Animate() function to
	//				update the parts animation (if any).
	// ---------------------------------------------------------------------
	virtual void Tick();

	virtual void ChangeCameraShyStatus(bool shy);

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

	virtual bool SetAnim(const uint8* anim, int length);
	virtual bool SetFrameRate(const uint8 rate);

	virtual void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen = 0,
					uint8 lineColourBlue = 0,
					uint8 stippleon =0,
					uint8 stippleoff = 0,
					uint32 stippleStart  =0);

	virtual void ResetLines();

	CompoundAgent *GetParent() const { return myParent; }
	void SetParent( CompoundAgent *parent ) { myParent = parent; }

	virtual void Tint(const uint16* tintTable);
	
protected:
	bool myPixelPerfectHitTestFlag;
	EntityImage* myEntity;
	Vector2D myRelativePosition;
	int	myRelPlane;
	CompoundAgent *myParent;

	int myType;
};


inline void CompoundPart::SussPosition( const Vector2D& position )
{
	myEntity->SetPosition( Map::FastFloatToInteger(position.x) + Map::FastFloatToInteger(myRelativePosition.x), 
		Map::FastFloatToInteger(position.y) + Map::FastFloatToInteger(myRelativePosition.y) );
}

inline void CompoundPart::SussPlane( int mainplane )
{
	myEntity->SetPlane( mainplane + myRelPlane );
}



#endif // COMPOUNDPART_H
