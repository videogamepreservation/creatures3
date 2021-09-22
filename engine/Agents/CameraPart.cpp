#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CameraPart.h"
#include "../Display/EntityImageWithEmbeddedCamera.h"
#include "../Display/RemoteCamera.h"


//make sure that this is serializable
CREATURES_IMPLEMENT_SERIAL( CameraPart)

CameraPart::CameraPart()
:myCameraEntity(NULL)
{
	myType = (partPlain | partCamera);
}

CameraPart::~CameraPart()
{
	delete myCameraEntity;
}

CameraPart::CameraPart( FilePath const& gallery,
		int baseImage, 
		int numImages, 
		Vector2D& relPos,
		int relplane,
		uint32 viewWidth,
		uint32 viewHeight,
		uint32 cameraWidth,
		uint32 cameraHeight):
CompoundPart( gallery, baseImage, relPos, relplane, numImages )
{


	// all of the default values here will be reset when the part
	// is actually added to the compound agent
   	myType = (partPlain | partCamera);
	 myCameraEntity = new EntityImageWithEmbeddedCamera(gallery,
									numImages,
									relplane,//plane of entityimage camera will be on pixel behind
									Map::FastFloatToInteger(relPos.x),
									Map::FastFloatToInteger(relPos.y),
									baseImage,					
									baseImage,
									viewWidth,
									viewHeight,
									cameraWidth,
									cameraHeight);

}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool CameraPart::Write(CreaturesArchive& archive) const
{
	CompoundPart::Write(archive);

	archive << myCameraEntity;
	return true;
}

bool CameraPart::Read(CreaturesArchive& archive)
{
	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{
		if(!CompoundPart::Read(archive))
			return false;

		archive >> myCameraEntity;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}

void CameraPart::SussPosition( const Vector2D& position )
{
	CompoundPart::SussPosition(position);
	myCameraEntity->SetPosition( Map::FastFloatToInteger(position.x + myRelativePosition.x),
								 Map::FastFloatToInteger(position.y + myRelativePosition.y));
}

void CameraPart::SussPlane( int mainplane )
{
	CompoundPart::SussPlane(mainplane);
	myCameraEntity->SetPlane( mainplane + myRelPlane );
}


RemoteCamera* CameraPart::GetCamera()
{
	return myCameraEntity->GetCamera();
}

