// --------------------------------------------------------------------------
// Filename:	SharedGallery.h
// Class:		SharedGallery
// Purpose:		This class handles requests to create galleries.
//				
//
// Description: This holds a map of gallery names to gallery.
//				When a request for a new gallery comes in it creates
//				a memorymapped file.  Then the new gallery is created 
//				whose bitmaps point to the relevent parts of the 
//				memory mapped file.
//
//				If an existing gallery is requested then the pointer for that
//				gallery is passed back.
//			
//
//
//				Note: at the moment there is no limit on how many files
//				you can have open.  There is no reference checking to tell#
//				when to close galleries down			
//				
//
// History:
// 11Dec98	Alima		created
// Jan 98				The shared gallery now can handle three different
//						types of gallery.  Backgrounds, Sprites and Norns
// --------------------------------------------------------------------------
#ifndef SHARED_GALLERY_H
#define SHARED_GALLERY_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif
//#include "Gallery.h"
#include <map>
#include <string>
#include "CreatureGallery.h"
#include "../Creature/Definitions.h"

class Gallery;
class ClonedGallery;

typedef std::map<std::string,Gallery*>::iterator GALLERYMAP_ITERATOR;
typedef std::map<Gallery*,std::string >::iterator CLONEDGALLERYMAP_ITERATOR;
typedef std::map<Gallery*,std::string>::iterator CREATUREGALLERYMAP_ITERATOR;
class SharedGallery
{
	enum{BACKGROUND_GALLERY = 0,
		SPRITE_GALLERY,
		NORN_GALLERY,
	};
	
public:

	~SharedGallery();
	Gallery* CreateGallery(FilePath const &galleryName);
	Gallery* CreateGallery(std::string moniker,
									  uint32 uniqueID,
									  int32 PVariantGenus[NUMPARTS],
									  int32 PVariant[NUMPARTS],
									  uint32 ValidParts,
									  uint8 Sex,
									  uint8 Age,
									  int32 BodySprites[NUMPARTS],
									  CreatureGallery* gallery,
									  CompressedGallery creatureParts[NUMPARTS],
										int32 numSpritesInFile,
										bool onePassOnly = false);

	Gallery* CreateClonedGallery(FilePath const &galleryName,
								uint32 baseimage,
								uint32 numImages);
	void AddClonedGallery( ClonedGallery *gallery );

	Gallery* LookForCreatureGallery(FilePath const &galleryName);

	void ReplaceCreatureGallery(Gallery* gallery);


	static SharedGallery& theSharedGallery();

	void ConvertGallery(FilePath const & name, uint32 to);

// ----------------------------------------------------------------------
// Method:      RemoveGallery
// Arguments:   None
//
// Returns:     true if something was deleted
// Description: Deletes the given gallery if the reference count is zero			
//			
// ----------------------------------------------------------------------
	static bool RemoveGallery(Gallery* gallery);
	static bool RemoveCreatureGallery(Gallery*& gallery);
	static bool RemoveClonedGallery(Gallery*& gallery);

	static bool QueryGalleryInUse(FilePath const &galleryPath);
	static void DumpGalleries();

	void DestroyGalleries();
	void CleanCreatureGalleryFolder();
	void PreloadBackgrounds();

protected:
	static bool QueryGalleryInUseHelper(std::string galleryName);

private:

	// private constructor to ensure only one exists
	SharedGallery();


	// Copy constructor and assignment operator
	// Declared but not implemented
	SharedGallery (const SharedGallery&);
	SharedGallery& operator= (const SharedGallery&);

	static SharedGallery mySharedGallery;
	//map the gallery name to the new gallery
	static std::map< std::string, Gallery*> myGalleries;
	static std::map<Gallery*,std::string> myClonedGalleries;
	static std::map<Gallery*,std::string> myCreatureGalleries;
};



#endif //SHARED_GALLERY_H
