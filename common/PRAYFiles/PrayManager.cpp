// --------------------------------------------------------------------------------------
// Filename:	PrayManager.cpp
// Class:		PrayManager
// Purpose:		To encapsulate the Pray Files and provide accessor functionality
// Description:
//  The Pray Manager manages nice lists of chunks. And allows you to find them easily
//
// History:
//  21Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "PrayManager.h"
#include "PrayChunk.h"
#include "../FileLocaliser.h"
#include "PrayStructs.h"
#include "PrayException.h"

#ifdef _WIN32
#include "../zlib113/zlib.h"
#else
// on real OSes, zlib is a system lib :-)
#include <zlib.h>
#endif

#include <stdio.h>

void PrayManager::GarbageCollect(bool force)
{
	if (force)
		myChunkList.clear();
	else
	{
		for(std::map<std::string,PrayChunkPtr>::iterator it = myChunkList.begin(); it != myChunkList.end(); it++)
		{
			if ((*it).second->myCount == 1)
			{
				std::map<std::string,PrayChunkPtr>::iterator cit = it;
				it++;
				myChunkList.erase(cit);
				if (it != myChunkList.end())
					it--;
				else
					break;
			}
		}
	}
}


void PrayManager::RescanFolders()
{
	myFileToChunkMap.clear();
	myChunkFlags.clear();
	GarbageCollect(true);
	// Next we scan each file in turn, adding it to the lists :)
	FileLocaliser f;
	std::vector<std::string>::iterator it;
	std::list<std::string>::iterator eit;
	for(eit = myChunkFileExtensions.begin(); eit != myChunkFileExtensions.end(); eit++)
	for(it = dirnames.begin(); it != dirnames.end(); it++)
	{
		//Scan folder it :)
		std::vector<std::string> files;
		f.LocaliseDirContents(*it,langid,files,"*." + *eit);
		std::vector<std::string>::iterator fit;
		for(fit = files.begin(); fit != files.end(); fit++)
		{
			AddFile(*fit);
		}
	}
}

void PrayManager::AddFile(std::string filename)
{
	std::map<std::string,FileOffset> localmap;
	std::map<std::string, FlagsType> localflags;
	FILE* f;
	if ((f = fopen(filename.c_str(),"rb")) == NULL)
	{
		return;
	}
	PrayFileHeader p;
	if (fread(&p,1,sizeof(p),f) != sizeof(p))
	{
		fclose(f);
		return;
	}

	// Check that it is a pray file

	if ((p.majik[0] != 'P') ||
		(p.majik[1] != 'R') ||
		(p.majik[2] != 'A') ||
		(p.majik[3] != 'Y'))
	{
		fclose(f);
		return;
	}

	// Read all the chunks one by one and index them in the maps

	while (!feof(f))
	{
		int ofs = ftell(f);
		PrayChunkHeader ch;
		if (fread(&ch,1,sizeof(ch),f) != sizeof(ch))
		{
			break;		
		}
		FileOffset fo(filename,ofs);
		localmap[ch.name] = fo;
		std::string type;
		type = "    ";
		type.at(0) = ch.type[0];
		type.at(1) = ch.type[1];
		type.at(2) = ch.type[2];
		type.at(3) = ch.type[3];
		localflags[ch.name] = std::make_pair(ch.flags,type);
		if (fseek(f,ch.size,SEEK_CUR))
		{
			fclose(f);
			return;
		}
	}
	fclose(f);

	// Okay - promote the localmap to the supermap :)

	std::map<std::string,FileOffset>::iterator it;
	for(it = localmap.begin(); it != localmap.end(); it++)
		myFileToChunkMap[(*it).first] = (*it).second;
	std::map<std::string,FlagsType>::iterator fit;
	for(fit = localflags.begin(); fit != localflags.end(); fit++)
		myChunkFlags[(*fit).first] = (*fit).second;

}

int PrayManager::CheckChunk(std::string thisChunk)
{
	std::map<std::string,PrayChunkPtr>::iterator it;
	it = myChunkList.find(thisChunk);
	if (it != myChunkList.end())
	{
		// Woohoo we found it :)
		return 1;
	}
	// Aww - not cached :(
	// Let's check if it is on disk...
	std::map<std::string,FlagsType>::iterator fit;
	fit = myChunkFlags.find(thisChunk);
	if (fit == myChunkFlags.end())
	{
		// Awww Not on disk, let's abort :)
		return 0;
	}
	// Hmm, it's on disk, lets see if its compressed or not.
	// The flags are defined as (bit0 = compressed)
	if (((*fit).second.first & 1) == 1)
	{
		return 3;
	}
	// Okie, it's on disk, it's not compressed - cost is two...
	return 2;
}

std::string PrayManager::GetChunkParentFile(std::string chunkName)
{
	if (CheckChunk(chunkName) == 0)
	{
		throw PrayException("Chunk not found in PrayManager::GetChunkParentFile - "+chunkName, PrayException::eidChunkNotFound);
	}
	FileOffset locationOfChunk = myFileToChunkMap[chunkName];
	return (locationOfChunk.first);
}

int PrayManager::GetChunkSize(std::string name)
{
	if (CheckChunk(name) == 0)
	{
		throw PrayException("Chunk not found in PrayManager::GetChunk - "+name, PrayException::eidChunkNotFound);
	}
	std::map<std::string,PrayChunkPtr>::iterator it;
	it = myChunkList.find(name);
	if (it != myChunkList.end())
	{
		// Woohoo we found it :)
		return (*it).second->GetSize();
	}

	// Hmm, not in memory, let's get its headers from disk...

	FileOffset locationOfChunk = myFileToChunkMap[name];

	FILE* f;
	if ((f = fopen(locationOfChunk.first.c_str(),"rb")) == NULL)
	{
		// Boohoo, the file got pulled out from under us :(
		throw PrayException("Error Opening file in PrayManager::GetChunk - "+name+" @ "+locationOfChunk.first,
			PrayException::eidFilePulledOut);
	}
	if (fseek(f,locationOfChunk.second,SEEK_CUR))
	{
		// Aww, we can't seek to that chunk :(
		fclose(f);
		throw PrayException("File seek failed in PrayManager::GetChunk - "+name+" @ "+locationOfChunk.first,
			PrayException::eidFileTooShortInSeek);
	}
	PrayChunkHeader ch;
	if (fread(&ch,1,sizeof(ch),f) != sizeof(ch))
	{
		fclose(f);
		// Aww, we couldn't read the header :(
		fclose(f);
		throw PrayException("File too short in PrayManager::GetChunk (Reading Chunk Header) - "+name + " @ " + locationOfChunk.first,
			PrayException::eidFileTooShortForHeader);
	}
	// We now know the size of the Chunk's data and its compressed data.
	fclose(f);
	// And now we know the size of the chunk (usize);
	return ch.usize;
}

PrayChunkPtr PrayManager::GetChunk(std::string thisChunk)
{
	if (CheckChunk(thisChunk) == 0)
	{
		throw PrayException("Chunk not found in PrayManager::GetChunk - "+thisChunk, PrayException::eidChunkNotFound);
	}
	std::map<std::string,PrayChunkPtr>::iterator it;
	it = myChunkList.find(thisChunk);
	if (it != myChunkList.end())
	{
		// Woohoo we found it :)
		return PrayChunkPtr((*it).second);
	}
	// Aww we didn't find it :(
	// Aah well, we know it exists (or do we?)
	// Lets go get it :)

	FileOffset locationOfChunk = myFileToChunkMap[thisChunk];

	FILE* f;
	if ((f = fopen(locationOfChunk.first.c_str(),"rb")) == NULL)
	{
		// Boohoo, the file got pulled out from under us :(
		throw PrayException("Error Opening file in PrayManager::GetChunk - "+thisChunk+" @ "+locationOfChunk.first,
			PrayException::eidFilePulledOut);
	}
	if (fseek(f,locationOfChunk.second,SEEK_CUR))
	{
		// Aww, we can't seek to that chunk :(
		fclose(f);
		throw PrayException("File seek failed in PrayManager::GetChunk - "+thisChunk+" @ "+locationOfChunk.first,
			PrayException::eidFileTooShortInSeek);
	}
	PrayChunkHeader ch;
	if (fread(&ch,1,sizeof(ch),f) != sizeof(ch))
	{
		// Aww, we couldn't read the header :(
		fclose(f);
		throw PrayException("File too short in PrayManager::GetChunk (Reading Chunk Header) - "+thisChunk + " @ " + locationOfChunk.first,
			PrayException::eidFileTooShortForHeader);
	}
	// We now know the size of the Chunk's data and its compressed data.
	if ((ch.flags & 1) == 0)
	{
		// We can simply instantiate a pointer, and put our data in :)
		PrayChunkPtr retVal(ch.usize,this);
		if (fread(retVal->myPtr,1,ch.size,f) != ch.size)
		{
			// Aww the file didn't have enough data in it :(
			fclose(f);
			throw PrayException("File to short in PrayManager::GetChunk (Reading uncompressed Data) - "+thisChunk + " @ " + locationOfChunk.first,
				PrayException::eidFileTooShortForUData);
		}
		// Hurrah - we had enough data - let's return :)
		// After making an entry into our nice cachemap
		myChunkList[thisChunk] = retVal;
		fclose(f);
		return retVal;
	}
	// Hmm, we have found the chunk, but we have to uncompress it :(
	uint8* compressedData = new uint8[ ch.size ];
	if (fread(compressedData,1,ch.size,f) != ch.size)
	{
		fclose(f);
		delete [] compressedData;
		// Aww, we pooped on the load compressed data :(
		throw PrayException("File to short in PrayManager::GetChunk (Reading compressed Data) - "+thisChunk + " @ " + locationOfChunk.first,
				PrayException::eidFileTooShortForCData);
	}
	fclose(f);
	PrayChunkPtr returnVal(ch.usize,this);
	//Let's decompress the data straight into the chunk :)

	unsigned long s,us;
	s = ch.size; us = ch.usize;

	uncompress(returnVal->myPtr,&us,compressedData,s);

	delete [] compressedData;

	if (us != ch.usize)
	{
		// Aww, the uncompressed data is the wrong size :(
		throw PrayException("Data size mismatch after decompress in PrayManager::GetChunk (Checking uncompressed data) - "+thisChunk + " @ "+ locationOfChunk.first,
			PrayException::eidDataMismatch);
	}
	// Yay, decompression successful :)

	//Make an entry in the map :)
	myChunkList[thisChunk] = returnVal;

	return returnVal;
}

void PrayManager::GetChunks(std::string thisType, std::vector<std::string>& thisVec)
{
	thisVec.clear();
	std::map<std::string,FlagsType>::iterator it;
	for(it = myChunkFlags.begin(); it != myChunkFlags.end(); it++)
	{
		if ((*it).second.second == thisType)
			thisVec.push_back((*it).first);
	}
}

void PrayManager::AddChunkToFile(std::string thisName, std::string thisType, std::string thisFile,
									int thisSize, uint8* thisData, bool doCompress)
{
	//If Chunk exists, Throw
	if (CheckChunk(thisName))
		throw PrayException("Chunk already exists in PrayManager::AddChunkToFile - "+thisName, PrayException::eidChunkExists);

	//Next check if the file we want exists.
	FILE* f;
	if ((f = fopen(thisFile.c_str(),"rb")) != NULL)
	{
		//Okay it does exist, so check it's a PrayFile
		PrayFileHeader ph;
		if (fread(&ph,1,sizeof(ph),f) != sizeof(ph))
		{
			// Erk - not a prayfile by far...
			fclose(f);
			throw PrayException("File "+thisFile+" is _not_ a prayfile in PrayManager::AddChunkToFile", PrayException::eidFileTooShort);
		}
		if ((ph.majik[0] != 'P') ||
			(ph.majik[1] != 'R') ||
			(ph.majik[2] != 'A') ||
			(ph.majik[3] != 'Y'))
		{
			// Erk - not praymajik :(
			fclose(f);
			throw PrayException("File "+thisFile+" does not look like a prayfile to me. (PrayManager::AddChunkToFile)", 
				PrayException::eidNotPrayFile);
		}
		//Okay - it's a prayfile...
		fclose(f);
		f = fopen(thisFile.c_str(),"ab");
	}
	else
	{
		// Aah - the file exists not, so let us create it :)
		if ((f = fopen(thisFile.c_str(),"wb")) == NULL)
			throw PrayException("File "+thisFile+" could not be opened in PrayManager::AddChunkToFile",PrayException::eidFileNotOpen);
		PrayFileHeader ph;
		ph.majik[0] = 'P'; ph.majik[1] = 'R'; ph.majik[2] = 'A'; ph.majik[3] = 'Y'; 
		fwrite(&ph,1,sizeof(ph),f);
	}
	//Okay then f is now a writable file ready for us to dump a chunk to...
	//Stage One, really nice and simple if not compressed...
	PrayChunkHeader ch;

	for(unsigned int i=0;i<128;i++)
	{
		if (thisName.length() < (i+1))
			ch.name[i] = 0;
		else
			ch.name[i] = thisName.at(i);
	}
	ch.type[0] = thisType.c_str()[0];
	ch.type[1] = thisType.c_str()[1];
	ch.type[2] = thisType.c_str()[2];
	ch.type[3] = thisType.c_str()[3];

	ch.usize = thisSize;
	ch.flags = doCompress?1:0;
	if (!doCompress)
	{
		ch.size = ch.usize;
		fwrite(&ch,1,sizeof(ch),f);
		fwrite(thisData,1,thisSize,f);
	}
	else
	{
		uint8* cData = new uint8[ ch.usize + 4096 ];
		unsigned long len,ulen;
		len = ch.usize + 4096; ulen = ch.usize;
		if (compress(cData,&len,thisData,ulen) != Z_OK)
		{
			// oh blast :( - Failed compress :(
			delete [] cData;
			fclose(f);
			throw PrayException("Oh Pants! The compress failed!!!! (PrayManager::AddChunkToFile)", PrayException::eidCompressError);
		}
		// Hurrah - data compressed okay :)
		ch.size = len;
		fwrite(&ch,1,sizeof(ch),f);
		fwrite(cData,1,len,f);
		delete [] cData;
	}
	fclose(f);
	// Done :)
}

/*
void PrayManager::DeleteChunk(std::string chunkName)
{
	// This is a potentially medium cost, potentially high cost routine.
	// The two options we deal with are:
	//  1. Chunk is at end of the file - we use 
}

  */
