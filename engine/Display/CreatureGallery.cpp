// --------------------------------------------------------------------------
// Filename:	CreatureGallery.cpp
// Class:		CreatureGallery
// Purpose:		This class handles the images of all the creatures in the 
//				system. 
//				All Creature data is stored in one large database.
//				
//				
//				
//
// Description: 
//				A 16MB memory mapped file is allocated for all creatures.
//				  The file has 
//				a header and slots for 18 creatures precedeed by a key 
//				(the creature's unique id) and an offset to the next creature
//				 data.  The CreatureFile is a linked list of creature data.
//				When a new creature file is requested the file is created
//				and then slotted into the next free slot (where the key is null).
//				When a creature must be deleted all we need do is set the key
//				to null.
//
//				There should only be one instance of the Creature file.
//				The slots are for the size of the largest possible creature.
//
//				 		
//
//
//
// History:
// -------  
// 14Jan99	Alima		Created
//					
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CreatureGallery.h"
#include "ClonedGallery.h"
#include "CompressedGallery.h"
#include "NormalGallery.h"
#include "SharedGallery.h"
#include <string>
#include "../App.h"
#include "../General.h"
#include "ErrorMessageHandler.h"
#ifdef _WIN32
#include "../resource.h"
#endif
#include "../Creature/Skeleton.h"
#include "../../common/Catalogue.h"
#include "DisplayEngine.h"
#ifdef _WIN32
#include	"io.h"
#else
#include "../unix/FileFuncs.h"
#endif


//std::string theCreatureGalleryFile("CreatureGallery");
// I want to read these from the registry really so that I can extend
// the template and keep track of it
const uint32 ONE_MEG = 1048576;

////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////

CreatureGallery::CreatureGallery(int32 sizeOfSpriteSet,std::string name)
:mySizeOfASetOfNornSprites(sizeOfSpriteSet),
myMaxCreatures(1),
myCurrentPartNumber(0),
myRewindToHeaders(0),
myGallery(NULL),
myPart(0),
myNumberOfBytesWritten(0)
{
	// just check for nonsense values
	if(mySizeOfASetOfNornSprites == 0)
		mySizeOfASetOfNornSprites = 6;

	myCreatureBuildingStage = STAGE_ONE;
	// default size is 24 meg
	myTemplateSize = mySizeOfASetOfNornSprites *1048576; // all creature files should fit in
	myCreatureGallerySize =myMaxCreatures * myTemplateSize; //room for 4 creatures 
	
	char path[_MAX_PATH];
	theApp.GetDirectory(CREATURE_DATABASE_DIR, path);
	strcat(path,name.c_str());
	myName = std::string(path) + ".creaturegallery";
	// delete the old and possibly corrupt Creature Gallery
	DeleteFile(myName.c_str());

	BuildDatabase();
}

void CreatureGallery::BuildDatabase()
{

	bool newFileCreated = false;
	
	if(FileExists(myName.c_str()) == false)
	{
	
		//	only do this if you want to create a file.  The template file
		// should be distributed.
		//make a dummy file
		try
		{
		File file;
		file.Create(myName);
		uint32 dummyData = 1;
		file.Write(&dummyData,sizeof(uint32));
		file.Close();
		newFileCreated = true;
		}
		catch(File::FileException& e)
		{
			throw CreatureGalleryException(e.what(),__LINE__);
		}
	}

	try
	{
		// open the memory mapped file of the required size first
		myMemoryMappedFile.Open(myName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			myCreatureGallerySize);

		// if we need to put in the moniker keys
		if(newFileCreated)
		{
			CreateTemplateFile();
		}
	}
	catch(MemoryMappedFile::MemoryMappedFileException& e) 
	{
		throw CreatureGalleryException(e.what(),__LINE__);
	}
	catch(File::FileException& e)
	{
		throw CreatureGalleryException(e.what(),__LINE__);
	}
	
	RemoveAll();
}

CreatureGallery::~CreatureGallery()
{	
	myMemoryMappedFile.Close();
	SharedGallery::theSharedGallery().RemoveCreatureGallery(myGallery);
	BOOL res = DeleteFile(myName.c_str());
}

// ----------------------------------------------------------------------
// Method:		CreateTemplateFile
// Arguments:	None
// Returns:		None
// Description:	Creates a creature gallery template file big enough for 16
//				creatures.  The key (moniker) for eah slot at this point 
//				is zero.
//				
// ----------------------------------------------------------------------
void CreatureGallery::CreateTemplateFile()
{

	uint32 bytesWritten = 0;

	uint8* data = NULL;//myFileData;
	uint32* lastOffset = NULL;//data;


	uint32 done = 0;
	while ((done * myTemplateSize)< myCreatureGallerySize)
	{
		bytesWritten = 0;
		done++;
		// write moniker as empty key
		myMemoryMappedFile.WriteUINT32(0);

		//we will want to write the offset here
		myMemoryMappedFile.WriteUINT32(done*myTemplateSize);

		bytesWritten += 8;

		myMemoryMappedFile.Seek(done*myTemplateSize,
			File::Start);
	}

}


////////////////////////////////////////////////////////////////////////////
// Methods
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------
// Method:		AddCompressedCreature
// Arguments:	moniker - the unique id for a creature by which we identify
//							its genome.  Also used as a key for the 
//						composite creature gallery file.
//				numSpritesInFile - yes
// Returns:		a pointer to the 
// Description:	Creates a creature gallery getting the correct body parts
//				as defined in the genome.  Then slots the data in the 
//				first free slot.
//				
// ----------------------------------------------------------------------
Gallery* CreatureGallery::AddCompressedCreature(std::string moniker,
									  uint32 uniqueID,
									  int32 PVariantGenus[NUMPARTS],
									  int32 PVariant[NUMPARTS],
									  uint32 validParts,
									  uint8 Sex,
									  uint8 Age,
									  int32 BodySprites[NUMPARTS],
									  CompressedGallery creatureParts[NUMPARTS],
									  int32 numSpritesInFile)
{


	
		
	switch(	myCreatureBuildingStage)
	{

	case STAGE_ONE: if(!Part1CreatureBuilder( BodySprites,
										creatureParts,
										numSpritesInFile,
										uniqueID,
										validParts))
										myCreatureBuildingStage = STAGE_COMPLETE;
		break;
	case STAGE_TWO: Part2CreatureBuilder(validParts,BodySprites,creatureParts);
		break;
	case STAGE_THREE: Part3CreatureBuilder(creatureParts,validParts);
		break;
	case STAGE_FOUR: Part4CreatureBuilder(creatureParts,validParts);
		break;
	case STAGE_FIVE:
		{

			*myNumImagesHolder = myCurrentPartNumber;

			FilePath galleryName(moniker + ".C16", IMAGES_DIR);

#ifdef _WIN32
			myGallery = new CompressedGallery(galleryName,
											myMemoryMappedFile.GetFileMapping(),
											0,
											8,
											myNumberOfBytesWritten);
#else
// TODO: non-win32 version...
#endif
			ASSERT(myGallery);

			myCreatureBuildingStage = STAGE_COMPLETE;
			return myGallery;

		}
	} //end switch

return NULL;
}

bool CreatureGallery::Part1CreatureBuilder( int32 BodySprites[NUMPARTS],
									  CompressedGallery creatureParts[NUMPARTS],
									  int32 numSpritesInFile,
									  uint32 uniqueID,
									  uint32 ValidParts)
{
	ASSERT(myMemoryMappedFile.Valid());

	// bypass the shared gallery for this
	// because we will delete it ourselves in a moment
	

	int32 BitmapSize = 0;						// # bytes bitmap written to disc
	int32 HeaderSize = numSpritesInFile * 8;	// # bytes in file header
    const int S16HeaderSize = 6;

	// add all the scan lines to the header file
	
	uint32 flags = C16_FLAG_565 |  C16_16BITFLAG;

	// Find the first free slot in our composite file
	uint32 lastOffset = FindSlot( 0 );

	// if this is not zero something is wrong or we have reached the
	// end of the file.

	if(lastOffset < myCreatureGallerySize)
		{
		// go to the start of the file keyfile
		myMemoryMappedFile.Seek(lastOffset,File::Start);

		// fill in the unique id
		myMemoryMappedFile.WriteUINT32(uniqueID);
		myNumberOfBytesWritten+=(sizeof(uint32));

		// go past the offset as that should never change
		myMemoryMappedFile.Seek(sizeof(uint32),File::Current);
		myNumberOfBytesWritten+=(sizeof(uint32));
		// here the sprite file "proper" starts
	
		// write the flags which denote a 656
		// or 655 file and 16bit tag
		myMemoryMappedFile.WriteUINT32(flags);
		myNumberOfBytesWritten+=(sizeof(uint32));

		// save a pointer to the number of images
		// so we can fill it in later
	
		myMemoryMappedFile.ReadUINT16Ptr(myNumImagesHolder);
		myNumberOfBytesWritten+=(sizeof(uint16));

		// get all the sprite files we will need

		// write the data consecutively much better scheme

	
		myRewindToHeaders = myMemoryMappedFile.GetPosition();

		myCurrentPartNumber =0;
		myCreatureBuildingStage = STAGE_TWO;
		return true;
	}

	return false;
}

// write out the bitmap headers, we don't know what the actual data
// offsets are so leave in placeholders that we will fill in later
// remember not to count those bytes twice 
void CreatureGallery::Part2CreatureBuilder(uint32 ValidParts,
										   int32 BodySprites[NUMPARTS],
									  CompressedGallery creatureParts[NUMPARTS])
{
	

		uint32 count = 0;

		if(myPart < ValidParts)
		{
					
			// Record sprite# in dest of the first image for this
			// part (fr building limbs)
			BodySprites[myPart] = myCurrentPartNumber;


			CompressedBitmap* bitmap = (CompressedBitmap*)creatureParts[myPart].GetBitmap(0);

			// For each image in gallery...
			count = creatureParts[myPart].GetCount();
			for	(uint32 i=0; i<count; i++) 
			{
				// save the position for my offset
				ASSERT(bitmap);

			//	// save the start of the scan line data
				myNumberOfBytesWritten+=bitmap->SaveHeaderAndOffsetData(myMemoryMappedFile);
				myCurrentPartNumber++;
				bitmap++;
			}
				myCreatureBuildingStage = STAGE_TWO;
				myPart++;
		}
		else
		{
			myPart = 0;
				myCreatureBuildingStage = STAGE_THREE;
		}
	
}

// workout what the data offsets are for each bitmap after writing the
// pixel data to the file
void CreatureGallery::Part3CreatureBuilder( CompressedGallery creatureParts[NUMPARTS],
										    uint32 ValidParts)
{
	// For each gallery...
	uint32 startOfPixelData = 0;
	uint32 count = 0;
	for	(int Part=0; Part<ValidParts; Part++)
	{
	
		CompressedBitmap* bitmap = (CompressedBitmap*)creatureParts[Part].GetBitmap(0);
		// and for each image in gallery...
		count = creatureParts[Part].GetCount();
		for	(uint32 i=0; i<count; i++)
			{
			// take 8 away because the creatures will 
			// start reading from the pixel format not the
			/// key information
				startOfPixelData = myMemoryMappedFile.GetPosition()-8; // last offset is zero
				ASSERT(bitmap);
		
				bitmap->UpdateOffsetValues(startOfPixelData);
				// Write bitmap data behind the header
				myNumberOfBytesWritten+=bitmap->SaveData(myMemoryMappedFile);
				bitmap++;
			}

				// Destroy source gallery
	myCreatureBuildingStage = STAGE_FOUR;
	}//end for	
}
	
// we now have the up to date header information for
// each bitmap - the offset values are now correct so rewrite them
void CreatureGallery::Part4CreatureBuilder( CompressedGallery creatureParts[NUMPARTS],
										    uint32 ValidParts)
{
	if(myPart == 0)
	{
		// now store the correct header information
		myMemoryMappedFile.Seek(myRewindToHeaders,File::Start);
	}
		CompressedBitmap* bitmap = NULL;
		uint32 count =0;

		if(myPart < ValidParts)
		{

			bitmap = (CompressedBitmap*)creatureParts[myPart].GetBitmap(0);

			// For each image in gallery...
			count = creatureParts[myPart].GetCount();
			for	(uint32 i=0; i<count; i++) 
			{
				// save the position for my offset
			
				ASSERT(bitmap);

			//	// save the start of the scan line data
				// don't bother counting these bytes again
				bitmap->SaveHeaderAndOffsetData(myMemoryMappedFile);
				bitmap++;
			}
			myPart++;
			myCreatureBuildingStage = STAGE_FOUR;
		}
		else
		{
			myPart =0;
			myCreatureBuildingStage = STAGE_FIVE;
		}
}

// ----------------------------------------------------------------------
// Method:		FindSlot
// Arguments:	key - either a moniker or NULL to find the first free
//						slot.
// Returns:		offset of gallery or the total size of the creature
//				gallery if we are full.
// Description:	Creates a creature gallery getting the correct body parts
//				as defined in the genome.  Then slots the data in the 
//				first free slot.
//				
// ----------------------------------------------------------------------
uint32 CreatureGallery::FindSlot(uint32 key)
{
	myMemoryMappedFile.Reset();
	ASSERT(myMemoryMappedFile.Valid());

	// from the start of the file get the first moniker
//	uint32* data = myFileData;
	uint32 lastOffset=0;
	uint32 sanityCheck = lastOffset;

	// read the moniker
	while(myMemoryMappedFile.ReadUINT32() != key)//GetUINT32At(data) != key)
	{
		// read the offset
		lastOffset = myMemoryMappedFile.ReadUINT32();
		
		// do a check for the file size here!
		if(lastOffset < myCreatureGallerySize)
		{
			// move past the offset
			// move to the next moniker key
			myMemoryMappedFile.Seek(lastOffset,File::Start);
		}
		else
		{
			return myCreatureGallerySize;
		}
	}

	// we have found the slot give the data offset
	return lastOffset;


}

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
bool CreatureGallery::RemoveCreature(uint32 uniqueID)
{
		// Find the first free slot in our composite file
	uint32 lastOffset = FindSlot(uniqueID);

	// if this is zero something is wrong or we have reached the
	// end of the file.
	if(lastOffset < myCreatureGallerySize)
		{
		// go top this norns file position
		myMemoryMappedFile.Seek(lastOffset,File::Start);
			
		// blank out the key
		myMemoryMappedFile.WriteUINT32(0);
		
		return true;
		}
	return false;
}

void CreatureGallery::RemoveAll()
{
	// Find the first free slot in our composite file
	uint32 lastOffset = 0;
	uint8* data  = NULL;
	// if this is zero something is wrong or we have reached the
	// end of the file.
	while(lastOffset < myCreatureGallerySize)
		{
		// move to the next creature file
		myMemoryMappedFile.Seek(lastOffset,File::Start);
	
		// blank out the moniker
		myMemoryMappedFile.WriteUINT32(0);
		lastOffset+=myTemplateSize;
		}
}



