#ifndef STUB_SOUNDLIB_H
#define STUB_SOUNDLIB_H

// Stub version of soundlib
// NOTE: SERIALISATION WON'T BE COMPATIBLE WITH NON-STUB VERSIONS!

#include "../../PersistentObject.h"

#define MAX_ACTIVE_SOUNDS	32

typedef enum {NO_SOUND_ERROR=0, SOUNDCACHE_UNINITIALIESED,
			  SOUNDCACHE_TOO_SMALL,  SOUND_NOT_FOUND,
			  SOUND_HANDLE_UNDEFINED, SOUND_CHANNELS_FULL,
			  SOUND_MIXER_SUSPENDED, SOUND_MIXER_CANT_OPEN} SOUNDERROR;

typedef int SOUNDHANDLE;

// Volumes range from -5000 (silence) to 0 (full volume)
const int SoundMinVolume = -5000;


// SoundManager - front end class for all sound routines
//
//
// The Sound Manager is a means of manipulating a sound cache.
// The user can call sounds to be played, leaving the
// manager to update the cache by cleaning out the oldest
// unused sounds.
//
// Sounds can be of two types - effects, or controlled sounds
//
// Controlled sounds can be looped or played  once only, and
// can be adjusted (panning, volume) while playing.  They
// may also be stopped at any point.
//
// Effects play once only, and cannot be adjusted once started.
// Additionally, effects may be delayed by a number of ticks.
// Delayed sounds are placed on a queue maintained by the manager.


class SoundManager :  public PersistentObject
{
	CREATURES_DECLARE_SERIAL( SoundManager )
public:
	//////////////////////////////////////////////////////////////////////////
	// Exceptions
	//////////////////////////////////////////////////////////////////////////
	class SoundException: public BasicException
	{
	public:
		SoundException(std::string what, uint16 line):
		BasicException(what.c_str()),
		lineNumber(line){;}

		uint16 LineNumber(){return lineNumber;}
	private:
		uint16 lineNumber;
	};

	enum SIDText
	{
		sidFileNotFound=0,
		sidNotAllFilesCouldBeMunged,
		sidUnknown,
		sidResourcesAlreadyInUse,
		sidWaveFormatNotSupported,
		sidInvalidParameter,
		sidNoAggregation,
		sidNotEnoughMemory,
		sidFailedToCreateDirectSoundObject,
		sidFailedToSetCooperativeLevel,
		sidPrimaryBufferNotCreated,
		sidPrimaryBufferCouldNotBeSetToNewFormat,
	};



// functions
public:
	SoundManager();
	~SoundManager();

	void StopAllSounds();
	void Update();					// Called by on timer

	BOOL SoundEnabled();					//  Is mixer running?

	SOUNDERROR InitializeCache(int size);	//	Set size (K) of cache in bytes
											//  Flushes the existing cache

	SOUNDERROR FlushCache();				//  Clears all stored sounds from
											//  The sound cache

	SOUNDERROR SetVolume(long volume);		//  Sets the overall sound volume

	SOUNDERROR FadeOut();					//  Begin to fade all sounds (but
											//  leaves them "playing silently"
	
	SOUNDERROR FadeIn();					//  Fade all sounds back in

	SOUNDERROR SuspendMixer();				//  Stop the mixer playing
											//  (Use on KillFocus)

	SOUNDERROR RestoreMixer();				//  Restart the mixer
											//  After it has been suspended

	SOUNDERROR PreLoadSound(DWORD wave);	//  Ensures a sound is loaded into the
											//  cache

	SOUNDERROR PlaySoundEffect(DWORD wave, int ticks=0, long volume=0, long pan=0);
											//  Load and Play sound immediately or
											//  preload sound and queue it to be played
											//  after 'ticks' has elapsed

	SOUNDERROR StartControlledSound(DWORD wave, SOUNDHANDLE &handle, long volume=0, long pan=0, BOOL looped=FALSE);
											//  Begins a controlled sound and returns its handle

	SOUNDERROR UpdateControlledSound(SOUNDHANDLE handle, long volume, long pan);
	
	SOUNDERROR StopControlledSound(SOUNDHANDLE handle, BOOL fade=FALSE);
											//  Stops the specified sound (handle is
											//  then no longer valid) Sound can be
											//  optionally faded out

	BOOL FinishedControlledSound(SOUNDHANDLE handle);
											//  Has the selected sound finished playing?

	bool PlayMidiFile(std::string& fileName);

	void StopMidiPlayer();

	void SetVolumeOnMidiPlayer(int32 volume);

	void MuteMidiPlayer(bool mute);

	void SetMNGFile(std::string& mng);

	bool IsMixerFaded();

private:

	virtual bool Write( CreaturesArchive& archive ) const;
	virtual bool Read( CreaturesArchive& archive );

};

#endif

