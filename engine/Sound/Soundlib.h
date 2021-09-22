#ifndef _WIN32
#include "stub/stub_Soundlib.h"
#else
// directsound version follows:


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#ifndef _SOUNDMAN_H
#define _SOUNDMAN_H

#include <mmreg.h>
#include <dsound.h>

#include <set>
#include "../PersistentObject.h"


#define MAX_ACTIVE_SOUNDS	32

typedef enum {NO_SOUND_ERROR=0, SOUNDCACHE_UNINITIALIESED,
			  SOUNDCACHE_TOO_SMALL,  SOUND_NOT_FOUND,
			  SOUND_HANDLE_UNDEFINED, SOUND_CHANNELS_FULL,
			  SOUND_MIXER_SUSPENDED, SOUND_MIXER_CANT_OPEN} SOUNDERROR;

typedef int SOUNDHANDLE;

//typedef std::vector<SoundQueueItem*>::iterator SOUNDQUEUEITEM_ITERATOR; 
//typedef std::vector<CachedSound *>::iterator CACHEDSOUND_ITERATOR; 

// Volumes range from -5000 (silence) to 0 (full volume)
const int SoundMinVolume = -5000;

// CachedSound - Data structure storing members of sound cache
class CachedSound : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( CachedSound )
public:
	DWORD name;						// Four letter name tag
	int used;						// Number indicating recent use
	DWORD size;						// Size of sample in bytes
	int copies;						// Number of active copies
	IDirectSoundBuffer *buffer;		// Store of sound data

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

	CachedSound();
	~CachedSound();

};

// Active Sample - Data structure storing samples currently
// being played by the Sound Manager.
// Controlled Sounds lock their samples, so that they are not
// deleted without the 'knowledge' of the object that created
// them.
// ActiveSamples can be faded using 'fade_rate.'  They will
// automatically be deleted once silent
class ActiveSample
{
public:
	IDirectSoundBuffer	*pSample;	//	pointer to direct sound buffer
	DWORD				wID;		//	id of sample
	BOOL				locked;		//  Do not delete when finished
	long				fade_rate;	//	amount added to volume each tick
	CachedSound			*cloned;	//	pointer to cache entry
									//	that this has been copied from
									//	NULL = original buffer
	long				volume;		//  volume sound should play at before
									//  overall manager volume is applied

	ActiveSample();
};

// A SoundQueue is maintained to allow a delay before a sound
// is played.  This enables compound sounds to be created from
// several separate components.
class SoundQueueItem : public PersistentObject
{
		CREATURES_DECLARE_SERIAL( SoundQueueItem )
public:
	DWORD		wave;			// Name of sound to be played
	int			ticks;			// Number of ticks remaining
	long		volume;
	long		pan;
};

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

// to play midi sounds
class MidiModule;

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

	bool IsMixerFaded() { return faded; }

private:
	SOUNDHANDLE PlayCachedSound(CachedSound *wave, int volume=0,
							int pan=0, BOOL loop=FALSE);
	// returns channel
	CachedSound *OpenSound(DWORD wave);			// Loads sound data and creates a buffer

	void CloseSound(CachedSound *wave);			// Deselects buffer and deletes
	
	void StopSound(SOUNDHANDLE handle);			// Release sound from DSObject if necessary

	std::string mungeFile;

	int GetWaveSize(DWORD wave);

	CachedSound*	EnsureInCache(DWORD wave);		//  Places wave in cache and returns
												//	a pointer to it (if present)

	SOUNDERROR MakeRoomInCache(int size);		//	Ensure that 'size' bytes are free

	SOUNDERROR RemoveFromCache(CachedSound* index);		//  Remove Wave at 'index' from array

	void FlushSoundQueue();						//	Remove all sounds from sound queue

	void UpdateSoundQueue();					//  Called every tick to advance sounds
												//  up the queue and delete 'used' entries
	SOUNDERROR SetTargetVolume(long volume);	//  Sets the manager to fade towards the
												//  given volume

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

	// members
	MidiModule* myMidiModule;
			
	BOOL	mixer_suspended;					//  Mixer currently out of operation

	int 	maximum_size;						//  Size of cache in bytes
	int		current_size;						//  Current size of cache used

	int		last_used;

	std::set<CachedSound *>	sounds;			//  Flexible array of sounds in cache

	// Share the DirectSound object and primary buffers between different sound managers
	static int					references;
	static LPDIRECTSOUND		pDSObject;
	static IDirectSoundBuffer	*pPrimary;	//  pointer to primary buffer

	long				sounds_playing;
	ActiveSample		active_sounds[MAX_ACTIVE_SOUNDS];
	DWORD				sound_index;

	BOOL				sound_initialised;

	std::vector<SoundQueueItem *>			sound_queue;

	long				overall_volume;			//  Overall (unfaded) volume

	long				target_volume;			//  Volume being aimed for

	long				current_volume;			//  Volume currently playing

	bool				faded;					//  Flags if the system is
												//  not audible (or fading out)
};

#endif

#endif // _WIN32
