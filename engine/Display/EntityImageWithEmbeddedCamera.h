// --------------------------------------------------------------------------
// Filename:	EntityImageWithEmbeddedCamera.cpp
// Class:		EntityImageWithEmbeddedCamera
// Purpose:		This class is one of the interfaces to the rest of C2e.  This class 
//				is closely based on the C2 entity and will evolve as required.
//				The difference between a This entityimage contains a camera over 
//				which the usual animations can be displayed.
//
//				The camera appears one pixel behind the overlay
//
//
//				
//				
// History:
// ------- 
// 16Jun99	Alima		Created. 
//							
//
// --------------------------------------------------------------------------

#ifndef ENTITY_IMAGE_WITH_EMBEDDED_CAMERA_H
#define ENTITY_IMAGE_WITH_EMBEDDED_CAMERA_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "EntityImage.h"



class RemoteCamera;

class EntityImageWithEmbeddedCamera : public EntityImage
{
	CREATURES_DECLARE_SERIAL( EntityImageWithEmbeddedCamera )

public:
	EntityImageWithEmbeddedCamera();
	virtual ~EntityImageWithEmbeddedCamera();

	EntityImageWithEmbeddedCamera(FilePath const& galleryName,
								int32 numimages,
								int32 plane,//plane
								int32 worldx, // world x
								int32 worldy, // world y
								uint32 baseimage,					// base image
								uint32 currentimage,
								uint32 viewWidth,
								uint32 viewHeight,
								uint32 cameraWidth,
								uint32 cameraHeight);			// current image

	virtual void AdjustXY(int32 x, int32 y);

	virtual void SetWorldXY(int32 x, int32 y);

	virtual void SetPlane(int plane);
		
	virtual void SetPosition(int32 x, int32 y);

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

	RemoteCamera* GetCamera(){return myCamera;}
private:
	RemoteCamera* myCamera;

};

#endif