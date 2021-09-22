// --------------------------------------------------------------------------
// Filename:	BodyPartOverlay.h
// Class:		BodyPartOverlay
// Purpose:		This class allows overlays such as facial expressions
//				and footgear (ok shoes) to be placed on body parts.
//			
//				
//
// Description: There may be a set of overlays that we wish to use to replace
//				body parts.  We can have up to 10 overlays each with their
//				own set of attachments.  Where they share attachments the
//				the same coordinates will appear.
//			
//			
//
// History:
// ------- 
// 6Mar98	Alima			Created
// --------------------------------------------------------------------------

#ifndef BODY_PART_OVERLAY_H
#define BODY_PART_OVERLAY_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../Entity.h"
#include "../PersistentObject.h"

#define NUM_LAYERS 4

class BodyPart;

class BodyPartOverlay : public PersistentObject
{
		CREATURES_DECLARE_SERIAL( BodyPartOverlay )
public:
	BodyPartOverlay();
	
	BodyPartOverlay(BodyPart* bodyPart,
					int32 part,// part number
					int32 genus,// Genus (NORN, GRENDEL, ETTIN SIDE)
					int32 sex,// IDEAL Sex to look for (MALE/FEMALE)
					int32 age,// IDEAL age & variant to look for
					int32 variant);

	~BodyPartOverlay();

	void	Init();

/*
	uint32 ValidFsp(int Part,			// part number
			   int32 Genus,			// Genus (NORN, GRENDEL, ETTIN, SIDE)
			   int32 Sex,				// IDEAL Sex to look for (MALE/FEMALE)
			   int32 Age,				// IDEAL age & variant to look for
			   int32 Variant,
			   char* Ext,			// file extension ("spr" or "att")
			   int32 Dir);				// subdirectory type, eg. BODY_DATA_DIR#*/

/*		DWORD ValidOverlayFsp(int Part,			// part number
			   int Genus,			// Genus (NORN, GRENDEL, ETTIN, SIDE)
			   int Sex,				// IDEAL Sex to look for (MALE/FEMALE)
			   int Age,				// IDEAL age & variant to look for
			   int Variant,
			   char* Ext,			// file extension ("spr" or "att")
			   int Dir);				// subdirectory type, eg. BODY_DATA_DIR

	char* BuildOverlayFsp(DWORD fsp,char* ext, int SubDir  =-1 );*/

	void ReloadOverlay(	int32 part,// part number
					int32 genus,// Genus (NORN, GRENDEL, ETTIN SIDE)
					int32 sex,// IDEAL Sex to look for (MALE/FEMALE)
					int32 age,// IDEAL age & variant to look for
					int32 variant);

	void SetBodyPartFileName(int part,
							 int genus,
							 int sex,
							 	int age,
							 int variant);

	
	void Hide();

	bool SetOverlay(int32 set,int32 direction, int32 layer);
	int GetOverlay(int32 layer = -1);

								

	void GetBodyPartFileName(std::string& name){name =  myBodyPartFileName;}
		
	bool UpdateOverlay(int32 direction);


	// new serialization stuff
	virtual bool Write( CreaturesArchive &ar ) const;
	virtual bool Read( CreaturesArchive &ar );

private:
		// Copy constructor and assignment operator
	// Declared but not implemented
	BodyPartOverlay (const BodyPartOverlay&);
	BodyPartOverlay& operator= (const BodyPartOverlay&);

	std::string myBodyPartFileName;
	
	BodyPart* myBodyPart;

	int32 myClothingSetForEachLayer[NUM_LAYERS];

};

#endif//BODY_PART_OVERLAY