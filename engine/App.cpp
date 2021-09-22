// app.cpp

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "App.h"


// not currently used.
//#include "CPUID.h"

#include "Caos/CAOSMachine.h"
#include "Caos/Orderiser.h"
#include "Display/DisplayEngine.h"
#include "Display/EntityImage.h"
#include "Display/ErrorMessageHandler.h"
#include "Display/MainCamera.h"
#include "General.h"		// for logging macros
#include "World.h"
#ifdef _WIN32
#include "../common/RegistryHandler.h"
#endif
#include "C2eServices.h"
#include "Sound/Soundlib.h"
#include "Display/SharedGallery.h"

#include "Sound/MusicTimer.h"
#include "Sound/MusicManager.h"
#include "Sound/MusicGlobals.h"

#include "AgentManager.h"

#include "Creature/SensoryFaculty.h"

#include "../common/PRAYFiles/PrayManager.h"
#include "Creature/Brain/BrainScriptFunctions.h"
#include "Caos/AutoDocumentationTable.h"
#include "TimeFuncs.h"

#ifndef _WIN32
// VK_ scancodes
#include "unix/KeyScan.h"
// for stat()
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <fstream>
//#include <sstream>
////////////////////// GLOBALS ////////////////////////

App theApp;

const int App::ourTickLengthsAgo = 10;



SoundManager*  theSoundManager	= NULL;		// pointer to global Sound Manager
MusicManager*  theMusicManager = NULL;	// pointer to global Music Manager

/////////////////////////////////////////////////////////////////////////////
// App


App::App()
{
	myWorld = NULL;
#ifdef _WIN32
	myMainHWND = NULL;
	myCursor = NULL;
#endif
	mySystemTick = 0;
	myPlaneForConnectiveAgentLines = 9998;
	myShouldHighlightAgentsKnownToCreatureFlag = false;
	myShouldSkeletonsAnimateDoubleSpeedFlag = false;
	myCreaturePermissionToHighlight = 0;
	myPlayAllSoundsAtMaximumLevel = false;

	mySaveNextTick = false;
	myLoadThisWorldNextTick ="";
	myQuitNextTick = false;
	myPrayManager = NULL;
	myProgressBar = NULL;

	myAutoKillAgentsOnError = false;

#ifdef _WIN32
	myMutex = 0;
#endif // _WIN32
	memset( &myBedTime,0,sizeof( SYSTEMTIME ) );
	memset( &myRecordOfYourBirthday,0,sizeof( SYSTEMTIME ) );
	memset( &myMaxPlayingTime,0,sizeof( SYSTEMTIME ) );
	memset( &myEndGameTime,0,sizeof( SYSTEMTIME));

	IUseAdditionalRegistrySettings = false;

	myRecentTickLengths.resize(ourTickLengthsAgo);
	std::fill(myRecentTickLengths.begin(), myRecentTickLengths.end(), (uint32)(GetWorldTickInterval() * 0.9));
	myLastTickGap = -1;
	myRecentTickPos = 0;
	myPasswordForNextWorldLoaded = "";
	myIHaveAPassword = false;

	myRenderDisplay = true;
	myRenderDisplayNextTick = false;
	myFastestTicks = false;
	myToggleFullScreenNextTick = false;
	myResizedFlag = false;
	myMovedFlag = false;
	myDisplaySettingsErrorNextTick = false;
	myDelayedResizeFlag = false;
	myDelayedMovingFlag = false;

	myScrollingSpeedRangeUp.push_back(1);
	myScrollingSpeedRangeUp.push_back(2);
	myScrollingSpeedRangeUp.push_back(4);
	myScrollingSpeedRangeUp.push_back(8);
	myScrollingSpeedRangeUp.push_back(16);
	myScrollingSpeedRangeUp.push_back(32);
	myScrollingSpeedRangeUp.push_back(64);
	myScrollingSpeedRangeDown.push_back(0);
	myScrollingSpeedRangeDown.push_back(1);
	myScrollingSpeedRangeDown.push_back(2);
	myScrollingSpeedRangeDown.push_back(4);
	myScrollingSpeedRangeDown.push_back(8);
	myScrollingSpeedRangeDown.push_back(16);
	myScrollingSpeedRangeDown.push_back(32);
	myScrollingMask = 1 + 2 + 4 + 8;
	myMaximumDistanceBeforePortLineWarns = 600.0f;
	myMaximumDistanceBeforePortLineSnaps = 800.0f;
}

App::~App(){;}

/////////////////////////////////////////////////////////////////////////////
// App initialization

#ifdef _WIN32
bool App::Init( HWND wnd )
#else
bool App::Init()
#endif
{

#ifdef _WIN32
	myMainHWND = wnd;
#endif

	SharedGallery::theSharedGallery().CleanCreatureGalleryFolder();

#ifdef _WIN32
    // Prevent multiple copies of game from running.
	while (!CheckForMutex())
	{
		std::string message = theCatalogue.Get("app_error", 0);
		if (::MessageBox(theMainWindow, message.c_str(), GetGameName().c_str(), MB_SYSTEMMODAL | MB_RETRYCANCEL) == IDCANCEL)
	        return false;
	}

/*
	// Code that checks the processor - here for reference
	{
		DWORD dwCPU = GetProcessorType();
		std::ostringstream processor;
		processor << std::hex <<
			std::string("CPU: Type = 0x") << (unsigned int)(dwCPU & CPU_TYPE) << std::string(" Family = 0x") << (unsigned int)(dwCPU & CPU_FAMILY) <<
			std::string(" Model = 0x") << (unsigned int)(dwCPU & CPU_MODEL) << std::string(" Stepping = 0x") << (unsigned int)(dwCPU & CPU_STEPPING) <<
			std::string("\n") << '\0';

		std::ostringstream frequency;
		frequency << "Time stamp frequency: " << (int)GetHighPerformanceTimeStampFrequency() << std::endl;
	}
*/

	// Copy protection (primitive)
	while (!CheckForCD())
	{
		std::string message = theCatalogue.Get("app_cd_needed", 0);
		if (::MessageBox(theMainWindow, message.c_str(), GetGameName().c_str(), MB_SYSTEMMODAL | MB_RETRYCANCEL) == IDCANCEL)
	        return false;
	}

	// Check disk space
	if (!CheckAllFreeDiskSpace())
	{
		std::string message = theCatalogue.Get("app_insufficient_disk_space", 0);
		if (::MessageBox(theMainWindow, message.c_str(), GetGameName().c_str(), MB_YESNO) == IDNO)
			return false;
	}
#else

#warning No mutex, diskspace or CD protection checks.

#endif

	// Load syntax
	theCAOSDescription.LoadDefaultTables();

	// Serialise out caos.syntax for CAOS tool to load in
	std::string syntaxfile;
	syntaxfile = std::string(GetDirectory( MAIN_DIR )) + "caos.syntax";
	if (!theCAOSDescription.SaveSyntax(syntaxfile))
	{
		ErrorMessageHandler::Show("app_error", 1, "App::Init", syntaxfile.c_str());
	}
	CAOSMachine::InitialiseHandlerTables();

	GetLocalTime(&myGameStartTime);

	std::string langid = "en";	// default to english (ugh)
#ifdef _WIN32
	theRegistry.GetValue(theRegistry.DefaultKey(),
						"Language",
						langid,
						HKEY_CURRENT_USER);
#else
	UserSettings().Get( "Language", langid );
#endif


	// set flight recorder mask
	int32 mask = 1; // default to runtime errors
#ifdef _WIN32
	theRegistry.GetValue(theRegistry.DefaultKey(),
						"FlightRecorderMask",
						mask,
						HKEY_CURRENT_USER);
#else
	UserSettings().Get( "FlightRecorderMask", (int&)mask );
#endif
	theFlightRecorder.SetCategories( mask );

	myPrayManager = new PrayManager(langid);
	myPrayManager->AddDir( GetDirectory( PRAYFILE_DIR ) );
	myPrayManager->AddDir( GetDirectory( CREATURES_DIR ) );

    // Seed random number generators
	srand(GetTimeStamp() + GetRealWorldTime() + 1);
	RandQD1::seed(GetTimeStamp() + GetRealWorldTime());

	// set up the main view before we do anything in the world
	SetUpMainView();
	theMainView.Enable();
	CreateProgressBar();

	DoLoadWorld("Startup");

	InitialiseFromGameVariables();

	// this now has to be done after we know whether we use midi or not
	SetUpSound();

	// then..
	InitLocalisation();

	return true;
}


void App::UpdateApp()
{
	uint32 newStartStamp = GetTimeStamp();
	myLastTickGap = newStartStamp - myStartStamp;
	myStartStamp = newStartStamp;

	theMainView.MakeTheEntityHandlerResetBoundsProperly();

	// Perform world functions which are triggered by CAOS commands,
	// but have to be run in the main game loop rather than within 
	// a virtual machine.

	if (myDisplaySettingsErrorNextTick)
	{
#ifdef _WIN32
		std::string s = theCatalogue.Get(theDisplayErrorTag, (int)DisplayEngine::sidDodgyPixelFormat2);
		theMainView.PrepareForMessageBox();
		::MessageBox(NULL, s.c_str(), "Creatures 3", MB_OK|MB_ICONSTOP);
		theMainView.EndMessageBox();
#else
		// TODO: implement non-win32 version
#endif
		myDisplaySettingsErrorNextTick = false;
	}

	if (myMovedFlag)
	{
		internalWindowHasMoved();
		myMovedFlag = false;
	}
	if (myResizedFlag)
	{
		internalWindowHasResized();
		myResizedFlag = false;
	}
	if (myDelayedMovingFlag)
	{
		myDelayedMovingFlag = false;
		myMovedFlag = true;
		internalWindowHasMoved();
	}
	if (myDelayedResizeFlag)
	{
		myDelayedResizeFlag = false;
		myResizedFlag = true;
		internalWindowHasResized();
	}

	if (mySaveNextTick)
	{
		myWorld->Save();
		mySaveNextTick = false;
	}
	if (myQuitNextTick)
	{
		theFlightRecorder.Log(16, "Signalling termination...\n");
#ifdef SignalTerminateApplication
		SignalTerminateApplication();
#else
#warning "TODO: Implement SignalTerminateApplication"
#endif
		myQuitNextTick = false;
	}

	if (!myLoadThisWorldNextTick.empty())
	{
		ASSERT(myWorld);
		DoLoadWorld(myLoadThisWorldNextTick);
		myLoadThisWorldNextTick = "";
	}

	// In Creatures Adventures we have the option to
	// let the parent control timings and stuff like that
	// this is invoked by gamevars
	HandleAdditionalRegistrySettings();

	// Look for global input
	HandleInput();

	// Increase system tick
	mySystemTick++;

	myWorld->TaskSwitcher();

	if (myRenderDisplay || myRenderDisplayNextTick)
	{
		theMainView.Render();
		myRenderDisplayNextTick = false;
	}

	// Find time tick took
	uint32 endStamp = GetTimeStamp();
	uint32 tickLength = endStamp - myStartStamp;
	++myRecentTickPos;
	if (myRecentTickPos >= ourTickLengthsAgo)
		myRecentTickPos = 0;
	myRecentTickLengths[myRecentTickPos] = tickLength;
}




bool App::GetDirectories()
{
	const char* directories[] = {
			"Main Directory",
			"Sounds Directory",
			"Images Directory",
			"Genetics Directory",
			"Body Data Directory",
			"Overlay Data Directory",
			"Backgrounds Directory",
            "Catalogue Directory",
			"Bootstrap Directory",
			"Worlds Directory",
			"Exported Creatures Directory",
			"Resource Files Directory",
			"Journal Directory",
			"Creature Database Directory"
		  };

	int dir;
	// set pointers to directories to empty
	for ( dir = 0; dir < NUM_DIRS; dir++ )
	{
		memset((void*)m_dirs[dir], (int)NULL, (size_t)MAX_PATH );
	}

	// get all registry data regarding the directories
	// do this before command line is parsed

#ifdef _WIN32
	// windows version

	for( dir = 0; dir < NUM_DIRS; dir++ )
	{
		std::string buffer;
		std::string value((char*)(directories[dir]));
		theRegistry.GetValue(theRegistry.DefaultKey(),
						value,
						buffer,	
						HKEY_LOCAL_MACHINE);
		if(buffer.empty())
		{
			ErrorMessageHandler::NonLocalisable("NLE0001: There is no registry entry for the following:\nHKEY_LOCAL_MACHINE\\%s\\%s", std::string("App::GetDirectories"), theRegistry.DefaultKey().c_str(), directories[dir]);
			return false;
		}
		else
		{
			// if there's no trailing backslash add one:
			if (buffer[buffer.size()-1]!='\\')
				buffer.append("\\");
			uint32 attributes = GetFileAttributes(buffer.data());
		
			if (attributes == -1)
			{
				ErrorMessageHandler::NonLocalisable("NLE0002: The following directory doesn't exist:\n%s\nMake sure the registry entry is correct!", std::string("App::GetDirectories"), buffer.c_str());
				return false;
			}

			strcpy( m_dirs[dir], (const char*)buffer.data() );
		}
	}

#else
	// posix version


	for( dir = 0; dir < NUM_DIRS; dir++ )
	{
		std::string buffer;
		std::string key((char*)(directories[dir]));
		MachineSettings().Get( key, buffer );

		if(buffer.empty())
		{
			ErrorMessageHandler::NonLocalisable(
				"NLE0001: '%s' setting not found.",
				std::string("App::GetDirectories"),
				key.c_str() );
			return false;
		}
		// if there's no trailing slash add one:
		if (buffer[buffer.size()-1]!='/')
			buffer.append("/");

		// valid dir path?
		struct stat inf;
		bool groovy=true;
		if( stat( buffer.c_str(), &inf ) != 0 )
			groovy = false;
		else
		{
			if( !S_ISDIR( inf.st_mode ) )
				groovy=false;	// not a directory.
		}

		if( !groovy)
		{
			ErrorMessageHandler::NonLocalisable(
				"NLE0002: '%s' doesn't appear to be a valid directory\n"
				"(Make sure the setting is correct!)",
				std::string("App::GetDirectories"),
				buffer.c_str());
			return false;
		}

		strcpy( m_dirs[dir], (const char*)buffer.data() );

	// end of posix-specific version
#endif
			
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////



void App::ShutDown()
{

	theFlightRecorder.Log(16,"App Shutting down...");
	// tell the camera to stop running
	// (before world object disappears!)
	theMainView.ShutDown();


	if ( myPrayManager )
	{
		delete myPrayManager;
		myPrayManager = NULL;
	}

	if( theMusicManager )
	{
		delete theMusicManager;
		theMusicManager = NULL;
	}

	if( theMusicSoundManager )
	{
		delete theMusicSoundManager;
		theMusicSoundManager=NULL;
	}


	if( theSoundManager )
	{
		delete theSoundManager;
		theSoundManager=NULL;
	}

	if(myProgressBar)
	{
		delete myProgressBar;
	}

	if( myWorld )
	{
		delete myWorld;
		myWorld = NULL;
	}


	SharedGallery::theSharedGallery().DestroyGalleries();


#ifdef _WIN32
    // Provides mutual exclusion with itself / any emergency kits etc.
	if(myMutex)
	{
		ReleaseMutex(myMutex);
		CloseHandle(myMutex);
	}
#else
	// TODO: non-win32 version
#endif

	theFlightRecorder.Log(16, "TheApp has shutdown...\n");
}

// ----------------------------------------------------------------------
// Method:      SetUpMainView  
// Arguments:   None
//			    
// Returns:     None
// Description: initializes the mainview with the background.
// ----------------------------------------------------------------------
void App::SetUpMainView()
{
	// no need to send the full path to the background
	// anymore

	// get the camera up and running
	bool fullscreen = true;
#ifdef _WIN32
	theRegistry.GetValue(theRegistry.DefaultKey(), std::string("FullScreen"), fullscreen, HKEY_CURRENT_USER);
#else
	int inthack = 1;
	UserSettings().Get( "FullScreen", inthack );
	fullscreen = (inthack != 0);
#endif

	// This needs to be moved to whichever object
	// wants control of the cameras and resetting 
	// meta rooms
	// but for now...
	std::string value("Default Background");
	std::string defaultBackground("GreatOutdoors"); 
#ifdef _WIN32
	theRegistry.GetValue(theRegistry.DefaultKey(),
						value,
						defaultBackground,	
						HKEY_CURRENT_USER);
#else
	UserSettings().Get( value, defaultBackground );
#endif


	if (!theMainView.StartUp(0, // world coordinates of view
						0,
						fullscreen, // fullscreen or windowed?
#ifdef _WIN32
						GetHandle(),		// window handle
#endif
						defaultBackground, //default background file
						0,	// world coordinate
						0))	// of top left corner of background
	{	
		theMainView.Disable();
	}
	else
	{
		theMainView.MakeMapImage();
		DisableMapImage();
	}

}


// ----------------------------------------------------------------------
// Method:      EnableMainView 
// Arguments:   None
//			
// Returns:     None
//
// Description: Restarts the display engine
//			
// ----------------------------------------------------------------------
void App::EnableMainView()
{
	theMainView.Enable();
}

// ----------------------------------------------------------------------
// Method:      DisableMainView 
// Arguments:   None
//			
// Returns:     None
//
// Description:Stops the display engine from trying to display
//			
// ----------------------------------------------------------------------
void App::DisableMainView()
{
	theMainView.Disable();
}
void App::EnableMapImage()
{
	theMainView.EnableMapImage();
}

void App::DisableMapImage()
{
	theMainView.DisableMapImage();
}

void App::WindowHasMoved()
{
	myDelayedMovingFlag = true;
}

void App::WindowHasResized()
{
	myDelayedResizeFlag = true;
}

void App::internalWindowHasResized()
{
	theMainView.ResizeWindow();
	theAgentManager.ExecuteScriptOnAllAgents(SCRIPTWINDOWRESIZED, NULLHANDLE, INTEGERZERO, INTEGERZERO);
}

void App::internalWindowHasMoved()
{
	theMainView.MoveWindow();
}

void App::SetUpSound( void ) 
{
	// Now want two sound managers, allocated 1 meg each,
	// one for game sound, one for game music:
	try {
		theSoundManager=new SoundManager();
	}
	catch(BasicException& e) {
		theSoundManager = NULL;
		theMusicSoundManager = NULL;

		ErrorMessageHandler::Show(e, std::string("App::SetUpSound"));
		return;
	}
	

	if (theSoundManager) {
		theSoundManager->InitializeCache(2*1024);
	}

	// don't go to the expense of creating the music manager
	// if it is not needed...

	if( IOnlyPlayMidiMusic == false) {
		try {
			theMusicSoundManager = new SoundManager();
		}
		catch(SoundManager::SoundException& e) {
			theMusicSoundManager = NULL;

			ErrorMessageHandler::Show(e, std::string("App::SetUpSound"));
			return;
		}

		if(theMusicSoundManager) {
			theMusicSoundManager -> InitializeCache(1024);
		} else {
			// no music message?
			return;
		}


		// Create the global music manager
		theMusicManager = new MusicManager;

		if(!theMusicManager) {
			// message...
			return;
		}

		// Now try to load the encrypted script into the music manager
		if (! theMusicManager -> LoadScrambled ()) {
			// Unsuccessful
			// **** Should report the error in some way here ?

			// Dispose of the music manager
			delete theMusicManager;
			theMusicManager = NULL;
		}


		// !!! no thread is used for now if C3 decided that one is needed later
		// then then reinstate UpdateThread
		/* Audio thread handle elsewhere
		
		// Begin a thread to update the sound / music manager
		// Create a running thread

		// PC 18/6/98 - changed priority to normal

		audioThread = AfxBeginThread( AudioThreadUpdate,
									  NULL,
									  THREAD_PRIORITY_NORMAL,
									  0,
									  0,
									  NULL); */

		if (theMusicManager) {
			theMusicManager -> Play();
			//pMusicManager -> BeginTrack("Underground");
			//pMusicManager -> UpdateSettings (0.5, 0.5);
		}
	}
}

void App::HandleIncomingRequest( ServerSide& server )
{
	myRequestManager.HandleIncoming( server );
}



// ----------------------------------------------------------------------
// Method:      ChangeResolution 
// Arguments:   None
//			
// Returns:     None
//
// Description: Finds out whether the requested display mode is viable and 
//				switches to it.  For smoother results change the size of
//				window too. Note that the display modes are enumrated at
//				start up of the display engine
//			
// ----------------------------------------------------------------------
void App::ChangeResolution()
{
	// just a test cycle through resolutions
	// each application would have a way
	// of allowing the user to select a resolution
	static int32 res[3][2] = {640,480,
							800,600,
							1024, 768};
	static int x = 0;

	theMainView.ChangeDisplayMode(res[x][0],res[x][1]);
	internalWindowHasResized();
	x++;

	if(x > 2)
		x = 0;
}

// ----------------------------------------------------------------------
// Method:      ToggleFullScreenMode 
// Arguments:   None
//			
// Returns:     true if changed OK
//				false otherwise
//
// Description: On each request it flips the engine between fullscreen
//				and windowed mode.
//			
// ----------------------------------------------------------------------
void App::ToggleFullScreenMode()
{
	theMainView.ToggleFullScreenMode();
	theAgentManager.ExecuteScriptOnAllAgents(SCRIPTWINDOWRESIZED, NULLHANDLE, 
		INTEGERZERO, INTEGERZERO);
}

bool App::InitLocalisation()
{
	theCatalogue.Clear();

	std::string langid = "en";	// default to english (ugh)
#ifdef _WIN32
	theRegistry.GetValue(theRegistry.DefaultKey(),
						"Language",
						langid,
						HKEY_CURRENT_USER);
#else
	UserSettings().Get( "Language", langid );
#endif

	try
	{
		std::string catpath;

		catpath = GetDirectory( CATALOGUE_DIR );
		theCatalogue.AddDir( catpath, langid );
	
	}
	catch( Catalogue::Err& e )
	{
		ErrorMessageHandler::Show(e, "App::InitLocalisation");
		return false;
	}
	
	if (!SensoryFaculty::SetupStaticVariablesFromCatalogue())
		return false;
	InitBrainMappingsFromCatalogues();
	// Add the Catalogue Entries for the pray manager....
	std::string prayCatalogueName = "Pray System File Extensions";
	if (theCatalogue.TagPresent(prayCatalogueName) && (myPrayManager != NULL))
	{
		GetResourceManager().ClearChunkFileExtensionList();
		GetResourceManager().GarbageCollect(true);
		for(int i=0; i < theCatalogue.GetArrayCountForTag(prayCatalogueName); i++)
		{
			
			std::string anExtension = theCatalogue.Get(prayCatalogueName,i);
			GetResourceManager().AddChunkFileExtension(anExtension);
		}
		GetResourceManager().RescanFolders();
	}

	theApp.InitLocalCatalogueFilesFromTheWorldsDirectory();

	return true;
}

bool App::InitLocalCatalogueFilesFromTheWorldsDirectory()
{
	std::string langid = "en";	// default to english (ugh)
#ifdef _WIN32
	theRegistry.GetValue(theRegistry.DefaultKey(),
						"Language",
						langid,
						HKEY_CURRENT_USER);
#else
	UserSettings().Get( "Language", langid );
#endif

	try
	{
		std::string catpath;

		// also get the local catalogue files
		if(GetWorldDirectoryVersion(CATALOGUE_DIR,catpath))
		{
			theCatalogue.AddDir(catpath,langid);
		}
	}
	catch( Catalogue::Err& e )
	{
		ErrorMessageHandler::Show(e, "App::InitLocalisation");
		return false;
	}

	return true;
}

// get the world directory path of the given folder
// e.g. ..\\Worlds\\Myworld\\Images
bool App::GetWorldDirectoryVersion(int dir, std::string& path,bool createFilePlease /*=false*/)
{
	if(!myWorld)
		return false;

	char buf[MAX_PATH];
	GetDirectory(dir,buf);
	path = buf;
	// get the words between the last two backslashes
	size_t end  = path.find_last_of("\\");
	size_t start = 	path.find_last_of("\\",end-1);
	if(end == -1 || start == -1)
	{
		// no main directory version!
		return false;
	}

	// hurrah now we have the name of the directory!
	std::string name = path.substr(start+1, end-start-1);

	return GetWorldDirectoryVersion(name, path, createFilePlease);
}

// for attic files, we separate out this part of the function
bool App::GetWorldDirectoryVersion(const std::string& name, std::string& path,bool createFilePlease /*=false*/)
{
	if(!myWorld)
		return false;

	// now get the worlds directory
	char buf[MAX_PATH];
	theApp.GetDirectory(WORLDS_DIR,buf);
	path = buf;
	path+=  myWorld->GetWorldName() + "\\"+ name + "\\";

#ifdef _WIN32
	if(GetFileAttributes(path.data())== -1)
	{
		if(createFilePlease)
		{
			return CreateDirectory( path.data(),NULL ) ? true : false;
		}
		else // just report that there was no directory at all
		{
			return false;
		}
	}
#else
	// TODO: non-win32 version
	ASSERT(false);
#endif


	return true;

}


bool App::CreateNewWorld(std::string& worldName)
{
	bool ret;
#ifdef _WIN32
	FilePath path( worldName, WORLDS_DIR );
	ret = CreateDirectory( path.GetFullPath().data(),NULL ) ? true : false;
#else
	// TODO: non-win32 version
#endif

	return ret;
}


void App::HandleAdditionalRegistrySettings()
{
	if(IUseAdditionalRegistrySettings)
	{
		bool quitgame = false;
	
		static bool nornsAreTiredAlready = false;
		
		SYSTEMTIME currentTime;
		GetLocalTime(&currentTime);

		if(!IsValidTime(currentTime))
			return;

		if(IsValidDate(myRecordOfYourBirthday))
		{
			if(myRecordOfYourBirthday.wMonth == currentTime.wMonth &&
				myRecordOfYourBirthday.wDay == currentTime.wDay)
			{
				myWorld->ActivateBirthdayBanner();
			}
		}

		// check to see if it is past bedtime
		if(IsValidTime(myEndGameTime))
		{
			// oops we went right past the hour somehow better quit
			// now
			if( currentTime.wHour > myEndGameTime.wHour )
			{
				quitgame = true;
				DoParentMenuTests(currentTime.wMinute,
								quitgame,
								currentTime,
								nornsAreTiredAlready);


			} // if we are on the hour or approaching it
			else if (myEndGameTime.wHour == currentTime.wHour )
			{
				if(currentTime.wMinute >= myBedTime.wMinute)
					quitgame = true;

			DoParentMenuTests(myEndGameTime.wMinute,
								quitgame,
								currentTime,
								nornsAreTiredAlready);

				
				
			}
			else if(myEndGameTime.wHour - currentTime.wHour == 1 )
			{
				DoParentMenuTests(60,
								quitgame,
								currentTime,
								nornsAreTiredAlready);
			}
		
		

		}// end valid times

		// check to see if we have been playing for too long already
	/*	if(IsValidTime(myGameStartTime) && IsValidGameTime(myMaxPlayingTime))
		{
			// check seconds
			if(myMaxPlayingTime.wHour == 0 && myMaxPlayingTime.wMinute ==0
				&& myMaxPlayingTime.wSecond != 0)
			{
				if(currentTime.wSecond - myGameStartTime.wSecond >= myMaxPlayingTime.wSecond)
				{
					quitgame = true;
				}
			}
			else if(myMaxPlayingTime.wHour == 0 && myMaxPlayingTime.wMinute !=0)
			{
				int testTime =currentTime.wMinute;
				// if we are on the hour
				if(testTime == 0)
				{
					testTime = 60;
				}

				if(testTime - myGameStartTime.wMinute > myMaxPlayingTime.wMinute)
				{
					myQuitNextTick = true;
					quitgame = true;
				}
				else
				{
						DoParentMenuTests(myGameStartTime.wMinute + myMaxPlayingTime.wMinute,
								quitgame,
								currentTime,
								nornsAreTiredAlready);

				}
			}
			else
			{
				int diff = currentTime.wHour - myGameStartTime.wHour;
				if(diff >=myMaxPlayingTime.wHour )
				{
				quitgame = true;
				}
			}

			
		}*/

		// overall if we need to quit
		// flood the norns with sleepyness
		if(quitgame && !nornsAreTiredAlready)
		{
			myWorld->MakeAllNornsTired();
			nornsAreTiredAlready = true;
			myWorld->ShowCountDownClock(true,8);
		}
	}
}

void App::DoParentMenuTests(int bedTimeMinutes,
							bool& quitgame,
							SYSTEMTIME& currentTime,
							bool& nornsAreTiredAlready)
{
	static int bedTimeReminders;
	static bool workHereIsDone = false;

	if(workHereIsDone)
		return;

	int diff = bedTimeMinutes - currentTime.wMinute; 
// if there is less than five minutes to go
	if(diff  <= 5)
		quitgame = true;

	// start showing shut down messages now
	// every five or so minutes
	


	if(diff <= 5)
	{
		if(bedTimeReminders == 0 || bedTimeReminders == 1)
			bedTimeReminders = 2;

		// if we haven't done this already then...
		if(bedTimeReminders == 2)
		{
			bedTimeReminders++;
			// let the clock stay up now
			myWorld->ShowCountDownClock(false,8);
		}

		
		//flood creatures with sleepiness and tiredness now
		if(!nornsAreTiredAlready)
		{
			myWorld->MakeAllNornsTired();
			nornsAreTiredAlready = true;
			quitgame = false;
		}

		// make the clock count down to zero
		switch(diff)
		{
		case 5: myWorld->UpdateCountDownClock(0);
			break;

		case 4: myWorld->UpdateCountDownClock(1);
			break;

		case 3: myWorld->UpdateCountDownClock(2);
			break;
		case 2:	myWorld->UpdateCountDownClock(3);
			break;
		case 1: myWorld->UpdateCountDownClock(4);
			break;
		case 0: 
			{
				workHereIsDone = true;
				myWorld->UpdateCountDownClock(5);
				myWorld->ShowCountDownClockQuit();
			//	 mySaveNextTick = true;
			//	 myQuitNextTick = true;
				break;
			}
		default: myWorld->UpdateCountDownClock(5);
				myWorld->ShowCountDownClockQuit();
				workHereIsDone = true;
			break;
		}

	
	}
	else if(diff <= 10)
	{
	
		// if we haveskip the 15 minute warning...
		if(bedTimeReminders == 0)
			bedTimeReminders = 1;

		// if we haven't done this already then...
		if(bedTimeReminders == 1)
		{
			bedTimeReminders++;
			// just flash up
			myWorld->ShowCountDownClock(true,7);
		}
	}
	else if(diff <= 15)
	{
		// if we haven't done this already then...
		if(bedTimeReminders == 0)
		{
			bedTimeReminders++;
			// just flash up
			myWorld->ShowCountDownClock(true,0);
		}
	}
	

}


#ifdef _WIN32
// shouldn't this be in window.cpp?
void App::BeginWaitCursor()
{
	// set the wait cursor

	 myCursor = SetCursor(LoadCursor( NULL,
		MAKEINTRESOURCE(  IDC_WAIT)));  

	 ShowCursor(true);

}

void App::EndWaitCursor()
{
	// set the cursor back to whatever it was
	SetCursor(myCursor);
}
#endif



void App::HandleInput()
{	
	// scan the pending input events
	for(int i=0; i<myInputManager.GetEventCount(); ++i )
	{
		const InputEvent* ev = &myInputManager.GetEvent(i);

		if( ev->EventCode == InputEvent::eventTranslatedChar)
		{
			int key = ev->KeyData.keycode;

			if( myInputManager.GetTranslatedCharTarget())
			{
				myInputManager.GetTranslatedCharTarget()->SendChar( key );
			}
		}

		if( ev->EventCode == InputEvent::eventKeyDown)
		{
			int key = ev->KeyData.keycode;

			// deal with debug key presses
			if (DebugKeyNow())
			{
				// VK_PAUSE is handled in MainWindowProc, so it works
				// even when the game is debug paused (to unpause it)
				// Similarly VK_SPACE
				if (key == VK_INSERT)
					theMainView.ToggleMapImage();
				else if (key == VK_PRIOR)
					GetWorld().GetPreviousMetaRoom();
				else if (key == VK_NEXT)
					GetWorld().GetNextMetaRoom();
				else if (key == VK_HOME)
					ChangeResolution();
				else if (key == VK_DELETE)
					myShouldHighlightAgentsKnownToCreatureFlag = 
						!myShouldHighlightAgentsKnownToCreatureFlag;
				else if (key == VK_END)
					myShouldSkeletonsAnimateDoubleSpeedFlag =
						!myShouldSkeletonsAnimateDoubleSpeedFlag;
			}
			else if (DebugKeyNowNoShift())
			{
				if (key==VK_NUMPAD0 || key==VK_NUMPAD1 || key==VK_NUMPAD2 || key==VK_NUMPAD3 || key==VK_NUMPAD4 || key==VK_NUMPAD5 || key==VK_NUMPAD6)
					SetWhichCreaturePermissionToHighlight(
						key==VK_NUMPAD0 ? 1 :
						key==VK_NUMPAD1 ? 2 :
						key==VK_NUMPAD2 ? 4 :
						key==VK_NUMPAD3 ? 8 :
						key==VK_NUMPAD4 ? 16:
						key==VK_NUMPAD5 ? 32:
						0
					);
			}
		}
	}
}

// called also when the game is reloaded
void App::InitialiseFromGameVariables()
{
	// initialse some variables from the world

	CAOSVar& planeForLines = theApp.GetWorld().GetGameVar("engine_plane_for_lines");
	if (planeForLines.GetType() == CAOSVar::typeInteger)
		myPlaneForConnectiveAgentLines = planeForLines.GetInteger()==0?9998:planeForLines.GetInteger();

	CAOSVar& soundLevels = theApp.GetWorld().GetGameVar("engine_playAllSoundsAtMaximumLevel");
	ASSERT(soundLevels.GetType() == CAOSVar::typeInteger);	

	myPlayAllSoundsAtMaximumLevel = soundLevels.GetInteger()==0 ? false: true;

	CAOSVar& skeletonUpdate = theApp.GetWorld().GetGameVar("engine_SkeletonUpdateDoubleSpeed");
	ASSERT(skeletonUpdate.GetType() == CAOSVar::typeInteger);
	// default is false
	myShouldSkeletonsAnimateDoubleSpeedFlag = skeletonUpdate.GetInteger() ==0 ? false: true;
	
	CAOSVar& var = theApp.GetWorld().GetGameVar( "cav_useparentmenu" );
	ASSERT(var.GetType() == CAOSVar::typeInteger);
	IUseAdditionalRegistrySettings = var.GetInteger() ==0 ? false: true;

	CAOSVar& midi = theApp.GetWorld().GetGameVar( "engine_usemidimusicsystem" );
	ASSERT(midi.GetType() == CAOSVar::typeInteger);
	IOnlyPlayMidiMusic = midi.GetInteger() ==0 ? false: true;

	memset( &myBedTime,0,sizeof( SYSTEMTIME ) );
	memset( &myRecordOfYourBirthday,0,sizeof( SYSTEMTIME ) );
	memset( &myMaxPlayingTime,0,sizeof( SYSTEMTIME ) );


	std::string string;

	// no big deal if these fail do not report it to the kid
	// user's birthday
	CAOSVar& birthdate = theApp.GetWorld().GetGameVar("cav_birthdate");
	if (birthdate.GetType() == CAOSVar::typeString)
		birthdate.GetString(string);

	ConstructSystemTime(myRecordOfYourBirthday,string);

		
	// user's quittime
	CAOSVar& quit = theApp.GetWorld().GetGameVar("cav_quittime");

	if (quit.GetType() == CAOSVar::typeString)
		quit.GetString(string);

	ConstructSystemTime(myBedTime,string);

	// user's gamelength
	CAOSVar& gamelength = theApp.GetWorld().GetGameVar("cav_gamelength");

	if (gamelength.GetType() == CAOSVar::typeString)
		gamelength.GetString(string);

	ConstructSystemTime(myMaxPlayingTime,string);

	CAOSVar& perDay =  theApp.GetWorld().GetGameVar("cav_gamelengthIsPerDay");
	if(perDay.GetType() == CAOSVar::typeInteger)
	{
		// if it is per day then we should have serialized the last gamestart time
		// compare it to todays date and if it is the same day
		// set the ingame start time to what was saved otherwise continue
		int per = perDay.GetInteger();
		if(per == 1)
		{
			bool gameOver = false;
			SYSTEMTIME timeEnd = myWorld->GetLastGameEndTime();
			SYSTEMTIME length = myWorld->GetLastPlayLength();

			if(IsValidGameTime(length) &&
				IsValidTime(myGameStartTime) && IsValidTime(timeEnd))
			{
				// only go so far as same day month year
				if(timeEnd.wDay == myGameStartTime.wDay &&
					timeEnd.wMonth == myGameStartTime.wMonth &&
					timeEnd.wYear == myGameStartTime.wYear)
				{
					// check to see if we have been playing for too long already
				
					// check seconds
					if(myMaxPlayingTime.wHour == 0 && myMaxPlayingTime.wMinute ==0
						&& myMaxPlayingTime.wSecond != 0)
					{
						if(length.wSecond - myGameStartTime.wSecond >= myMaxPlayingTime.wSecond)
						{
							gameOver = true;
						}
						else
						{
							myMaxPlayingTime.wSecond = myGameStartTime.wSecond  - length.wSecond ;
						}
					} // minutes
					else if(myMaxPlayingTime.wHour == 0 && myMaxPlayingTime.wMinute !=0)
					{
						int testTime =length.wMinute;
						// if we are on the hour
						if(testTime == 0)
						{
							testTime = 60;
						}

						if(testTime - myGameStartTime.wMinute >= myMaxPlayingTime.wMinute)
						{
							gameOver = true;
						}
						else
						{
							myMaxPlayingTime.wMinute = myGameStartTime.wMinute  - testTime ;
						}
					}
					else
					{
						int diff = length.wHour - myGameStartTime.wHour;
						if(diff >=myMaxPlayingTime.wHour )
						{
							gameOver = true;
						}
						else
						{
							myMaxPlayingTime.wHour = myGameStartTime.wHour  - length.wHour ;
						}
					}
				}// end if same day
			}// end if times are valid
			if(gameOver)
			{
				// set the max playing time to 2 minute so that 
				// when we start checking we bail out
				memset( &myMaxPlayingTime,0,sizeof( SYSTEMTIME ) );
				myMaxPlayingTime.wMinute = 2;
			}
		}// end if per day
	}

	// decide which of the two times are sooner
	GetTimeGameShouldEnd();


	CAOSVar& HH = theApp.GetWorld().GetGameVar("engine_creature_pickup_status");
	if (HH.GetType() == CAOSVar::typeInteger) // Has to be an integer
	{
		myCreaturePickupStatus = HH.GetInteger();
	}
				
	CAOSVar& CDBW = theApp.GetWorld().GetGameVar("engine_distance_before_port_line_warns");
	if (CDBW.GetType() == CAOSVar::typeFloat)
	{
		myMaximumDistanceBeforePortLineWarns = CDBW.GetFloat();
	}
	CAOSVar& CDBS = theApp.GetWorld().GetGameVar("engine_distance_before_port_line_snaps");
	if (CDBS.GetType() == CAOSVar::typeFloat)
	{
		myMaximumDistanceBeforePortLineSnaps = CDBS.GetFloat();
	}
}

float App::GetTickRateFactor()
{
	ASSERT(myRecentTickLengths.size() == ourTickLengthsAgo);

	uint32 totalTime = 0;
	for (int i = 0; i < ourTickLengthsAgo; ++i)
		totalTime += myRecentTickLengths[i];

	float averageTickLength = (float)totalTime / (float)ourTickLengthsAgo;
	float factor = averageTickLength / (float)GetWorldTickInterval();

	return factor;
}	

bool App::CreateProgressBar()
{
	if (myProgressBar)
		return true;

	if (theCatalogue.TagPresent("progress_indicators"))
	{
		try
		{
			// sprite file
			std::string spriteFilename = theCatalogue.Get("progress_indicators", 1);
			FilePath galleryName(spriteFilename,IMAGES_DIR);
			myProgressBar = new EntityImage(galleryName, 0, 1000, 0, 0, 0, 0);
			myProgressBar->Unlink();

			return true;
		}
		catch( BasicException& e)
		{
			ErrorMessageHandler::Show(e, "App::CreateProgressBar");
			return false;
		}
	
	}
	return false;	
}

void App::StartProgressBar(int catOffset)
{
#ifdef _WIN32
	// ugh. This stuff shouldn't be in app.

	if(myProgressBar && !myWorld->IsStartUpWorld() &&
		!DisplayEngine::theRenderer().ProgressBarAlreadyStarted())
	{
		theMainView.SetLoading(false);
		// background file, if available
		std::string progressBackgroundName = theCatalogue.Get("progress_indicators", 0);
		if (!progressBackgroundName.empty())
		{
			RECT progressBackground;
			progressBackground.left = 0;
			progressBackground.right = 800;
			progressBackground.top = 0;
			progressBackground.bottom = 600;
			theMainView.ChangeMetaRoom(progressBackgroundName, progressBackground, 0, 0, 0, 0);
		}
		theMainView.SetLoading(true);

		// popup dialog
		int32 popposx;
		int32 popposy;
		{
			// set pose
			myProgressBar->SetPose(atoi(theCatalogue.Get("progress_indicators", catOffset + 3)));
			Bitmap* bitmap = myProgressBar->GetCurrentBitmap();
			// centre on screen
			
			popposx = atoi(theCatalogue.Get("progress_indicators", catOffset + 1));
			popposy = atoi(theCatalogue.Get("progress_indicators", catOffset + 2));

			if (popposx == -1)
				popposx = (DisplayEngine::theRenderer().GetSurfaceWidth() - bitmap->GetWidth()) / 2;
			if (popposy == -1)
				popposy = (DisplayEngine::theRenderer().GetSurfaceHeight() - bitmap->GetHeight()) / 2;

			DisplayEngine::theRenderer().ClientToScreen(popposx,popposy);
			bitmap->SetPosition(Position(popposx,popposy));
			// render it
			bool ok = DisplayEngine::theRenderer().RenderBitmapToFrontBuffer(bitmap);
		}

		// progress bar pose
		myProgressBar->SetPose(atoi(theCatalogue.Get("progress_indicators", catOffset)));
		Bitmap* bitmap = myProgressBar->GetCurrentBitmap();
		// offset of progress indicator relative to popup dialog
		int32 barposx = popposx + atoi(theCatalogue.Get("progress_indicators", catOffset + 4));
		int32 barposy = popposy + atoi(theCatalogue.Get("progress_indicators", catOffset + 5));
		bitmap->SetPosition(Position(barposx,barposy));
		// tell display engine about it
		bool ok = DisplayEngine::theRenderer().CreateProgressBar(bitmap);
	}
#endif
// TODO: non-win32 progress bar.
}

void App::SpecifyProgressIntervals(int updateIntervals)
{
#ifdef _WIN32
	if (myProgressBar && !myWorld->IsStartUpWorld())
		DisplayEngine::theRenderer().StartProgressBar(updateIntervals);
#endif
}

void App::UpdateProgressBar()
{
#ifdef _WIN32
	if(myProgressBar && !myWorld->IsStartUpWorld())
	{
		DisplayEngine::theRenderer().UpdateProgressBar();
	}
#endif
}

void App::EndProgressBar()
{
#ifdef _WIN32
	theMainView.SetLoading(false);
	DisplayEngine::theRenderer().EndProgressBar();
#endif
}

bool App::ConstructSystemTime(SYSTEMTIME& time, std::string& string)
{
	std::string word;
	// month s
	std::string date("\\"); 
	std::string colon(":");
	

	size_t start, end, next, p0 ; 
	bool done = false ;    
	start = end = next = 0 ;
	int value =0;


	// Find out whether this is time or date format
	//  dates are separated by backslashes	
	start = string.find_first_of(date, next);
	if(start < string.length())
	{
			// day month year
		for(int i = 0; i < 3; i++)
		{
		
			// Find start of date digits.        
			start = string.find_first_not_of(date, next) ;
			// Find end of date digits.        
			p0 = string.find_first_of(date, start) ;
			// Check for end of string.
			end = (p0 >= string.length()) ? string.length() : p0;
		
			// Copy all the date digits.
			word = string.substr(next, end -start) ;

			
			value = atoi(word.c_str()); 
			
			next = end + 1;       
			// if we have reached the end of the string without
			// getting all three date positions?
			if( next >= string.length() && i < 2)            
				return false;
			else
			{
				switch(i)
				{
				case 0:
					{
						time.wDay = value;	
						break;
					}
				case 1:
					{
						time.wMonth = value;
						break;
					}
				case 2:
					{
						time.wYear = value;
						break;
					}
				}
				
			}
		}// end for i < 3
	}
	else
	{
		// times are separated by colons
		start = string.find_first_of(colon, next);
		if(start < string.length())
		{

			// hours minutes seconds
			for(int i = 0; i < 3; i++)
			{
			
				// Find start of time digits.        
				start = string.find_first_not_of(colon, next) ;
				// Find end of time digits.        
				p0 = string.find_first_of(colon, start) ;
				// Checking for end of string.
				end = (p0 >= string.length()) ? string.length() : p0;
				// Copy the digit.
				word = string.substr(next, end -start) ;

				value = atoi(word.c_str()); 
				
				// if we have reached the end of the string without
				// getting all three time positions?
				next = end + 1;        
				if( next >= string.length() && i < 2)            
					return false;
				else
				{
					switch(i)
					{
					case 0:
						{
							time.wHour = value;	
							break;
						}
					case 1:
						{
							time.wMinute = value;
							break;
						}
					case 2:
						{
							time.wSecond = value;
							break;
						}
					}
					
				}
			}// end for i < 3
		}
	}
	return true;
}


	
void App::SetPassword(std::string& password)
{
	myPasswordForNextWorldLoaded = password;
	myIHaveAPassword = true;
}
	
std::string App::GetPassword()
{
	return myPasswordForNextWorldLoaded;
}

bool App::DoINeedToGetPassword()
{
	bool temp = myIHaveAPassword;
	myIHaveAPassword = false;
	return temp;
}

void App::RefreshGameVariables()
{
	InitialiseFromGameVariables();
	myWorld->InitialiseFromGameVariables();
}

void App::GetTimeGameShouldEnd()
{
	// default time is bedtime ther are check later to see if this is valid
	GameEndHelper(myEndGameTime,myBedTime);

	if(IsValidTime(myGameStartTime) )
	{

	
			if(IsValidGameTime(myMaxPlayingTime))
			{
				// find out which time (h:m:) is earlier

				// calculate when the endgame would be on based on length of play

				div_t hour = div(myGameStartTime.wHour + myMaxPlayingTime.wHour,24);

				myEndGameTime.wHour = hour.rem;

				// remember that we might be over the 60 minutes in which we
				// should increase the hour
				div_t minutes = div(myGameStartTime.wMinute + myMaxPlayingTime.wMinute,60);

				myEndGameTime.wMinute = minutes.rem;

				myEndGameTime.wHour += minutes.quot;

				if(myEndGameTime.wHour < myBedTime.wHour)
				{
					return;
				}
	
				if(IsValidTime(myBedTime))			
				{
					// copy in the bedtimne then
					if(myBedTime.wHour < myEndGameTime.wHour)
					{
						GameEndHelper(myEndGameTime,myBedTime);
						return;
					}


					if(myBedTime.wHour == myEndGameTime.wHour)
					{
						if(myEndGameTime.wMinute < myBedTime.wMinute)
						{

							return;
						}

						if(myBedTime.wMinute  < myEndGameTime.wMinute)
						{
							GameEndHelper(myEndGameTime,myBedTime);
							return;
						}

						// if they are equal in minutes then all is well
					}
				}
			}
		
	}
}

void App::GameEndHelper(SYSTEMTIME& dest, SYSTEMTIME& source)
{

	dest.wDay = source.wDay;
	dest.wDayOfWeek = source.wDayOfWeek;
	dest.wHour = source.wHour;
	dest.wMilliseconds = source.wMilliseconds;
	dest.wMinute = source.wMinute;
	dest.wMonth = source.wMonth;
	dest.wSecond = source.wSecond;
	dest.wYear = source.wYear;

}

void App::DoLoadWorld(std::string worldName)
{
	// try loading main world
	if (myWorld)
		delete myWorld;
	myWorld = new World;
	myWorld->Init();
	if (!myWorld->Load(worldName, false))
	{
		// failed to load main world, try backup
		if (myWorld)
			delete myWorld;
		myWorld = new World;
		myWorld->Init();
		myWorld->Load(worldName, true);
	}
}

bool App::ProcessCommandLine(std::string commandLine)
{
	if (commandLine.substr(0, 11) == "--autokill ")
	{
		myAutoKillAgentsOnError = true;
		commandLine = commandLine.substr(11);
	}

	// Get the game name if any from the command line
	if(!commandLine.empty())
		SetGameName(commandLine);
	else
	{
		ErrorMessageHandler::NonLocalisable("NLE0007: You must specify a game name in the command line.\nFor example, Creatures 3 or Creatures Adventures\n\nFor soak tests, specify --autokill before the game name\nto kill agents which generate run time errors.",
			std::string("App::ProcessCommandLine"));
		return false;
	}

	return true;
}




// Now for the engine game variable documentation tables.....


TableSpec ourGameVariables[] =
{
	TableSpec("Game Variables"),
	TableSpec("Variable (for @#GAME@)", "Expected Type", "Default", "Description"),

	TableSpec("@engine_debug_keys@","Integer","1", "This determines if the debug keys are operative. If the value is <b>1</b> then the keys described in the @#Debug Keys@ table are functional.  For non-numeric keypad keys, you must hold Shift down with the key."),
	TableSpec("@engine_full_screen_toggle@","Integer","1", "Alt+Shift+Enter usually toggles full screen mode.  Set to 0 to disable this."),
	TableSpec("@engine_SkeletonUpdateDoubleSpeed@", "Integer", "0", "If non-zero, skeleton updates happen each tick. If Zero, they happen every other tick (default)"),
	TableSpec("@engine_creature_pickup_status@", "Integer", "0", "If zero, right click on a creature is pickup.  If one, right click is holding hands.  If two, hold shift to pickup, don't hold shift to hold hands.  If three, as two but creature must be a selectable one as according to the Grettin game variable."),
	TableSpec("@engine_dumb_creatures@", "Integer", "0", "If non-zero, creatures do not burble."),
	TableSpec("@engine_pointerCanCarryObjectsBetweenMetaRooms@", "Integer", "0", "If zero, the pointer drops objects when a metaroom change occurs. If non-zero, then it continues to carry them allowing objects to be moved between metarooms by the pointer."),
	TableSpec("@engine_password@", "String", "&lt;NONE&gt;", "If set, this allows the determining of the password for the world. This does not set anything other than the pswd returned - enabling world-switchers to correctly deal with the passwords."),
	TableSpec("@engine_creature_template_size_in_mb@", "Integer", "1", "This is the amount of data space allocated to the creature gallery sprite files. If a single life stage image set exceeds this value - problems will ensue"),
	TableSpec("@engine_near_death_track_name@","String", "&lt;NONE&gt;", "This sets the track (and optionally MNG file) for the track which the engine will play when a creature gets near death within the game."),
	TableSpec("@engine_plane_for_lines@", "Integer", "9998", "This sets the plane for lines drawn on entity images. This includes Connective agent lines and the debug cabins"),
	TableSpec("@engine_synchronous_learning@", "Integer", "0", "By default learning is asynchronous, so the creature learns from all @#STIM@s.  Set to 1 to make the creatures learn only from @#STIM@s caused by the action they are thinking about carried out by the agent their attention is on."),
	TableSpec("@engine_zlib_compression@","Integer","6", "Saved worlds and other archives are compressed using zlib.  This sets the compression level. The value ranges between: 0 - No compression, 1 - Best speed, 9 - Best compression."),
	
	TableSpec(), // Birth deserves a seperate table
	TableSpec("@engine_multiple_birth_first_chance@", "Float", "0.0", "This sets the chance (from 0 to 1) that a mate event will result in multiple pregnancies"),
	TableSpec("@engine_multiple_birth_subsequent_chance@", "Float", "0.0", "This sets the subsequent chance (from 0 to 1) that more than two children are conceived"),
	TableSpec("@engine_multiple_birth_maximum@", "Integer", "1", "This sets the maximum number of births allowed by the engine"),
	TableSpec("@engine_multiple_birth_identical_chance@", "Float", "0.5", "This sets the chance that the children will be genetically identical"),

	TableSpec(), // Time specs
	TableSpec("@engine_LengthOfDayInMinutes@", "Integer", "0", "This sets the number of minutes in a day. (Game time)"),
	TableSpec("@engine_LengthOfSeasonInDays@", "Integer", "0", "This sets the number of days in a season. (Game time)"),
	TableSpec("@engine_NumberOfSeasons@", "Integer", "0", "This sets the number of seasons in a year. (Game Time)"),

	TableSpec(), // Volume and sound Specs
	TableSpec("@engine_volume@", "Integer", "0", "This sets the normal volume for the game."),
	TableSpec("@engine_mute@", "Integer", "0", "This sets whether (non zero) or not (zero) to mute the MIDI player"),
	TableSpec("@engine_playAllSoundsAtMaximumLevel@","Integer", "0", "If non-zero, all sounds played by agents are set to full volume and center pan."),
	TableSpec("@engine_usemidimusicsystem@", "Integer", "0", "If non-zero, this tells the engine to use MIDI music instead of the MUNGED data usually used."),

	TableSpec(), // Now the cav specifics
	TableSpec("@cav_birthdate@", "String", "&lt;NONE&gt;", "If set, the engine uses this to inform the birthday agent when it is the player's birthday."),
	TableSpec("@cav_quittime@","String","&lt;NONE&gt;", "If set, the engine uses this to inform the countdown clock agent to show its countdown, and also to quit the game at the specified time."),
	TableSpec("@cav_gamelengthIsPerDay@", "String", "&lt;NONE&gt;", "If set, the engine uses this to inform the countdown clock agent when the maximum play time for the day is up."),
	TableSpec("@cav_useparentmenu@", "Integer", "0", "If non-zero, this causes the engine to deal with extra information such as the countdown clock and birthday agents. These are activated by registry settings."),
	TableSpec("@cav_CountdownClockAgent@", "Agent", "NULL", "If not null, countdown messages are sent to this agent."),
	TableSpec("@cav_BirthdayBannerAgent@", "Agent", "NULL", "If not null, birthday messages are sent to this agent."),

	TableSpec(),
	TableSpec("@engine_distance_before_port_line_warns@", "Float", "600.0", "At this distance, port lines will pulse red not blue to indicate that they are nearning their maximum length before they snap"),
	TableSpec("@engine_distance_before_port_line_snaps@", "Float", "800.0", "At this distance, port lines will simply snap."),
};

TableSpec ourDebugKeys[] =
{
	TableSpec("Debug Keys"),
	TableSpec("Key", "Action"),

	TableSpec("Pause", "Performs a DEBUG pause on the game (no more ticking messages are sent)"),
	TableSpec("Space", "Even when debug paused, this causes a tick to execute - allowing single stepping of the world"),

	TableSpec(),
	TableSpec("Insert", "Turns the mapline display on or off"),
	TableSpec("Page Up", "Moves to the previous Metaroom"),
	TableSpec("Page Down", "Moves to the next Metaroom"),
	TableSpec("Home", "Cycles through the resolutions available to the display engine"),
	TableSpec("Delete", "Toggles the highlight box around agents which a creature is paying attention to"),
	TableSpec("End", "Toggles the skeleton update speed between normal and double speed"),

	TableSpec(),

	TableSpec("Numpad 0", "Sets creature permission highlights to CanActivate1"),
	TableSpec("Numpad 1", "Sets creature permission highlights to CanActivate2"),
	TableSpec("Numpad 2", "Sets creature permission highlights to CanDeactivate"),
	TableSpec("Numpad 3", "Sets creature permission highlights to CanHit"),
	TableSpec("Numpad 4", "Sets creature permission highlights to CanEat"),
	TableSpec("Numpad 5", "Sets creature permission highlights to CanPickUp"),
	TableSpec("Numpad 6", "Turns creature permission highlights off"),

};
int dummyAppOwnedTables = AutoDocumentationTable::RegisterTable(ourGameVariables, sizeof(ourGameVariables)) +
						  AutoDocumentationTable::RegisterTable(ourDebugKeys, sizeof(ourDebugKeys));

int App::EorWolfValues(int andMask, int eorMask)
{
	int wolf = 0;

	if (myRenderDisplay)
		wolf += 1;
	if (myFastestTicks)
		wolf += 2;
	if (myRenderDisplayNextTick)
		wolf += 4;
	if (myAutoKillAgentsOnError)
		wolf += 8;

	wolf = (wolf & andMask) ^ eorMask;

	myRenderDisplay = (wolf & 1) == 1;
	myFastestTicks = (wolf & 2) == 2;

	myRenderDisplayNextTick = (wolf & 4) == 4;
	myAutoKillAgentsOnError = (wolf & 8) == 8;

	// In case autokill has changed
#ifdef ResetWindowTitle
	ResetWindowTitle();
#else
#warning "TODO: Implement ResetWindowTitle"
#endif
	return wolf;
}

bool App::DebugKeyNowNoShift()
{
	return (theApp.GetWorld().GetGameVar("engine_debug_keys").GetInteger() == 1);
}

bool App::DebugKeyNow()
{
	return DebugKeyNowNoShift() && GetInputManager().IsKeyDown(VK_SHIFT);
}

int App::GetZLibCompressionLevel()
{
	if (myWorld)
		return GetWorld().GetGameVar("engine_zlib_compression").GetInteger();
	else
		return 6;
}

#ifdef _WIN32
bool App::CheckForCD()
{
	// Look for msnope32.dll in the windows system directory
    char systemDirectory[MAX_PATH + 1];
	GetSystemDirectory(systemDirectory, MAX_PATH);
	std::string overrideFilename = std::string(systemDirectory) + "\\msnope32.dll";
	int overrideAttributes = GetFileAttributes(overrideFilename.c_str());
	if (overrideAttributes != -1)
	{
		// if we have it, it's a developer overriding the protection
		return true;
	}

	// Otherwise look for a CD..
	// Get all drive names
    char allDrives[4096];
	GetLogicalDriveStrings(4095, allDrives);

	char* pDrive = allDrives;
	while (*pDrive)
	{
		// Check next drive for being a CD
		std::string driveName = pDrive;
		if (GetDriveType(driveName.c_str()) == DRIVE_CDROM)
		{
			// Look for Creatures 3 icon file on the CD
			std::string fileToFind = driveName + "Install\\Install\\Creatures 3.ico";
			int filePresent = GetFileAttributes(fileToFind.c_str());
			if(filePresent != -1)
			{
				return true;
			}
		}

		// add on length, and the null
		pDrive += driveName.size();
		++pDrive;
	}

	// Nothing found, reject
	return false;
}

bool App::CheckForMutex()
{
	std::string mutex_name = GetGameName() + "_unique";
	myMutex = CreateMutex(NULL, TRUE, mutex_name.c_str());
    if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(myMutex);
		CloseHandle(myMutex);
		return false;
	}
	return true;
}

bool App::CheckAllFreeDiskSpace()
{
    char systemDirectory[MAX_PATH + 1];
	GetSystemDirectory(systemDirectory, MAX_PATH);
	if (!CheckFreeDiskSpace(systemDirectory, true))
		return false;

	for (int i = 0; i < NUM_DIRS; ++i)
	{
		if (!CheckFreeDiskSpace(m_dirs[i], false))
			return false;
	}

	return true;
}

bool App::CheckFreeDiskSpace(std::string path, bool systemDirectory)
{

// for non-win32 we'll just assume there is enough space
#ifdef _WIN32

	if (path.size() < 3)
		return true;

	path = path.substr(0, 3);

	int32 expected = 0;
	if (systemDirectory)
	{
		// 75Mb by default
		expected = 75 * 1024 * 1024;
		theRegistry.GetValue(theRegistry.DefaultKey(), "DiskSpaceCheckSystem", expected, HKEY_CURRENT_USER);
	}
	else
	{
		// 32Mb by default
		expected = 32 * 1024 * 1024;
		theRegistry.GetValue(theRegistry.DefaultKey(), "DiskSpaceCheck", expected, HKEY_CURRENT_USER);
	}

	DWORD sectorsPerCluster = 0;
	DWORD bytesPerSector = 0;
	DWORD numberOfFreeClusters = 0;
	DWORD totalNumberOfClusters = 0;
	BOOL ok = GetDiskFreeSpace(path.c_str(), &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters);
	if (ok)
	{
		unsigned int size = numberOfFreeClusters * sectorsPerCluster * bytesPerSector;
		if (size < expected)
			return false;
	}
#else
	#warning // No free-disk-space check for non-win32
#endif

	return true;
}

#endif // _WIN32





