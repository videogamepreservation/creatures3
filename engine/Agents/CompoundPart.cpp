// -------------------------------------------------------------------------
// Filename:    CompoundPart.cpp
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
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CompoundPart.h"
#include "CompoundAgent.h"
#include "../Display/EntityImageClone.h"

CREATURES_IMPLEMENT_SERIAL( CompoundPart )

CompoundPart::CompoundPart()
:myRelativePosition(0.0f,0.0f),
myRelPlane(0),
myEntity(NULL),
myType(partPlain),
myPixelPerfectHitTestFlag(true)
{
}


CompoundPart::CompoundPart( FilePath const& gallery, int baseimage, Vector2D& relPos, int relplane, int numImages  )
:myType(partPlain),
myPixelPerfectHitTestFlag(true)
{
	myRelativePosition = relPos;
	myRelPlane = relplane;
	myEntity = new EntityImage( gallery, numImages, myRelPlane, -7654, -7654, baseimage, baseimage );
}


CompoundPart::~CompoundPart()
{
	if (myEntity)
	{
		delete myEntity;
		myEntity = NULL;
	}
}

// virtual
void CompoundPart::Tick()
{
	myEntity->Animate();
}

// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool CompoundPart::Write(CreaturesArchive& archive) const
{
	archive << myRelativePosition << myRelPlane;

	archive << myEntity;
	archive << myParent;
	archive << myPixelPerfectHitTestFlag;
	return true;
}

bool CompoundPart::Read(CreaturesArchive& archive)
{
	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{
		archive >> myRelativePosition >> myRelPlane;

		archive >> myEntity;
		archive >> myParent;
		archive >> myPixelPerfectHitTestFlag;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

bool CompoundPart::SetAnim(const uint8* anim, int length)
{
	if (!myEntity->ValidateAnim( anim, length ))
		return false;

	myEntity->SetAnim( anim, length );
	return true;
}

bool CompoundPart::SetFrameRate(const uint8 rate)
{
	myEntity->SetFrameRate(rate);
	return true;
}
// virtual
void CompoundPart::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stippleStart /*=0*/) 
{
	if(myEntity)
		myEntity->DrawLine(x1,y1,x2,y2,
		lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stippleStart);
}


// virtual
void CompoundPart::ResetLines()
{
	if(myEntity)
		myEntity->ResetLines();
}

void CompoundPart::Show()
{
	if(myEntity)
		myEntity->Link();
}

void CompoundPart::Hide()
{
	if(myEntity)
		myEntity->Unlink();
}


bool CompoundPart::Visibility(int scope)
{

	if(myEntity)
	{
		return myEntity->Visible(scope);
	}

	return false;
}

void CompoundPart::ChangeCameraShyStatus(bool shy)
{
	if( myEntity ) myEntity->YouAreCameraShy(shy);
}

void CompoundPart::Tint(const uint16* tintTable)
{
	EntityImage* entityImage = GetEntity();

	// create a temporary cloned entity image

	ClonedEntityImage*	clone =	new ClonedEntityImage(entityImage->GetGallery()->GetName(),
			entityImage->GetPlane(),
			entityImage->GetPosition().GetX(),
			entityImage->GetPosition().GetY(),
			entityImage->GetAbsoluteBaseImage(),
			entityImage->GetGallery()->GetCount(),
			entityImage->GetAbsoluteBaseImage() );

	clone->SetPose(entityImage->GetPose());

	delete entityImage;
	SetEntity(clone);
	clone->GetGallery()->Recolour(tintTable);
	clone->Link(true);	
}

void CompoundPart::DrawMirrored(bool mirror)
{
	if(myEntity)
		myEntity->DrawMirrored(mirror);

}