#ifndef GENOME_STORE_H
#define GENOME_STORE_H

// GenomeStore keeps a vector of monikers

#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include "History/LifeEvent.h"

#include <string>
#include <vector>
class CreaturesArchive;
class Genome;
class FilePath;

class GenomeStore
{
public:
	~GenomeStore();

	void ClearSlot(int index);
	void ClearAllSlots();
	bool MoveSlotFrom(int index, GenomeStore& fromStore, int fromIndex);
	bool CloneSlot(int index, std::string monikerCloneFrom);
	bool IdenticalTwinFrom(int index,
				   GenomeStore& motherStore, int motherIndex,
				   GenomeStore& fatherStore, int fatherIndex,
				   std::string twinMoniker);

	void ClonedEntirely();

	bool LoadEngineeredFile(int index, std::string engineeredFilename);
	bool CrossoverFrom(int index, GenomeStore& motherStore, int motherIndex,
					   GenomeStore& fatherStore, int fatherIndex,
					   int motherChance, int motherDegree,
					   int fatherChance, int fatherDegree,
					   bool natural);

	std::string MonikerAsString(int index); // for display purposes only
	static FilePath GenomeStore::GetFilePath(std::string moniker);
	static std::string Filename(std::string moniker);

	int GetSlotCount();

#ifdef _DEBUG
	bool AssertValid();
#endif

	friend CreaturesArchive &operator<<( CreaturesArchive &archive, GenomeStore const &genomeStore );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, GenomeStore &genomeStore );

protected:
	bool FillSlot(int index, std::string newMoniker);
	std::string GenerateUniqueMoniker(const std::string& monikerSeed1, const std::string& monikerSeed2, int genus);
	std::string TryGenerateUniqueMoniker(const std::string& monikerSeed1, const std::string& monikerSeed2, int genus);

	static bool FileExists(std::string moniker);

	void RegisterConceptionEventAndBasicData(Genome& genome, const std::string& monikerMum, const std::string& monikerDad, LifeEvent::EventType event);

protected:
	std::vector<std::string> myMonikers;

};


#endif GENOME_STORE_H