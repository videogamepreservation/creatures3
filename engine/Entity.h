// Header file for Entity classes
// Windows version
// --------------------------------------------------------------------------
// Filename:	Entity.h
// Class:		Entity, Body, Limb
// Purpose:		See below for full descriptions written previously
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
//							called EntityImage.  
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
// --------------------------------------------------------------------------
#ifndef ENTITY_H
#define ENTITY_H

#pragma hdrstop

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"
#include "PersistentObject.h"
#include "Display/EntityImage.h"
#include "Creature/SkeletonConstants.h"



/* Notes:

	An entity is a 'thing' that exists in the game world (unlike a sprite
	for example, which only exists in the computer). Entities are contained
	within the various types of Object (creatures, machines, etc.); some
	objects may contain several Entities.

	Entities are the things that get redrawn when a rectangle is invalidated.
	A library of pointers to all extant entities is stored in EntityLib[]
	for this purpose.

	Entities are ALWAYS encapsulated in Objects of one kind or another. Thus,
	although each entity contains its own X&Y members, these get set as
	appropriate by the Object which owns them, and don't need to be
	initialised when creating the entity itself. Thus, entities can be
	created dynamically by the objects that contain them.


	Entities:


	Entity				Simplest form of entity, instantiated for use
	|					within SimpleObjects and CompoundObjects.
	|					Also serves as base class for articulated entities,
	| 					for encapsulation within Creatures.
	| 					Whereas a Sprite uniquely represents a given bitmap,
	| 					several Entities can refer to the same Sprite
	| 					- an Entity represents the simplest individual
	| 					object in the world; several Entities may LOOK
	| 					the same, but ARE different.
	|
	|
	BodyPart
	| |
	| Body				Parts of articulated objects. A Limb is connected to
	| 					one or no daughter Limbs (eg. a thigh Limb connects
	| 					to a shin Limb); A Body is similar, but connects
	Limb                to several Limb chains. The BodyPart class contains
						methods & data that are common to both types
						Body & Limb classes contain pointers to BodyData
						or LimbData classes respectively.

						The BodyPart Entities contain pointers to LimbData
						or BodyData structs, which contain the XY
						information that is common to all BodyParts with
						the same appearance.



	All entities are pointed to by EntityLib[]

// 15Jan99	Alima removed Entity it has now been replaced by EntityImage in
//			 full.  That means EntityImage does that link thing with the
//			worlds entityImagelib.
end of notes */



////////////////////////// constants //////////////////////////////////////


// Indices for each body limb chain
enum {
	BODY_LIMB_HEAD,
	BODY_LIMB_LEFT_LEG,
	BODY_LIMB_RIGHT_LEG,
	BODY_LIMB_LEFT_ARM,
	BODY_LIMB_RIGHT_ARM,
	BODY_LIMB_TAIL
};  


const int MAX_BODY_LIMBS = 6;						// # emerging limbs allowed
											// in one Body
const int MAXANGLES = 4;					// # angles each articulated
											// entity (limb/body) has
const int MAXVIEWS = MAXANGLES*4;			// # sprites needed for each limb
							



// moved to a static variable in EntityImage called ourMaxAnimCount
//const int MAXANIM = 99;						// # bytes in an Anim string (incl. terminator)



			////////////////////////////////////////////////////////
			// LimbData struct: contains all XY offset data for a //
			// Limb of a given appearance. Each Limb points to    //
			// the appropriate object of type LimbData.			  //
			////////////////////////////////////////////////////////

struct LimbData {

	byte    StartX[MAXVIEWS];	// offset into sprite of 'start' hotspot
	byte	StartY[MAXVIEWS];
	byte	EndX[MAXVIEWS];
	byte	EndY[MAXVIEWS];		// ditto for 'end' spot for each view

};

			////////////////////////////////////////////////////////
			// BodyData struct: contains all XY offset data for a //
			// Body of a given appearance. Each Body points to    //
			// the appropriate object of type BodyData.			  //
			////////////////////////////////////////////////////////

struct BodyData {

	byte	JoinX[MAX_BODY_LIMBS][MAXVIEWS];	// xy of point of attachment for each
	byte	JoinY[MAX_BODY_LIMBS][MAXVIEWS];	// limb at every view

};





//////////////////////////// class definitions ///////////////////////////





			////////////////////////////////////////////////
			//////////////// ENTITY CLASSES ////////////////
			////////////////////////////////////////////////



// An entity is the most primitive thing that exists in the game world.
// On its own it represents simple, one-pose, unintelligent objs but it
// is inherited by other classes for creating more complex objs
//<snip entity Class>

			///////////////////////////////////////////////////////////
			// BodyPart class: virtual parent to Body & Limb classes //
			///////////////////////////////////////////////////////////

class Skeleton;	// for reference
class BodyPartOverlay;

class BodyPart : public EntityImage 
{
	CREATURES_DECLARE_SERIAL( BodyPart )

protected:
	int32		myAngle;				// current angle (0-MAXANGLES)
	int32		myView;				// offset to correct sprite, given current
								// limb Angle and Dirn of whole Body

	// we mirror east facing (or right) into west facing sprites (left)
	// we need a record of the view we are mirroring and of the actual view for
	// skeleton to get the correct body data for the actual view.
	int32		myViewForMirroring;

	int32		myLastAge;

	int32		myBodyPartType;
	bool		myMirroredFlag;


								
	std::string myBodyPartFileName;

	// detect when you are wearing a full body suit 
	// so that the body suit can override any other sprites
	bool myOverlayShouldOverrideTheBaseBodySprite;

public:

	BodyPart();

	BodyPart(int32  plane,
            int32  worldx,
            int32  worldy,
			FilePath const& gallery,		// gallery containing images
			int32 numimages,
            uint32 baseimage,
			int part,
			int genus,
			int sex,
			int age,
			int variant,
			int bodyPartType,
			bool mirror);

	void Reload(FilePath const &gallery,		// gallery containing images
				uint32 baseimage,
				int age);

	void CheckLayeringEffects();

	virtual ~BodyPart();

    inline int GetView() const
    {   return myView;}

	inline int GetAngle() const
    {   return myAngle;}

	inline void SetAngle(int32 angle)
    {   myAngle = angle;}


	void GetBodyPartFileName(std::string& name);
	void SetBodyPartFileName(int part,
								   int genus,
								   int sex,
								   int age,
								   int variant);

	void CreateClothes(int32 Part,// part number
					   int32 Genus,// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int32 Sex,// IDEAL Sex to look for (MALE/FEMALE)
					   int32 Age,// IDEAL age & variant to look for
					   int32 Variant);

	bool SetOverlay(int32 set, int32 layer);
	int GetOverlay(int32 layer = -1);



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

	virtual void SetViewAndImage(int Direction); // set info needed for plotting:
									// Given direction of whole body, calc
									// the correct image view for this
									// body part from its angle & store
									// in View, then calculate a pointer to
									// the correct image for this view.
								

protected:
	BodyPartOverlay* myOverlay;
	int32 myPart;
	int32 myGenus;
	int32 mySex;
	int32 myVariant;

};








			////////////////////////////////////////////////////
			// Limb class: used for singly-linked parts of a  //
			// CompositeObject. Inherited from BodyPart       //
			////////////////////////////////////////////////////


// chain of limb nodes, each points to one or no other nodes
// root of tree is special class, containing several daughter branches

// note: all tables of EndXYs are in order: east+angle0...east+angle3,
// west+angle0...west+angle3, south, north (same order as sprites)

class CreatureHead;
class LimbOverlay;

class Limb : public BodyPart {

	CREATURES_DECLARE_SERIAL( Limb )

protected:
	Limb*	myNextLimb;			// daughter limb (or NULL if none)
	
	// these are the limb data that refer to my original body
	// part sprites
	LimbData myLimbData;				// my attachment point data

	public:

	//--- constructors ---

	inline Limb* GetNextLimb() {return myNextLimb;}
	inline void SetNextLimb(Limb* limb) {myNextLimb=limb;}

	inline LimbData* GetLimbData() {return &myLimbData;}

	Limb()
	{
	};


	virtual ~Limb();					// destruct a limb & remove it from
								// EntityLib[]

	Limb(Limb* nextlimb,			// daughter limb
		 LimbData* type,			// type of limb
		 FilePath const& gallery,	// image gallery
		 uint32 baseimage,
		 int32 numimages,
		 int32 Part,			// part number
		 int32 Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
		 int32 Sex,				// IDEAL Sex to look for (MALE/FEMALE)
		 int32 Age,				// IDEAL age & variant to look for
		 int32 Variant,
		 int32 bodyPartType,
		 bool mirror);			// image set

	void Reload(int32& partNumber,
		      LimbData* (*GetLimbDataPtr)( int,			// part number
					   int,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int,				// IDEAL Sex to look for (MALE/FEMALE)
					   int,				// IDEAL age & variant to look for
					   int),  				  // type of limb,          				  // type of limb
				FilePath const& gallery,
				int32 BodySprites[NUMPARTS],
				int32 numimages,
				int32 age);

	bool SetClothing(int32 set, int32 layer);


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

	virtual void SetViewAndImage(int Direction);

	virtual void PrepareForReloadingGallery();
	
private:

};


			/////////////////////////////////////////////////////////
			// Body class: inherited from BodyPart. Represents the //
			// body of a CompositeObject, from which chains of     //
			// Limbs emerge.                                       //
			/////////////////////////////////////////////////////////

const int NUMBER_OF_PREGNANCY_SPRITES = 4;

class BodyOverlay;
//class BodySuit;

class Body : public BodyPart {

	CREATURES_DECLARE_SERIAL( Body )

protected:

	BodyData myBodyData;
	BodyData* myCurrentBodyData;
//	BodySuit* myClothes;
	uint32 myPregnancyStage;

public:

	inline BodyData* GetBodyData() {return myCurrentBodyData;}

	Body();							// constr

	virtual ~Body();						// destruct a body & remove it from
									// EntityLib[]

	Body(BodyData* type,			// BodyData struct for body type
		 FilePath const& gallery,	// image gallery
		  uint32 baseimage,			// image# to use
		  	int32 numimages,
		  int32 plane,
		  int32 Part,			// part number
		 int32 Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
		  int32 Sex,				// IDEAL Sex to look for (MALE/FEMALE)
		  int32 Age,				// IDEAL age & variant to look for
		  int32 Variant,
		 int32 bodyPartType,
		 bool mirror);             	// abs plane for body image

	void Reload(  BodyData* type,        // BodyData struct for body type
					FilePath const& gallery,
					uint32 baseimage,
					int32 age				// IDEAL age & variant to look for
					);




	virtual void SetViewAndImage(int Direction);

	bool SetClothing(int32 set, int32 layer);

	void SetPregnancyStage(float progesteroneLevel);


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

};




///////////// inline functions ////////////////

// construct a blank body and include its name in EntityLib[]
// (done by Entity constructor)

inline Body::Body()
{
}



#endif // ENTITY_H
