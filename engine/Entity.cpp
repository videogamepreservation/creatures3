// --------------------------------------------------------------------------
// Filename:	Entity.cpp
// Class:		...
// Purpose:		taken from the header file (see the header for more details
//				written by Steve)
//	
//	An entity is a 'thing' that exists in the game world (unlike a sprite
//	for example, which only exists in the computer). Entities are contained
//	within the various types of Object (creatures, machines, etc.); some
//	objects may contain several Entities.
//
//	Entities are the things that get redrawn when a rectangle is invalidated.
//	A library of pointers to all extant entities is stored in EntityLib[]
//	for this purpose.
//
//	Entities are ALWAYS encapsulated in Objects of one kind or another. Thus,
//	although each entity contains its own X&Y members, these get set as
//	appropriate by the Object which owns them, and don't need to be
//	initialised when creating the entity itself. Thus, entities can be
//	created dynamically by the objects that contain them.
//				
//
// Description: 
//			
//				
//
// History:
// -------  Steve Grand		created
// 11Nov98	Alima			Started restructuring to separate the visual parts
//							of entity into a contained class that the display
//							engine deals with.  The new contained class is 
//							called EntityImage.  The new member is myImage.
//							I will not remove the current sprite system as I
//							don't want to break the engine just yet.
//	
// 04Dec98	Alima			Started removing all old display engine stuff
//							Objects now pass the name of the gallery 
//							containing their sprites to the new EntityImage
//
//							*********************************************
//							* Breaking Serialization until Entity Image *
//							* can serialize itself.	This should not be	*
//							* a problem since a blank world gets created*
//							* at the moment anyway.						*
//							*********************************************
// 15Jan99	Alima removed Entity it has now been replaced by EntityImage in
//			 full.  That means EntityImage does that link thing with the
//			worlds entityImagelib.
//							
// --------------------------------------------------------------------------

// Small Furries Entity class methods


// N.B. ALIMA IS IN THE PROCESS OF TIDYING THIS UP

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "App.h"
#include "World.h"
//#include "Creature\LimbOverlay.h"
//#include "Creature\BodyOverlay.h"
#include "Creature/BodyPartOverlay.h"
//#include "Creature\BodySuit.h"
#include "General.h"
#include "Creature/Definitions.h"

#include <string.h>

CREATURES_IMPLEMENT_SERIAL( BodyPart )
CREATURES_IMPLEMENT_SERIAL( Body)
CREATURES_IMPLEMENT_SERIAL( Limb)








            ////////////////////////////////////////////////
            //////////////// ENTITY CLASSES ////////////////
            ////////////////////////////////////////////////



            ////////////////////////////////////////////////////
            // Entity class: base class for all entities
			//////




            ///////////////////////////////////////////////////////////
            // BodyPart class: virtual parent to Body & Limb classes //
            ///////////////////////////////////////////////////////////

// null constructor
BodyPart::BodyPart()
:	myAngle (0), 
	myView(0),
	myOverlayShouldOverrideTheBaseBodySprite(0),
	myOverlay(0),
	myPart(0),
	myGenus(0),
	mySex(0),
	myVariant(0),
	myLastAge(0),
	myBodyPartType(0),
	myMirroredFlag(0)
{
	//BaseImage is taken care of in myImage contsructor

}



BodyPart::BodyPart(int32  plane,
            int32  worldx,
            int32  worldy,
			FilePath const &gallery,		// gallery containing images
			int32 numimages,
            uint32 baseimage,
			int part,
			int genus,
			int sex,
			int age,
			int variant,
			int bodyPartType,
			bool mirror):
EntityImage(gallery,
					numimages,
					plane,
					worldx,
					worldy,
					baseimage,
					baseimage ),
					myAngle(0),
					myView(0),
	myOverlayShouldOverrideTheBaseBodySprite(0),
	myOverlay(0),
	myPart(part),
	myGenus(genus),
	mySex(sex),
	myVariant(variant),
	myLastAge(age)

{
	myBodyPartType = bodyPartType;
	myMirroredFlag = mirror;


	SetBodyPartFileName(myPart,
						myGenus,
						mySex,
						age,
						myVariant);
	CreateClothes(myPart,		
				myGenus,
				mySex,
				age,
				myVariant);

	CreateGalleryUponSerialisation(false);
}

BodyPart::~BodyPart()
{
	if(myOverlay)
	{
		delete myOverlay;
		myOverlay = NULL;
	}
}

// assume that when we reload the only change will be in the age
void BodyPart::Reload(
					  FilePath const &gallery,		// gallery containing images
					  uint32 baseimage,
					 int age)
{

	myLastAge = age;
	Unlink();

	CreateClothes(myPart,		
					myGenus,
					mySex,
					age,
					myVariant);

	EntityImage::ReloadGallery(gallery,baseimage);


	Link(true);
}

bool BodyPart::SetOverlay(int32 set, int32 layer)
{

	// if we are forcing the clothes to layer 0 then
	// override all other sprites

	if(layer == 0 || layer == -1)
		{
			if(set != -1 )
			{
			myOverlayShouldOverrideTheBaseBodySprite = true;
			layer = 1;
			}
			else  
			{
			myOverlayShouldOverrideTheBaseBodySprite = false;
			}
			
		}

	bool clothesToWear = false;

	// if one limb has clothes they all do

	if(myOverlay)
	{
	clothesToWear = myOverlay->SetOverlay(set,myView,layer);
	}

	return clothesToWear;

}

int BodyPart::GetOverlay(int32 layer /*=-1*/)
{
	return myOverlay->GetOverlay(layer);
}

void BodyPart::CreateClothes( int32 part,			// part number
		   int32 genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
		   int32 sex,				// IDEAL Sex to look for (MALE/FEMALE)
		   int32 age,				// IDEAL age & variant to look for
		   int32 variant)
{
	// if we already have some clothes then just reinitialize them
	if(myOverlay)
	{
		myOverlay->ReloadOverlay(part,
								genus,	// Genus (NORN, GRENDEL, ETTIN SIDE)
								sex,	// IDEAL Sex to look for (MALE/FEMALE)
								age,	// IDEAL age & variant to look for
								variant);
	}
	else
	{
	myOverlay = new BodyPartOverlay(this,
							part,			// part number
							genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
							sex,				// IDEAL Sex to look for (MALE/FEMALE)
							age,				// IDEAL age & variant to look for
							variant);

	}
}


void BodyPart::SetBodyPartFileName(int part,
								   int genus,
								   int sex,
								   int age,
								   int variant)
{
	uint32 fsp = ValidFsp(part,
									genus,
									sex,
									age,
									variant,
									"C16",
									IMAGES_DIR);
	if(fsp)
	{
	
		myBodyPartFileName = BuildFsp(fsp, "");

		int x = myBodyPartFileName.find_last_of(".");
		std::string ext = myBodyPartFileName.substr(0, x);

		myBodyPartFileName = ext;
	}
}

void BodyPart::GetBodyPartFileName(std::string& name)
{
	// if there is an overlay in use send that
	// otherwise send the normal bodypart file
		name = myBodyPartFileName;

}







// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool BodyPart::Write(CreaturesArchive &archive) const
	{

    // call base class function first
    EntityImage::Write( archive );

	archive << myAngle << myView;
	archive << 	myPart << myGenus <<
	mySex << myVariant << myLastAge;

	archive << myOverlayShouldOverrideTheBaseBodySprite;
	archive << 	myOverlay;
	archive << myBodyPartFileName;
	archive << myBodyPartType;
	archive << myMirroredFlag;

	return true;
	}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool BodyPart::Read(CreaturesArchive &archive)
{
	// Check version
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
	    if(!EntityImage::Read( archive ))
			return false;

		archive >> myAngle >> myView;
		archive >> 	myPart >> myGenus >>
		mySex >> myVariant >> myLastAge;
		archive >> myOverlayShouldOverrideTheBaseBodySprite;

		archive >> 	myOverlay;
		archive >> myBodyPartFileName;
	
		archive >> myBodyPartType;
		archive >> myMirroredFlag;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	// success
	CreateClothes(myPart,		
					myGenus,
					mySex,
					myLastAge,
					myVariant);

	return true;
}




// set info needed for plotting BodyPart entities:
// Given direction of whole body, calc the correct sprite view for this
// body part from its angle & store in View, then calculate a pointer to
// the correct image for this view.

void BodyPart::SetViewAndImage(int Direction)
{
	// assume that we are not mirroring.  We only mirror west (or left) facing
	// sprites for right facing sprites for certain body part that it looks ok for.
	// We have a separate account of the actual
	// view so that skeleton can get the correct attachment data.

	// note also that we don't mirror clothing as they are usually drawn with 
	// differently for east and west poses patterning etc.
	DrawMirrored(false);
	// an item of clothing can opt to override all other sprites
	// to by pass the layering effect.

	CheckLayeringEffects();
    // calc View (offset into images for this bodypart) from Direction &
    // Angle of limb. NOTE: image order is:-
    // all right-facing angles; all left-facing angles; 1 facing camera;
    // 1 facing away from camera (north)
	int tempview = 0;
    switch (Direction) {

    case EAST:
        myView = myAngle;
		myViewForMirroring = myView;
        break;

    case WEST:
		DrawMirrored(true);
        myView = myAngle+MAXANGLES;
		myViewForMirroring = myAngle;
        break;

    case SOUTH:
        myView = myAngle+ (2*MAXANGLES);//MAXANGLES*2;//myAngle+ (2*MAXANGLES);
		myViewForMirroring = myView;
        break;

    default:
        myView = myAngle+(3*MAXANGLES);//MAXANGLES*2+1;//myAngle+(3*MAXANGLES);
		myViewForMirroring = myView;
        break;
    }

//*************************************************************************
    // Heads are allowed 4 special 'angles': facing camera & facing away.
    // If limb angle is greater than normal 4 allowed, it must be a
    // head facing to/from camera...
//*************************************************************************
	// but make sure that you can't have body facing away and head towards 
	// the camera  and vice versa!!!
    if (myAngle >= MAXANGLES) 
    {
		// default is south
		// towards camera say cheese for SOUTH head pose
		myView = MAXANGLES * 2;


		if(Direction == EAST || Direction == WEST)
		{
			// body is facing east or west so we can have the head looking north
			if (myAngle == MAXANGLES + 1 )
			{
				myView = MAXANGLES*3;
			}
		}
		else if(Direction == NORTH)
		{
			// if body is looking away from the camera
			// force head to look that way too
			myView = MAXANGLES*3;
		}
		myViewForMirroring = myView;
    }


	// don't mirror these body parts
	if(myBodyPartType == 	PART_LHUMERUS  || myBodyPartType == PART_LRADIUS ||
	myBodyPartType == PART_RHUMERUS || myBodyPartType == PART_RRADIUS || 
	myBodyPartType ==PART_LEFT_EAR ||
	myBodyPartType == PART_RIGHT_EAR || !myMirroredFlag)
	{
		DrawMirrored(false);
		SetCurrentIndexFromBase(myView);
	}
	else
	{
	// assume from base index
	SetCurrentIndexFromBase(myViewForMirroring);
	}

	// note that we do not mirror overlays because in clothing the two directions
	// may have different patterning
	if(myOverlay && myOverlay->UpdateOverlay(myView))
			DrawMirrored(false);

}


void BodyPart::CheckLayeringEffects()
{
	// an item of clothing can opt to override all other sprites
	// to by pass the layering effect.
	if(myOverlayShouldOverrideTheBaseBodySprite)
	{
		HideCurrentBitmap();
	}
	else
	{
		ShowCurrentBitmap();
	}
}





            ////////////////////////////////////////////////////
            // Limb class: used for singly-linked parts of a  //
            // CompositeObject. Inherited from BodyPart       //
            ////////////////////////////////////////////////////


// Construct a Limb & initialise it. Register it in EntityLib[]

Limb::Limb(
           Limb* nextlimb,          				  // daughter limb
           LimbData* type,          				  // type of limb
           FilePath const& gallery,
			uint32 baseimage,
		   int32 numimages,
		    int32 part,			// part number
		int32 genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
		  int32 sex,				// IDEAL Sex to look for (MALE/FEMALE)
	  int32 age,				// IDEAL age & variant to look for
	  int32 variant,
	  int32 bodyPartType,
	  bool mirror):// image set
BodyPart(0, //plane
         0,//worldx
         0,//worldy
		gallery,		// gallery containing images
		numimages,
		  baseimage,
	  part,
		genus,
		sex,
		age,
		variant,
		bodyPartType,
		mirror)
{
    myNextLimb=nextlimb; 
	if(type)
    myLimbData=*type;	// take personal copy of attachment table
						// but only if there is one given because Heads
						// like to create them themselves
					
    myAngle = 1;                          // default angle
    SetViewAndImage(EAST);              // init dflt view & index


	CreateClothes(part,		
				genus,
				sex,
				age,
				variant);
}

void Limb::Reload(int32& partNumber,
           LimbData* (*GetLimbDataPtr)( int,			// part number
					   int,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int,				// IDEAL Sex to look for (MALE/FEMALE)
					   int,				// IDEAL age & variant to look for
					   int),  				  // type of limb
           FilePath const& gallery,
			int32 BodySprites[NUMPARTS],
		   int32 numimages,
			int32 age
			)
{

	 myLimbData=*((*GetLimbDataPtr)(partNumber, myGenus, mySex,	age, myVariant));	// take personal copy of attachment table
						// but only if there is one given because Heads
						// like to create them themselves
	
	 BodyPart::Reload(gallery,		// gallery containing images
					  BodySprites[partNumber],
					  age);

	 partNumber++;

	 
	if(myNextLimb)
	 {
		 myNextLimb->Reload(partNumber,GetLimbDataPtr,gallery,BodySprites,numimages,age);
	 }
}


void Limb::PrepareForReloadingGallery()
{
	// what a useless value what'll you do if the gallery
	// isn't removed anyway?
	EntityImage::PrepareForReloadingGallery();

	if(myNextLimb)
	{
		 myNextLimb->PrepareForReloadingGallery();
	}
}



void Limb::SetViewAndImage(int direction)
{
	// let the base class do the work
	BodyPart::SetViewAndImage(direction);

}



// Destructor

Limb::~Limb()
{
    Unlink();                       // remove from EntityLib[]

}


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool Limb::Write(CreaturesArchive &archive) const
	{

	int i;

    // call base class function first
    BodyPart::Write( archive );

    // now do the stuff for our specific class
	for	(i=0; i<MAXVIEWS; i++) 
		archive << myLimbData.StartX[i] << myLimbData.StartY[i] 
		<< myLimbData.EndX[i] << myLimbData.EndY[i];

	
	archive << myNextLimb;		// walk down whole chain of limbs, serialising them


	return true;
	}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Limb::Read(CreaturesArchive &archive)
{
	// Check version info
	int32 version = archive.GetFileVersion();
	int i =0;

	if ( version >= 3 )
	{

		// call base class function first
		if(!BodyPart::Read( archive ))
			return false;

		// now do the stuff for our specific class

		for	(i=0; i<MAXVIEWS; i++) 
			archive >> myLimbData.StartX[i]
			>> myLimbData.StartY[i] 
			>> myLimbData.EndX[i] >> myLimbData.EndY[i];

		archive >> myNextLimb;		// reinstate whole chain of limbs

	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}





bool Limb::SetClothing(int32 set, int32 layer)
{   

	// if the first limb in the chain is clothed
	// then they all are
	return BodyPart::SetOverlay(set,layer);

	// do not do the limb chain as they can be set individually
}




            /////////////////////////////////////////////////////////
            // Body class: inherited from BodyPart. Represents the //
            // body of a CompositeObject, from which chains of     //
            // Limbs emerge. Friend of Limb class.                 //
            /////////////////////////////////////////////////////////


// Construct a body & initialise it. Include it in EntityLib[]


Body::Body(
           BodyData* type,          					// BodyData struct for body type
           FilePath const& gallery,
			uint32 baseimage,
		   int32 numimages,           	// image set
           int32 plane,
		   int32 part,			// part number
		int32 genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
		  int32 sex,				// IDEAL Sex to look for (MALE/FEMALE)
	  int32 age,				// IDEAL age & variant to look for
	  int32 variant,
	  int32 bodyPartType,
	  bool mirror):               					// abs plane for body image
BodyPart(0,
			0,
			0,
		gallery,
			numimages,
		 baseimage,
		 	part,
			genus,
			sex,
			age,
			variant,
			bodyPartType,
			mirror),
		 myPregnancyStage(0)
{
    myBodyData = *type;// take personal copy of attachment table
	myCurrentBodyData= &myBodyData; // start off pointing at your own body data

	SetPlane(plane);


}

void Body::Reload(  BodyData* type,        // BodyData struct for body type
					FilePath const& gallery,
					uint32 baseimage,
					int32 age)
{
	myBodyData = *type;// take personal copy of attachment table
	myCurrentBodyData= &myBodyData; // start off pointing at your own body data


	 BodyPart::Reload(gallery,		// gallery containing images
				  baseimage,
				  age);

}


void Body::SetViewAndImage(int direction)
{
	// let the base class do the work
	BodyPart::SetViewAndImage(direction);


	// if you are pregnant then you can't wear clothes
	// pregnancy stage zero is the normal sprite anyway
	if(myPregnancyStage > 0)
	{
		int setsize = 16;
		// now set the pregnancy stage
		if(myMirroredFlag)
		{
			SetCurrentIndexFromBase((myPregnancyStage*setsize)+myViewForMirroring);
		}
		else
		{
			SetCurrentIndexFromBase((myPregnancyStage*setsize)+myView);
		}
	}

}


bool Body::SetClothing(int32 set,int32 layer)
{   


	return 	BodyPart::SetOverlay(set,layer);
}


void Body::SetPregnancyStage(float progesteroneLevel)
{
	myPregnancyStage = Map::FastFloatToInteger(progesteroneLevel*(float)(NUMBER_OF_PREGNANCY_SPRITES-1));
}

// destruct a body & remove it from EntityLib[]
Body::~Body()
{

    Unlink();                               // remove obj from ObjectLib[]

	
}



// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool Body::Write(CreaturesArchive &archive) const
	{

	int i,j;

    // call base class function first
    BodyPart::Write( archive );

	archive << myPregnancyStage;

    // now do the stuff for our specific class
	for	(i=0; i<MAX_BODY_LIMBS; i++) 
		for	(j=0; j<MAXVIEWS; j++) 
			archive << myBodyData.JoinX[i][j] << myBodyData.JoinY[i][j];

	return true;
	}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Body::Read(CreaturesArchive &archive)
{
	// Check version info
	int32 version = archive.GetFileVersion();
	int i,j;

	if ( version >= 3 )
	{
		// call base class function first
		if(!BodyPart::Read( archive ))
			return false;

		archive >> myPregnancyStage;

		for	(i=0; i<MAX_BODY_LIMBS; i++) 
			for	(j=0; j<MAXVIEWS; j++) 
				archive >> myBodyData.JoinX[i][j] >> myBodyData.JoinY[i][j];

	}
	else
	{
		_ASSERT(false);
		return false;
	}

return true;
}



