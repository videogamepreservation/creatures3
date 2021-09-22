// --------------------------------------------------------------------------
// Filename:	CreatureGallery.h
// Class:		CreatureGallery
// Purpose:		This class handles the images of all the creatures in the 
//				system. 
//				All Creature data is stored in one large database.
//				
//				
//				
//
// Description: 
//				A 16MB file is allocated for all creatures.  The file has 
//				a header and slots for creatures precedeed by a key 
//				(the creature's moniker) and an offset to the next creature
//				 data.  The CreatureGallery is a linked list of creature data.
//				When a new creature file is requested the file is created
//				and then slotted into the next free slot (where the key is null).
//				When a creature must be deleted all we need do is set the key
//				to null.
//
//				There should only be one instance of the Creature file
//				 		
//
//
//
// History:
// -------  
// 14Jan99	Alima		Created
//					
// --------------------------------------------------------------------------

#ifndef		CREATUREGALLERY_H
#define		CREATUREGALLERY_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../Creature/Definitions.h"
#include	"MemoryMappedFile.h"
#include	"Gallery.h"
#include	"../../common/BasicException.h"
#include "CompressedGallery.h"

class Gallery;

class CreatureGallery
{

public:



// ----------------------------------------------------------------------
// Method:      Constructor 
// Arguments:   None
//				
// Returns:     None
//
// Description: Creates a Creature Gallery	
//						
// ----------------------------------------------------------------------
	CreatureGallery();

	// one creature gallery per creature
	CreatureGallery(int32 sizeOfSpriteSet,std::string name);

	virtual ~CreatureGallery();


// ----------------------------------------------------------------------
// Method:		AddCreature
// Arguments:	moniker - the unique id for a creature by which we identify
//							its genome.  Also used as a key for the 
//						composite creature gallery file.
// Returns:		a pointer to the 
// Description:	Creates a creature gallery getting the correct body parts
//				as defined in the genome.  Then slots the data in the 
//				first free slot.
//				
// ----------------------------------------------------------------------
/* Gallery* AddCreature(uint32 moniker,
				  int32 PVariantGenus[NUMPARTS],
				  int32 PVariant[NUMPARTS],
				  uint32 ValidParts,
				   uint16 ImagesPerPart[NUMPARTS],
				  uint8 Sex,
				  uint8 Age,
				  uint16 tintTable[65536],
				  int32 BodySprites[NUMPARTS]);
*/

Gallery* AddCompressedCreature(std::string moniker,
							  uint32 uniqueID,
							  int32 PVariantGenus[NUMPARTS],
							  int32 PVariant[NUMPARTS],
							  uint32 ValidParts,
							  uint8 Sex,
							  uint8 Age,
							  int32 BodySprites[NUMPARTS],
							  CompressedGallery creatureParts[NUMPARTS],
								int32 numSpritesInFile);

// ----------------------------------------------------------------------
// Method:		RemoveCreature
// Arguments:	moniker - the moniker key of the slot to free
//						
// Returns:		true if the slot was found false otherwise
//				
// Description:	Frees the given creature slot by setting the key to 
//				zero. 
//				
// ----------------------------------------------------------------------
bool RemoveCreature(uint32 uniqueID);
void RemoveAll();

// bool Resize(int newMaxCreatures,int sizeOfASetOfNornSprites);

bool IsComplete(){return myCreatureBuildingStage == STAGE_COMPLETE;}



//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
class CreatureGalleryException: public BasicException
{
public:
	CreatureGalleryException(std::string what, uint32 line):
	  BasicException(what.c_str()),
	lineNumber(line){;}

	uint32 LineNumber(){return lineNumber;}
private:


	uint32 lineNumber;

};

private:

	enum
	{
		STAGE_ONE =0,
		STAGE_TWO,
		STAGE_THREE,
		STAGE_FOUR,
		STAGE_FIVE,
		STAGE_COMPLETE,
	}myCreatureBuildingStage;


	// Copy constructor and assignment operator
	// Declared but not implemented
	CreatureGallery (const CreatureGallery&);
	CreatureGallery& operator= (const CreatureGallery&);

	bool Part1CreatureBuilder( int32 BodySprites[NUMPARTS],
							 CompressedGallery creatureParts[NUMPARTS],
							 int32 numSpritesInFile,
							 uint32 uniqueID,
							 uint32 ValidParts);

	void Part2CreatureBuilder(uint32 ValidParts,int32 BodySprites[NUMPARTS],
									  CompressedGallery creatureParts[NUMPARTS]);
	void Part3CreatureBuilder(  CompressedGallery creatureParts[NUMPARTS],
		 uint32 ValidParts);
	void Part4CreatureBuilder( CompressedGallery creatureParts[NUMPARTS],
		 uint32 ValidParts);

// ----------------------------------------------------------------------
// Method:		FindSlot
// Arguments:	key - either a moniker or NULL to find the first free
//						slot.
// Returns:		a pointer to the data for this slot
// Description:	Creates a creature gallery getting the correct body parts
//				as defined in the genome.  Then slots the data in the 
//				first free slot.
//				
// ----------------------------------------------------------------------
uint32 FindSlot(uint32 key);

void BuildDatabase();

// used once to create the Composite Creature gallery file
void CreateTemplateFile();
	// the start of the data in my memory mapped file
//	unsigned char* myFileData;

	MemoryMappedFile myMemoryMappedFile;

	uint32 myTemplateSize;
	uint32 myCreatureGallerySize;

	int32 myMaxCreatures;
	int32 mySizeOfASetOfNornSprites;

	std::string myName;
	uint32 myCurrentPartNumber;							// # images written to dest so far
	uint32 myRewindToHeaders;
	uint16* myNumImagesHolder;
	uint32 myPart;
	uint32 myNumberOfBytesWritten;

	Gallery* myGallery;

};

#endif // CREATUREGALLERY_H
