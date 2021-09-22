#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "GenomeStore.h"
#include "Genome.h"
#include "../File.h"
#include "../FilePath.h"
#include "../App.h"
#include "../World.h"

#ifndef C2E_OLD_CPP_LIB
// stringstream is a newish addition to the std lib...
#include <sstream>
#include <iomanip>
#endif // C2E_OLD_CPP_LIB


// -------------------------------------------------------------------------
// Public

void GenomeStore::ClearSlot(int index)
{
	ASSERT(AssertValid());
	std::string moniker = MonikerAsString(index);
	if (!moniker.empty())
	{
		theApp.GetWorld().MarkFileForAttic(GetFilePath(moniker));
		myMonikers[index] = "";
	}
	ASSERT(AssertValid());
}

void GenomeStore::ClearAllSlots()
{
	int n = myMonikers.size();
	for (int i = 0; i < n; ++i)
		ClearSlot(i);
}

GenomeStore::~GenomeStore()
{
	// make sure ClearAllSlots was called before this
#ifdef _DEBUG
	int n = myMonikers.size();
	for (int i = 0; i < n; ++i)
		ASSERT(myMonikers[i].empty());
#endif
}

// returns false if slot invalid
bool GenomeStore::MoveSlotFrom(int index, GenomeStore& fromStore, int fromIndex)
{
	ASSERT(AssertValid());
	std::string moniker = fromStore.MonikerAsString(fromIndex);
	if (moniker.empty())
	{
		ClearSlot(index);
		return true;
	}	

	if (FillSlot(index, moniker))
	{
		ASSERT(fromIndex < fromStore.myMonikers.size());
		ASSERT(!fromStore.myMonikers[fromIndex].empty());
		fromStore.myMonikers[fromIndex] = "";
		ASSERT(AssertValid());
		return true;
	}

	ASSERT(AssertValid());
	return false;
}

// Return false if file not found
bool GenomeStore::LoadEngineeredFile(int index, std::string engineeredFilename)
{
	ASSERT(AssertValid());

	std::vector<std::string> files;
	GetFilesInDirectory(theApp.GetDirectory(GENETICS_DIR), files, engineeredFilename + ".gen");
	if (files.size() == 0)
	{
		std::string worldDir;
		theApp.GetWorldDirectoryVersion(GENETICS_DIR, worldDir, true);
		GetFilesInDirectory(worldDir, files, engineeredFilename + ".gen");
	}

	if (files.size() == 0)
		return false;

	std::string file = files[Rnd(0, files.size() - 1)];
	FilePath filepathEng(file, GENETICS_DIR);
	std::string filenameEng = filepathEng.GetFullPath();

	if (!File::FileExists(filenameEng))
	{
		ASSERT(false);
		return false;
	}

	Genome newGenome;
	newGenome.ReadFromFile(filenameEng, 0, 0, 0, "");

	int genus = newGenome.GetGenus();
	std::string newMoniker = GenerateUniqueMoniker("", "", genus);
	newGenome.SetMoniker(newMoniker);

	// set the G_GENUS gene to show we don't know our parents - 
	// the engine only guarantees to know them when it made them
	// itself
	newGenome.DeclareUnverifiedParents();

	std::string filenameWorld = Filename(newMoniker);
	newGenome.WriteToFile(filenameWorld);
	theApp.GetWorld().MarkFileCreated(GetFilePath(newMoniker));

	if (!FillSlot(index, newMoniker))
		return false;

	RegisterConceptionEventAndBasicData(newGenome, "", file, LifeEvent::typeEngineered);

	ASSERT(AssertValid());

	return true;
}

// Return false if file not found
bool GenomeStore::CloneSlot(int index, std::string monikerCloneFrom)
{
	if (monikerCloneFrom.empty())
		return false;

	ASSERT(AssertValid());

	std::string fileFrom = Filename(monikerCloneFrom);
	if (!File::FileExists(fileFrom))
	{
		ASSERT(false);
		return false;
	}

	Genome newGenome;
	newGenome.ReadFromFile(fileFrom, 0, 0, 0, "");

	int genus = newGenome.GetGenus();
	std::string newMoniker = GenerateUniqueMoniker(monikerCloneFrom, "", genus);
	newGenome.SetMoniker(newMoniker);

	// set the G_GENUS gene to show we don't know our parents - 
	// the engine only guarantees to know them when it made them
	// itself
	newGenome.DeclareUnverifiedParents();

	std::string filenameWorld = Filename(newMoniker);
	newGenome.WriteToFile(filenameWorld);
	theApp.GetWorld().MarkFileCreated(GetFilePath(newMoniker));

	if (!FillSlot(index, newMoniker))
		return false;

	RegisterConceptionEventAndBasicData(newGenome, monikerCloneFrom, "", LifeEvent::typeCloned);

	ASSERT(AssertValid());

	return true;
}

// returns false if not valid gene variables
bool GenomeStore::CrossoverFrom(int index, GenomeStore& motherStore, int motherIndex,
				   GenomeStore& fatherStore, int fatherIndex,
				   int motherChance, int motherDegree,
				   int fatherChance, int fatherDegree,
				   bool natural)
{
	ASSERT(AssertValid());

	std::string motherMoniker = motherStore.MonikerAsString(motherIndex);
	std::string fatherMoniker = fatherStore.MonikerAsString(fatherIndex);
	if (motherMoniker.empty() || fatherMoniker.empty())
		return false;

	Genome motherGenome(motherStore, motherIndex, 0, 0, 0);
	Genome fatherGenome(fatherStore, fatherIndex, 0, 0, 0);


	Genome childGenome;
	childGenome.Cross("", &motherGenome, &fatherGenome,
			motherChance, motherDegree, fatherChance, fatherDegree);

	int genus = childGenome.GetGenus();
	std::string childMoniker = GenerateUniqueMoniker(motherMoniker, fatherMoniker, genus);
	childGenome.SetMoniker(childMoniker);

	std::string filenameWorld = Filename(childMoniker);
	childGenome.WriteToFile(filenameWorld);
	theApp.GetWorld().MarkFileCreated(GetFilePath(childMoniker));

	if (!FillSlot(index, childMoniker))
		ASSERT(false);

	RegisterConceptionEventAndBasicData(childGenome, motherMoniker, fatherMoniker,
		natural ? LifeEvent::typeConcieved : LifeEvent::typeSpliced);
	
	ASSERT(AssertValid());

	return true;
}


// returns false if not valid gene variables
bool GenomeStore::IdenticalTwinFrom(int index,
				   GenomeStore& motherStore, int motherIndex,
				   GenomeStore& fatherStore, int fatherIndex,
				   std::string twinMoniker)
{
	ASSERT(AssertValid());

	std::string motherMoniker = motherStore.MonikerAsString(motherIndex);
	std::string fatherMoniker = fatherStore.MonikerAsString(fatherIndex);
	if (motherMoniker.empty() || fatherMoniker.empty() || twinMoniker.empty())
		return false;

	std::string fileFrom = Filename(twinMoniker);
	if (!File::FileExists(fileFrom))
	{
		ASSERT(false);
		return false;
	}

	Genome newGenome;
	newGenome.ReadFromFile(fileFrom, 0, 0, 0, "");

	int genus = newGenome.GetGenus();
	std::string newMoniker = GenerateUniqueMoniker(motherMoniker, fatherMoniker, genus);
	newGenome.SetMoniker(newMoniker);

	std::string filenameWorld = Filename(newMoniker);
	newGenome.WriteToFile(filenameWorld);
	theApp.GetWorld().MarkFileCreated(GetFilePath(newMoniker));

	if (!FillSlot(index, newMoniker))
		ASSERT(false);

	RegisterConceptionEventAndBasicData(newGenome, motherMoniker, fatherMoniker,
		LifeEvent::typeConcieved);
	
	ASSERT(AssertValid());

	return true;
}

std::string GenomeStore::MonikerAsString(int index)
{
	ASSERT(AssertValid());
	if (index >= 0 && index < myMonikers.size())
		return myMonikers[index];
	else
		return "";
}	

// -------------------------------------------------------------------------
// Protected

// return false if index invalid
bool GenomeStore::FillSlot(int index, std::string newMoniker)
{
	ASSERT(AssertValid());
	ASSERT(!newMoniker.empty());

	ClearSlot(index);
	if (index >= myMonikers.size())
		myMonikers.resize(index + 1);

	ASSERT(index < myMonikers.size());

	if (index >= 0)
	{
		myMonikers[index] = newMoniker;
		ASSERT(AssertValid());
		return true;
	}
	ASSERT(AssertValid());
	return false;
}

#ifdef _DEBUG
bool GenomeStore::AssertValid()
{
	int n = myMonikers.size();
	for (int i = 0; i < n; ++i)
	{
		std::string moniker = myMonikers[i];
		if (!moniker.empty())
			ASSERT(FileExists(moniker));
	}
	return true;
}
#endif

// static
bool GenomeStore::FileExists(std::string moniker)
{
	std::string filename = Filename(moniker);
	if (filename.empty())
		return false;
	else
		return File::FileExists(filename);
}

// static
std::string GenomeStore::Filename(std::string moniker)
{
	ASSERT(!moniker.empty());
	if (moniker.empty())
		return "";

	FilePath filepath = GetFilePath(moniker);
	std::string filename;
	filepath.GetWorldDirectoryVersionOfTheFile(filename, true);

	return filename;
}

FilePath GenomeStore::GetFilePath(std::string moniker)
{
	ASSERT(!moniker.empty());
	FilePath filepath(moniker + ".gen", GENETICS_DIR);
	return filepath;
}

std::string GenomeStore::GenerateUniqueMoniker(const std::string& monikerSeed1, const std::string& monikerSeed2, int genus)
{
	int tryCount = 0;
	std::string moniker;
	do
	{
		moniker = TryGenerateUniqueMoniker(monikerSeed1, monikerSeed2, genus);
		++tryCount;
		ASSERT(moniker.size() == 32);
	}
	while(FileExists(moniker));

	// If it takes more than one go then, unless we're _very_
	// unlucky, TryGenerateUniqueMoniker isn't unique enough.
	ASSERT(tryCount == 1);
	
	ASSERT(!moniker.empty());
	return moniker;
}

// Statistically unique across all computers anywhere
std::string GenomeStore::TryGenerateUniqueMoniker(const std::string& monikerSeed1, const std::string& monikerSeed2, int genus)
{
#ifndef C2E_OLD_CPP_LIB
	std::ostringstream friendlyTagSream;
	friendlyTagSream << "Moniker Friendly Names " << genus + 1;
	std::string friendlyTag = friendlyTagSream.str();
#else
	char buf[128];
	sprintf( buf, "Moniker Friendly Names %d", genus+1);
	std::string friendlyTag(buf);
#endif
	if (!theCatalogue.TagPresent(friendlyTag))
		friendlyTag = "Moniker Friendly Names";

	int friendlyMax = theCatalogue.GetArrayCountForTag(friendlyTag);
	int friendly = Rnd(0, friendlyMax - 1);
	std::string friendlyPart = theCatalogue.Get(friendlyTag, friendly);
	
	// ensure _exactly_ four characters
	friendlyPart = friendlyPart + friendlyPart + friendlyPart + friendlyPart + "xxxx";
	friendlyPart.assign(friendlyPart, 0, 4);
	
	// Next we need a generation number :)
	int generationOne = 0, generationTwo = 0;
	{
#ifndef C2E_OLD_CPP_LIB
		std::istringstream monikerStream1(monikerSeed1);
		monikerStream1 >> generationOne;
		std::istringstream monikerStream2(monikerSeed2);
		monikerStream2 >> generationTwo;
#else
		// TODO: check that this produces correct generation numbers!
		generationOne = monikerSeed1[0]*100 +
			monikerSeed1[1]*10 +
			monikerSeed1[2];
		generationTwo = monikerSeed2[0]*100 +
			monikerSeed2[1]*10 +
			monikerSeed2[2];
#endif
	}
	// Generation number defined as Max+1 (Because Helen Birchmore (Yes Blame her!) said so :)
	int ownGeneration = (generationOne < generationTwo) ? generationTwo + 1 : generationOne + 1;
	// special case for cloning to preserve generation
	if (!monikerSeed1.empty() && monikerSeed2.empty())
		ownGeneration = generationOne;

	std::string moniker = GenerateUniqueIdentifier(monikerSeed1, monikerSeed2);
#ifndef C2E_OLD_CPP_LIB
	std::ostringstream out;
	out << std::setfill('0') << std::setw(3) << ownGeneration;
	return out.str() + "-" + friendlyPart + "-" + moniker;
#else
	// buf is defined above
	sprintf( buf, "%03d", ownGeneration );
	return std::string(buf) + "-" + friendlyPart + "-" + moniker;
#endif
}



CreaturesArchive &operator<<( CreaturesArchive &archive, GenomeStore const &genomeStore )
{
	archive << genomeStore.myMonikers;
	return archive;
}

CreaturesArchive &operator>>( CreaturesArchive &archive, GenomeStore &genomeStore )
{
	archive >> genomeStore.myMonikers;
	return archive;
}

int GenomeStore::GetSlotCount()
{
	return myMonikers.size();
}

void GenomeStore::RegisterConceptionEventAndBasicData(Genome& genome, const std::string& monikerMum, const std::string& monikerDad, LifeEvent::EventType event)
{
	std::string moniker = genome.GetMoniker();
	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	CreatureHistory& history = historyStore.GetCreatureHistory(moniker);

	// store basic info in creature history
	history.myGenus = genome.GetGenus();
	history.myCrossoverMutationCount = genome.myCrossoverMutationCount;
	history.myCrossoverCrossCount = genome.myCrossoverCrossCount;

	// fire conception event
	history.AddEvent(event, monikerMum, monikerDad);

	// if natural, trigger pregnant and impregnated events
	if (event == LifeEvent::typeConcieved)
	{
		CreatureHistory& motherHistory = historyStore.GetCreatureHistory(monikerMum);
		motherHistory.AddEvent(LifeEvent::typeBecamePregnant, moniker, monikerDad);
		CreatureHistory& fatherHistory = historyStore.GetCreatureHistory(monikerDad);
		fatherHistory.AddEvent(LifeEvent::typeImpregnated, moniker, monikerMum);
	}
	else if (event == LifeEvent::typeCloned)
	{
		CreatureHistory& motherHistory = historyStore.GetCreatureHistory(monikerMum);
		motherHistory.AddEvent(LifeEvent::typeClonedSource, moniker, "");
	}
}


void GenomeStore::ClonedEntirely()
{
	ASSERT(AssertValid());
	for( int i=0; i < GetSlotCount(); i++ )
	{
		std::string beforeMoniker = MonikerAsString(i);
		if (!beforeMoniker.empty())
		{
			// clone it
			CloneSlot(i, beforeMoniker);

			// mark the old file as created to remove it from the "to be deleted"
			// list which ClearSlot (called by FillSlot called by CloneSlot)
			// will have put it on
			theApp.GetWorld().MarkFileCreated(GetFilePath(beforeMoniker));

			// preserve gender and variant
			std::string afterMoniker = MonikerAsString(i);
			HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
			CreatureHistory& historyBefore = historyStore.GetCreatureHistory(beforeMoniker);
			CreatureHistory& historyAfter = historyStore.GetCreatureHistory(afterMoniker);
			historyAfter.myGender = historyBefore.myGender;
			historyAfter.myVariant = historyBefore.myVariant;
		}
	}
	ASSERT(AssertValid());
}

