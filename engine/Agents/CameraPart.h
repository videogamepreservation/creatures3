#ifndef CameraPart_H
#define CameraPart_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CompoundPart.h"


class EntityImageWithEmbeddedCamera;
class RemoteCamera;

class CameraPart : public CompoundPart
{
	CREATURES_DECLARE_SERIAL( CameraPart )
public:
	CameraPart();
	CameraPart( FilePath const& gallery,
		int baseImage, 
		int numImages, 
		Vector2D& relPos,
		int relplane,
		uint32 viewWidth,
		uint32 viewHeight,
		uint32 cameraWidth,
		uint32 cameraHeight);

	virtual ~CameraPart();


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

	virtual void SussPosition( const Vector2D& position );

	// ---------------------------------------------------------------------
	// Method:		SussPlane
	// Arguments:	mainplane - plot plane of the owning CompoundAgent
	// Returns:		None
	// Description:	Moves the part to the correct plane, with respect
	//				to the owning CompoundAgent.
	// ---------------------------------------------------------------------
	virtual void SussPlane( int mainplane );

	RemoteCamera* GetCamera();

protected:

	EntityImageWithEmbeddedCamera* myCameraEntity;
};

#endif // CameraPart_H
