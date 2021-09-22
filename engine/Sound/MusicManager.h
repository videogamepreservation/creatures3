// --------------------------------------------------------------------------
// Filename:	Music Manager.h
// Class:		MusicManager
// Purpose:		Provides interface between soundtracks and 'outside world'
//
// Description:
//
// Provides management for named soundtracks.  The soundtrack should be
// updated regularly with mood and threat settings (given as values
// ranging from 0.0 to 1.0).  Other exposed state variables can also be
// set if necessary.
//
// The class data can now be serialised.  Note that serialisation should
// be used only to read / write the data, not to create the manager
// 
// History:
// 8Apr98	PeterC	Created
// 13Apr98	PeterC	Altered interupt track to keep playing until the next
//					track arrives
// 15Apr98	PeterC	MusicManager class is now serializable
// 19Apr98	PeterC	Removed StopPlaying
// 19Apr98	PeterC	Removed StopPlaying
//					Play, Pause, BeginTrack, InteruptTrack, Fade all wait
//					until the next update before taking effect
// 13Aug98	PeterC	Added support for dual format serialisation
// --------------------------------------------------------------------------

#ifndef _MUSIC_MANAGER_H
#define _MUSIC_MANAGER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "../../common/C2eTypes.h"
#include "MusicUpdatable.h"
#include "../PersistentObject.h"
#include <string>
#include <vector>

const std::string ME_NO_TRACK = "none";

class MusicLayer;
class MusicAction;
class MusicTrack;

class MusicManager : public MusicUpdatable
	{
	// ----------------------------------------------------------------------
	// Simple state needs to be serialised (current track playing, volume,
	// mood, threat
	// ----------------------------------------------------------------------
	CREATURES_DECLARE_SERIAL( MusicManager )
	public:
		

		// ----------------------------------------------------------------------
		// Method:		MusicManager
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicManager();

		// ----------------------------------------------------------------------
		// Method:		~MusicManager
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicManager();

		// ----------------------------------------------------------------------
		// Method:		DestroyContents
		// Arguments:	None
		// Returns:		None
		// Description: Deletes tracks etc - useful in preparation for a new MNG
		// ----------------------------------------------------------------------
		void DestroyContents();

		// ----------------------------------------------------------------------
		// Method:		Load
		// Arguments:	LPCTSTR script - script describing music system
		// Returns:		NULL if successful, or pointer to error in script
		// Description:	Parses entire soundtrack from a script
		// ----------------------------------------------------------------------
		LPCTSTR Load(LPCTSTR script);

		// ----------------------------------------------------------------------
		// Method:		LoadScrambled
		// Arguments:	None
		// Returns:		true if script was successfully parsed
		// Description:	Loads and compiles script from munged file
		// ----------------------------------------------------------------------
		bool LoadScrambled();

		// ----------------------------------------------------------------------
		// Method:		UpdateSettings
		// Arguments:	mood   - Creature mood (0.0 to 1.0)
		//				threat - Creature threat (0.0 to 1.0)
		// Returns:		Nothing
		// Description:	Updates any active soundtrack's mood status
		// ----------------------------------------------------------------------
		void UpdateSettings(MusicValue mood, MusicValue threat);

		// ----------------------------------------------------------------------
		// Method:		Update
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Updates the music manager.  Should be triggered by the
		//				associated thread every 55ms
		// ----------------------------------------------------------------------
		void Update();

		// ----------------------------------------------------------------------
		// Method:		Play
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Begins / Resumes the soundtrack
		//				This will take effect at the next call to Update()
		// ----------------------------------------------------------------------
		void Play()
			{statusEvent = PlayEvent;}

		// ----------------------------------------------------------------------
		// Method:		Pause
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Suspends all soundtracks
		//				This will take effect at the next call to Update()
		// ----------------------------------------------------------------------
		void Pause()
			{statusEvent = PauseEvent;}


		// ----------------------------------------------------------------------
		// Method:		SetVariable
		// Arguments:	name  - name of the variable
		//				value - new value
		// Returns:		true if variable could be set
		// Description:	Provides means of updating any read/write variables
		//				exposed by the manager
		// ----------------------------------------------------------------------
		bool SetVariable(LPCTSTR name, MusicValue value);

		// ----------------------------------------------------------------------
		// Method:		ReadVariableContents
		// Arguments:	name  - name of the variable
		//				value - to hold contents
		// Returns:		true if variable existed
		// Description:	Provides means of reading variables	exposed by the 
		//				manager
		// ----------------------------------------------------------------------
		bool ReadVariableContents(LPCTSTR name, MusicValue &value);

		// ----------------------------------------------------------------------
		// Method:		GetVariable
		// Arguments:	varName - name of the variable
		//				lvalue  - true if value can be written to
		// Returns:		Pointer to variable, or NULL if not defined
		// Description:	Searches through list of named / pre-defined variables
		// ----------------------------------------------------------------------
		MusicValue* GetVariable(LPCTSTR varName, bool lvalue = false);

		// ----------------------------------------------------------------------
		// Method:		BeginTrack
		// Arguments:	name - name of soundtrack to begin
		// Returns:		Nothing
		// Description:	Reset state, and commence playing
		//				Once the new track finishes, the old one will 
		//				automatically restart from the beginning
		//				This will take effect at the next call to Update()
		// ----------------------------------------------------------------------
		void BeginTrack(LPCTSTR name)
			{trackEvent = BeginEvent; nextTrackName = name;}


		// ----------------------------------------------------------------------
		// Method:		InteruptTrack
		// Arguments:	name     - name of soundtrack to interupt with
		// Returns:		Nothing
		// Description:	Fades out current track quickly, and starts the new track 
		//				playing.
		//				This will take effect at the next call to Update()
		// ----------------------------------------------------------------------
		void InteruptTrack(LPCTSTR name)
			{trackEvent = InteruptEvent; nextTrackName = name;}


		// ----------------------------------------------------------------------
		// Method:		Fade
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Gradually fades out the currently playing track, leaving
		//				nothing in its place
		//				This will take effect at the next call to Update()
		// ----------------------------------------------------------------------
		void Fade()
			{trackEvent = FadeEvent;}

		// ----------------------------------------------------------------------
		// Method:		SetVolume
		// Arguments:	vol - new volume
		// Returns:		Nothing
		// Description:	Sets the overall volume of all tracks playing
		// ----------------------------------------------------------------------
		void SetVolume(MusicValue vol);

		// ----------------------------------------------------------------------
		// Method:		GetVolume
		// Arguments:	None
		// Returns:		Overall volume for all tracks playing
		// ----------------------------------------------------------------------
		MusicValue GetVolume() {return volume;}

		// ----------------------------------------------------------------------
		// Method:		GetMood
		// Arguments:	None
		// Returns:		Target mood setting (mood gradually tends towards the
		//				target, to avoid sudden changes)
		// ----------------------------------------------------------------------
		MusicValue GetMood() {return targetMood;}

		// ----------------------------------------------------------------------
		// Method:		GetThreat
		// Arguments:	None
		// Returns:		Target threat setting (mood gradually tends towards the
		//				target, to avoid sudden changes)
		// ----------------------------------------------------------------------
		MusicValue GetThreat() {return targetThreat;}

		// ----------------------------------------------------------------------
		// Method:		IsPlaying
		// Arguments:	None
		// Returns:		true if manager is not resting
		// ----------------------------------------------------------------------
		bool IsPlaying() {return status == Playing;}

		// ----------------------------------------------------------------------
		// Method:		GetCurrentTrackName
		// Arguments:	None
		// Returns:		Name of active track, or NULL if none is selected
		// ----------------------------------------------------------------------
		LPCTSTR GetCurrentTrackName();

		// ----------------------------------------------------------------------
		// Method:		StartReadingTracks
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Prepares to start reading the track names
		// ----------------------------------------------------------------------
		void StartReadingTracks();

		// ----------------------------------------------------------------------
		// Method:		GetNextTrack
		// Arguments:	name - To hold name of track
		// Returns:		true if there was still a track to read
		// Description:	Reads the next track from the list
		// ----------------------------------------------------------------------
		bool GetNextTrack(std::string& name);

		// ----------------------------------------------------------------------
		// Method:		NumberOfTracks
		// Arguments:	None
		// Returns:		Count on number of tracks stored
		// Description:	Reads the next track from the list
		// ----------------------------------------------------------------------
		inline int NumberOfTracks() const
        {   return tracks.size();}



	// ----------------------------------------------------------------------
	// Method:		Write
	// Arguments:	archive - archive being written to
	// Returns:		true if successful
	// Description:	Overridable function - writes details to archive,
	//				taking serialisation into account
	// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;


	// ----------------------------------------------------------------------
	// Method:		Read
	// Arguments:	archive - archive being read from
	// Returns:		true if successful
	// Description:	Overridable function - reads detail of class from archive
	// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);



	private:

		// ----------------------------------------------------------------------
		// Method:		ActivatePlay
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Begins / Resumes the soundtrack
		// ----------------------------------------------------------------------
		void ActivatePlay();

		// ----------------------------------------------------------------------
		// Method:		ActivatePause
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Suspends all soundtracks
		// ----------------------------------------------------------------------
		void ActivatePause();

		// ----------------------------------------------------------------------
		// Method:		ActivateBeginTrack
		// Arguments:	name - name of soundtrack to begin
		// Returns:		Nothing
		// Description:	Reset state, and commence playing
		//				Once the new track finishes, the old one will 
		//				automatically restart from the beginning
		// ----------------------------------------------------------------------
		void ActivateBeginTrack(LPCTSTR name);

		// ----------------------------------------------------------------------
		// Method:		ActivateInteruptTrack
		// Arguments:	name     - name of soundtrack to interupt with
		// Returns:		Nothing
		// Description:	Fades out current track quickly, and starts the new track 
		//				playing.
		// ----------------------------------------------------------------------
		void ActivateInteruptTrack(LPCTSTR name);

		// ----------------------------------------------------------------------
		// Method:		ActivateFade
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Gradually fades out the currently playing track, leaving
		//				nothing in its place
		// ----------------------------------------------------------------------
		void ActivateFade();

		// ----------------------------------------------------------------------
		// Method:		PreParse
		// Arguments:	script - script, pointing to the start of the track
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Allocates the name and variables for the track, allowing
		//				other layers to refer to it
		// ----------------------------------------------------------------------
		MusicError PreParse(MusicScript &script);
		
		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script - script, pointing to the start of the track
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the track from the script
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script);

		// ----------------------------------------------------------------------
		// Method:		GetTrack
		// Arguments:	trackName - name of the track
		// Returns:		Pointer to track, or NULL if not defined
		// Description:	Searches through list of named tracks within manager
		// ----------------------------------------------------------------------
		MusicTrack *GetTrack(LPCTSTR trackName);

		// ----------------------------------------------------------------------
		// Method:		Approach
		// Arguments:	currentValue - value being altered
		//				desiredValue - value being approached
		//				change		 - maximum change
		// Returns:		Nothing
		// Description:	Slowly moves a value towards a desired value
		// ----------------------------------------------------------------------
		void Approach(MusicValue &currentValue,
					  MusicValue desiredValue,
					  MusicValue change) const;

		// ----------------------------------------------------------------------
		// Member attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// status
		// Current play status of manager
		// ----------------------------------------------------------------------
		enum {Paused, Playing} status;

		// ----------------------------------------------------------------------
		// playEvent
		// Flags status to be imposed next update
		// ----------------------------------------------------------------------
		enum {NoStatusEvent, PlayEvent, PauseEvent} statusEvent;

		// ----------------------------------------------------------------------
		// trackEvent
		// Flags the track to be played / faded next update
		// ----------------------------------------------------------------------
		enum {NoTrackEvent, BeginEvent, InteruptEvent, FadeEvent} trackEvent;

		// ----------------------------------------------------------------------
		// nextTrackName
		// Name of the next track to be played (if there is a relevent track
		// event)
		// ----------------------------------------------------------------------
		std::string nextTrackName;

		// ----------------------------------------------------------------------
		// volume
		// volume of the track (0.0 = silent, 1.0 = maximum)
		// this is accessible as 'volume' to actions/expressions
		// ----------------------------------------------------------------------
		MusicValue volume;

		// ----------------------------------------------------------------------
		// All of the following can be accessed (read only) by actions and
		// expressions
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// currentMood
		// Mood of the creatures (this is regulated so that it does not jump
		// too drastically)
		// ----------------------------------------------------------------------
		MusicValue currentMood;

		// ----------------------------------------------------------------------
		// targetMood
		// Desired mood for the creatures.  currentMood will slowly tend towards
		// this
		// ----------------------------------------------------------------------
		MusicValue targetMood;

		// ----------------------------------------------------------------------
		// currentThreat
		// Threat to the creatures (this is regulated so that it does not jump
		// too drastically)
		// ----------------------------------------------------------------------
		MusicValue currentThreat;

		// ----------------------------------------------------------------------
		// targetThreat
		// Desired threat to the creatures.  currentThreat will slowly tend towards
		// this
		// ----------------------------------------------------------------------
		MusicValue targetThreat;

		// ----------------------------------------------------------------------
		// tracks
		// Array of all available soundtracks
		// ----------------------------------------------------------------------
		std::vector <MusicTrack *> tracks;
		typedef std::vector<MusicTrack*>::iterator tracksIterator;

		// ----------------------------------------------------------------------
		// desiredTrack
		// The Track desired if a change of MNG file is required.
		// This "supercedes" whilst cooperating with currentTrack
		// ----------------------------------------------------------------------
		std::string desiredTrack;

		// ----------------------------------------------------------------------
		// currentMNG
		// The MNG file currently loaded into the MusicManager's SoundLib.
		// ----------------------------------------------------------------------
		std::string currentMNG;

		// ----------------------------------------------------------------------
		// currentTrack
		// Track currently playing
		// ----------------------------------------------------------------------
		MusicTrack *currentTrack;

		// ----------------------------------------------------------------------
		// oldTrack
		// Track currently retiring (if any)
		// ----------------------------------------------------------------------
		MusicTrack *oldTrack;

		// ----------------------------------------------------------------------
		// time
		// Overall music system time (seconds since startup)
		// ----------------------------------------------------------------------
		MusicValue time;

		// ----------------------------------------------------------------------
		// currentReading
		// Variable used when reading the names of all the tracks
		// ----------------------------------------------------------------------
		int currentReading;
	};

#endif
