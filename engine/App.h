// --------------------------------------------------------------------------
// Filename:	App.h
// Class:		App
// Purpose:		This contains the application class
//				
//
// Description: 
//			
//				
//
// History:
// -------  Steve Grand		created
// 04Dec98	Alima			Commented out old display engine stuff and put
//							in new display engine hooks.
//	
//////////////////////////////////////////////////////////////////////////////

#ifndef SmallFurryCreatures_h
#define SmallFurryCreatures_h

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"
#include "resource.h"
#ifdef _WIN32
#include "Display/Window.h"
#endif
#include "TimeFuncs.h"
#include "Caos/RequestManager.h"
#include "InputManager.h"
#include "AppConstants.h"

#ifndef _WIN32
// registry replacement (could be used for win32 too...)
#include "../common/Configurator.h"
#endif



class World;
class Agent;
class PrayManager;

////////////////////// GLOBALS ////////////////////////



extern class App 		theApp;
extern class SoundManager*  theSoundManager;				// pointer to sound manager object
extern class MusicManager*  theMusicManager;		// pointer to global music manager object

class EntityImage;

class App
{ 
public:
	App();
	~App();

#ifndef _WIN32
	// Sets up the registry-replacing config files, which are accessed by
	// UserSettings() and MachineSettings()
	// _Should_ really be in Init() or constructor, but we need this
	// separate function because App::Init() is called _after_ the window
	// is opened but the startup code accesses other App members before
	// the window is opened.
	//
	// Sigh.
	//
	// (Startup code is in Display/Window.cpp or Display/SDL/SDL_Main.cpp)
	bool InitConfigFiles( const std::string& userfile,
		const std::string& machinefile );
#endif

	const char*	GetDirectory( int dir )	{ return (const char*)m_dirs[dir]; }
	const char*	GetDirectory( int dir, char* buffer )
				{ return (const char*)(strcpy( buffer, 
				(const char*)m_dirs[dir] ) ); }

	void DisplaySettingsErrorNextTick()
	{
		myDisplaySettingsErrorNextTick = true;
	}

	void InitialiseFromGameVariables();
	// get the world directory path of the given folder
	// e.g. ..\\Worlds\\Myworld\\Images
	bool GetWorldDirectoryVersion(int dir, std::string& path,bool createFilePlease =false);
	// or a named one that isn't in the main world (attic etc.)
	bool GetWorldDirectoryVersion(const std::string& name, std::string& path,bool createFilePlease /*=false*/);

	inline int GetLinePlane()
	{
		return myPlaneForConnectiveAgentLines;
	}

	inline int GetCreaturePickupStatus()
	{
		return myCreaturePickupStatus;
	}

	inline bool ShouldSkeletonsAnimateDoubleSpeed() {
		return myShouldSkeletonsAnimateDoubleSpeedFlag;
	}
	inline void ShouldSkeletonsAnimateDoubleSpeed(bool b) {
		myShouldSkeletonsAnimateDoubleSpeedFlag = b;
	}

	inline bool ShouldHighlightAgentsKnownToCreature() {
		return myShouldHighlightAgentsKnownToCreatureFlag;
	}
	inline void SetWhetherWeShouldHighlightAgentsKnownToCreature(bool b) {
		myShouldHighlightAgentsKnownToCreatureFlag = b;
	}
	inline void SetWhichCreaturePermissionToHighlight(int i) {
		myCreaturePermissionToHighlight = i;
	}
	inline int GetWhichCreaturePermissionToHighlight() 
	{
		return myCreaturePermissionToHighlight;
	}
	
	inline float GetMaximumDistanceBeforePortLineWarns()
	{
		return myMaximumDistanceBeforePortLineWarns;
	}
	inline float GetMaximumDistanceBeforePortLineSnaps()
	{
		return myMaximumDistanceBeforePortLineSnaps;
	}

	// return a reference to the world
	World& GetWorld()
		{return *myWorld;}

	int GetZLibCompressionLevel();

	// Return a reference to the resource manager
	PrayManager& GetResourceManager()
		{return *myPrayManager;}

	InputManager& GetInputManager()
		{ return myInputManager; }

#ifdef _WIN32
	HWND GetHandle()
		{ return myMainHWND; }
	HWND GetMainHWND()
		{ return myMainHWND; }
#endif // _WIN32

	std::string GetGameName() {return myGameName;}
	void SetGameName(const std::string& gameName) { myGameName = gameName; }

	static int GetWorldTickInterval()
	{
		return 50;
	}

	bool DoYouOnlyPlayMidiMusic()
	{return IOnlyPlayMidiMusic;}

	bool PlayAllSoundsAtMaximumLevel(){return myPlayAllSoundsAtMaximumLevel;}

	// the macro language can request that the app
	// uses the parent menu settings
	void HandleAdditionalRegistrySettings();
	void HandleInput();

#ifdef _WIN32
	bool Init( HWND wnd );
#else
	bool Init();
#endif

	void UpdateApp();
	void ShutDown();
	void HandleIncomingRequest( ServerSide& server );

	void ChangeResolution();
	void ToggleFullScreenMode();
	void SetUpMainView(); // here until I know what the room system will
							// be about
	void DisableMapImage();
	void EnableMapImage();
	void WindowHasResized();
	void WindowHasMoved();

	void DisableMainView();
	void EnableMainView();

	void ToggleMidi(){IOnlyPlayMidiMusic = !IOnlyPlayMidiMusic;}

	bool GetDirectories();
	bool InitLocalisation();
	bool CreateNewWorld(std::string& worldName);

	void BeginWaitCursor();
	void EndWaitCursor();

	inline uint32 GetSystemTick() const { return mySystemTick; }
	float GetTickRateFactor();
	bool GetFastestTicks() const { return myFastestTicks; }
	int GetLastTickGap() const { return myLastTickGap; }

	bool CreateProgressBar();
	void StartProgressBar(int catOffset);
	void SpecifyProgressIntervals(int updateIntervals);

	void UpdateProgressBar();
	void EndProgressBar();

	void DoParentMenuTests(int bedTimeMinutes,bool& quitgame,
								SYSTEMTIME& currentTime,
								bool& nornsAreTiredAlready);
	bool DoYouUseAdditionalRegistrySettings(){return IUseAdditionalRegistrySettings;}
	bool ConstructSystemTime(SYSTEMTIME& time, std::string& string);
	SYSTEMTIME& GetStartTime(){return myGameStartTime;}
	void GetTimeGameShouldEnd();
	void GameEndHelper(SYSTEMTIME& dest, SYSTEMTIME& source);

	void SetPassword(std::string& password);
	std::string GetPassword();
	bool DoINeedToGetPassword();

	void RefreshGameVariables();

	bool ProcessCommandLine(std::string commandLine);
	bool AutoKillAgentsOnError() const { return myAutoKillAgentsOnError; }
	int EorWolfValues(int andMask, int eorMask);

	bool DebugKeyNow();
	bool DebugKeyNowNoShift();

private:
	void DoLoadWorld(std::string worldName);
	void internalWindowHasResized();
	void internalWindowHasMoved();

#ifdef _WIN32
	bool CheckForCD();
	bool CheckForMutex();
	bool CheckAllFreeDiskSpace();
	bool CheckFreeDiskSpace(std::string path, bool systemDirectory);
#endif

public:
	bool mySaveNextTick;
	std::string myLoadThisWorldNextTick;
	bool myQuitNextTick;
	bool myToggleFullScreenNextTick;

	int myScrollingMask;
	std::vector<byte> myScrollingSpeedRangeUp;
	std::vector<byte> myScrollingSpeedRangeDown;

	static const int ourTickLengthsAgo;

#ifndef _WIN32
	// registry replacement

	// access settings specific to current player
	Configurator& UserSettings()
		{ return myUserSettings; }

	// access settings global to whole installation (eg directory paths)
	Configurator& MachineSettings()
		{ return myMachineSettings; }
#endif

private:

#ifndef _WIN32
	// registry replacement
	Configurator myUserSettings;
	Configurator myMachineSettings;
#endif

	bool myResizedFlag;
	bool myMovedFlag;
	bool myDisplaySettingsErrorNextTick;
	bool myDelayedResizeFlag;
	bool myDelayedMovingFlag;
	bool InitLocalCatalogueFilesFromTheWorldsDirectory();

	bool	myShouldSkeletonsAnimateDoubleSpeedFlag;
	bool	myShouldHighlightAgentsKnownToCreatureFlag;
	int		myCreaturePermissionToHighlight;
	int		myPlaneForConnectiveAgentLines;

	int myCreaturePickupStatus;
//	bool myShouldCreatureClicksBeHoldingHands;


#ifdef _WIN32
	char	m_dirs[NUM_DIRS][MAX_PATH];
#else
	char	m_dirs[NUM_DIRS][512];
#endif

	void	SetUpSound(void);
	bool	IOnlyPlayMidiMusic;
#ifdef _WIN32
	HWND	myMainHWND;
	HCURSOR myCursor;
	LPCTSTR m_pszRegistryKey;   // used for registry entries
    HANDLE		myMutex;
#endif // _WIN32

	// lots of Creatures Adventures stuff thinly veiled
	// as generic stuff
	bool	IUseAdditionalRegistrySettings;
	SYSTEMTIME myBedTime;
	SYSTEMTIME myMaxPlayingTime;
	SYSTEMTIME myGameStartTime;
	SYSTEMTIME myRecordOfYourBirthday;
	SYSTEMTIME myEndGameTime;

	World*		 myWorld;
	PrayManager* myPrayManager;
	RequestManager		myRequestManager;

	// The system framework will keep the InputManager fed (see window.cpp)
	InputManager		myInputManager;

	std::string myGameName;

	uint32  mySystemTick;
	EntityImage* myProgressBar;
	bool myPlayAllSoundsAtMaximumLevel;
	bool myAutoKillAgentsOnError;

	std::vector<uint32> myRecentTickLengths;
	int myRecentTickPos;
	std::string myPasswordForNextWorldLoaded;
	bool myIHaveAPassword;
	
	bool myRenderDisplay;
	bool myRenderDisplayNextTick;
	bool myFastestTicks;

	float myMaximumDistanceBeforePortLineWarns;
	float myMaximumDistanceBeforePortLineSnaps;

	uint32 myStartStamp;
	int myLastTickGap;
};


/////////////////////////////////////////////////////////////////////////////

#endif
