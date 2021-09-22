// --------------------------------------------------------------------------
// Filename:	CreatureHead.h
// Class:		CreatureHead
// Purpose:		This class allows the head of a creature to be a limb but also
//				to possess limbs that are positioned in relation to it much the
//				 same as body works.  The head currently has hai and two ears.
//				
//			
//				
//
// Description: The head attaches its limbs in the same was as the skeleton
//				 attaches limbs to the body therefore the head has a set of
//				 appendages
//			
//			
//
// History:
// ------- 
// 6Mar99	Alima		Created
// --------------------------------------------------------------------------

#ifndef CREATURE_HEAD_H
#define CREATURE_HEAD_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/Vector2D.h"
#include "../Entity.h"
#include "SkeletonConstants.h"

// be careful here because the tip of head and the hair share the same attachment
const int MAX_HEAD_LIMBS = 3;

// Indices for each limb chain
enum {
	HEAD_LIMB_LEFT_EAR=0,
	HEAD_LIMB_RIGHT_EAR,
	HEAD_LIMB_HAIR
};  

const int MOUTH_ATTACHMENT_POINT = MAX_HEAD_LIMBS;

class CreatureHead : public Limb
{
	CREATURES_DECLARE_SERIAL( CreatureHead )

public:
	CreatureHead(){;}
	
	CreatureHead(
           Limb* nextlimb,          				  // daughter limb
           FilePath const& gallery,
			 int32 BodySprites[NUMPARTS],
		   int32 numimages,
		   int32 Part,			// part number
		  int32 Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
		  int32 Sex,				// IDEAL Sex to look for (MALE/FEMALE)
		  int32 Age,				// IDEAL age & variant to look for
		  int32 Variant,
		  	LimbData* (*GetLimbDataPtr)( int,			// part number
					   int,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int,				// IDEAL Sex to look for (MALE/FEMALE)
					   int,				// IDEAL age & variant to look for
					   int),
		  int32 HeadSpritesCount,
		  int32 partBodyType,
		  bool mirror);

		void Reload(int32 age,
					FilePath const& gallery,
					LimbData* (*GetLimbDataPtr)( int,			// part number
					   int,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int,				// IDEAL Sex to look for (MALE/FEMALE)
					   int,				// IDEAL age & variant to look for
					   int),
					    int32 BodySprites[NUMPARTS]);

	~CreatureHead();
		
	void AttachLimbChain(int32 position,Limb* limbChain);	// attach a chain of Limbs to
											// this joint (after constructn)

	void UpdatePosn(uint8 currentDirection,
							  int32 Expression,
							  int32 Eyes,
							  uint8& earSet);

	void GetHeadData( int32 Part,			// part number
					   int32 Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int32 Sex,				// IDEAL Sex to look for (MALE/FEMALE)
					   int32 Age,				// IDEAL age & variant to look for
					   int32 Variant);

	void SetCurrentPoseString(int direction, int32 Expression,
										  int32 Eyes,
										  uint8& earSet);

	void SetHeadImage(int32 Expression, int32 Eyes, 
								uint8& currentEarSet);
	int GetOverlayIndex(int bodypart);

	Vector2D GetMouthMapPosition();

	int GetClothingSet(int bodypart , int layer);
	//bool SetOverlayIndex(int bodypart,int set,int layer);
	bool WearOutfit(int set, int layer);
	bool WearItem(int part, int set, int layer );

	void GetHeadPartFileName(int headpart, std::string& path);

	enum HairStates
	{
		TIDY_HAIR =0,
		NOT_SO_TIDY_HAIR,
		MESSY_HAIR
	};

	void RuffleHair();
	void TidyHair();

	virtual void PrepareForReloadingGallery();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	bool Visibility(int scope);

	Limb* GetLimb(int index) { return myLimbs[index]; }

	int GetCurrentDirection() { return myCurrentDirection; }
private:

	struct HeadData {
		byte	JoinX[MAX_HEAD_LIMBS+1][MAXVIEWS];	// xy of point of attachment for each
		byte	JoinY[MAX_HEAD_LIMBS+1][MAXVIEWS];	// limb at every view
	};

	CreatureHead (const CreatureHead&);
	CreatureHead& operator= (const CreatureHead&);

	Limb* myLimbs[MAX_HEAD_LIMBS];
	HeadData myHeadData;

	uint8 myCurrentDirection;
	HairStates myHairState;

	int32 myHeadSpritesCount;
};
#endif //CREATURE_HEAD
