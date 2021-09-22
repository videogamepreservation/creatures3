
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "CreatureHead.h"
#include "Skeleton.h"
#include "../General.h"
#include "../App.h"
#include "../Display/MainCamera.h"
#include "BodyPartOverlay.h"
#include <fstream>

CREATURES_IMPLEMENT_SERIAL( CreatureHead )

CreatureHead::CreatureHead(Limb* nextlimb,          				  // daughter limb
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
			 int32 bodyPartType,
			bool mirror
		   )
		   :Limb(nextlimb,          				  // daughter limb
			NULL,          // type of limb will be worked out by me later
			gallery,
			BodySprites[PART_HEAD],
		   numimages,
		   Part,
		   Genus,
		   Sex,
		   Age,
		   Variant,
		   bodyPartType,
		   mirror),
		   myCurrentDirection(0),
		   myHeadSpritesCount(HeadSpritesCount)
		   
{
	GetHeadData(  Part,			// part number
				   Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
				   Sex,				// IDEAL Sex to look for (MALE/FEMALE)
				   Age,				// IDEAL age & variant to look for
				   Variant);



	for (int32 i=0; i<MAX_HEAD_LIMBS; i++)      // set all Appendages to NULL
		myLimbs[i] = NULL;           // in case nowt gets attached


	// left ear
	Limb* Root = new Limb(
        NULL,
        (*GetLimbDataPtr)(PART_LEFT_EAR, Genus, Sex,	Age, Variant),
         gallery,
        BodySprites[PART_LEFT_EAR],
		numimages,
		PART_LEFT_EAR,
		Genus,
		Sex,
		Age,
		Variant,
		PART_LEFT_EAR,
		mirror);


	AttachLimbChain(HEAD_LIMB_LEFT_EAR,Root);

	// right ear
	Root = new Limb(
        NULL,
        (*GetLimbDataPtr)(PART_RIGHT_EAR, Genus, Sex,	Age, Variant),
         gallery,
        BodySprites[PART_RIGHT_EAR],
		numimages,
		PART_RIGHT_EAR,
		Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
	   Sex,				// IDEAL Sex to look for (MALE/FEMALE)
	   Age,				// IDEAL age & variant to look for
	   Variant,
	   PART_RIGHT_EAR,
	   mirror);

	
	AttachLimbChain(HEAD_LIMB_RIGHT_EAR,Root);

	// hair
	Root = new Limb(
        NULL,
        (*GetLimbDataPtr)(PART_HAIR, Genus, Sex,	Age, Variant),
         gallery,
        BodySprites[PART_HAIR],
		numimages,
		PART_HAIR,
		Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
	   Sex,				// IDEAL Sex to look for (MALE/FEMALE)
	   Age,				// IDEAL age & variant to look for
	   Variant,
	   PART_HAIR,
	   mirror);


	AttachLimbChain(HEAD_LIMB_HAIR,Root);

	myHairState = TIDY_HAIR;

}


void CreatureHead::Reload(int32 age,
					  FilePath const& gallery,
					  	   	LimbData* (*GetLimbDataPtr)( int,			// part number
					   int,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int,				// IDEAL Sex to look for (MALE/FEMALE)
					   int,				// IDEAL age & variant to look for
					   int),
					   int32 BodySprites[NUMPARTS])
{
	BodyPart::Reload(gallery,		// gallery containing images
					  BodySprites[PART_HEAD],
					  age);

	GetHeadData( myPart,			// part number
			   myGenus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
			   mySex,				// IDEAL Sex to look for (MALE/FEMALE)
			   age,				// IDEAL age & variant to look for
			   myVariant);


	int32 partNumber = PART_LEFT_EAR;
	for (int32 i=0; i<MAX_HEAD_LIMBS; i++)
	{
		myLimbs[i]->Reload(partNumber,
			GetLimbDataPtr,
		 gallery,
        BodySprites,
		0,
		age
		);           // in case nowt gets attached
	}


}

CreatureHead::~CreatureHead()
{
	int i;
    Limb *l,*p;

    for (i=0; i<MAX_HEAD_LIMBS; i++) {            // delete all limb chains...
        while ((l=myLimbs[i])!=NULL) {    // while some chain left
            p = NULL;                          // (used for NULLing ptrs)
            while (l->GetNextLimb()) {           // find its last element
                p = l;                      // & next to last
                l = l->GetNextLimb();
            }
            if (p)                          // annul pointer to this element
                p->SetNextLimb(NULL);            // (within limb itself...
            else
                myLimbs[i] = NULL;             // ...or within Body)
            delete l;                       // & remove last element
        }                                   // find new last element & repeat
    }                
}

// attach a chain of Limbs to this joint
void CreatureHead::AttachLimbChain(int32 position,Limb* limbChain)
{
	myLimbs[position] = limbChain;
}


void CreatureHead::UpdatePosn(uint8 currentDirection,
							  int32 Expression,
							  int32 Eyes,
							  uint8& earSet)
{
   myCurrentDirection = currentDirection;
    int32 i,OffsetX,OffsetY;
    Limb* currentLimb;
    LimbData* currentLimbData;
    int32 Lpv;

	//*********************!!!!!!!!!!***************************
	// temporary bodge to get the head direction - won't need to
	// when we get the new pose strings since all parts can
	// now have 4 angles in each direction
	//*********************!!!!!!!!!!***************************

	if(myView >= MAXANGLES*2) 
		myCurrentDirection = 1;

	if(myView >= MAXANGLES*3)
		myCurrentDirection = 0;


    // attach limbs on the head

    // Body's entity WorldX/Y now contain abs xy of topleft of body.
    // Now propagate this through to all dependent limbs...
    for (i=0; i<MAX_HEAD_LIMBS; i++) 
    {    // for each limb chain
        OffsetX = GetWorldX() + myHeadData.JoinX[i][myView]; // start point of 1st
        OffsetY = GetWorldY() + myHeadData.JoinY[i][myView]; // limb in chain

        for (currentLimb = myLimbs[i];                     // descend limb chain
             currentLimb!=NULL;
             currentLimb = currentLimb->GetNextLimb()) 
        {

            currentLimbData = currentLimb->GetLimbData();                        // ptr to data fr limb type
            Lpv = currentLimb->GetView();                         // temp store fr View

            // Move limb to correct position.
            currentLimb->SetPosition(OffsetX - currentLimbData->StartX[Lpv], OffsetY - currentLimbData->StartY[Lpv]);

            // get start point of next limb
            OffsetX += (currentLimbData->EndX[Lpv] - currentLimbData->StartX[Lpv]);          
            OffsetY += (currentLimbData->EndY[Lpv] - currentLimbData->StartY[Lpv]);  
        }
    }


}

Vector2D CreatureHead::GetMouthMapPosition()
{
	Vector2D retcode;
	retcode.x = myHeadData.JoinX[MOUTH_ATTACHMENT_POINT][myView] +
				GetWorldX();

	retcode.y = myHeadData.JoinY[MOUTH_ATTACHMENT_POINT][myView] + 
				GetWorldY();
	return retcode;
}

//Read an attachment table file.  The file is an exact binary list of items in the order
// that would be used for initialisation of a HeadData struct and a LimbData.
void CreatureHead::GetHeadData( int32 Part,			// part number
					   int32 Genus,			// Genus (NORN, GRENDEL, ETTIN SIDE)
					   int32 Sex,				// IDEAL Sex to look for (MALE/FEMALE)
					   int32 Age,				// IDEAL age & variant to look for
					   int32 Variant)

{
	
	DWORD Fsp;
	int32 i,j,x,y;

	Fsp = ValidFsp(Part,Genus,Sex,Age,Variant,			// find most appropriate file
					BODY_DATA_EXTENSION,BODY_DATA_DIR);

    if (Fsp)
    {
	    std::ifstream in(BuildFsp(Fsp,BODY_DATA_EXTENSION,BODY_DATA_DIR));
	    for	(j=0; j<MAXVIEWS; j++) 
        {	
				// the 1st set is the start of the limb data
			    in >> x >> y;
				myLimbData.StartX[j]=x;
				myLimbData.StartY[j]=y; 

				// the 2nd set is the mouth which
				// I (Daniel) will store because it's useful - orright?
				in >> x >> y;
				myHeadData.JoinX[MOUTH_ATTACHMENT_POINT][j] = x;
				myHeadData.JoinY[MOUTH_ATTACHMENT_POINT][j] = y;

				// the 3rd set is the left ear
				 in >> x >> y;
			    myHeadData.JoinX[HEAD_LIMB_LEFT_EAR][j] = x;
			    myHeadData.JoinY[HEAD_LIMB_LEFT_EAR][j] = y;

				// the 4th set is the right ear
				in >> x >> y;
				myHeadData.JoinX[HEAD_LIMB_RIGHT_EAR][j] = x;
			    myHeadData.JoinY[HEAD_LIMB_RIGHT_EAR][j] = y;
			

				// the fifth set is both the hair and the limbdata end
				in >> x >> y;
				myHeadData.JoinX[HEAD_LIMB_HAIR][j] = x;
			    myHeadData.JoinY[HEAD_LIMB_HAIR][j] = y;

		
				myLimbData.EndX[j]=x;
				myLimbData.EndY[j]=y;
		     
	    }		
    }
    else
    {
	    for	(j=0; j<MAXVIEWS; j++) 
        {
		    for	(i=0; i<MAX_HEAD_LIMBS; i++) 
			    myHeadData.JoinX[i][j] = myHeadData.JoinY[i][j] = 0;
	    }	
		
		myLimbData.StartX[0]=0;
		myLimbData.StartY[0]=0; 
    }										// data remains valid till next call here
}


void CreatureHead::SetCurrentPoseString(int direction,
										 int32 Expression,
										  int32 Eyes,
										  uint8& earSet)
{
    int32 i,j;
    Limb* currentLimb;

    static int ChainLength[MAX_HEAD_LIMBS] = {            // max # entries for
        1,          //  left ear                        // each limb chain
        1,          // right ear
        1,          // hair
        };         // tail

	
   // This is easy all head limbs must be the same angle
	// and direction as the head

    for (i=0; i<MAX_HEAD_LIMBS; i++) {                    // for each limb chain...

        currentLimb = myLimbs[i];                          // ptr to 1st limb in ch

        for (j=0;
             (j<ChainLength[i])&&(currentLimb);
             j++) {                                 // fr each valid element...
                currentLimb->SetAngle(myAngle);                // ...set angle
			 currentLimb->SetViewAndImage(direction);
             currentLimb = currentLimb->GetNextLimb();                     // & move down chain
        }
    }

	SetHeadImage(Expression,Eyes,earSet);
}


/*********************************************************************
* Public: SetHeadImage.
* Adjust image for heads, which have special extra poses
* for mood (0=nor 1=happy 2=sad 3=angry 4=surprised 5=sleepy) and 
* blinking (0=closed 1=open).
* TO DO: UPDATE EXPRESSIONS
*********************************************************************/
void CreatureHead::SetHeadImage(int32 Expression,
								int32 Eyes, 
								uint8& currentEarSet)
{

	 // if we are wearing a body suit through which the head should not show
	 	// an item of clothing can opt to override all other sprites
	// to by pass the layering effect.

	CheckLayeringEffects();

	// carry on and do facial expression stuff
	// because other things may actually be displaying the
	// head eg the norn bubble in CAV
    ASSERT(Eyes == 0 || Eyes == 1);
	int32 earSet = EARS_NORMAL;
	currentEarSet = EARS_NORMAL;
  //  const int nSetSize = 20;
	 int nSetSize =32;// number of heads in each expression
	if(Expression == EXPR_NORMAL && Eyes == 1)
	{
		if (myView < 0 || myView >= myHeadSpritesCount)
			throw BasicException(ErrorMessageHandler::Format("creature_head_sprite_out_of_range", 0, "CreatureHead::SetHeadImage", myView, myHeadSpritesCount).c_str());
		SetCurrentIndex(myView);

	}
	else
	{
		int32 nExpressionOffset = Expression * nSetSize;
		int32 nEyeOffset = (1 - Eyes) * (nSetSize / 2);
		int32 newView = nExpressionOffset+nEyeOffset+myView;
		if (newView < 0 || newView >= myHeadSpritesCount)
			throw BasicException(ErrorMessageHandler::Format("creature_head_sprite_out_of_range", 1, "CreatureHead::SetHeadImage",
			newView, myHeadSpritesCount, nExpressionOffset, Expression, nSetSize, Eyes, nEyeOffset, myView).c_str());
		SetCurrentIndex(newView);
	}


	// now set the image for your ears
	switch(Expression)
	{

	case EXPR_NORMAL:
		{
			earSet = EARS_ANGRY;
			break;
		}
	case EXPR_HAPPY:
		{
			earSet = EARS_ANGRY;
				break;
		}
	case EXPR_SAD:
		{
			earSet = EARS_DROOPY;
				break;
		}
	case EXPR_ANGRY:
		{
			earSet = EARS_ANGRY;
				break;
		}
	case EXPR_SURPRISE:
		{
			earSet = EARS_PRICKED;
				break;
		}
    case EXPR_SLEEPY:
		{
			earSet = EARS_DROOPY;
			break;
		}
/*	case EXPR_VERY_SLEEPY:
		{
			earSet = EARS_DROOPY;
			break;
		}
	case EXPR_VERY_HAPPY:
		{
			earSet = EARS_PRICKED;
			break;
		}
	case EXPR_MISCHEVIOUS:
		{
		earSet = EARS_NORMAL;
		break;
		}
	case EXPR_SCARED:
		{
			earSet = EARS_DROOPY;
			break;
		}
	case EXPR_ILL:
		{
			earSet = EARS_DROOPY;
			break;
		}
	case EXPR_HUNGRY:
		{
		earSet = EARS_NORMAL;
		break;
		}
*/	default:
		{
		earSet = EARS_ANGRY;
		break;
		}
	}

	nSetSize = 16; // only 16 ears in the set
	// ears are no longer overlays
	int32 nExpressionOffset = earSet * nSetSize;
	currentEarSet = earSet;

	// do not mirror ears as it doesn't work as they are not just flipped left to right
	// but drawn droopy etc!!
	myLimbs[HEAD_LIMB_LEFT_EAR]->DrawMirrored(false);
	myLimbs[HEAD_LIMB_LEFT_EAR]->SetCurrentIndexFromBase(nExpressionOffset+myView);//layer zero
	myLimbs[HEAD_LIMB_RIGHT_EAR]->SetCurrentIndexFromBase(nExpressionOffset+myView);//layer zero
	myLimbs[HEAD_LIMB_RIGHT_EAR]->DrawMirrored(false);
	// now work out the hair state
	nExpressionOffset = myHairState*nSetSize;
	myLimbs[HEAD_LIMB_HAIR]->SetCurrentIndexFromBase(nExpressionOffset+myViewForMirroring);

	// note that we do not mirror overlays because in clothing the two directions
	// may have different patterning
	if(myOverlay && myOverlay->UpdateOverlay(myView))
			DrawMirrored(false);
}


int CreatureHead::GetOverlayIndex(int bodypart)
{
	
	int index = -1;
	if(bodypart < MAX_HEAD_LIMBS)
	{
		Limb* limb = myLimbs[bodypart];
			return limb->GetCurrentIndex();
	
	}

	return index;

}

int CreatureHead::GetClothingSet(int bodypart , int layer)
{
	
	int index = -1;
	if(bodypart < MAX_HEAD_LIMBS)
	{
		Limb* limb = myLimbs[bodypart];
			return limb->GetOverlay(layer);
	
	}

	return index;

}

bool CreatureHead::WearOutfit(int set, int layer)
{
	bool clothesYes = false;

	for(int i =0; i< MAX_HEAD_LIMBS; i++)
	{
		Limb* limb = myLimbs[i];
		clothesYes =	limb->SetClothing(set, layer);
	}
	
	return clothesYes;
	
}

bool CreatureHead::WearItem(int bodypart, int set , int layer)
{
	bool clothesYes = false;

	if(bodypart < MAX_HEAD_LIMBS)
	{
		Limb* limb = myLimbs[bodypart];
		limb->SetClothing(set, layer);
		return true;
	}
	return false;
	
}

void CreatureHead::GetHeadPartFileName(int headpart, std::string& path)
{
	if(headpart < MAX_HEAD_LIMBS)
	{
		Limb* limb = myLimbs[headpart];
		limb->GetBodyPartFileName(path);

	}
}

void CreatureHead::RuffleHair()
{
	switch(myHairState)
	{
	case (TIDY_HAIR): myHairState = NOT_SO_TIDY_HAIR;
		break;
	case (NOT_SO_TIDY_HAIR):myHairState = MESSY_HAIR;
		break;
	default:
		break;
	}
}

void CreatureHead::TidyHair()
{
	switch(myHairState)
	{
	case (NOT_SO_TIDY_HAIR): myHairState = TIDY_HAIR;
		break;
	case (MESSY_HAIR):myHairState = NOT_SO_TIDY_HAIR;
		break;
	default:
		break;
	}
}

void CreatureHead::PrepareForReloadingGallery()
{
	EntityImage::PrepareForReloadingGallery();
	for(int32 i = 0; i < MAX_HEAD_LIMBS; i++)
	{
		myLimbs[i]->PrepareForReloadingGallery();
	}
}

bool CreatureHead::Write(CreaturesArchive& archive) const
{
	Limb::Write(archive);
	int i;
	for( i = 0; i < MAX_HEAD_LIMBS; ++i )
	{
		archive << myLimbs[i];
	}

	archive << myCurrentDirection;
	archive << myHairState;
	
	archive << myHeadSpritesCount;

	return true;
}

bool CreatureHead::Read(CreaturesArchive& archive)
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		int i;

		if(!Limb::Read(archive))
			return false;

		for( i = 0; i < MAX_HEAD_LIMBS; ++i )
		{
			archive >> myLimbs[i];
		}
		
		archive >> myCurrentDirection;
		archive >> i;
		myHairState = HairStates(i);

		archive >> myHeadSpritesCount;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}

bool CreatureHead::Visibility(int scope)
{
	// return true if any part of the creature is on screen
	Limb* currentLimb = NULL;

	for (int i=0; i<MAX_HEAD_LIMBS; i++) 
	{                    // for each limb chain

        for (currentLimb = myLimbs[i];                     // descend limb chain
             currentLimb!=NULL;
             currentLimb = currentLimb->GetNextLimb()) 
			 {
				 if(currentLimb->Visible(scope))
					 return true;
			 }
	}
	return false;
}
