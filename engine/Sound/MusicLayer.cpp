// --------------------------------------------------------------------------
// Filename:	Music Layer.cpp
// Class:		MusicLayer
// Purpose:		Base class for a layer within a soundtrack
//
// Description:
//
// A layer represents an individual "channel" within a soundtrack.  It
// is repeatedly interrogated by the soundtrack, and will, when appropriate,
// return the wave to be played, volume it should be played at, and the
// effect it should be played through.
//
// Each layer can optionally have an action attached which runs before
// anything else.  This can set any variables associated with the layer,
// including 'volume', and can read variables attached to the track
// itself, and to other layers
//
// The complete library of available waves and effects are stored as static
// members of MusicLayer.  A reference count of layers is kept, and when
// this reaches zero, the waves and effects are deleted
//
// History:
// 3Apr98	PeterC	Created
// 19Jun98	PeterC	Forced volume and panning to sensible bounds in
//					ConvertVolume() and ConvertPan()
// 28Jul98	PeterC	Changed pSound to musicSoundManager
// 06Aug98	PeterC	Removed ASSERT() from ReadStage in PlaySound()
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicLayer.h"
#include "MusicWave.h"
#include "MusicEffect.h"
#include "MusicScript.h"
#include "MusicAction.h"
#include "MusicGlobals.h"
#include "Soundlib.h"
#include "../Display/ErrorMessageHandler.h"
#include "../File.h"
#include "../App.h"

#include "../Scramble.h"

#include "../Map/Map.h" // For Map::FastFloatToInteger

#include <limits.h>
#include <list>


// ----------------------------------------------------------------------
// Effects
// static array of effects used within layers
// **** could have a much more efficient memory system here
// ----------------------------------------------------------------------
std::vector<MusicEffect *> MusicLayer::effects;
		

// ----------------------------------------------------------------------
// Waves
// static array of waves used within layers
// **** could have a much more efficient memory system here
// ----------------------------------------------------------------------
std::vector<MusicWave *> MusicLayer::waves;


// ----------------------------------------------------------------------
// totalLayers
// static reference count of existing expressions, maintained by the
// constructor and destructor.  When this returns to zero, the list of
// constants will be deleted
// ----------------------------------------------------------------------
int MusicLayer::totalLayers = 0;

// ----------------------------------------------------------------------
// Method:		MusicLayer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicLayer::MusicLayer() :
	volume(1.0),
	initialVolume(1.0)
	{
	// Keep a reference count
	totalLayers++;
	}

// ----------------------------------------------------------------------
// Method:		~MusicLayer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicLayer::~MusicLayer()
	{
	// Get rid of any remaining sounds
	StopPlaying();

	// Decrease the reference count
	totalLayers--;

	// Shouldn't be able to delete more than we've created
	ASSERT(totalLayers>=0);

	// When all the layers have been deleted, delete the libraries of
	// waves and expressions
	if (totalLayers == 0)
		{
		// Yes - clear out the store of effects
		effectsIterator it;
		for (it = effects.begin(); it != effects.end(); it++ )
			{
			// First delete the allocated space
			delete(*it);
			}

		// Now clear out the entries
		effects.clear();

		// Yes - clear out the store of effects
		wavesIterator wave;
		for (wave = waves.begin(); wave != waves.end(); wave++ )
			{
			// First delete the allocated space
			delete (*wave);
			}

		// Now clear out the entries
		waves.clear();

		}
	}

// ----------------------------------------------------------------------
// Method:		PreParse
// Arguments:	script - script, pointing to the start of the layer
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Allocates the name and variables for the layer, allowing
//				other layers to refer to it
// ----------------------------------------------------------------------
MusicError MusicLayer::PreParse(MusicScript &script)
	{
	// First of all, we have an indication of the type of layer
	// (LoopLayer, AleotoricLayer etc.)  Skip past this
	script.Advance();

	// Next, an open bracket
	if (script.GetCurrentType() != MusicScript::StartArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Now the name should follow
	if (script.GetCurrentType() != MusicScript::String)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	SetName( script.GetCurrentToken() );

	script.Advance();

	// Now, a closing bracket
	if (script.GetCurrentType() != MusicScript::EndArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();
	
	// Then an opening brace, and we're inside the descriptors' section
	if (script.GetCurrentType() != MusicScript::StartSection)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Keep looking through this section, until we find the matching
	// closing brace, picking out any variables as we meet them

	// We're going to add one each time we find a '{', and subtract
	// one each time we find a '}'.  When indents reaches zero, we've
	// found our way back out
	int indents = 1;
	while (script.GetCurrentType() != MusicScript::EndOfFile)
		{
		switch (script.GetCurrentType())
			{
			case (MusicScript::StartSection):
				{
				// Indent by one more
				indents++;
				script.Advance();
				break;
				}
			case (MusicScript::EndSection):
				{
				// Reduce indents
				indents--;
				script.Advance();

				// Have we finished the layer?
				if (indents == 0)
					{
					// Yes - We've finished parsing the layer
					return MUSIC_OK;
					}
				break;
				}
			case (MusicScript::String):
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
					// Not a variable declaration - skip past it
					script.Advance();
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

	// If we got this far, no closing brace was found before the
	// end of the file
	return MUSIC_SYNTAX_ERROR;
	}

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script - script, pointing to the start of the layer
//				manager - music manager owning this layer
//				track  - track owning layer
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to Parse the layer from the script
// ----------------------------------------------------------------------
MusicError MusicLayer::Parse(MusicScript &script,
				   MusicManager &manager,
				   MusicTrack &track)
	{
	// Empty virtual class
	return (MUSIC_OK);
	}

// ----------------------------------------------------------------------
// Method:		Update
// Arguments:	uberVolume  - overall soundtrack volume
//				time		- current time (seconds elapsed since
//							  start of track)
//				beat		- current length of a beat in seconds
//				beatCount   - total beats elapsed
// Returns:		Nothing
// Description:	Updates the layer, taking into account overall volume
// ----------------------------------------------------------------------
void MusicLayer::Update(MusicValue uberVolume,
							  MusicValue time,
							  MusicValue beat,
							  MusicValue beatCount)
	{
	// Handle the low level update - this will reset the counter
	// to the next update

	MusicUpdatable::Update(time);

	// Stop / update any controlled sounds

	// Move backwards through the list, to avoid problems when
	// members are deleted
	int i = controlled.size() -1;

	controlledIterator it;
	it = controlled.begin();

	std::list<ControlledSound *> toBeDeleted;


	for (it = controlled.begin(); it !=controlled.end(); it++)
		{
		ControlledSound *sound= *(it);
		if ( sound -> IsFinished() )
			{
			// Then delete it - this will tell the sound manager to stop it
			toBeDeleted.push_front(sound);
			}
		else
			{
			// Update the sound with the layer volume
			MusicValue vol = sound -> GetVolume() * volume * uberVolume;

			if (theMusicSoundManager) {
				theMusicSoundManager -> UpdateControlledSound( sound -> GetHandle(),
															ConvertVolume(vol),
															ConvertPan(sound -> GetPan() ) );
			}

			}
		}
    
		
	std::list<ControlledSound *>::iterator deleted; 

	for(deleted = toBeDeleted.begin(); deleted != toBeDeleted.end(); deleted++)
	{
		for(it = controlled.begin(); it != controlled.end();it++)
		{
			if(*it == *deleted)
			{
				controlled.erase(it);
				break;
			}
		}
		delete (*deleted);
	}

	toBeDeleted.clear();

	// Parse through the sound queue, playing any sounds that have
	// waited long enough

	// Move backwards through the queue, to avoid problems when
	// members are deleted

	// **** This could be significantly improved if speed turns out to be
	// **** a problem

	std::list<QueuedSound *> queuesToBeDeleted;
	queueIterator queueit = queue.begin();

	 i = 0;
	for (queueit = queue.begin(); queueit != queue.end();queueit++)
		{
		QueuedSound *sound= (*queueit);

		// Convert the wave ID into a token by simply adding 0xff000000
		DWORD nameToken = 0xff000000 + (DWORD) sound->GetWave();

		if (sound -> IsReady(time) )
			{
			// Play it, taking layer and track volume into account
			SoundHandle handle;
		
			if (theMusicSoundManager) {
				theMusicSoundManager -> StartControlledSound(nameToken,
											   handle,
											   ConvertVolume(uberVolume * volume*sound->GetVolume()),
											   ConvertPan(sound->GetPan()));
			}

			// Add it to the list of controlled sounds
			AddControlledSound(handle, sound->GetVolume(), sound->GetPan() );

			// Now remove it from the list
		
			queuesToBeDeleted.push_front(sound);
			}

		// Continue down the list
		i--;
		}

	std::list<QueuedSound *>::iterator deletedqueue; 

	for(deletedqueue = queuesToBeDeleted.begin(); 
	deletedqueue != queuesToBeDeleted.end(); deletedqueue++)
	{
		for(queueit = queue.begin(); queueit != queue.end();queueit++)
		{
			if(*queueit == *deletedqueue)
			{
				queue.erase(queueit);
				break;
			}
		}
		delete (*deletedqueue);
	}

	queuesToBeDeleted.clear();

	}

// ----------------------------------------------------------------------
// Method:		BeginPlaying
// Arguments:	uberVolume  - overall music volume
//				time		- current time (in seconds)
// Returns:		Nothing
// Description:	Reset state, and commence playing
// ----------------------------------------------------------------------
void MusicLayer::BeginPlaying(MusicValue uberVolume, MusicValue time)
	{
	// Reset all the variables
	Reset();

	// Run the initialisation script, if defined
	Initialise();

	// Reset the timer
	UpdateImmediately(time);

	// And restore the volume to its original setting
	volume = initialVolume;
	}

// ----------------------------------------------------------------------
// Method:		GetVariable
// Arguments:	varName - name of the variable
//				lvalue  - true if value can be written to
// Returns:		Pointer to variable, or NULL if not defined
// Description:	Searches through list of named / pre-defined variables
// ----------------------------------------------------------------------
MusicValue *MusicLayer::GetVariable(LPCTSTR varName, bool lvalue)
	{
	// Volume is the only pre-defined value here - check for it
	if (strcmp(varName,"Volume")==0)
		{
		return &volume;
		}

	// Now see if the name matches any of the user-defined variables
	return (MusicVariableContainer::GetVariable(varName,lvalue));
	}

// ----------------------------------------------------------------------
// Method:		StopPlaying
// Arguments:	None
// Returns:		Nothing
// Description:	Stops any outstanding sounds / loops
// ----------------------------------------------------------------------
void MusicLayer::StopPlaying()
	{
	// Dispose of any sounds still in the queue
	queueIterator it;
	for (it = queue.begin(); it != queue.end(); it++ )
	{
	// First delete the allocated space
	delete (*it);
	}

	// Now clear out the entries
	queue.clear();

	// Stop the remaining controlled sounds (by deleting them)
	controlledIterator cont;

	for (cont = controlled.begin(); cont != controlled.end(); cont++  )
		{
		// First delete the allocated space
		delete (*cont);
		}

	// Now clear out the entries
	controlled.clear();
	}

// ----------------------------------------------------------------------
// Static member functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Method:		ParseEffect
// Arguments:	script - script, pointing to the start of the layer
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Adds an effect to the shared library
// ----------------------------------------------------------------------
MusicError MusicLayer::ParseEffect(MusicScript &script)
	{
	// Create a new effect to be added to the static effect library
	MusicEffect *effect = new MusicEffect;

	// Attempt to parse it, leaving the script pointing to the
	// next item
	MusicError error = effect->Parse(script);

	// Was there an error ?
	if (error != MUSIC_OK)
		{
		// Yes - get rid of the monster we've created
		delete effect;
		}
	else
		{
		// Add the new effect to the static library
		effects.push_back(effect);
		}

	return error;
	}

// ----------------------------------------------------------------------
// Method:		GetWaveID
// Arguments:	name - name of the wave
// Returns:		ID of the effect or MUSIC_NO_WAVE
// Description:	Attempts to locates a wave within the wave library.  If
//				The wave has not already been encountered, it will be
//				added to the library
//
//				**** Note that this does not presently verify that the
//				**** wave file actually exists
// ----------------------------------------------------------------------
MusicWaveID MusicLayer::GetWaveID(LPCTSTR name)
	{
	// Iterate through the library, until a wave with the matching
	// name is found
	int size = waves.size();
	wavesIterator it;
	int i = 0;
	for (it = waves.begin();it != waves.end();it++, i++)
		{
		if ( (*it)->MatchName(name) )
			{
			// Found it!
			return ( MusicWaveID (i) );
			}
		}

	// The wave was not found - create a new wave and add it to the end 
	// of the list
	MusicWave *newWave = new MusicWave(name);
	waves.push_back(newWave);

	// The new wave will be the last thing in the list and thus its
	// ID will be equal to size

	return (MusicWaveID) size;

	}


// ----------------------------------------------------------------------
// Method:		GetEffectID
// Arguments:	name - name of the effect
// Returns:		ID of the effect or MUSIC_NO_EFFECT
// Description:	Locates an effect within the effect library
// ----------------------------------------------------------------------
MusicEffectID MusicLayer::GetEffectID(LPCTSTR name)
	{
	// Iterate through the library, until an effect with the matchin
	// name is found
	effectsIterator it;
	int i = 0;
	for (it = effects.begin();it != effects.end();it++, i++)
		{
		if ((*it)->MatchName(name) )
			{
			return ( MusicEffectID (i) );
			}
		}

	// Didn't find a matching effect
	return (MUSIC_NO_EFFECT);
	}

// ----------------------------------------------------------------------
// Method:		ConvertName
// Arguments:	name - name of wave file (four letters only)
// Returns:		Token comprising the four letters of the name
// Description:	Temporary helper function, left in until the soundmanager
//				can be updated to use full names
// ----------------------------------------------------------------------
DWORD MusicLayer::ConvertName(LPCTSTR name)
	{
	DWORD token=0;

	int i;
	for ( i=3; i>=0; i--)
		{
		token <<= 8;
		token += (DWORD) name[i];
		}

	return token;
	}


// ----------------------------------------------------------------------
// Method:		PlaySound
// Arguments:	uberVolume  - overall soundtrack volume
//				time		- current time (seconds elapsed since
//							  start of track)
//				beat		- current length of a beat in seconds
//				wave		- Id of wave to be played
//				vol			- volume of sound (0.0 to 1.0)
//				effect		- Id of effect from shared library
// Returns:		Nothing
// Description:	Plays a sound through a named effect
// ----------------------------------------------------------------------
void MusicLayer::PlaySound(MusicValue uberVolume,
						   MusicValue time,
						   MusicValue beat,
						   MusicWaveID wave,
						   MusicValue vol,
						   MusicEffectID effect)
	{
	// First check there's actually a wave to play
	if (wave==MUSIC_NO_WAVE)
		{
		// Nope!
		return;
		}


	// Convert the wave ID into a token by simply adding 0xff000000
	DWORD nameToken = 0xff000000 + (DWORD) wave;

	// Was there any effect?
	if (effect==MUSIC_NO_EFFECT)
		{
		// Just play it at with no delay and no panning,
		// scaled by the layer volume, and the track volume
		//
		SoundHandle handle;
		
		if (theMusicSoundManager) {
			theMusicSoundManager -> StartControlledSound(nameToken,
										   handle,
										   ConvertVolume(uberVolume * volume),
										   0);
		}

		// Add the sound to the list of controlled sounds
		// At full volume (1.0) and panned to the centre (0.0)
		AddControlledSound(handle, 1.0, 0.0);
		}
	else
		{
		MusicEffect *playEffect = effects[effect];

		ASSERT( playEffect );

		if (playEffect -> BeginReadingStages( beat ) )
			{
			MusicValue vol, pan, delay;

			// Read the details of the first sound
			playEffect -> ReadStage( vol, pan, delay);	

			// Play this first sound immediately,
			// scaling the volume by the layer volume, and track volume
			SoundHandle handle;
			
			if (theMusicSoundManager) {
				theMusicSoundManager -> StartControlledSound(nameToken,
											   handle,
											   ConvertVolume(uberVolume*volume*vol),
											   ConvertPan(pan));
			}

			// Store this sound on the list of controlled sound
			AddControlledSound(handle, vol, pan);

			// Delays are cumulative - store the time at which the next sound
			// should be played
			MusicValue currentDelay = delay + time;

			// Now place any remaining stages on the effect queue
			while ( playEffect -> ReadStage( vol, pan, delay) )
				{
				// The volume will be scaled when this gets played
				QueueSound( wave, vol, pan, currentDelay);

				// Accumulate the delay to the next sound
				currentDelay += delay;
				}
			}
		}
	}

// ----------------------------------------------------------------------
// Method:		StartLoop
// Arguments:	uberVolume  - overall soundtrack volume
//				wave		- Id of wave to be played
//				vol			- volume of sound (0.0 to 1.0)
//				pan			- stereo position (-1.0 to 1.0)
// Returns:		Handle to the given sound or NO_SOUND_HANDLE
// Description:	Starts a sound looping, and returns a handle to it
//				Note that no effects can be applied to the sound
// ----------------------------------------------------------------------
SoundHandle MusicLayer::LoopSound(MusicValue uberVolume,
								  MusicWaveID wave,
								  MusicValue vol,
								  MusicValue pan)
	{
	// First check there's actually a wave to play
	if (wave==MUSIC_NO_WAVE)
		{
		// Nope!
		return NO_SOUND_HANDLE;
		}

	// Convert the wave ID into a token by simply adding 0xff000000
	DWORD nameToken = 0xff000000 + (DWORD) wave;

	/*// Convert the stored wave id into a token
	MusicWave *playWave = waves[wave];

	ASSERT(playWave);

	DWORD nameToken = ConvertName( playWave -> GetName() );*/

	// Play the sound, scaling by the layer volume and the track volume
	SoundHandle handle;
	if (theMusicSoundManager) {
		if (theMusicSoundManager -> StartControlledSound( nameToken,
											handle,
	 										ConvertVolume(vol * volume * uberVolume),
											ConvertPan(pan),
											TRUE) == NO_SOUND_ERROR)
			{
			return handle;
			}
	}
	
	return NO_SOUND_HANDLE;
	}

// ----------------------------------------------------------------------
// Method:		UpdateLoop
// Arguments:	uberVolume  - overall soundtrack volume
//				handle		- handle identifying loop
//				vol			- new volume for loop
//				pan			- new position for loop
// Returns:		Nothing
// Description:	Updates the volume and stereo position for a loop
// ----------------------------------------------------------------------
void MusicLayer::UpdateLoop(MusicValue uberVolume,
							SoundHandle handle,
							MusicValue vol,
							MusicValue pan)
	{
	if (theMusicSoundManager) {
		theMusicSoundManager -> UpdateControlledSound( handle,
										 ConvertVolume(vol * volume * uberVolume),
										 ConvertPan(pan) );
	}
	}

// ----------------------------------------------------------------------
// Method:		StopLoop	
// Arguments:	handle - handle identifying loop
// Returns:		Nothing
// Description:	Stops the loop playing
// ----------------------------------------------------------------------
void MusicLayer::StopLoop(SoundHandle handle)
	{
	// Fade the sound out
	if (theMusicSoundManager) {
		theMusicSoundManager -> StopControlledSound(handle, TRUE);
	}
}

// ----------------------------------------------------------------------
// Method:		AddControlledSound
// Arguments:	soundHandle    - handle of the new sound
//				intendedVolume - volume of the sound, before the layer
//								 volume has been taken into account
//				initialPan	   - initial pan setting
// Returns:		Nothing
// Description:	Adds a new controlled sound to the list
// ----------------------------------------------------------------------
void MusicLayer::AddControlledSound(SoundHandle soundHandle,
									MusicValue intendedVolume,
									MusicValue initialPan)
	{
	// Make sure it's a valid sound we're adding
	if (soundHandle != NO_SOUND_HANDLE)
		{
		// Add the new sound to the end of the list
		controlled.push_back( new ControlledSound(soundHandle,
											intendedVolume,
											initialPan) );
		}
	}

// ----------------------------------------------------------------------
// Method:		QueueSound
// Arguments:	wave  - id of wave to be played
//				vol   - volume of sound (unscaled)
//				pan   - panning of sound
//				delay - time, in seconds, when sound should play
// Returns:		Nothing
// Description:	Places the sound on a queue, to be played later
// ----------------------------------------------------------------------
void MusicLayer::QueueSound(MusicWaveID wave,
							MusicValue vol,
							MusicValue pan,
							MusicValue delay)
	{
	// Add the sound to the end of the array
	// **** not the most efficient way, but it will do for now
	QueuedSound* queuedSound =  new QueuedSound(wave,vol,pan,delay);
	queue.push_back(queuedSound);
	}

// ----------------------------------------------------------------------
// Method:		Munge
// Arguments:	fileName - name of file to contained munged waves
//				script   - script describing all tracks
// Returns:		Nothing
// Description:	Munges all the wave files together into one big file,
//				including a header
//
// File format:
//	Integer 0 - Number of waves
//			1 - Offset to scrambled script
//			2 - Size of script
//			3,4 - Offset to first wave, followed by length
//			5,6 - Offset to second wave, followed by length
//		    ..  - etc.
//			First Wave (missing first 16 bytes)
//			etc.
// ----------------------------------------------------------------------
void MusicLayer::Munge(LPCTSTR name, LPCTSTR script)
	{
	// Create a scrambled version of the script 
	int scriptLength = strlen(script);
	char *scrambled = new char[scriptLength];
	strncpy(scrambled,script,scriptLength);
	Scramble(scrambled,scriptLength);

	File munged(name);

	int size = waves.size();

	// First write out the total number of waves
	munged.Write(&size,sizeof(int));


	// Calculate the position for the first byte after the header
	// ( this allows an integer for the total number of waves,
	// two integers for the scripts, and two for each wave

	int currentStart = (1 + (size + 1) * 2) * sizeof(int);

	// First write the offset to, and length of the script
	munged.Write(&currentStart,sizeof(int));
	munged.Write(&scriptLength,sizeof(int));

	// Point to the next position after the script
	// (i.e. that of the first wave)
	currentStart += scriptLength;

	// Now write out the offset and length of
	// each wave in turn
	int i;
	for ( i=0;i<size;i++)
	{
		munged.Write(&currentStart,sizeof(int));

		// Create the full pathname
		char buf[_MAX_PATH];
		theApp.GetDirectory(SOUNDS_DIR,buf);
		std::string mungeName(buf);

		mungeName += (*(waves.begin() + i))->GetName();
		mungeName += ".wav";

		// Find the size of the to-be-munged file
		File mungette;
	try
		{
			// We going to strip out the first sixteen bytes
			// of the file, to hassle hackers.
			int fileSize = mungette.GetSize() - 16;
			munged.Write(&fileSize,sizeof(int));

			// Leave this pointing just after this file
			currentStart += fileSize;
		}
		catch(File::FileException&)
		{
			ErrorMessageHandler::Show("sound_error", 1, "MidiModule::Munge", mungeName.c_str());
	
			// Get rid of the scrambled version of the script
			delete scrambled;
			return;
		}
	}

	// Now we've written the header, copy the scrambled script
	munged.Write(scrambled, scriptLength);

	// Then copy each of the files in turn
	for (i=0;i<size;i++)
	{
		// Create the full pathname
		// Create the full pathname
		char buf[_MAX_PATH];
		theApp.GetDirectory(SOUNDS_DIR,buf);
		std::string mungeName(buf);

		mungeName += (*(waves.begin() + i))->GetName();
		mungeName += ".wav";

		try
		{
			File mungette;
	
			// We going to strip out the first sixteen bytes
			// of the file, to hassle hackers.

			int fileSize = mungette.GetSize() - 16;
			mungette.Seek(16,File::Start);

			// Copy the entire file into memory
			char *data = new char[fileSize];
			mungette.Read(data,fileSize);
			munged.Write(data,fileSize);
			delete data;
			
	
		}
		catch(File::FileException&)
		{		
			ErrorMessageHandler::Show("sound_error", 1, "MidiModule::Munge", mungeName.c_str());

			// Get rid of the scrambled version of the script
			delete scrambled;

		}
		
	}

	// Get rid of the scrambled version of the script
	delete scrambled;

	}

// ----------------------------------------------------------------------
// ControlledSound methods
// ----------------------------------------------------------------------


// ----------------------------------------------------------------------
// Method:		ControlledSound
// Arguments:	soundHandle    - handle of the new sound
//				intendedVolume - volume of the sound, before the layer
//								 volume has been taken into account
//				initialPan	   - initial pan setting
// Returns:		Nothing
// Description:	Constructor
// ----------------------------------------------------------------------
MusicLayer::ControlledSound::ControlledSound(SoundHandle soundHandle,
				MusicValue intendedVolume,
				MusicValue initialPan)
				: handle(soundHandle),
				  volume(intendedVolume),
				  pan(initialPan)
	{
	}

// ----------------------------------------------------------------------
// Method:		~ControlledSound
// Arguments:	None
// Returns:		Nothing
// Description:	Destructor - lets go of the sound
// ----------------------------------------------------------------------
MusicLayer::ControlledSound::~ControlledSound()
	{
	if (theMusicSoundManager) {
		theMusicSoundManager -> StopControlledSound(handle);
	}
}

// ----------------------------------------------------------------------
// Method:		ConvertVolume
// Arguments:	vol - volume ranging from 0.0 to 1.0 
// Returns:		Volume in SoundManager scale (ranges from -10000 to 0)
// ----------------------------------------------------------------------
long MusicLayer::ConvertVolume(MusicValue vol)
	{
	long converted = SoundMinVolume - Map::FastFloatToInteger((sqrt(vol) * (MusicValue) SoundMinVolume));

	// Fix the volume to be within sensible bounds
	if (converted < SoundMinVolume)
		{
		converted = SoundMinVolume;
		}
	else
		{
		if (converted > 0)
			{
			converted = 0;
			}
		}

	return converted;
	
	}

// ----------------------------------------------------------------------
// Method:		ConvertPan
// Arguments:	pan - panning, ranging from -1.0 to 1.0 
// Returns:		Volume in SoundManager scale (ranges from -10000 to 10000)
// ----------------------------------------------------------------------
long MusicLayer::ConvertPan(MusicValue pan)
{
	long converted = Map::FastFloatToInteger(pan * 10000.0);

	// Fix the panning to be within sensible bounds
	if (converted < -10000)
	{
		converted = -10000;
	}
	else
	{
		if (converted > 10000)
		{
			converted = 10000;
		}
	}
	return converted;
}
