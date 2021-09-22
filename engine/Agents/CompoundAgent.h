#ifndef COMPOUNDAGENT_H
#define COMPOUNDAGENT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Agent.h"
#include "../Display/EntityImage.h"
#include "CompoundPart.h"


#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER


#include <vector>


enum
{
    CREATURE = 0x01,
    POINTER = 0x02
};



class CompoundAgent : public Agent
{
	typedef Agent base;

	CREATURES_DECLARE_SERIAL( CompoundAgent )


public:

	//-- constructors
	CompoundAgent();							// serialisation constr
	CompoundAgent(
		int family, int genus, int species, uint32 id,
		FilePath const& gallery,
		int numimages,
		int baseimage,
		int plane);

	virtual void Trash();
	virtual ~CompoundAgent();				// destruct an obj & remove frm objlib




  

	virtual void MoveTo(float x,float y);		// set position and redraw

	bool AddPart( int partid, CompoundPart* part );
	void DestroyPart( int id );

	CompoundPart* GetPart( int partid );

	int GetIndex( CompoundPart* part );

	virtual void ChangeCameraShyStatus(bool shy);

	bool IsPartValid( int partid )
	{
		_ASSERT(!myGarbaged);

		if( partid < 0 || partid >= myParts.size() )
			return false;
		else
			return (myParts[partid] != NULL);
	}

	virtual EntityImage* GetPrimaryEntity();

	// virtual helper functions for macro commands
	virtual bool SetAnim(const uint8* anim, int length,int partid);
	virtual bool SetFrameRate(const uint8 rate,int partid);
	virtual bool AnimOver(int partid);					// helper fn for OVER macro
	virtual bool ShowPose(int pose,int partid=0);		// helper fn for POSE macro
	virtual int GetPose(int partid);					// helper function for POSE
	virtual bool SetBaseImage(int image,int partid=0);	// default helper fn for BASE macro
	virtual int GetBaseImage(int part);
	virtual void SetPlane(int plane, int part = 0);
	virtual bool ValidatePart(int partid);


	virtual int GetPlane(int part = 0);			// return *principal* plot plane
	virtual void SetNormalPlane(int plane, int part = 0);
	virtual int ClickAction(int x, int y);

	virtual void SetGallery(FilePath const& galleryName, int baseimage, int part = 0);
	virtual bool HitTest(Vector2D& point);

	virtual void DrawMirrored(bool mirror);

	//-- master update routines - called from clock tick
	virtual void Update();
	virtual void HandleUI(Message* Msg);


	virtual void DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed = 0,
					uint8 lineColourGreen = 0,
					uint8 lineColourBlue = 0,
					uint8 stippleon =0,
					uint8 stippleoff = 0,
					uint32 stippleStart =0);

	virtual void ResetLines();

	virtual bool SetPixelPerfectTesting(bool flag,int partNumber);

	virtual void Hide();
	virtual void Show();
	virtual bool Visibility(int scope);

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

	virtual int GetNormalPlane(int part = 0)
	{
		_ASSERT(!myGarbaged);
		return myNormalBasePlaneForPartZero;
	}

	virtual void ChangePhysicalPlane(int plane);

	virtual void Tint(const uint16* tintTable, int part = 0);

protected:
	void Init();					// Basic initialisation used by constructors
	typedef std::vector<CompoundPart*> PartCollection;
	typedef std::vector<CompoundPart*>::iterator PartIterator;
	PartCollection	myParts;

	int myNormalBasePlaneForPartZero;
};

#endif // COMPOUNDAGENT_H
