
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SimpleAgent.h"
#include "../World.h"
#include "../App.h"
#include "../AgentManager.h"
#include "../Display/Gallery.h"




CREATURES_IMPLEMENT_SERIAL( SimpleAgent)
          


// Basic initialisation used by constructors
void SimpleAgent::Init()
{
    myNormalPlane=0;
}


// Virtual
uint32 SimpleAgent::GetMagicNumber()
{
	return 3141592654U;
}


// Serialisation & subclass constr
SimpleAgent::SimpleAgent()
{
	Init();
	myAgentType = AgentHandle::agentNormal | AgentHandle::agentSimple;
}


// Construct a SimpleAgent (or base class part, for subclasses) by supplying
// all necessary information
// Note: last few params have defaults for dumb objects

// changing the constructor to take the gallery name
SimpleAgent::SimpleAgent(	int family, int genus, int species,uint32 id,
						 FilePath const &gallery,		// Images us
							int numimages,
							int baseimage,
                            int plane,              // normal plot plane
							bool cloneMyImages)
{
	try
	{
		Classifier c(family, genus, species);
		myClassifier = c;
		myID = id;
		myAgentType = AgentHandle::agentNormal | AgentHandle::agentSimple;
		Init();						// defaults

		// create a unique gallery for this agent as
		// we will need to draw to it


		// TODO: Rethink this coordinate (myPositionVector) passing in now we
		// have myInvalidPosition in Agent.h?

		if(cloneMyImages)
		{
		myEntityImage =	new  ClonedEntityImage(gallery,
				plane,
				Map::FastFloatToInteger(myPositionVector.x),
				Map::FastFloatToInteger(myPositionVector.y),
				baseimage,
				numimages,
				baseimage );
		}
		else
		{
		myEntityImage =	new  EntityImage(gallery,
					numimages,
					plane,
					Map::FastFloatToInteger(myPositionVector.x),
					Map::FastFloatToInteger(myPositionVector.y),
					baseimage,
					0 );
		}

		myNormalPlane=plane;                              // remember changeable stuff
		InitialisePickupPointsAndHandles();
		myCurrentWidth = myEntityImage->GetWidth();
		myCurrentHeight = myEntityImage->GetHeight();
	}
	catch (BasicException& e)
	{
		myFailedConstructionException = e.what();
	}
	catch (...)
	{
		myFailedConstructionException = "NLE0008: Unknown exception caught in simple agent constructor";
	}
}

// Destructor
SimpleAgent::~SimpleAgent()
{
}


// virtual
void SimpleAgent::DrawLine( int32 x1,
					int32 y1,
					int32 x2,
					int32 y2 ,	 
					uint8 lineColourRed /*= 0*/,
					uint8 lineColourGreen /*= 0*/,
					uint8 lineColourBlue /*= 0*/,
						 uint8 stippleon /* =0*/,
							 uint8 stippleoff/* = 0*/,
							 uint32 stippleStart /* =0*/) 
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
		myEntityImage->DrawLine(x1,y1,x2,y2,
		lineColourRed,lineColourGreen,lineColourBlue,
		stippleon,stippleoff,stippleStart);
}


// virtual
void SimpleAgent::ResetLines()
{
	_ASSERT(!myGarbaged);

	if(myEntityImage)
		myEntityImage->ResetLines();
}

// Update function for all SimpleAgents. Called every clock tick.
void SimpleAgent::Update()
{
	_ASSERT(!myGarbaged);

	base::Update();

	_ASSERT(!myGarbaged);

	myEntityImage->Animate();

	myCurrentWidth = myEntityImage->GetWidth();
	myCurrentHeight = myEntityImage->GetHeight();

}

// retn message# (ACTIVATE1 etc) appropriate for a click at a given
// position, else return -1
int SimpleAgent::ClickAction(int x,int y)
{
	_ASSERT(!myGarbaged);

	// some agents may want to set up a cycle of left clicks
	DecideClickActionByState();
	return myDefaultClickAction;
}


// return *principal* plot plane
int SimpleAgent::GetPlane(int part)
{
	_ASSERT(!myGarbaged);

	return(myEntityImage->GetPlane());
}





///////////////////// macro helper functions ///////////////////


// Helper function for ANIM macro - read a square bracketed string from ip[], 
// and use as animation string for entity
bool SimpleAgent::SetAnim(const uint8* anim, int length,int part)
{
	_ASSERT(!myGarbaged);

	if (!myEntityImage->ValidateAnim( anim, length ))
		return false;
	myEntityImage->SetAnim( anim, length );
	return true;
}

// virtual helper function for OVER macro.
// return TRUE if current animation is over.
bool SimpleAgent::AnimOver(int part)
{
	_ASSERT(!myGarbaged);
	return myEntityImage->AnimOver();
}

// virtual helper function for POSE macro.
// Stop any anim and set a new pose.
bool SimpleAgent::ShowPose(int pose,int part)
{
	_ASSERT(!myGarbaged);

	if (!myEntityImage->ValidatePose( pose ))
		return false;
	myEntityImage->SetPose(pose);

	myCurrentWidth = myEntityImage->GetWidth();
	myCurrentHeight = myEntityImage->GetHeight();
	return true;
}

// virtual helper function for POSE
int SimpleAgent::GetPose(int part)
{
	_ASSERT(!myGarbaged);

	return myEntityImage->GetPose();
}



// Helper fn for BASE macro - set base sprite for obj
bool SimpleAgent::SetBaseImage(int image,int part)
{
	_ASSERT(!myGarbaged);
	return myEntityImage->SetBaseAndCurrentIndex(image);
}

int SimpleAgent::GetBaseImage(int part)
{
	_ASSERT(!myGarbaged);
	return myEntityImage->GetBaseAndCurrentIndex();
}



// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool SimpleAgent::Write(CreaturesArchive &archive) const
{
	_ASSERT(!myGarbaged);
    base::Write( archive );
	archive << myNormalPlane;
	return true;
}

bool SimpleAgent::Read(CreaturesArchive &archive)
{
	_ASSERT(!myGarbaged);


	int32 version = archive.GetFileVersion();
	if(version >= 3)
	{

		// call base class function first
		if(!base::Read( archive ))
			return false;

		archive >> myNormalPlane;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}



void SimpleAgent::MoveTo(float x,float y)
{
	_ASSERT(!myGarbaged);
	base::MoveTo(x, y);
	if (myEntityImage)
		myEntityImage->SetPosition
			(Map::FastFloatToInteger(x), 
			 Map::FastFloatToInteger(y));
}

void SimpleAgent::SetNormalPlane(int p, int part)
{
	_ASSERT(!myGarbaged);
	myNormalPlane = p;
	ChangePhysicalPlane(p);
}

int SimpleAgent::GetNormalPlane(int part)
{
	_ASSERT(!myGarbaged);
	return myNormalPlane;
}


void SimpleAgent::Trash()
{
	_ASSERT(!myGarbaged);

	// This must be last line in the function
	base::Trash();
}


void SimpleAgent::Tint(const uint16* tintTable, int part)
{
	_ASSERT(!myGarbaged);

	// we could delete everytime or do a check to 
	// see whether we have a cloned entity already and reload?
	if(myEntityImage)
	{
		// create a temporary cloned entity image
		
		ClonedEntityImage*	clone =	new  ClonedEntityImage(myEntityImage->GetGallery()->GetName(),
				myEntityImage->GetPlane(),
				Map::FastFloatToInteger(myPositionVector.x),
				Map::FastFloatToInteger(myPositionVector.y),
				myEntityImage->GetAbsoluteBaseImage(),
				myEntityImage->GetGallery()->GetCount(),
				myEntityImage->GetAbsoluteBaseImage() );

		clone->SetPose(myEntityImage->GetPose());

		delete myEntityImage;
		myEntityImage = clone;

		clone->GetGallery()->Recolour(tintTable);
		clone->Link(true);	

	}
}

