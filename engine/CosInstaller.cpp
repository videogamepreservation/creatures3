// --------------------------------------------------------------------------
// Filename:	CosInstaller.cpp
// Class:		CosInstaller
// Purpose:		This class is a quick way of reading in cos files and processing
//				the commands therein.
//				
// History:
// ------- 
// 05Feb98	Alima		Created. 
//							
//
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CosInstaller.h"

#include <fstream>
#ifndef C2E_OLD_CPP_LIB
#include <sstream>
#endif
#include <algorithm>
#include <strstream>

#include "App.h"
#include "World.h"
#include "Caos/CAOSMachine.h"
#include "Caos/Orderiser.h"
#include "Display/ErrorMessageHandler.h"

void eatwhite(std::istream& in)
{
	char ch = in.peek();
	while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
	{
		in.get();
		ch = in.peek();
	}
}

// this is the old constructor kept so that
// we can still load any files that are in the 
// bootstrap directory and not in subfolders
CosInstaller::CosInstaller()
{
	// there is no local world version of the bootstrap folder
	// so no extra searches needed here.
	// get list of files in bootstrap directory
	// if there is a switcher folder (which is always the very first in the list)
	// then load it if not then load whatever is in the base.  This is so as not
	// to disturb C3 layouts

	std::string path(theApp.GetDirectory(BOOTSTRAP_DIR));
	std::string switcher(path);
	switcher+="000 Switcher\\";
#ifdef _WIN32
	if(GetFileAttributes(switcher.data()) !=-1)
	{
		LoadCosFiles(switcher,false);
	}
	else
	{
		LoadCosFiles(path,false);
	}
#else
	ASSERT( false );	// TODO - non-win32 version
#endif
}

// This is the new world switcher style constructor where you tell it
// which bootstrap folders to load.
// the cosinstaller weeds through the list of bootstarp folders
// and installs all new cos files and updates the scripts from
// any other folders as required.
// not that the very first bootstrap folder is the world switcher
CosInstaller::CosInstaller(std::vector<std::string>& bootstrapFoldersToLoad)
{
#ifdef _WIN32
	// should probably just use fwd slash for all.
	const char* pathsep = "\\";
#else
	const char* pathsep = "/";
#endif
	if(bootstrapFoldersToLoad.empty())
		return;

	FilePath path( "", BOOTSTRAP_DIR );

	std::vector<std::string> allBootStraps;

	GetDirsInDirectory( path.GetFullPath(), allBootStraps );

	// put them in alphabetical
	std::sort(allBootStraps.begin(), allBootStraps.end());


	//delete the bootstrap folder reserved for the world switcher
	allBootStraps.erase(allBootStraps.begin());

	// if the size of the unloaded bootstraps is equal to the
	// number of bootstrap folders found then just load them all in alphabetical
	// order.
	std::vector<std::string>::const_iterator it;
	std::vector<std::string>::iterator itLoadAll;

	int size = bootstrapFoldersToLoad.size();

	SetUpProgressBar(bootstrapFoldersToLoad);

	// here we sort out exactly which folders we need to look at
	// first get the oldest folder that we need to begin our journey with
	// this will be the top of the list of the unloaded list.

	// Get rid of all folders above that.

	itLoadAll =  std::find(allBootStraps.begin(),
		allBootStraps.end(), *(bootstrapFoldersToLoad.begin()) );


	// erase up to but not including the first new folder
	allBootStraps.erase(allBootStraps.begin(),itLoadAll);




	// now go through the full list and do this:
	// if the folder appears on the Toload list then we need to load all
	// installion scripts as well as event scripts
	// if the folder does not appear on the ToLoad list then we *only* load the
	// event scripts for that folder
	for(it = allBootStraps.begin(); it != allBootStraps.end(); it++)
	{

		// the folder is in the to load list so load everything
		itLoadAll = std::find(bootstrapFoldersToLoad.begin(),
								bootstrapFoldersToLoad.end(), (*it));
		if(itLoadAll != bootstrapFoldersToLoad.end())
		{
			LoadCosFiles(path.GetFullPath() + (*it) + pathsep,true);
		}
		else // the folder is old so just update the event scripts
		{
			bool updateScriptoriumOnly = true;
			LoadCosFiles(path.GetFullPath() + (*it)+ pathsep,true,updateScriptoriumOnly);
		}
	}

	allBootStraps.clear();
	theApp.EndProgressBar();

}


void CosInstaller::LoadCosFiles(const std::string& bootstrap_dir,
								bool showProgress/* = false*/,
								bool updateScriptoriumOnly /*=false*/ )
{
	

	std::vector<std::string> files;
	GetFilesInDirectory(bootstrap_dir.data(), files);

	// sort into alphabetical order, so we can
	// easily force map.cos to be at the start
	// by calling it !map.cos
	std::sort(files.begin(), files.end());

	// read them all in

	int n = files.size();

	for (int i = 0; i < n; ++i)
	{
		theApp.UpdateProgressBar();

		// only read in files ending in .cos
		int len = files[i].size();
		if (len >= 4)
		{
			if (files[i].substr(len - 4, 4) == ".cos")
				ReadScriptFile(std::string(bootstrap_dir) + files[i]);
		}
	}
}


void CosInstaller::SetUpProgressBar(std::vector<std::string>& bootstrapFoldersToLoad)
{
	theApp.StartProgressBar(2);

	int n = 0;
	std::vector<std::string> files;

	std::vector<std::string>::const_iterator it;

	FilePath path( "", BOOTSTRAP_DIR );


	for(it = bootstrapFoldersToLoad.begin(); it != bootstrapFoldersToLoad.end(); it++)
	{
		GetFilesInDirectory(path.GetFullPath() + (*it).c_str() + "\\", files);

		// read them all in

		n += files.size();
		files.clear();
	}

	theApp.SpecifyProgressIntervals(n);
}

// read  just one cos file right away
CosInstaller::CosInstaller(std::string& script)
{
	if (script != "")
		ReadScriptFile(script);
}

CosInstaller::~CosInstaller()
{
//	OutputFormattedDebugString("Destructor COSInstaller...");
	myTextBuffer = "";
	myInstallScripts.clear();
	myExecuteScripts.clear();
//	OutputFormattedDebugString("Done\n");
}


bool CosInstaller::AddScript(Classifier classifier)
{
	Orderiser o;
	MacroScript* m;

	// get the script compiled
	m = o.OrderFromCAOS( myTextBuffer.data() );

	if (m)
	{
		m->SetClassifier( classifier );
		if( theApp.GetWorld().GetScriptorium().InstallScript( m ) )
		{
			// ok
		}
#ifdef _DEBUG
		else
		{
	#ifdef C2E_OLD_CPP_LIB
		char hackbuf1[1024];
		std::ostrstream out(hackbuf1,sizeof(hackbuf1) );
	#else
			std::ostringstream out;
	#endif
			if (!myCurrentFileForErrorMessages.empty())
				out << myCurrentFileForErrorMessages << std::endl;
			out << theCatalogue.Get("script_error", 1);
			classifier.StreamClassifier(out);
			classifier.StreamAgentNameIfAvailable(out);
			out << theCatalogue.Get("script_error", 2);
			out << '\0';
			ErrorMessageHandler::Show("script_error", 4, "CosInstaller::AddScript", out.str());
		}
#endif
		// Don't delete this, as it is referenced from the scriptorium
		// delete m;
	}
	else
	{
#ifdef C2E_OLD_CPP_LIB
		char hackbuf2[1024];
		std::ostrstream out(hackbuf2,sizeof(hackbuf2) );
#else
		std::ostringstream out;
#endif
		if (!myCurrentFileForErrorMessages.empty())
			out << myCurrentFileForErrorMessages << std::endl;
		classifier.StreamClassifier(out);
		classifier.StreamAgentNameIfAvailable(out);
		out << std::endl;
		out << o.GetLastError() << std::endl;
		CAOSMachine::FormatErrorPos(out, o.GetLastErrorPos(), myTextBuffer.data());
		out << '\0';

		ErrorMessageHandler::Show("script_error", 4, "CosInstaller::AddScript", out.str());
		return false;
	}
	
	// now clear the text buffer 
	myTextBuffer.erase(myTextBuffer.begin(), myTextBuffer.end());

	return true;
}

// when loading in a series of new products we may only want to update the scriptorium
bool CosInstaller::ReadScriptFile(std::string const& filename, bool updateScriptoriumOnly /*=false*/ )
{
	std::ifstream in(filename.data());
	myCurrentFileForErrorMessages = filename;
	bool OK = ReadScriptStream(in,updateScriptoriumOnly);
	myCurrentFileForErrorMessages = "";
	return OK;
}

// when loading in a series of new products we may only want to update the scriptorium
bool CosInstaller::ReadScriptStream(std::istream& in, bool updateScriptoriumOnly)
{
	char buf[1024];
	buf[0] = 0;

	eatwhite(in);
	in.getline(buf, 1024);
	do 
	{
		// check what kind of statement we are getting and
		// store it in our array
		if(!strncmp(buf,"scrp",4))
		{
			// create the classifier
			std::istrstream get_class(buf);			
			char buf_class[1024];
			get_class >> buf_class; // strip scrp
			
			int32 family = 0;
			int32 genus = 0;
			int32 species = 0;
			int32 event = 0;

			get_class >> family;
			get_class >> genus;
			get_class >> species;
			get_class >> event;

			// get the text
			while(in.good() && strncmp(buf,"endm",4))
			{
				eatwhite(in);
				in.getline(buf, 1024);

				// don't add endm or comments			
				if(strncmp(buf,"endm", 4) && strncmp(buf, "*", 1))
				{
					myTextBuffer += buf;
					myTextBuffer += " ";
				}
		
			}

			if (!AddScript(Classifier(family,genus,species,event)))
				return false;
		}
		else if(!strncmp(buf,"rscr", 4))
		{
			// finish on a remove script
			break;
		}
		else if (!strncmp(buf, "iscr", 4))
		{
		}
		else if (!strncmp(buf, "*", 1))
		{
		}
		else
		{
			if(strncmp(buf,"endm", 4) && strncmp(buf, "*", 1))
			{
				myTextBuffer += buf;
				myTextBuffer += " ";
			}

			while(in.good() && strncmp(buf,"endm", 4) && strncmp(buf,"scrp", 4) && strncmp(buf,"rscr", 4))
			{
				eatwhite(in);
				in.getline(buf, 1024);
				// don't add endm, scrp or comments			
				if(strncmp(buf,"endm", 4) && strncmp(buf,"scrp", 4) && strncmp(buf, "*", 1) && strncmp(buf,"rscr", 4))
				{
					myTextBuffer += buf;
					myTextBuffer += " ";
				}
			}

			if(!updateScriptoriumOnly)
			{
			// then add to my Exection scripts
			myExecuteScripts.push_back(myTextBuffer);
			}
			myTextBuffer.erase(myTextBuffer.begin(), myTextBuffer.end());
		}

		if (strncmp(buf,"scrp",4) && strncmp(buf,"rscr", 4))
		{
			eatwhite(in);
			in.getline(buf, 1024);
		}
	}
	while(in.good());

	// finished now execute the immediate scripts
	if (!ExecuteScripts())
		return false;

	return true;

}

bool CosInstaller::ExecuteScripts()
{
	MacroScript* m;
	CAOSMachine vm;
	std::ostream* out=NULL;


	std::vector<std::string>::iterator it;

	for(it = myExecuteScripts.begin(); it!= myExecuteScripts.end();it++)
	{
		Orderiser o;

		m = o.OrderFromCAOS( (*it).data() );
		
		if( m )
		{
			try {
				vm.StartScriptExecuting
					(m, NULLHANDLE, NULLHANDLE, 
					INTEGERZERO, 
					INTEGERZERO);
				vm.SetOutputStream(out);
				vm.UpdateVM(-1);
			}
			catch( CAOSMachine::RunError& e )
			{
#ifdef C2E_OLD_CPP_LIB
		char hackbuf3[1024];
		std::ostrstream out(hackbuf3,sizeof(hackbuf3) );
#else
				std::ostringstream out;
#endif
				if (!myCurrentFileForErrorMessages.empty())
					out << myCurrentFileForErrorMessages << std::endl;
				out << e.what();
				vm.StreamIPLocationInSource(out);
				out << std::endl;
				out << '\0';


				ErrorMessageHandler::Show("script_error", 4, "CosInstaller::ExecuteScripts", out.str());

				// clean up after the error
				vm.StopScriptExecuting();
			}
			catch(BasicException& e)
			{
#ifdef C2E_OLD_CPP_LIB
				char hackbuf4[1024];
				std::ostrstream out(hackbuf4,sizeof(hackbuf4) );
#else
				std::ostringstream out;
#endif
				if (!myCurrentFileForErrorMessages.empty())
					out << myCurrentFileForErrorMessages << std::endl;
				out << e.what();
				vm.StreamIPLocationInSource(out);
				out << std::endl;
				out << '\0';

				ErrorMessageHandler::Show("script_error", 4, "CosInstaller::ExecuteScripts", out.str());
					// clean up after the error
				vm.StopScriptExecuting();

			}

			// finished with this script now.
			delete m;
		}
		else
		{
#ifdef C2E_OLD_CPP_LIB
			char hackbuf5[1024];
			std::ostrstream out(hackbuf5,sizeof(hackbuf5) );
#else
			std::ostringstream out;
#endif
			if (!myCurrentFileForErrorMessages.empty())
				out << myCurrentFileForErrorMessages << std::endl;
			out << o.GetLastError() << std::endl;
			CAOSMachine::FormatErrorPos(out, o.GetLastErrorPos(), (*it).data());
			out << '\0';

			ErrorMessageHandler::Show("script_error", 4, "CosInstaller::ExecuteScripts", out.str());
			return false;
		}
	}

	// now clear the scripts
	myExecuteScripts.erase(myExecuteScripts.begin(), myExecuteScripts.end());

	return true;
}

