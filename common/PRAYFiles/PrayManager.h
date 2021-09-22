// --------------------------------------------------------------------------------------
// Filename:	PrayManager.h
// Class:		PrayManager
// Purpose:		To encapsulate the Pray Files and provide accessor functionality
// Description:
//  The Pray Manager manages nice lists of chunks. And allows you to find them easily
//
// History:
//  21Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------

#ifndef PRAYMANAGER_H
#define PRAYMANAGER_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif
#include <map>
#include <string>
#include <vector>
#include <list>
#include "PrayChunk.h"
#include "../C2eTypes.h"

typedef std::pair<std::string,int> FileOffset;
typedef std::pair<int,std::string> FlagsType;

class PrayManager
{
public:
	// ----------------------------------------------------------------------------------
	// Method:		AddDir
	// Arguments:	dirname - std::string - The directory name to add
	// Returns:		(None)
	// Description:	Adds the parameter to the localise list. It does not rescan yet.
	// Costs:		Immediate: Light, Later: Light to Medium
	// ----------------------------------------------------------------------------------
	inline void AddDir(std::string dirname) { dirnames.push_back(dirname); }

	// ----------------------------------------------------------------------------------
	// Method:		RescanFolders
	// Arguments:	(None)
	// Returns:		(None)
	// Description:	Scans the folders in the dirnames vector for pray files.
	//				Note that this also causes the manager to execute a forced
	//				GarbageCollect and therefore it is a rather heavy function if called
	//				more than once in the normal execution of a program.
	// Costs:		Immediate: Meduium to heavy, Later: Medium to Heavy
	// ----------------------------------------------------------------------------------
	void RescanFolders();

	// ----------------------------------------------------------------------------------
	// Constructor
	// Arguments:	thisLang - std::string - the Language ID we are working with
	// Returns:		(None)
	// Description: This constructs a manager, with the given language ID - PrayFiles
	//				loaded by this manager will first be localised with the filelocaliser
	// ----------------------------------------------------------------------------------
	PrayManager(std::string thisLang) { langid = thisLang; }

	// ----------------------------------------------------------------------------------
	// Method:		CheckChunk
	// Arguments:	thisChunk - std::string - The Chunk name to scan for
	// Returns:		0 if chunk does not exist, otherwise the weight of finding the chunk.
	// Description:	This method scans the chunk map to find its filereference. If it
	//				exists, then the chunk can be returned. 
	//				If the chunk does not exist, 0 is returned.
	//				If the chunk is in the cache, 1 is returned
	//				If the chunk is not cached, but is uncompressed, 2 is returned
	//				If the chunk is on disk and compressed, 3 is returned.
	// Costs:		Immediate: Light, Later: (None)
	// ----------------------------------------------------------------------------------
	int CheckChunk(std::string thisChunk);

	// ----------------------------------------------------------------------------------
	// Method:		GetChunk
	// Arguments:	thisChunk - std::string - The Chunk name to return the data for
	// Returns:		a PrayChunkPtr representing the data. Throws NULL on failure.
	// Description: Returns a PrayChunkPtr which represents the Chunk.
	//				The PrayChunkPtr allows ref-counts to be kept on the data chunks.
	// Costs:		Immediate: Light to Heavy, Later: (None)
	// ----------------------------------------------------------------------------------
	PrayChunkPtr GetChunk(std::string thisChunk);

	// ----------------------------------------------------------------------------------
	// Method:		GetChunks
	// Arguments:	thisType - std::string - The Chunk type to find
	//				thisVec	- std::vector<std::string> - A vector to put the chunks in
	// Returns:		(None)
	// Description:	Clears thisVec & adds the names of all chunks of the type passed in.
	// Costs:		Immediate: Light, Later: (None)
	// ----------------------------------------------------------------------------------
	void GetChunks(std::string thisType, std::vector<std::string>& thisVec);

	// ----------------------------------------------------------------------------------
	// Method:		AddChunkToFile
	// Arguments:	thisName - std::string - The name for the chunk.
	//				thisType - std::string - The type for the chunk.
	//				thisFile - std::string - The file to put the chunk in.
	//				thisSize - int         - The size for the chunk
	//				thisData - uint8*      - A pointer to the data to copy
	//              doCompress - bool      - Whether or not to compress the data
	// Returns:     (None)
	// Description: This method takes the data, and creates a new chunk on file for it.
	//				If the file does not exist, it is created. 
	//				That chunk is not then available until you RescanFolders()
	// Costs:		Immediate: Heavy, Later, Medium
	// ----------------------------------------------------------------------------------
	void AddChunkToFile(std::string thisName, std::string thisType, std::string thisFile,
						int thisSize, uint8* thisData, bool doCompress);

	// ----------------------------------------------------------------------------------
	// Method:		GarbageCollect
	// Arguments:	force - bool - Whether to force the drop of all or not
	// Returns:		(None)
	// Description:	If force is set, the manager forgets all chunks it has cached. If
	//				a chunk is currently referenced, it isn't deleted, but once all refs
	//				go out of scope, the chunks are deleted. All chunks must be reloaded
	//				from disk later. If force is not set, only those chunks which have
	//				no references outside of the manager are deleted.
	// Costs:		Immediate: Low, Later: Medium to Heavy
	// ----------------------------------------------------------------------------------
	void GarbageCollect(bool force);

	// ----------------------------------------------------------------------------------
	// Method:		GetChunkSize
	// Arguments:	name - std::string - The name of the chunk to return the size of
	// Returns:     The size of the chunk
	// Description: If the chunk is compressed, it returns the uncompressed size.
	// Costs:		Immediate: Low, Later: None
	// ----------------------------------------------------------------------------------
	int GetChunkSize(std::string name);

	// ----------------------------------------------------------------------------------
	// Method:		GetChunkParentFile
	// Arguments:	chunkName - std::string - The name of the chunk to find
	// Returns:		std::string - the name of the file (including full path)
	// Description: This call breaks the encapsulation of the chunk abstraction layer.
	//				However it allows the engine to clean up after creature exports etc.
	// Costs:		Immediate: Low, Later: None
	// ----------------------------------------------------------------------------------
	std::string GetChunkParentFile(std::string chunkName);

	// Adds an extension.
	void AddChunkFileExtension(std::string& chunkExtension)
	{
		myChunkFileExtensions.push_back(chunkExtension);
	}

	// Clears extension list
	void ClearChunkFileExtensionList() { myChunkFileExtensions.clear(); }

	~PrayManager() { dirnames.clear(); myChunkList.clear(); myFileToChunkMap.clear(); myChunkFlags.clear(); }
private:

	void AddFile(std::string filename);

	std::string langid;
	std::vector<std::string> dirnames;

	// These are the Chunks we know already
	std::map<std::string,PrayChunkPtr> myChunkList;

	// This map is a map of chunkname -> fileoffset;
	
	std::map<std::string, FileOffset> myFileToChunkMap;
	std::map<std::string, FlagsType> myChunkFlags;

	std::list<std::string> myChunkFileExtensions;

};

#endif //PRAYMANAGER_H

