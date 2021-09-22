// --------------------------------------------------------------------------------------
// Filename:	PrayHandlers.cpp
// Class:		PrayHandlers
// Purpose:		Centralised CAOS routines for PRAY commands
// Description:
//  The PrayHandlers class provides the CAOS machine with the PRAY commands, and rValues
//
// History:
//  24Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "PrayHandlers.h"

#include "PrayManager.h"
#include "PrayChunk.h"
#include "PrayException.h"
#include "StringIntGroup.h"
#include "PrayBuilder.h"

#include "../../engine/Caos/CAOSMachine.h"
#include "../../engine/Caos/CAOSVar.h"
#include "../../engine/App.h"
#include "../../engine/AppConstants.h"
#include "../../engine/FilePath.h"
#include "../../engine/CosInstaller.h"
#include "../../engine/World.h"
#include "../../engine/Creature/Creature.h"
#include "../../engine/Creature/ReproductiveFaculty.h"
#include "../../engine/Display/DisplayEngine.h"
#include "../BasicException.h"

#ifndef _WIN32
#include <sys/stat.h>
#include <unistd.h>
// for MoveFile, DeleteFile etc...
#include "../../engine/unix/FileFuncs.h"
#endif

#include <string>
#include <vector>
#include <iostream>
#include <strstream>

#ifndef C2E_OLD_CPP_LIB
#include <sstream>
#endif

//#include <direct.h>

/*
BasicException helper_Convert(PrayException& pe)
{
#ifdef C2E_OLD_CPP_LIB
	char hackbuf[512];
	std::ostrstream str( hackbuf, sizeof(hackbuf) );
#else
	std::ostringstream str;
#endif
	str << "Exception in Pray System - eid = " << pe.GetCode() << ", msg = " << pe.GetMessage();
#ifdef C2E_OLD_CPP_LIB
	return BasicException(str.str());
#else
	return BasicException(str.str().c_str());
#endif
}
*/

#define helper_Convert(pe) BasicException(pe.GetMessage().c_str())

int PrayHandlers::IntegerRV_PRAY_TEST( CAOSMachine& vm )
{
	std::string name;
	vm.FetchStringRV(name);
	try
	{
		return theApp.GetResourceManager().CheckChunk(name);
	}
	catch (PrayException& pe)
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}
	catch (...)
	{
		vm.ThrowRunError( 0 );
	}
	return -1;
}

void PrayHandlers::Command_PRAY( CAOSMachine& vm )
{
	static CommandHandler subcmds [] = 
	{
		SubCommand_PRAY_REFR,
		SubCommand_PRAY_GARB
	};
	int thiscmd;
	thiscmd = vm.FetchOp();
	(subcmds[thiscmd])( vm );
}

int PrayHandlers::IntegerRV_PRAY( CAOSMachine& vm )
{
	static IntegerRVHandler subhandlers [] =
	{
		IntegerRV_PRAY_COUN,
		IntegerRV_PRAY_AGTI,
		IntegerRV_PRAY_DEPS,
		IntegerRV_PRAY_FILE,
		IntegerRV_PRAY_TEST,
		IntegerRV_PRAY_INJT,
		IntegerRV_PRAY_SIZE,
		IntegerRV_PRAY_EXPO,
		IntegerRV_PRAY_IMPO,
		IntegerRV_PRAY_MAKE
	};
	int thisrv;
	thisrv = vm.FetchOp();
	return (subhandlers[thisrv])( vm );
}

int PrayHandlers::IntegerRV_PRAY_MAKE( CAOSMachine& vm )
{
	int WhichJournal = vm.FetchIntegerRV();
	std::string journalName, prayName;
	vm.FetchStringRV(journalName);
	int WhichPray = vm.FetchIntegerRV();
	vm.FetchStringRV(prayName);
	CAOSVar& report = vm.FetchVariable();

	std::string aDir;
	if (WhichJournal == 0)
		theApp.GetWorldDirectoryVersion(JOURNAL_DIR, aDir, false);
	else
		aDir = theApp.GetDirectory(JOURNAL_DIR);

	if (aDir.empty())
		vm.ThrowRunError(CAOSMachine::sidPrayBuilderError, "Journal Folder not found");

	char folderBuffer[1024];
	getcwd(folderBuffer,1024);
	chdir(aDir.c_str());

	journalName = aDir + journalName;
	if (WhichPray == 0)
		aDir = theApp.GetDirectory(PRAYFILE_DIR);
	else
		aDir = theApp.GetDirectory(CREATURES_DIR);

	if (aDir.empty())
	{
		chdir(folderBuffer);
		vm.ThrowRunError(CAOSMachine::sidPrayBuilderError, "Specified Pray Folder not found");
	}
	prayName = aDir + prayName;


	PrayBuilder pb(journalName,prayName);
	report.SetString(pb.Output);
	chdir(folderBuffer);
	return pb.SuccessfulBuild?0:1;
}

void PrayHandlers::StringRV_PRAY( CAOSMachine& vm, std::string& str )
{
	static StringRVHandler substrings [] =
	{
		StringRV_PRAY_PREV,
		StringRV_PRAY_NEXT,
		StringRV_PRAY_AGTS
	};
	int thisst;
	thisst = vm.FetchOp();
	(substrings[thisst])( vm, str );
}

int PrayHandlers::IntegerRV_PRAY_SIZE( CAOSMachine& vm)
{
	std::string name;
	vm.FetchStringRV(name);
	try
	{
		return theApp.GetResourceManager().GetChunkSize(name);
	}
	catch (PrayException& pe)
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}
	catch (...)
	{
		vm.ThrowRunError( 0 );
	}
	// Woohoo - failure!!!
	return -1; // -1 == failure :)
}

int PrayHandlers::IntegerRV_PRAY_INJT( CAOSMachine& vm)
{
	std::string name;
	vm.FetchStringRV(name);
	int doInjt = vm.FetchIntegerRV();
	CAOSVar& report = vm.FetchVariable();
	
	if (Dependency_Helper(vm,name,doInjt))
	{
		report.SetString("Failed Dependency Scan");
		return -3; // Failed Dependency scan :(
	}

	try
	{
		// 1. Get Chunk from Manager...
		PrayChunkPtr chunk(theApp.GetResourceManager().GetChunk(name));
		// 2. With the chunk, now construct an input stream around it
		std::istrstream ist((char*)chunk->GetData(),chunk->GetSize());
		// 3. Now we have the stream, construct the StringIntGroup around it...
		StringIntGroup vals(ist);
		// 4. Now we have the group object, let's bingle on through the systems
		std::string scr;
		int count;
		if (!vals.FindInt("Agent Type",count))
			return -1;
		if (!vals.FindInt("Script Count",count))
			return -2;
		int loop;
		for(loop=0;loop<count;loop++)
		{
#ifdef C2E_OLD_CPP_LIB
			char hackbuf[512];
			std::ostrstream st( hackbuf, sizeof(hackbuf) );
#else
			std::stringstream st;
#endif
			st << "Script " << (loop+1);
			if (!vals.FindString(st.str(),scr))
			{
				// Script "n" has failed... let's report it...
				report.SetString(st.str());
				return -1;  // -1 == failed to find script in report var
			}
		}
		// Right then, we know that the scripts work, so let's play "Installey"
		if (!doInjt)
			return 0;
		for(loop=0;loop<count;loop++)
		{
#ifdef C2E_OLD_CPP_LIB
			char hackbuf[512];
			std::ostrstream st( hackbuf, sizeof(hackbuf) );
#else
			std::stringstream st;
#endif
			st << "Script " << (loop+1);
			try
			{
				std::string ignorableCAOS = "";
				CosInstaller c(ignorableCAOS);
				std::string scrpt;
				vals.FindString(st.str(),scrpt);
#ifdef C2E_OLD_CPP_LIB
				std::istrstream scrp( scrpt.c_str() );
#else
				std::istringstream scrp(scrpt);
#endif
				if (!c.ReadScriptStream(scrp,false))
				{
					report.SetString(st.str());
					return -2; // -2 == failed to install script in report var
				}
	
			}
			catch ( ... )
			{
				report.SetString(st.str());
				return -2; // -2 == failed to install script in report var.
			}
			
		}
	}
	catch (PrayException& pe)
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}
	catch (...)
	{
		vm.ThrowRunError( 0 );
	}
	// Woohoo - success!!!
	return 0; // 0 == success :)
}

void PrayHandlers::SubCommand_PRAY_REFR( CAOSMachine& vm )
{
	try
	{
		theApp.GetResourceManager().RescanFolders();
	}
	catch (PrayException& pe)
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}
	catch (...)
	{
		vm.ThrowRunError( 0 );
	}
}

void PrayHandlers::SubCommand_PRAY_GARB( CAOSMachine& vm )
{
	bool force = (vm.FetchIntegerRV() != 0);
	theApp.GetResourceManager().GarbageCollect( force );
}

int PrayHandlers::IntegerRV_PRAY_FILE( CAOSMachine& vm )
{
	// The purpose of this command is to install a file.
	std::string filename;
	vm.FetchStringRV(filename);
	int where = vm.FetchIntegerRV();
	bool doIt = vm.FetchIntegerRV() != 0;
	// Simply this...
	return InstallFile(filename,ConvertCategoryToAppId(where),doIt)?0:1;
}

int PrayHandlers::IntegerRV_PRAY_COUN( CAOSMachine& vm )
{
	std::string type;
	vm.FetchStringRV(type);
	std::vector<std::string> temp;
	theApp.GetResourceManager().GetChunks(type,temp);
	return temp.size();
}

int PrayHandlers::ConvertCategoryToAppId(int cat)
{
	switch (cat)
	{
	case 0: return MAIN_DIR;
	case 1: return SOUNDS_DIR;
	case 2: return IMAGES_DIR;
	case 3: return GENETICS_DIR;
	case 4: return BODY_DATA_DIR;
	case 5: return OVERLAYS_DIR;
	case 6: return BACKGROUNDS_DIR;
	case 7: return CATALOGUE_DIR;
	case 8: return -1; // Denied :) return BOOTSTRAP_DIR;
	case 9: return -1; // Denied :) return WORLDS_DIR;
	case 10: return CREATURES_DIR;
	case 11: return -1; // Denied :) return PRAYFILE_DIR;
	default:
		return -1;
	}
	return -1;
}

int PrayHandlers::IntegerRV_PRAY_AGTI( CAOSMachine& vm )
{
	std::string name;
	vm.FetchStringRV(name);
	std::string tag;
	vm.FetchStringRV(tag);
	int def = vm.FetchIntegerRV();
	try
	{
		// 1. Get Chunk from Manager...
		PrayChunkPtr chunk(theApp.GetResourceManager().GetChunk(name));
		// 2. With the chunk, now construct an input stream around it
		std::istrstream ist((char*)chunk->GetData(),chunk->GetSize());
		// 3. Now we have the stream, construct the StringIntGroup around it...
		StringIntGroup vals(ist);
		// 4. Now we have the group object, simply fetch the int...
		vals.FindInt(tag,def);
	}
	catch (PrayException& pe)
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}
	catch (...)
	{
		vm.ThrowRunError( 0 );
	}
	
	return def;
}

bool PrayHandlers::InstallFile(std::string& filename, int category, bool doInstall)
{
	// Determine source of the file...
	int src = 0;
	
	// Could we install the file if need be ?
	if (theApp.GetResourceManager().CheckChunk(filename))
		src = 1;

	std::string mainified = theApp.GetDirectory(category);
	mainified.append(filename);
#ifdef _WIN32
	if (GetFileAttributes(mainified.c_str()) != -1)
		src += 4;
#else
	struct stat statbuf;
	if( stat( mainified.c_str(), &statbuf ) == 0 )
		src += 4;
#endif

	// Now then, we know where the best/known place is for the file...
	
	if (!doInstall)
		return src != 0;

	// Hmm, we are installing the file, let's see first of all if we have it.

	if (src>1)
		return true;  // No need to install the file, it's there already

	// Okay, we need to install the file... we install it by writing it to "Worldified"

	if (src == 0)
	{
		// Erkle, the file is not installed, and not available, let's fail :(
		return false;
	}

	PrayChunkPtr theFile(theApp.GetResourceManager().GetChunk(filename));

	FILE* f;
	if ((f=fopen(mainified.c_str(),"wb")) == NULL)
	{
		// Erk - we have failed to install the file - exit silently :(
		return false;		// I.E. Fail on resource
	}

	// Okay the file was opened.....

	if (fwrite(theFile->GetData(),1,theFile->GetSize(),f) != theFile->GetSize())
	{
		// Aah - serious problem here - failed to write the file :(
		fclose(f);
		DeleteFile(mainified.c_str());
		return false;
	}

	// Okay, we wrote the file out, close it
	fclose(f);

	// If we are installing a catalogue, then reset the catalogue entries

	if (category == CATALOGUE_DIR)
	{
		theApp.InitLocalisation();
	}

	if ((category == IMAGES_DIR) || 
		(category == BACKGROUNDS_DIR) ||
		(category == OVERLAYS_DIR))
	{
		// Convert it...
		std::string theImageFileName = mainified;
		std::string tempFileName = mainified;
		std::string ext = theImageFileName.substr(theImageFileName.length()-3);
		if (ext == "s16" || ext == "S16")
			tempFileName += ".tmp.s16";
		else if (ext == "c16" || ext == "C16")
			tempFileName += ".tmp.c16";
		else if (ext == "blk" || ext == "BLK" || ext == "Blk" ||
			     ext == "bLk" || ext == "blK" || ext == "BLk" ||
				 ext == "BlK" || ext == "bLK" || ext == "BLK")
			tempFileName += ".tmp.blk";
		else
			tempFileName = "";
		if (tempFileName.empty() == false)
		{
			DisplayEngine::theRenderer().SafeImageConvert(
				theImageFileName,
				tempFileName,
				DisplayEngine::theRenderer().GetMyPixelFormat(),
				tempFileName);
			::DeleteFile(tempFileName.c_str());
		}
	}
	// And say "Wahey :)"
	return true;
}

int PrayHandlers::IntegerRV_PRAY_DEPS( CAOSMachine& vm )
{
	// We would do the deps here :)
	std::string name;
	vm.FetchStringRV(name);
	int doinst = vm.FetchIntegerRV();
	return Dependency_Helper(vm,name,doinst);
}

int PrayHandlers::Dependency_Helper(CAOSMachine& vm, std::string& name, int doinst)
{
	try
	{
		// 1. Get Chunk from Manager...
		PrayChunkPtr chunk(theApp.GetResourceManager().GetChunk(name));
		// 2. With the chunk, now construct an input stream around it
		std::istrstream ist((char*)chunk->GetData(),chunk->GetSize());
		// 3. Now we have the stream, construct the StringIntGroup around it...
		StringIntGroup vals(ist);

		// Our group object is now ready for use.. First of all, check for the C3 Agent Details...

		std::string dep;
		int count,cat;
		if (!vals.FindInt("Agent Type",count))
			return -1;
		if (!vals.FindInt("Dependency Count",count))
			return -2;
		int loop;
		for (loop=0;loop<count;loop++)
		{
#ifdef C2E_OLD_CPP_LIB
			char hackbuf1[128];
			char hackbuf2[128];
			std::ostrstream st( hackbuf1, sizeof( hackbuf1) );
			std::ostrstream cst( hackbuf2, sizeof( hackbuf2) );
#else
			std::stringstream st,cst;
#endif
			st << "Dependency " << (loop+1);
			cst << "Dependency Category " << (loop+1);
			if (!vals.FindString(st.str(),dep))
				return -(loop+3);
			if (!vals.FindInt(cst.str(),cat))
				return -((loop+3)+count+count);
			cat = ConvertCategoryToAppId(cat);
			if (cat == -1)
				return ((loop+1)+count+count);
		}

		// Okay then - the agent is correctly formed enough for us to do a dependency evaluate on...
		for(loop=0;loop<count;loop++)
		{
#ifdef C2E_OLD_CPP_LIB
			char hackbuf1[128];
			char hackbuf2[128];
			std::ostrstream st( hackbuf1, sizeof( hackbuf1) );
			std::ostrstream cst( hackbuf2, sizeof( hackbuf2) );
#else
			std::stringstream st,cst;
#endif
			st << "Dependency " << (loop+1);
			cst << "Dependency Category " << (loop+1);
			vals.FindString(st.str(),dep);
			vals.FindInt(cst.str(),cat);
			// We transform the cat (file category) into an App directory id...
			cat = ConvertCategoryToAppId(cat);
			if (cat == -1)
				return ((loop+1)+count+count);
			// Okay then, we have a valid file in a valid place...
			// Try to install (or check to install) the file
			if (!InstallFile(dep,cat,doinst!=0))
				// Okay then, we can't find the file anywhere sensible, so fail on this dep...
				return loop+1;
		}
	}
	catch (PrayException& pe)
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}
	catch (...)
	{
		vm.ThrowRunError( 0 );
	}
	
	return 0;
}

inline std::vector<std::string>::iterator helper_find(std::vector<std::string>& vec, std::string& what)
{
	std::vector<std::string>::iterator it = vec.begin();
	while (it != vec.end())
	{
		if ((*it) == what)
			return it;
		it++;
	}
	return it;
}

void PrayHandlers::StringRV_PRAY_PREV( CAOSMachine& vm, std::string& str )
{
	std::string type,name;
	vm.FetchStringRV(type);
	vm.FetchStringRV(name);
	std::vector<std::string> chunks;
	theApp.GetResourceManager().GetChunks(type,chunks);
	if (chunks.size() == 0)
	{
		str = "";
		return;
	}
	std::vector<std::string>::iterator it = helper_find(chunks,name);
	if (it == chunks.begin())
		it = chunks.end();
	str = *(--it);
}

void PrayHandlers::StringRV_PRAY_NEXT( CAOSMachine& vm, std::string& str )
{
	std::string type,name;
	vm.FetchStringRV(type);
	vm.FetchStringRV(name);
	std::vector<std::string> chunks;
	theApp.GetResourceManager().GetChunks(type,chunks);
	if (chunks.size() == 0)
	{
		str = "";
		return;
	}
	std::vector<std::string>::iterator it = helper_find(chunks,name);
	if (it != chunks.end())
		it++;
	if (it == chunks.end())
		it = chunks.begin();
	str = *it;
}

void PrayHandlers::StringRV_PRAY_AGTS( CAOSMachine& vm, std::string& str )
{
	std::string name;
	vm.FetchStringRV(name);
	std::string tag;
	vm.FetchStringRV(tag);
	vm.FetchStringRV(str);
	try
	{
		// 1. Get Chunk from Manager...
		PrayChunkPtr chunk(theApp.GetResourceManager().GetChunk(name));
		// 2. With the chunk, now construct an input stream around it
		std::istrstream ist((char*)chunk->GetData(),chunk->GetSize());
		// 3. Now we have the stream, construct the StringIntGroup around it...
		StringIntGroup vals(ist);
		// 4. Now we have the group object, simply fetch the string...
		vals.FindString(tag,str);
	}
	catch (PrayException& pe)
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}
	catch (...)
	{
		vm.ThrowRunError( 0 );
	}
}

int PrayHandlers::IntegerRV_PRAY_IMPO( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV( moniker );

	bool successReconcile = true;

	bool doStuff = (vm.FetchIntegerRV() == 1);
	bool porching = (vm.FetchIntegerRV() != 1);

	if (theApp.GetResourceManager().CheckChunk( moniker + ".creature" ) == 0)
		return 2; // That Moniker is not in the PRAY mappings

	try
	{
	// First read in list of stored genetic files...
		std::vector< std::string > geneNames;
		const PrayChunkPtr& geneListChunk = theApp.GetResourceManager().GetChunk(moniker + ".glist.creature");

		std::strstream geneStream((char*)geneListChunk->GetData(),geneListChunk->GetSize());

		CreaturesArchive geneListArchive( geneStream, CreaturesArchive::Load );
		geneListArchive >> geneNames;

	// Now restore those genetic files...

		std::vector< std::string >::const_iterator monIter;
		int monIndex;

		std::vector< CreatureHistory > histories( geneNames.size() );
		for( monIndex = 0; monIndex < geneNames.size(); ++monIndex )
		{
			geneListArchive >> histories[ monIndex ];
			successReconcile = successReconcile && 
				theApp.GetWorld().GetHistoryStore().
				ReconcileImportedHistory( geneNames[monIndex], histories[monIndex], false );

			// Test if this moniker is in the PRAY system.
			if (theApp.GetResourceManager().CheckChunk( geneNames[monIndex] + ".genetics" ) == 0)
				return 3; // One or more genetics files required are missing from the PRAY maps.
		}

		if (!doStuff)
			return successReconcile ? 0 : 1;

		for( monIter = geneNames.begin(), monIndex = 0;
			 monIter != geneNames.end();
			 ++monIter, ++monIndex )
		{
			{
				const PrayChunkPtr& gC = theApp.GetResourceManager().GetChunk( *monIter + ".genetics" );
				std::string geneticsName = GenomeStore::Filename( *monIter );
				File f;
				f.Create( geneticsName );
				f.Write( gC->GetData(), gC->GetSize() );
				f.Close();

				FilePath filepath(*monIter + ".gen", GENETICS_DIR);
				theApp.GetWorld().MarkFileCreated(filepath);
			}

			if( successReconcile ) // We already know this will work.
				theApp.GetWorld().GetHistoryStore().
					ReconcileImportedHistory( geneNames[monIndex], histories[monIndex], true );
		}


		AgentHandle handle;

		// Get that Chunk :)

		const PrayChunkPtr& creatureChunk = theApp.GetResourceManager().GetChunk(moniker + ".creature");

		std::strstream stream((char*)creatureChunk->GetData(),creatureChunk->GetSize());

		CreaturesArchive archive( stream, CreaturesArchive::Load );
		archive.SetAgentArchiveStyle( CreaturesArchive::ONE_AGENT_ONLY_NULL_OTHERS );
		archive.SetCloningACreature(true);
		archive >> handle;

		if( !successReconcile )
		{
			// need to clone everything in the genome store
			GenomeStore& theStore = handle.GetCreatureReference().GetGenomeStore();
			theStore.ClonedEntirely();

			HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
			// copy over gender and variant from our imported file.
			// this could be different from the history in the world, if 
			// we are importing a hatched version of a Norn that is still
			// in its egg in the world, for example
			for( int i=0; i < theStore.GetSlotCount(); i++ )
			{
				if( !theStore.MonikerAsString(i).empty() )
				{
					// delete the temporary gene file if not referenced
					if (theAgentManager.FindAgentForMoniker(geneNames[i]).IsInvalid())
					{
						FilePath filepath(geneNames[i] + ".gen", GENETICS_DIR);
						theApp.GetWorld().MarkFileForAttic(filepath);
					}

					// copy over gender/variant
					CreatureHistory& historyAfter = historyStore.GetCreatureHistory(theStore.MonikerAsString(i));
					historyAfter.myGender = histories[i].myGender;
					historyAfter.myVariant = histories[i].myVariant;
					// could copy name over here as well
				}
			}
		}
		
		theAgentManager.RegisterClone( handle );
		vm.SetTarg( handle );

		handle.GetCreatureReference().RemakeSkeletonAfterSerialisation();

		if( successReconcile )
			for( monIter = geneNames.begin(); monIter != geneNames.end(); ++monIter )
				theApp.GetWorld().GetHistoryStore().GetCreatureHistory( *monIter ).AddEvent( LifeEvent::typeImported, "", "" );

		CreatureHistory &history = theApp.GetWorld().GetHistoryStore().GetCreatureHistory( moniker );
		int count = history.Count();
		for( int historyIndex = 0; historyIndex < count; ++historyIndex )
		{
			std::string photo = history.GetLifeEvent(historyIndex)->myPhoto;
			if( !photo.empty() )
			{
				const PrayChunkPtr& gC = theApp.GetResourceManager().GetChunk( photo + ".photo" );
				File f;
				FilePath filepath( photo + ".s16", IMAGES_DIR, true, true );

				// force creation of path
				std::string dummy;
				filepath.GetWorldDirectoryVersionOfTheFile(dummy, true);

				f.Create( filepath.GetFullPath() );
				f.Write( gC->GetData(), gC->GetSize() );
				f.Close();

				std::string theImageFileName = filepath.GetFullPath();
				std::string tempFileName = theImageFileName;
				std::string ext = theImageFileName.substr(theImageFileName.length()-3);
				if (ext == "s16" || ext == "S16")
					tempFileName += ".tmp.s16";
				else if (ext == "c16" || ext == "C16")
					tempFileName += ".tmp.c16";
				else if (ext == "blk" || ext == "BLK" || ext == "Blk" ||
						 ext == "bLk" || ext == "blK" || ext == "BLk" ||
						 ext == "BlK" || ext == "bLK" || ext == "BLK")
					tempFileName += ".tmp.blk";
				else
					tempFileName = "";
				if (tempFileName.empty() == false)
				{
					DisplayEngine::theRenderer().SafeImageConvert(
						theImageFileName,
						tempFileName,
						DisplayEngine::theRenderer().GetMyPixelFormat(),
						tempFileName);
					::DeleteFile(tempFileName.c_str());
				}

				theApp.GetWorld().MarkFileCreated(filepath);
			}
		}

		// Move the file to the porch (unless no porching, e.g. for starter family)
		if (porching)
		{
			std::string creatureFile = theApp.GetResourceManager().GetChunkParentFile( moniker + ".creature" ).c_str();
			theApp.GetWorld().MoveFileToPorch(creatureFile);
		}
		theApp.GetResourceManager().RescanFolders();
//		handle.GetCreatureReference().SetNormalPlane( theAgentManager.UniqueCreaturePlane( handle ) );
	}
	catch ( PrayException& pe )
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}


	return successReconcile ? 0 : 1;
}

int PrayHandlers::IntegerRV_PRAY_EXPO( CAOSMachine& vm )
{
#ifdef C2E_OLD_CPP_LIB
	const int hackbufsize = 1024*1024;
	char* hackbuf = new char[hackbufsize];
#endif
	std::string chunkName;
	vm.FetchStringRV(chunkName);
	if (chunkName.empty())
		chunkName = "EXPC";
	chunkName = chunkName + chunkName + chunkName + chunkName;
	chunkName = chunkName.substr(0, 4);

	Creature& creature = vm.GetCreatureTarg(); //Check we're looking at a valid creature

	std::string moniker = creature.GetMoniker();
	CreatureHistory& cHist = theApp.GetWorld().GetHistoryStore().GetCreatureHistory( moniker );
	std::string name = cHist.myName;
																				
	std::string filename = name + "_" + moniker + ".creature";
	for (int tempLoop=0;tempLoop<filename.length();tempLoop++)
	{
		if (filename.at(tempLoop) == '-')
			filename.at(tempLoop) = '_';
	}
	FilePath path( filename, CREATURES_DIR );

// Store the (moniker) names of the genetic files to be stored as a seperate chunk
	GenomeStore& theStore = creature.GetGenomeStore();
	std::vector< std::string > geneNames;

	std::string interestingMoniker;

	for( int i=0; i < theStore.GetSlotCount(); i++ )
	{
		interestingMoniker = theStore.MonikerAsString(i);
		if( !interestingMoniker.empty() )
		{
			if (theApp.GetResourceManager().CheckChunk( interestingMoniker + ".genetics" ) != 0)
				return 1; // already on disk in some form

			geneNames.push_back( interestingMoniker );
		}
	}

	std::vector< std::string >::const_iterator monIter;
#ifdef C2E_OLD_CPP_LIB
	std::strstream geneStream( hackbuf, hackbufsize );
#else
	std::stringstream geneStream;
#endif
	{
		CreaturesArchive geneArchive( geneStream,
			CreaturesArchive::Save );
		geneArchive << geneNames;
		
		for( monIter = geneNames.begin(); monIter != geneNames.end(); ++monIter )
		{
			CreatureHistory &history = theApp.GetWorld().GetHistoryStore().GetCreatureHistory( *monIter );
			history.AddEvent( LifeEvent::typeExported, "", "", false );
			geneArchive << history;
		}
	}

	try
	{
		theApp.GetResourceManager().AddChunkToFile( moniker + ".glist.creature", 
													"GLST", 
													path.GetFullPath(), 
#ifdef C2E_OLD_CPP_LIB
													geneStream.tellp(),
													(uint8*)hackbuf,
#else
													geneStream.str().size(), 
													(uint8*)geneStream.str().data(),
#endif
													true );
	}
	catch ( PrayException& pe )
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}

	// Make Sure Creature is not connected to others (carried etc.)

	vm.GetTarg().GetAgentReference().BreakLinksToOtherAgents();

	// Now store the creature in an archive...
#ifdef C2E_OLD_CPP_LIB
	std::strstream stream( hackbuf, hackbufsize );
#else
	std::string string;
	string.reserve( 1024 * 1024 );
	std::stringstream stream( string );
	stream.str().reserve( 1024 * 1024 );
#endif
	{
		CreaturesArchive archive( stream, CreaturesArchive::Save );
		archive.SetAgentArchiveStyle( CreaturesArchive::ONE_AGENT_ONLY_NULL_OTHERS );
		archive << vm.GetTarg();
	}
	try
	{
		theApp.GetResourceManager().AddChunkToFile( moniker + ".creature", 
													"CREA", 
													path.GetFullPath(), 
#ifdef C2E_OLD_CPP_LIB
													stream.tellp(),
													(uint8*)hackbuf,
#else
													stream.str().size(), 
													(uint8*)stream.str().data(),
#endif
													true );

// fai!temp!
/*		static char x = 'a';
		std::string boo("c:\\moose.foo");
			boo+=x;
				theApp.GetResourceManager().AddChunkToFile( moniker + ".creature", 
													"ZOOB", 
													boo+x, 
													stream.str().size(), 
													(uint8*)stream.str().data(),
													false );
		x++;
*/

	}
	catch ( PrayException& pe )
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}


	for( monIter = geneNames.begin(); monIter != geneNames.end(); ++monIter )
	{

		std::string fileName = GenomeStore::Filename( *monIter );
		File f;
		f.Open( fileName );
		uint8* geneticData = new uint8[ f.GetSize() ];
		f.Read( geneticData, f.GetSize() );
		try
		{
			theApp.GetResourceManager().AddChunkToFile( *monIter + ".genetics",
														"GENE",
														path.GetFullPath(),
														f.GetSize(),
														geneticData,
														true );
		}
		catch ( PrayException& pe )
		{
			delete [] geneticData;
			f.Close();
			vm.ThrowRunError( helper_Convert( pe ) );
		}
		delete [] geneticData;
		f.Close();
	}

	CreatureHistory &history = theApp.GetWorld().GetHistoryStore().GetCreatureHistory( moniker );
	int count = history.Count();
	for( int historyIndex = 0; historyIndex < count; ++historyIndex )
	{
		std::string photo = history.GetLifeEvent(historyIndex)->myPhoto;
		if( !photo.empty() )
		{
			File f;
			f.Open( FilePath( photo + ".s16", IMAGES_DIR ).GetFullPath() );
			uint8* data = new uint8[ f.GetSize() ];
			f.Read( data, f.GetSize() );
			try
			{
				theApp.GetResourceManager().AddChunkToFile( photo + ".photo",
															"PHOT",
															path.GetFullPath(),
															f.GetSize(),
															data,
															true );
			}
			catch ( PrayException& pe )
			{
				delete [] data;
				f.Close();
				vm.ThrowRunError( helper_Convert( pe ) );
			}
			delete [] data;
			f.Close();
		}
	}

	StringIntGroup information;
	std::string str;creature.GetBodyPartFileName(0,str);
	information.Clear();

	// From members of LifeEvent.h
	information.AddInt("Exported At World Time", cHist.GetLifeEvent( cHist.Count() - 1 )->myWorldTick);
	information.AddInt("Creature Age In Ticks", cHist.GetLifeEvent( cHist.Count() - 1 )->myAgeInTicks);
	information.AddInt("Exported At Real Time", cHist.GetLifeEvent( cHist.Count() - 1 )->myRealWorldTime);
	information.AddInt("Creature Life Stage", cHist.GetLifeEvent( cHist.Count() - 1 )->myLifeStage);
	
	information.AddString("Exported From World Name", cHist.GetLifeEvent( cHist.Count() - 1 )->myWorldName);	
	information.AddString("Exported From World UID", cHist.GetLifeEvent( cHist.Count() - 1 )->myWorldUniqueIdentifier);	

	// From members of CreatureHistory.h
	information.AddString("Creature Name", cHist.myName);
	information.AddInt("Gender", cHist.myGender);
	information.AddInt("Genus", cHist.myGenus + 1);
	information.AddInt("Variant", cHist.myVariant);

	// Other
	information.AddString("Head Gallery", str);
	information.AddInt("Pregnancy Status", creature.Reproductive()->IsPregnant()?1:0);

#ifdef C2E_OLD_CPP_LIB
	std::ostrstream sigstream( hackbuf, hackbufsize );
#else
	std::stringstream sigstream;
#endif
	information.SaveToStream(sigstream);
	try
	{
		theApp.GetResourceManager().AddChunkToFile( moniker,
													chunkName,
													path.GetFullPath(),
#ifdef C2E_OLD_CPP_LIB
													sigstream.tellp(),
													(uint8*)hackbuf,
#else
													sigstream.str().size(),
													(uint8*)sigstream.str().data(),
#endif
													true );
	}
	catch ( PrayException& pe )
	{
		vm.ThrowRunError( helper_Convert( pe ) );
	}

	if ( vm.GetTarg() == vm.GetOwner() )
		vm.GetTarg().GetAgentReference().YouAreNowDoomed();
	else
		theAgentManager.KillAgent( vm.GetTarg() );


	for( monIter = geneNames.begin(); monIter != geneNames.end(); ++monIter )
	{
		CreatureHistory &history = theApp.GetWorld().GetHistoryStore().GetCreatureHistory( *monIter );
		CAOSVar p1, p2;
		p1.SetString(*monIter);
		p2.SetInteger(history.Count() - 1);
		theAgentManager.ExecuteScriptOnAllAgentsDeferred(SCRIPT_LIFE_EVENT, NULLHANDLE, p1, p2);
	}
	
	

	theApp.GetResourceManager().RescanFolders();
	return 0;
}

