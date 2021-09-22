// --------------------------------------------------------------------------
// Filename:	SharedGallery.cpp
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
//				Note: at the moment there is no limit on how many files
//				you can have open.  There is no reference checking to tell
//				when to close galleries down			
//				
//
// History:
// 11Dec98	Alima		created
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Gallery.h"
#include "SharedGallery.h"
#include "NormalGallery.h"
#include "CompressedGallery.h"
#include "BackGroundGallery.h"
#include "ClonedGallery.h"
#include "DisplayEngine.h"		// for displaying errors
#include "../General.h"
#include "../App.h"
#include "ErrorMessageHandler.h"
#include <algorithm>
#include <locale.h> //need non-template tolower
#include <fstream>

#ifndef _WIN32
#include "../unix/FileFuncs.h"
#endif
////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////
const std::string S16 = ".s16";
const std::string C16 = ".c16";

const int32 MINIMUM_BITMAP_WIDTH = 3;
const int32 MINIMUM_BITMAP_HEIGHT = 3;

////////////////////////////////////////////////////////////////////////////
// My static variables
////////////////////////////////////////////////////////////////////////////
SharedGallery SharedGallery::mySharedGallery;
std::map<std::string, Gallery*> SharedGallery::myGalleries;
std::map<Gallery*,std::string > SharedGallery::myClonedGalleries;
std::map<Gallery*,std::string > SharedGallery::myCreatureGalleries;


SharedGallery::SharedGallery()
{
}

// ----------------------------------------------------------------------
// Method:      theSharedGallery 
// Arguments:   None 		
//
// Returns:     reference to the shared gallery
//
// Description: This is the only way to use the shared gallery
//						
// ----------------------------------------------------------------------
SharedGallery& SharedGallery::theSharedGallery()
{
	return mySharedGallery;
}



SharedGallery::~SharedGallery()
{
}

// ----------------------------------------------------------------------
// Method:      CreateGallery
// Arguments:   galleryName - the name of the gallery file
//
// Returns:     pointer to the gallery created
// Description: checks whether the gallery already exists and if not
//				creates one and adds it to the list of galleries.  three
//				different types of gallery can be created depending on
//				the file extension.
//				background, compressed, and normal gallery.
//				
//				Now if you get an S16 extension check if there is a 
//				C16 version to use which will be preferred
// ----------------------------------------------------------------------
Gallery* SharedGallery::CreateGallery(FilePath const &galleryIn)
{
	FilePath galleryPath = galleryIn;
	std::string galleryName = galleryPath.GetFullPath();
	int x = galleryName.find_last_of(".");

	if(x == -1)
	{
		if(galleryName.empty())
		{
			ErrorMessageHandler::Show(theDisplayErrorTag,
								(int)DisplayEngine::sidEmptyGalleryString,
								std::string("SharedGallery::CreateGallery"));
		}
		else
		{
			ErrorMessageHandler::Show(theDisplayErrorTag,
									(int)DisplayEngine::sidNoGalleryFound,
									std::string("SharedGallery::CreateGallery"),
									galleryName.c_str());
		}

	
		return NULL;
	}

	std::string ext = galleryName.substr(x, 3);

	// check for a preferred C16 version
	if(ext[1] == 'S' || ext[1] == 's')	
	{
		// not compressed sprite gallery
		std::string tempGalleryName = galleryName;
		tempGalleryName[x+1] = 'C';
#ifdef _WIN32
		int32 attributes = GetFileAttributes(tempGalleryName.data());
		if(attributes != -1)
#else
		if(FileExists(tempGalleryName.data()))
#endif
		{
			galleryName = tempGalleryName;
			// get the revised extension
			ext = galleryName.substr(x, 3);
			galleryPath.SetExtension( "C16" );
		}
	}

	// see if we already have this gallery open
	std::string testString = galleryPath.GetFullPath();
	std::transform( testString.begin(), testString.end(), testString.begin(), tolower );
	GALLERYMAP_ITERATOR it = myGalleries.find(testString);
	
	Gallery* gallery = NULL;

	if(it == myGalleries.end())
	{
		// nobody should be asking for a creature gallery that doesn't exist
		gallery = LookForCreatureGallery(galleryPath);
		
		if(gallery)
			return gallery;

		try
		{

		
		// see whether we are dealing with a compressed file or whatever 
		// by looking at the file extension

			if(ext[1] == 'S' || ext[1] == 's')	
			{	// not compressed sprite gallery
				std::string filename = galleryPath.GetFullPath();
				std::string tmpFilePath = filename + ".tmp.s16";
				std::string prevFileName;
				DisplayEngine::theRenderer().SafeImageConvert(filename,
								tmpFilePath,
								DisplayEngine::theRenderer().GetMyPixelFormat(), 
								prevFileName);
				::DeleteFile(tmpFilePath.c_str());
				gallery = new NormalGallery(galleryPath);
			}
			else if(ext[1] == 'b' || ext[1] == 'B')
			{	// not compressed background tile gallery
				std::string filename = galleryPath.GetFullPath();
				std::string tmpFilePath = filename + ".tmp.blk";
				std::string prevFileName;
				DisplayEngine::theRenderer().SafeImageConvert(filename,
								tmpFilePath,
								DisplayEngine::theRenderer().GetMyPixelFormat(), 
								prevFileName);
				::DeleteFile(tmpFilePath.c_str());
				gallery = new BackgroundGallery(galleryPath);
			}
			else if(ext[1] == 'C' || ext[1] == 'c')
			{	//compressed sprite gallery
				std::string filename = galleryPath.GetFullPath();
				std::string tmpFilePath = filename + ".tmp.c16";
				std::string prevFileName;
				DisplayEngine::theRenderer().SafeImageConvert(filename,
								tmpFilePath,
								DisplayEngine::theRenderer().GetMyPixelFormat(), 
								prevFileName);
				::DeleteFile(tmpFilePath.c_str());
				gallery = new CompressedGallery(galleryPath);
			}
			else
			{
				if(galleryName.empty())
				{
					ErrorMessageHandler::Show(theDisplayErrorTag,
								(int)DisplayEngine::sidEmptyGalleryString,
								std::string("DisplayEngine::CreateGallery"));
				}
				else
				{
					ErrorMessageHandler::Format(theDisplayErrorTag,
									(int)DisplayEngine::sidNoGalleryFound,
									std::string("SharedGallery::CreateGallery"),
									galleryName.c_str());
				}

				// what are you doing giving me an unknown gallery?
				return NULL;
			}

			// this is just a warning for physics stuff
			if(!gallery->ValidateBitmapSizes(MINIMUM_BITMAP_WIDTH, MINIMUM_BITMAP_HEIGHT))
			{
				ErrorMessageHandler::Show(theDisplayErrorTag, 
					(int)DisplayEngine::sidBitmapSizeTooSmall,
				std::string("SharedGallery::CreateGallery"), 
				galleryName.c_str(),MINIMUM_BITMAP_WIDTH,MINIMUM_BITMAP_HEIGHT);

			
			}
		}
		catch(Gallery::GalleryException& e)
		{
			ErrorMessageHandler::Show(e, std::string("SharedGallery::CreateGallery"));
			return gallery;
		}
		catch(File::FileException& e)
		{
			ErrorMessageHandler::Show(e, std::string("SharedGallery::CreateGallery"));
			return gallery;
		}

		if(gallery->IsValid())
		{

			/*
#ifdef _DEBUG
			OutputFormattedDebugString( "adding gallery \"%s\": count before %d",
				gallery->GetName().GetFullPath().c_str(), myGalleries.size());
#endif*/
			std::string testString = galleryPath.GetFullPath();
			std::transform( testString.begin(), testString.end(), testString.begin(), tolower );
			myGalleries[testString] = gallery;
			/*
#ifdef _DEBUG
			OutputFormattedDebugString( " and after: %d\n", myGalleries.size());
#endif
	*/
			return gallery;
		}
		else
		{
			delete gallery;
			return NULL;
		}
	}
	else
	{
		// update this gallery's reference count
		((*it).second)->IncrementReferenceCount();
		return (*it).second;
	}
	return NULL;
}


Gallery* SharedGallery::LookForCreatureGallery(FilePath const& galleryName)
{

	CREATUREGALLERYMAP_ITERATOR it;// = myCreatureGalleries.find(galleryName);

	for(it = myCreatureGalleries.begin(); it!= myCreatureGalleries.end(); it++)
	{
		if((*it).second == galleryName.GetFullPath())
		{
		 //return (*it).first;
			break;
		}
	}

	if(it == myCreatureGalleries.end())
	{
		return NULL;
	}
	else
	{
		// update this gallery's reference count
		((*it).first)->IncrementReferenceCount();
		return (*it).first;
	}


return NULL;
}

// ----------------------------------------------------------------------
// Method:      CreateClonedGallery
// Arguments:   galleryName - the name of the gallery file
//
// Returns:     pointer to the gallery created
// Description: Creates a whole new cloned gallery
//				keep track of how many we have created????
//				
//			
// ----------------------------------------------------------------------
Gallery* SharedGallery::CreateClonedGallery(FilePath const &galleryIn,
											uint32 baseImage,
											uint32 numImages)
{
	FilePath galleryName = galleryIn;
	std::string fileName = galleryName.GetFullPath();
	int x = fileName.find_last_of(".");

	if(x == -1)
	{
		if(fileName.empty())
		{
			ErrorMessageHandler::Show(theDisplayErrorTag,
								(int)DisplayEngine::sidEmptyGalleryString,
								std::string("SharedGallery::CreateClonedGallery"));
		}
		else
		{
			ErrorMessageHandler::Show(theDisplayErrorTag,
									(int)DisplayEngine::sidNoGalleryFound,
									std::string("SharedGallery::CreateClonedGallery"),
									fileName.c_str());
		}

	
		return NULL;
	}

	std::string ext = fileName.substr(x, 3);

	// check for a preferred C16 version
	if(ext[1] == 'S' || ext[1] == 's')	
	{
		// not compressed sprite gallery
		std::string tempGalleryName = fileName;
		tempGalleryName[x+1] = 'C';
#ifdef _WIN32
		int32 attributes = GetFileAttributes(tempGalleryName.data());
		if(attributes != -1)
#else
		if(FileExists(tempGalleryName.data()))
#endif
		{
			fileName = tempGalleryName;
			// get the revised extension
			ext = fileName.substr(x, 3);
			galleryName.SetExtension( "C16" );
		}
	}





	Gallery* gallery = NULL;
	try
	{
		// keep track of cloned images
		gallery = new ClonedGallery(galleryName,
									baseImage,
									numImages);

	}
	catch(Gallery::GalleryException& e)
	{
		ErrorMessageHandler::Show(e, std::string("SharedGallery::CreateClonedGallery"));
		return gallery;
	}

	if(gallery->IsValid())
	{
		// the key this time is the gallery pointer
		myClonedGalleries[gallery] = galleryName.GetFullPath();
		return gallery;
	}
	else
	{
		delete gallery;
		return NULL;
	}
		

	return NULL;
}

void SharedGallery::AddClonedGallery( ClonedGallery *gallery )
{
	myClonedGalleries[gallery] = "Anon";
}

// ----------------------------------------------------------------------
// Method:      RemoveGallery
// Arguments:   None
//
// Returns:     true if something was deleted
// Description: Goes through and destroys all galleries				
//			
// ----------------------------------------------------------------------
bool SharedGallery::RemoveGallery(Gallery* gallery)
{
	ASSERT(gallery);

	if( RemoveClonedGallery( gallery ) ) return true;

	// see if we already have this gallery open

	std::string testString = gallery->GetName().GetFullPath();
	std::transform( testString.begin(), testString.end(), testString.begin(), tolower );
	GALLERYMAP_ITERATOR it = myGalleries.find(testString);

	bool found = ( it != myGalleries.end() );
	if( found )
	{
		((*it).second)->DecrementReferenceCount();
		if(!(((*it).second)->IsUsed()))
		{
			/*
#ifdef _DEBUG
			OutputFormattedDebugString( "deleting gallery \"%s\": count now %d\n",
				it->second->GetName().GetFullPath().c_str(), myGalleries.size() - 1);
#endif*/
			delete (*it).second;
			(*it).second = NULL;
			myGalleries.erase(it);
		}
	}
	else
	{

	}
	return found;
}


bool SharedGallery::RemoveClonedGallery(Gallery*& gallery)
{
//	if( RemoveCreatureGallery( gallery ) ) return true;

	// see if we already have this gallery open
	CLONEDGALLERYMAP_ITERATOR it = myClonedGalleries.find(gallery);

	bool found = ( it != myClonedGalleries.end() );
	if( found )
	{
		/*
#ifdef _DEBUG
			OutputFormattedDebugString( "deleting cloned gallery \"%s\": count now %d\n",
				gallery->GetName().GetFullPath().c_str(), myClonedGalleries.size() - 1);
#endif*/
		delete (*it).first;
		myClonedGalleries.erase(it);
	}
	return found;
}


bool SharedGallery::RemoveCreatureGallery(Gallery*& gallery)
{
	// no refernce counting just delete away
	// see if we already have this gallery open
	CLONEDGALLERYMAP_ITERATOR it = myCreatureGalleries.find(gallery);

	bool found = ( it != myCreatureGalleries.end() );
	if( found )
	{		

			delete (*it).first;
			//(*it).first = NULL;
			myCreatureGalleries.erase(it);

	}
	return false;
}

// ----------------------------------------------------------------------
// Method:      DestroyGalleries
// Arguments:   None
//
// Returns:     None
// Description: Goes through and destroys all galleries				
//			
// ----------------------------------------------------------------------
void SharedGallery::DestroyGalleries()
{
	if(myGalleries.size())
	{
	GALLERYMAP_ITERATOR it;
	for(it = myGalleries.begin(); it != myGalleries.end(); it++)
	{
		Gallery* gallery = (*it).second;
		delete (*it).second;
	}

	myGalleries.clear();
	}

	if(myClonedGalleries.size())
	{

	CLONEDGALLERYMAP_ITERATOR it2;
	for(it2 = myClonedGalleries.begin(); it2 != myClonedGalleries.end(); it2++)
	{
		delete (*it2).first;
	}

	myClonedGalleries.clear();
	}

	if(myCreatureGalleries.size())
	{
	CREATUREGALLERYMAP_ITERATOR it3;
	for(it3 = myCreatureGalleries.begin(); it3!= myCreatureGalleries.end(); it3++)
	{
		Gallery* gallery = (*it3).first;
		delete gallery;
	}

	myCreatureGalleries.clear();
	}

	CleanCreatureGalleryFolder();
}

// ----------------------------------------------------------------------
// Method:		CreateGallery
// Arguments:	uniqueid - Used as a key for the 
//						   composite creature gallery file.
//				numSpritesInFile - yes
// Returns:		a pointer to the created gallery
// Description:	Interface to the creature gallery getting the correct 
//				body parts as defined in the genome.  
//				
// ----------------------------------------------------------------------
Gallery* SharedGallery::CreateGallery(std::string moniker,
									  uint32 uniqueID,
									  int32 PVariantGenus[NUMPARTS],
									  int32 PVariant[NUMPARTS],
									  uint32 ValidParts,
									  uint8 Sex,
									  uint8 Age,
									  int32 BodySprites[NUMPARTS],
									  CreatureGallery* bodyPartGallery,
									   CompressedGallery creatureParts[NUMPARTS],
										int32 numSpritesInFile,
										bool onePassOnly /*= false*/)
{


	if(!bodyPartGallery)
		return NULL;

	try
	{

		Gallery* gallery = NULL;

		if(onePassOnly)
		{
			// gallery is null and creature gallery is not yet complete
			while (gallery == NULL && !bodyPartGallery->IsComplete())
			{
				// try having compressed Norns now
				// pass the request to the composite file handler
				gallery =  bodyPartGallery->
										AddCompressedCreature(moniker,
									   uniqueID,
									   PVariantGenus,
									   PVariant,
									   ValidParts,
									   Sex,
									   Age,
									   BodySprites,
									   creatureParts,
									   numSpritesInFile);
			}
		}
		else
		{
		// try having compressed Norns now
			// pass the request to the composite file handler
		gallery =  bodyPartGallery->
										AddCompressedCreature(moniker,
									   uniqueID,
									   PVariantGenus,
									   PVariant,
									   ValidParts,
									   Sex,
									   Age,
									   BodySprites,
									   creatureParts,
									   numSpritesInFile);
		}

		if(gallery)
		{
			// now tag the unique id on to the file name to get a unique
			// set of sprites
			FilePath newName = gallery->GetName();
			gallery->SetName(newName);


			if(gallery->IsValid())
			{
				gallery->SetFileSpec(uniqueID );
				// this gallery is new nobody should be using it yet
				gallery->ResetReferenceCount();

				ReplaceCreatureGallery(gallery);
				// the key this time is the gallery pointer
				return gallery;
			}
			else
			{

				std::string name("A creature gallery");
				ErrorMessageHandler::Show(theDisplayErrorTag,
					(int)DisplayEngine::sidGalleryNotCreated,
					std::string("SharedGallery::CreateGallery"),
					name.c_str());

				delete gallery;
				return NULL;
			}
		}
	}
	catch(CreatureGallery::CreatureGalleryException& e)
	{
		ErrorMessageHandler::Show(e, std::string("SharedGallery::CreateCreatureGallery"));
		return NULL;
	}
	return NULL;
}

void SharedGallery::ConvertGallery(FilePath const& galleryPath, uint32 to)
{
	std::string galleryName = galleryPath.GetFullPath();
	Gallery* gallery = NULL;
	// see whether we are dealing with a compressed file by looking
	// at the file extension
	int x = galleryName.find_last_of(".");

	if(x == -1)
	{
		ErrorMessageHandler::Show(theDisplayErrorTag,
			(int)DisplayEngine::sidGalleryNotFound,
			std::string("SharedGallery::ConvertGallery"),
			galleryName.c_str());
		return;
	}
	
	std::string ext = galleryName.substr(x, 3);

	if(ext[1] == 'S' || ext[1] == 's')	
	{	// not compressed sprite gallery
		gallery = new NormalGallery(galleryPath);
	}
	else if(ext[1] == 'b' || ext[1] == 'B')
	{	// not compressed background tile gallery
		gallery = new BackgroundGallery(galleryPath);
	}
	else if(ext[1] == 'C' || ext[1] == 'c')
	{	//compressed sprite gallery
		gallery = new CompressedGallery(galleryPath);
	}
	else
	{
		ErrorMessageHandler::Show(theDisplayErrorTag,
			(int)DisplayEngine::sidGalleryUnknownType,
			std::string("SharedGallery::ConvertGallery"),
			galleryName.c_str());

		// what are you doing giving me an unknown
		// gallery?
		return;
	}

	if(gallery)
	{
	gallery->ConvertTo(to);

	delete gallery;
	gallery = NULL;
	}

}


void SharedGallery::ReplaceCreatureGallery(Gallery* gallery)
{
	ASSERT(gallery);

	CREATUREGALLERYMAP_ITERATOR it;

	for(it = myCreatureGalleries.begin(); it!= myCreatureGalleries.end(); it++)
	{
		std::string name = (*it).second;
		if(name == gallery->GetName().GetFullPath())
		{
			break;
		}
	}

	if(it != myCreatureGalleries.end())
	{
	// found the gallery to replace lets delete it
		delete (*it).first;
		myCreatureGalleries.erase(it);
	

	}

	// add the new gallery whatever happened
	// the key this time is the gallery pointer
	myCreatureGalleries[gallery] = gallery->GetName().GetFullPath();

}

void SharedGallery::CleanCreatureGalleryFolder()
{
	// before we do anything else delete all creature galleries
	// that are not in use
	char path[_MAX_PATH];
	theApp.GetDirectory(CREATURE_DATABASE_DIR, path);
	if (path[0] == 0)
		return;

	std::vector<std::string> files;
	GetFilesInDirectory(path, files);

	std::string fileName;
	// no files in this directory!!!
	int i;
	for ( i = 0; i < files.size(); i++)
	{
		fileName = path + files[i];
		// delete only files ending in .creaturegallery
		int len = fileName.size();
		if (len >= 16)
		{
			if (fileName.substr(len - 16, 16) == ".creaturegallery")
				DeleteFile(fileName.c_str());
		}
	}
}

// see if we already have this gallery open
bool SharedGallery::QueryGalleryInUse(FilePath const &galleryPath)
{
	std::string worldGalleryName;
	if (galleryPath.GetWorldDirectoryVersionOfTheFile(worldGalleryName))
	{
		if (QueryGalleryInUseHelper(worldGalleryName))
			return true;
	}

	std::string galleryName = galleryPath.GetFullPath();
	return QueryGalleryInUseHelper(galleryName);
}


bool SharedGallery::QueryGalleryInUseHelper(std::string galleryName)
{
	// remove .s16 or .c16 if there already
	int x = galleryName.find_last_of(".");
	if (x != -1)
		galleryName = galleryName.substr(0, x);

	GALLERYMAP_ITERATOR it;

	std::transform( galleryName.begin(), galleryName.end(), galleryName.begin(), tolower );

/*for(it = myGalleries.begin(); it != myGalleries.end(); it++)
{
	std::string name = it->first;
}*/

	it = myGalleries.find(galleryName + ".c16");
	if (it != myGalleries.end())
		return true;

	it = myGalleries.find(galleryName + ".s16");
	if (it != myGalleries.end())
		return true;

	return false;
}

void SharedGallery::PreloadBackgrounds()
{
	char path[_MAX_PATH];
	theApp.GetDirectory(BACKGROUNDS_DIR, path);
	if (path[0] == 0)
		return;

	std::vector<std::string> files;
	GetFilesInDirectory(path, files);

	std::string fileName;
	// no files in this directory!!!
	int i;
	for ( i = 0; i < files.size(); i++)
	{
		// non background files in this folder will be rejected
		CreateGallery(FilePath(files[i],BACKGROUNDS_DIR));
	}
	
}

void SharedGallery::DumpGalleries()
{
	static int name = 0;
	char file[100];
	sprintf(file, "%s%d", "c:\\GalleryDump", name++);

	std::ofstream out(file);

	std::map< std::string, Gallery*>::iterator iter1;
	std::map<Gallery*,std::string>::iterator iter2;
	std::map<Gallery*,std::string>::iterator iter3;

	out << "myGalleries\n\n";
	for( iter1=myGalleries.begin(); iter1!=myGalleries.end(); ++iter1 )
	{
		out << iter1->first << " " << iter1->second->GetName().GetFullPath() << "\n";
	}

	out << "\n\nmyClonedGalleries\n\n";
	for( iter2=myClonedGalleries.begin(); iter2!=myClonedGalleries.end(); ++iter2 )
	{
		out << iter2->second << " " << iter2->first->GetName().GetFullPath() << "\n";
	}

	out << "\n\nmyCreatureGalleries\n\n";
	for( iter3=myCreatureGalleries.begin(); iter3!=myCreatureGalleries.end(); ++iter3 )
	{
		out << iter3->second << " " << iter3->first->GetName().GetFullPath() << "\n";
	}
}


