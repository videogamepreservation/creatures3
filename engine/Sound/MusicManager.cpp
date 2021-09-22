// --------------------------------------------------------------------------
// Filename:	Music Manager.cpp
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
// History:
// 24Apr98	PeterC	Created
// 07May98	PeterC	Altered to have all tracks continuously update
// 08May98	PeterC	Changed format of update declarations
// 13Apr98	PeterC	Altered interupt track to keep playing until the next
//					track arrives
// 15Apr98	PeterC	MusicManager class is now serializable
// 19Apr98	PeterC	Removed StopPlaying
//					Play, Pause, BeginTrack, InteruptTrack, Fade all wait
//					until the next update before taking effect
// 13Aug98	PeterC	Added support for dual format serialisation
// --------------------------------------------------------------------------


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicManager.h"
#include "MusicLayer.h"
#include "MusicTrack.h"
#include "MusicScript.h"
#include "MusicTimer.h"
#include "MusicGlobals.h"
#include "../Scramble.h"
#include "Soundlib.h"
#include "../Display/ErrorMessageHandler.h"
#include "../App.h"
#include "../File.h"

// ----------------------------------------------------------------------
// Simple state needs to be serialised (current track playing, volume,
// mood, threat
// ----------------------------------------------------------------------
CREATURES_IMPLEMENT_SERIAL( MusicManager)

// ----------------------------------------------------------------------
// Method:		MusicManager
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicManager::MusicManager() :
	currentMood(0.0),
	currentThreat(0.0),
	currentTrack(NULL),
	oldTrack(NULL),
	targetMood(0.0),
	targetThreat(0.0),
	volume(1.0),
	time(0.0),
	currentReading(0),
	status(Paused),
	statusEvent(NoStatusEvent),
	trackEvent(NoTrackEvent)
{
	desiredTrack = "";
	currentMNG = "music.mng";
}

// ----------------------------------------------------------------------
// Method:		~MusicManager
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicManager::~MusicManager()
{
	DestroyContents();
}

void MusicManager::DestroyContents()
	{
	// Stop everything dead
	ActivatePause();

	// Yes - clear out the store of tracks
	tracksIterator it;

	for (it = tracks.begin(); it != tracks.end(); it++ )
		{
		// First delete the allocated space
		delete (*it);
		}

	// Now clear out the entries
	tracks.clear();

	}

// ----------------------------------------------------------------------
// Method:		Load
// Arguments:	LPCTSTR script - script describing music system
// Returns:		NULL if successful, or pointer to error in script
// Description:	Parses entire soundtrack from a script
// ----------------------------------------------------------------------
LPCTSTR MusicManager::Load(LPCTSTR script)
	{
	MusicScript description(script);
	
	// First pass - establish all the names and variables
	MusicError error = PreParse(description);

	if (error != MUSIC_OK)
		{
		// Return the position within the given script
		return script + description.GetOffset();
		}

	// Second parse - fill in all the other parts
	description.Restart();
	error = Parse(description);

	if (error != MUSIC_OK)
		{
		// Return the position within the given script
		return script + description.GetOffset();
		}

	// Must have succeeded

	// Take this oportunity to run the initialise script, if
	// it has one
	Initialise();

	// No errors to report
	return NULL;
	}

// ----------------------------------------------------------------------
// Method:		LoadScrambled
// Arguments:	None
// Returns:		true if script was successfully parsed
// Description:	Loads and compiles script from Sounds/Music.mng
// ----------------------------------------------------------------------
bool MusicManager::LoadScrambled()
	{
	File file;
	try
	{
		char buf[_MAX_PATH];
		theApp.GetDirectory(SOUNDS_DIR,buf);

		std::string path(buf);
		path+=currentMNG;
		
		file.Open(path,GENERIC_READ);
	}
	catch(File::FileException& e)
	{

	ErrorMessageHandler::Show(e, "MusicManager::LoadScrambled");
	return false;
	}

	// Find the offset to the start of the script (second integer in file)
	file.Seek(sizeof(int),File::Start);
	int offset;
	file.Read(&offset,sizeof(int));

	// The length of the script follows
	int length;
	file.Read(&length,sizeof(int));

	// Now move to the beginning of the script
	file.Seek(offset,File::Start);

	// Create space to load in the script (including
	// its null-terminator)
	char *scrambled = new char[length+1];
	file.Read(scrambled,length);

	// Set the null terminator
	scrambled[length] = 0;

	// And decrypt the script
	Scramble(scrambled,length);

	// Now try and compile the script,
	// checking for success
	bool success = Load(scrambled) == NULL;

	// delete the script
	delete scrambled;

	return success;

	}

// ----------------------------------------------------------------------
// Method:		PreParse
// Arguments:	script - script, pointing to the start of the track
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Allocates the name and variables for the track, allowing
//				other layers to refer to it
// ----------------------------------------------------------------------
MusicError MusicManager::PreParse(MusicScript &script)
	{
	// Look straight through the file, picking out any variables
	// and preparsing any tracks
	while (script.GetCurrentType() != MusicScript::EndOfFile)
		{
		switch (script.GetCurrentType())
			{
			case (MusicScript::String):
				{
				// We're only interested if it's a track or a variable

				// Is this a track declaration?
				if (strcmp(script.GetCurrentToken(),"Track")==0)
					{
					// Create a new track, and add it to the manager's
					// list
					MusicTrack *track = new MusicTrack;

					// Attempt to find this tracks's name and variables
					MusicError error = track -> PreParse(script);

					// If there was an error, bail out
					if (error != MUSIC_OK)
						{
						delete track;
						return error;
						}

					// Add this to the array of tracks
					tracks.push_back(track);

					}
				else
					{
					// Is this a variable declaration?
					if (strcmp(script.GetCurrentToken(),"Variable")==0)
						{
						// Attempt to add this variable to the layer's
						// store
						MusicError error = ParseVariable( script );

						// If there was an error, bail out
						if (error != MUSIC_OK)
							{
							return error;
							}

						}
					else
						{
						// String was nothing we were interested in
						// skip past it
						script.Advance();
						}
					}
				break;
				}
			default:
				{
				// Ignore anything else
				script.Advance();
				}

			}
		}

	// If we got this far, all must have been fine
	return MUSIC_OK;
	}

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script - script, pointing to the start of the track
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to Parse the track from the script
// ----------------------------------------------------------------------
MusicError MusicManager::Parse(MusicScript &script)
	{
	// Keep reading in details until the file runs out
	while (script.GetCurrentType() != MusicScript::EndOfFile)
		{
		// We definitely should have a string next
		if (script.GetCurrentType() != MusicScript::String)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		// Want to make sure we find something to parse, or
		// we'll keep going round this loop forever
		bool foundSomething = false;

		// Variable(String name, Value init)
		if ( strcmp( script.GetCurrentToken(), "Variable") == 0)
			{
			// The variable should already have been found by
			// PreParse - just check it matches up

			// Should have an open bracket next
			script.Advance();

			if (script.GetCurrentType() != MusicScript::StartArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			// Now a name - check this already exists
			ASSERT( GetVariable(script.GetCurrentToken() ) );

			// Skip over: name,comma,value
			script.Advance();
			script.Advance();
			script.Advance();

			// Verify that there was an end bracket
			if (script.GetCurrentType() != MusicScript::EndArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			foundSomething = true;
			}

		// Note that we can safely continue checking whether or not
		// we just had a variable - if we reach the '}' none of the
		// following conditions will fire

		// Initialise{Action,..}
		if ( strcmp( script.GetCurrentToken(), "Initialise") == 0)
			{
			MusicError error=ParseInitialise(script,*this);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}

		// Update{Action,..}
		if ( strcmp( script.GetCurrentToken(), "Update") == 0)
			{
			MusicError error=ParseUpdate(script,*this);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}

		// UpdateRate(Value time)
		if ( strcmp( script.GetCurrentToken(), "UpdateRate") == 0)
			{
			MusicError error = ParseUpdateRate(script);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;
			}

		// Track(String name) { ... }
		if (strcmp( script.GetCurrentToken(), "Track") == 0)
			{
			script.Advance();

			// Find the name of the track
			std::string trackName;
			MusicError error = script.ParseArgument(trackName);
			if (error != MUSIC_OK)
				{
				return error;
				}

			// Locate the preparsed layer this refers to
			MusicTrack *track = GetTrack ( trackName.data() );

			if (!track)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// Now try and parse the track, trapping any errors
			error = track -> Parse ( script, *this);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}

		// Effect(String name) { ... }
		if (strcmp( script.GetCurrentToken(), "Effect") == 0)
			{
			// Attempt to add the effect to the library of effects
			// stored statically within layers
			MusicError error = MusicLayer::ParseEffect(script);
			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;
			}

		// we didn't find anything we recognised
		if (!foundSomething)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		}
	

	// Must have compiled properly if we got this far
	return MUSIC_OK;
	}

// ----------------------------------------------------------------------
// Method:		GetTrack
// Arguments:	trackName - name of the track
// Returns:		Pointer to track, or NULL if not defined
// Description:	Searches through list of named tracks within manager
// ----------------------------------------------------------------------
MusicTrack *MusicManager::GetTrack(LPCTSTR trackName)
	{
	// Iterate through all the tracks until a match is found
	tracksIterator it;
	for (it = tracks.begin();it != tracks.end();it++)
		{
		if ((*it)->MatchName(trackName))
			{
			return (*it);
			}
		}

	// We didn't find a match
	return NULL;
	}

// ----------------------------------------------------------------------
// Method:		Approach
// Arguments:	currentValue - value being altered
//				desiredValue - value being approached
//				change		 - maximum change
// Returns:		Nothing
// Description:	Slowly moves a value towards a desired value
// ----------------------------------------------------------------------
void MusicManager::Approach(MusicValue &currentValue,
							MusicValue desiredValue,
							MusicValue change) const
	{
	// Are we already within range ?
	MusicValue difference = desiredValue - currentValue;
	if ( difference <= change && difference >= - change)
		{
		// Yes - move straight there
		currentValue = desiredValue;
		}
	else
		{
		// Are we above or below our target ?
		if (difference > 0.0)
			{
			currentValue += change;
			}
		else
			{
			currentValue -= change;
			}
		}


	}

// ----------------------------------------------------------------------
// Method:		UpdateSettings
// Arguments:	mood   - Creature mood (0.0 to 1.0)
//				threat - Creature threat (0.0 to 1.0)
// Returns:		Nothing
// Description:	Updates any active soundtrack's mood status
// ----------------------------------------------------------------------
void MusicManager::UpdateSettings(MusicValue mood,
								  MusicValue threat)
	{
	// Update the desired mood and threat, ensuring that they remain
	// within the bounds [0.0,1.0]
	if (mood < 0.0)
		{
		targetMood = 0.0;
		}
	else
		{
		if (mood > 1.0)
			{
			targetMood = 1.0;
			}
		else
			{
			targetMood = mood;
			}
		}

	if (threat < 0.0)
		{
		targetThreat = 0.0;
		}
	else
		{
		if (threat > 1.0)
			{
			targetThreat = 1.0;
			}
		else
			{
			targetThreat = threat;
			}
		}
	}

// ----------------------------------------------------------------------
// Method:		Update
// Arguments:	None
// Returns:		Nothing
// Description:	Updates the music manager.  Should be triggered by the
//				associated thread every 55ms
// ----------------------------------------------------------------------
void MusicManager::Update()
	{

	// Handle any events that might be waiting
	switch( statusEvent )
		{
		case ( PlayEvent ):
			{
			ActivatePlay();
			break;
			}
		case ( PauseEvent ):
			{
			ActivatePause();
			break;
			}
		}

	switch ( trackEvent )
		{
		case ( BeginEvent ):
			{
			ActivateBeginTrack(nextTrackName.data());
			break;
			}
		case ( InteruptEvent ):
			{
			ActivateInteruptTrack(nextTrackName.data());
			break;
			}
		case ( FadeEvent ):
			{
			ActivateFade();
			break;
			}

		}

	// Now reset the event flags
	statusEvent = NoStatusEvent;
	trackEvent = NoTrackEvent;


	if (status != Playing)
		{
		return;
		}

	// Move on by 50ms
	time = time + musicTimerResolution;

	// Handle the low level update - this will reset the counter
	// to the next update, and perform any action scripts if necessary

	MusicUpdatable::Update(time);

	// Gradually move the current mood towards the desired mood
	// **** Just move them directly for now
	Approach(currentMood,targetMood,(MusicValue) 0.05);
	Approach(currentThreat,targetThreat, (MusicValue) 0.1);


	// Is there an old track fading out ?
	if (oldTrack)
		{
		// Yes - update it
		oldTrack -> Update( volume, time);

		// Has it finished yet?
		if ( oldTrack -> IsFinished() )
			{	
			// Yes - get rid of it
			oldTrack = NULL;

			// Then begin the new one, if it exist
			if (!desiredTrack.empty())
				{
				// Here we have to load in the new MNG Files...
				std::string track;
				currentMNG = desiredTrack.substr(0,desiredTrack.find('\\'));
				if (currentMNG == desiredTrack)
					currentMNG = "music.mng";
				DestroyContents();
				theMusicSoundManager->SetMNGFile(currentMNG);
				LoadScrambled();
				status = Playing;
				track = desiredTrack.substr(desiredTrack.find('\\')+1);
				currentTrack = GetTrack(track.c_str());
				desiredTrack.erase();
				}

			if (currentTrack)
				{
				currentTrack -> BeginPlaying(volume, time);
				}
			}

		}
	else
		{
		// Is there a track currently active ?
		if (!desiredTrack.empty())
			{
			// Here we have to load in the new MNG Files...
			std::string track;
			currentMNG = desiredTrack.substr(0,desiredTrack.find('\\'));
			DestroyContents();
			theMusicSoundManager->SetMNGFile(currentMNG);
			LoadScrambled();
			status = Playing;
			track = desiredTrack.substr(desiredTrack.find('\\')+1);
			currentTrack = GetTrack(track.c_str());
			desiredTrack.erase();
			}
		if (currentTrack)
			{
			// Yes - update it
			currentTrack -> Update( volume, time);
			}
		}
	}

// ----------------------------------------------------------------------
// Method:		ActivatePlay
// Arguments:	None
// Returns:		Nothing
// Description:	Begins / Resumes the soundtrack
// ----------------------------------------------------------------------
void MusicManager::ActivatePlay()
	{
	// Can't start if we were already going
	if (status == Playing)
		{
		return;
		}

	status = Playing;

	// There should only be one track ready to play, if that
	ASSERT ( !oldTrack );

	// Completely reset the timer
	time = 0.0;
	UpdateImmediately(0.0);

	if (currentTrack)
		{
		currentTrack -> BeginPlaying(volume, time);
		}

	}

// ----------------------------------------------------------------------
// Method:		ActivatePause
// Arguments:	None
// Returns:		Nothing
// Description:	Suspends all soundtracks
// ----------------------------------------------------------------------
void MusicManager::ActivatePause()
	{
	// Can't stop if we weren't going
	if (status == Paused)
		{
		return;
		}

	status = Paused;

	// Stop any active tracks completely
	if (currentTrack)
		{
		currentTrack -> StopPlaying();
		}

	if (oldTrack)
		{
		oldTrack -> StopPlaying();
		oldTrack = NULL;
		}

	}

// ----------------------------------------------------------------------
// Method:		SetVariable
// Arguments:	name  - name of the variable
//				value - new value
// Returns:		true if variable could be set
// Description:	Provides means of updating any read/write variables
//				exposed by the manager
// ----------------------------------------------------------------------
bool MusicManager::SetVariable(LPCTSTR name, MusicValue value)
	{
	// This only applies to user defined variables in the manager
	MusicValue *variable = MusicVariableContainer::GetVariable( name );

	// Did it exist ?
	if (variable)
		{
		// Yes - copy the new value across
		* variable = value;
		return true;
		}
	else
		{
		return false;
		}
	}

// ----------------------------------------------------------------------
// Method:		ReadVariableContents
// Arguments:	name  - name of the variable
//				value - to hold contents
// Returns:		true if variable existed
// Description:	Provides means of reading variables	exposed by the 
//				manager
// ----------------------------------------------------------------------
bool MusicManager::ReadVariableContents(LPCTSTR name, MusicValue &value)
	{
	// This only applies to any variables in the manager
	MusicValue *variable = GetVariable( name );

	// Did it exist ?
	if (variable)
		{
		// Yes - copy the new value across
		value = * variable;
		return true;
		}
	else
		{
		return false;
		}
	}

// ----------------------------------------------------------------------
// Method:		GetVariable
// Arguments:	varName - name of the variable
//				lvalue  - true if value can be written to
// Returns:		Pointer to variable, or NULL if not defined
// Description:	Searches through list of named / pre-defined variables
// ----------------------------------------------------------------------
MusicValue *MusicManager::GetVariable(LPCTSTR varName, bool lvalue)
	{
	// Mood and threat are exposed as rvalues only
	if (!lvalue)
		{
		if (strcmp(varName,"Mood") == 0)
			{
			return &currentMood;
			}
		if (strcmp(varName,"Threat") == 0)
			{
			return &currentThreat;
			}
		}

	// See if any of the user-defined variables apply
	return MusicVariableContainer::GetVariable(varName,lvalue);
	}

// ----------------------------------------------------------------------
// Method:		ActivateBeginTrack
// Arguments:	name - name of soundtrack to begin
// Returns:		Nothing
// Description:	Begin playing the named track, causing the last track
//				to fade out.
//				Note that BeginTrack("Silence") will fade out the current
//				track, without replacing it
// ----------------------------------------------------------------------
void MusicManager::ActivateBeginTrack(LPCTSTR name)
	{
	// Special case - check if we're being asked for silence
	if (strcmp(name,"Silence") == 0)
		{
		// Just fade out the current track
		Fade();
		return;
		}

	// First Of all, check whether we are in the same MNG or not
	std::string incomingMNG = name;
	if ( incomingMNG.find('\\') != -1 )
		incomingMNG = incomingMNG.substr(0,incomingMNG.find('\\'));
	else
		incomingMNG = "music.mng";
	std::string realName = name;
	realName = realName.substr(realName.find('\\')+1);

	if (incomingMNG != currentMNG)
	{
		// Here we have special heuristics for managing the desired track...
		// If we are swapping MNG files, then we won't know if we are allowed to swap until
		// after we enter Update...
		desiredTrack = name;
		if (status != Playing)
		{
			// Just leave this one waiting to play when we start up
			desiredTrack = name;
			return;
		}
		if (oldTrack)
		{
			// There is an old track still fading...
			// It isn't in the target MNG, and so...
			// We do nothing it seems
		}
		else if (currentTrack)
		{
			// Here we dump the current Track into OldTrack...
			oldTrack = currentTrack;
			oldTrack->BeginFadingOut(time);
			currentTrack = NULL;
		}
		else
		{
			// Okay then, there is nothing playing, let's Jump in with both feet
			DestroyContents();
			theMusicSoundManager->SetMNGFile(incomingMNG);
			currentMNG = incomingMNG;
			LoadScrambled();
			status = Playing;
			std::string parsedTrack = name;
			parsedTrack = parsedTrack.substr(parsedTrack.find('\\')+1);
			MusicTrack* track = GetTrack(parsedTrack.c_str());
			if (track)
			{
				currentTrack = track;
				currentTrack->BeginPlaying( volume, time );
				desiredTrack.erase();
			}
		}
		return;
	}

	// Find the track matching the name
	MusicTrack *track = GetTrack(realName.c_str());

	// If we weren't playing anyway...
	if (status != Playing)
		{
		// Just leave this one waiting to play when we start up
		currentTrack = track;
		return;
		}

	// Was a track actually found ?
	if (!track)
		{
		// No - pretend we didn't notice
		return;
		}

	// Is there an old track still fading?
	if (oldTrack)
		{
		// Is this new track the same as the one fading?
		if (oldTrack == track)
			{
			// Yes - make the old track the current one,
			// and restart it (it should fade back up
			// again)
			currentTrack = oldTrack;
			oldTrack = NULL;
			currentTrack -> BeginPlaying(volume, time);
			}
		else
			{
			// Leave the new track waiting to be played
			currentTrack = track;
			}
		}
	else
		{

		// Is there already a track playing?
		if (currentTrack)
			{
			// Yes - is it different to the new one ?
			// (Don't need to do anything if it is the same)
			if (currentTrack != track)
				{
				// Yes - put it into retirement
				oldTrack = currentTrack;
				oldTrack -> BeginFadingOut(time);

				// ... and leave the new one waiting to play
				currentTrack = track;
				}
			}
		else
			{
			// There's nothing else playing, so we can just go for it
			currentTrack = track;
			currentTrack -> BeginPlaying(volume,time);
			}
		}

	}

// ----------------------------------------------------------------------
// Method:		ActivateInteruptTrack
// Arguments:	name     - name of soundtrack to interupt with
// Returns:		Nothing
// Description:	Fades out current track quickly, and starts the new track 
//				playing.
// ----------------------------------------------------------------------
void MusicManager::ActivateInteruptTrack(LPCTSTR name)
	{
	// Special case - check if we're being asked for silence
	if (strcmp(name,"Silence") == 0)
		{
		// Just fade out the current track
		Fade();
		return;
		}

	std::string incomingMNG = name;
	if ( incomingMNG.find('\\') != -1 )
		incomingMNG = incomingMNG.substr(0,incomingMNG.find('\\'));
	else
		incomingMNG = "music.mng";

	std::string realName = name;
	realName = realName.substr(realName.find('\\')+1);

	if (incomingMNG != currentMNG)
	{
		// Here we have special heuristics for managing the desired track...
		// If we are swapping MNG files, then we won't know if we are allowed to swap until
		// after we enter Update...
		desiredTrack = name;
		if (status != Playing)
		{
			// Just leave this one waiting to play when we start up
			desiredTrack = name;
			return;
		}
		if (oldTrack)
		{
			// There is an old track still fading...
			// It isn't in the target MNG, and so...
			// We do nothing it seems
		}
		else if (currentTrack)
		{
			// Here we dump the current Track into OldTrack...
			oldTrack = currentTrack;
			oldTrack->BeginFadingOut(time,1.0);
			currentTrack = NULL;
		}
		else
		{
			// Okay then, there is nothing playing, let's Jump in with both feet
			DestroyContents();
			theMusicSoundManager->SetMNGFile(incomingMNG);
			currentMNG = incomingMNG;
			LoadScrambled();
			status = Playing;
			std::string parsedTrack = name;
			parsedTrack = parsedTrack.substr(parsedTrack.find('\\')+1);
			MusicTrack* track = GetTrack(parsedTrack.c_str());
			if (track)
			{
				currentTrack = track;
				currentTrack->BeginPlaying( volume, time );

			}
			desiredTrack.erase();
		}
		return;
	}

	// Find the track matching the name
	MusicTrack *track = GetTrack(realName.c_str());

	// If we weren't playing anyway...
	if (status != Playing)
		{
		// Just leave this one waiting to play when we start up
		currentTrack = track;
		return;
		}

	// Was a track actually found ?
	if (!track)
		{
		// No - pretend we didn't notice
		return;
		}

	// Is there an old track still fading?
	if (oldTrack)
		{
		// Is this new track the same as the one fading?
		if (oldTrack == track)
			{
			// Yes - make the old track the current one,
			// and restart it (it should fade back up
			// again)
			currentTrack = oldTrack;
			oldTrack = NULL;
			currentTrack -> BeginPlaying(volume, time);
			}
		else
			{
			// Leave the new track waiting to be played
			currentTrack = track;
			}
		}
	else
		{

		// Is there already a track playing?
		if (currentTrack)
			{
			// Yes - is it different to the new one ?
			// (Don't need to do anything if it is the same)
			if (currentTrack != track)
				{
				// Yes - put it into retirement, but make it fade
				// fast (1 seconds)
				oldTrack = currentTrack;
				oldTrack -> BeginFadingOut(time,1.0);

				// ... and leave the new one waiting to play
				currentTrack = track;
				}
			}
		else
			{
			// There's nothing else playing, so we can just go for it
			currentTrack = track;
			currentTrack -> BeginPlaying(volume,time);
			}
		}
	}

// ----------------------------------------------------------------------
// Method:		ActivateFade
// Arguments:	None
// Returns:		Nothing
// Description:	Gradually fades out the currently playing track, leaving
//				nothing in its place
// ----------------------------------------------------------------------
void MusicManager::ActivateFade()
	{
	// Don't do anything if we're not playing anyway
	if (status != Playing)
		{
		return;
		}

	// Is there a track already fading ?
	if (oldTrack)
		{
		// Yes - make sure nothing starts when it has finished
		currentTrack = NULL;
		}
	else
		{
		// Is there a track currently playing
		if (currentTrack)
			{
			// Yes - put it into retirement
			oldTrack = currentTrack;
			oldTrack -> BeginFadingOut(time);

			// and leave nothing waiting
			currentTrack = NULL;
			}
		}

	}

// ----------------------------------------------------------------------
// Method:		SetVolume
// Arguments:	vol - new volume
// Returns:		Nothing
// Description:	Sets the overall volume of all tracks playing
// ----------------------------------------------------------------------
void MusicManager::SetVolume(MusicValue vol)
	{
	// Could make this gradually fade
	volume = vol;
	}

// ----------------------------------------------------------------------
// Method:		StartReadingTracks
// Arguments:	None
// Returns:		Nothing
// Description:	Prepares to start reading the track names
// ----------------------------------------------------------------------
void MusicManager::StartReadingTracks()
	{
	// Point to the start of the list
	currentReading = 0;
	}

// ----------------------------------------------------------------------
// Method:		GetNextTrack
// Arguments:	name - To hold name of track
// Returns:		true if there was still a track to read
// Description:	Reads the next track from the list
// ----------------------------------------------------------------------
bool MusicManager::GetNextTrack(std::string& name)
	{
	// Don't bother if it's an invalid call, or we've reached the end
	if (currentReading <0 || currentReading >= tracks.size() )
		{
		return false;
		}

	// Succesfully return the name of the track and advance down the list
	name = tracks[currentReading++]->GetName();
	return true;
	}

// ----------------------------------------------------------------------
// Method:		GetCurrentTrackName
// Arguments:	None
// Returns:		Name of active track, or NULL if none is selected
// ----------------------------------------------------------------------
LPCTSTR MusicManager::GetCurrentTrackName()
	{
	if (currentTrack)
		{
		return currentTrack -> GetName();
		}
	else
		{
		return NULL;
		}
	}


#include "../General.h"

// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool MusicManager::Write(CreaturesArchive &archive) const
	{
	// Verision info:
	archive << (DWORD) 1;

	// Store the name of the currently active track (if any)
	std::string trackName;
	if (currentTrack)
		{
		trackName = currentTrack -> GetName();
		}
	else
		{
		// Nothing to play
		trackName = "Silence";
		}

    archive << trackName;

	// Only store basic state, otherwise we'd start off with tracks
	// still fading, or being halfway through playing
	archive << volume << targetMood << targetThreat;
	
	return true;
	}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool MusicManager::Read(CreaturesArchive &archive)
	{
	// Check version info
	DWORD version;
	archive >> version;
	if ( version == 1 )
		{
		// Read the name of the active track (if any) when stored
		std::string trackName;
        archive >> trackName;

		// Now read in the remaining basic state
		archive >> volume >> targetMood >> targetThreat;

		// Now start the last track playing, fading the previous track
		// out quickly
		InteruptTrack(trackName.data());

		return true;
		}
	else
		{
		return false;
		}

	}
