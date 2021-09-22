// --------------------------------------------------------------------------
// Filename:	Music Layer.h
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
// members of MusicLayer
//
// History:
// 3Apr98	PeterC	Created
// 07May98	PeterC  Prevented Update requesting next update time
//					Added list of controlled sounds (allows proper fading)
// 29May98	PeterC	Taken reduced volume range into account (ConvertVolume)
// 19Jun98	PeterC	Forced volume and panning to sensible bounds in
//					ConvertVolume() and ConvertPan()
// --------------------------------------------------------------------------

#ifndef _MUSIC_LAYER_H

#define _MUSIC_LAYER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicUpdatable.h"
#include "Soundlib.h"
#include "MusicGlobals.h"


#include <math.h>
#include <vector>
#include <set>

class MusicScript;
class MusicTrack;
class MusicManager;
class MusicWave;
class MusicEffect;
class MusicAction;


class MusicLayer : public MusicUpdatable
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicLayer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicLayer();

		// ----------------------------------------------------------------------
		// Method:		~MusicLayer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		virtual ~MusicLayer();

		// ----------------------------------------------------------------------
		// Method:		PreParse
		// Arguments:	script - script, pointing to the start of the layer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Allocates the name and variables for the layer, allowing
		//				other layers to refer to it
		// ----------------------------------------------------------------------
		virtual MusicError PreParse(MusicScript &script);

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script - script, pointing to the start of the layer
		//				manager - music manager owning this layer
		//				track  - track owning layer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the layer from the script
		// ----------------------------------------------------------------------
		virtual MusicError Parse(MusicScript &script,
						   MusicManager &manager,
						   MusicTrack &track);

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
		virtual void Update(MusicValue uberVolume,
						    MusicValue time,
						    MusicValue beat,
						    MusicValue beatCount);

		// ----------------------------------------------------------------------
		// Method:		BeginPlaying
		// Arguments:	uberVolume  - overall music volume
		//				time		- current time (in seconds)
		// Returns:		Nothing
		// Description:	Reset state, and commence playing
		// ----------------------------------------------------------------------
		virtual void BeginPlaying(MusicValue uberVolume, MusicValue time);

		// ----------------------------------------------------------------------
		// Method:		StopPlaying
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Stops any outstanding sounds / loops
		// ----------------------------------------------------------------------
		virtual void StopPlaying();

		// ----------------------------------------------------------------------
		// Method:		GetVariable
		// Arguments:	varName - name of the variable
		//				lvalue  - true if value can be written to
		// Returns:		Pointer to variable, or NULL if not defined
		// Description:	Searches through list of named / pre-defined variables
		// ----------------------------------------------------------------------
		virtual MusicValue *GetVariable(LPCTSTR varName, bool lvalue = false);

		// ----------------------------------------------------------------------
		// Static member functions
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Method:		GetWaveID
		// Arguments:	name - name of the wave
		// Returns:		ID of the effect or MUSIC_NO_WAVE
		// Description:	Attempts to locates a wave within the wave library.  If
		//				The wave has not already been encountered, it will be
		//				added to the library
		// ----------------------------------------------------------------------
		static MusicWaveID GetWaveID(LPCTSTR name);
		
		// ----------------------------------------------------------------------
		// Method:		GetEffectID
		// Arguments:	name - name of the effect
		// Returns:		ID of the effect or MUSIC_NO_EFFECT
		// Description:	Locates an effect within the effect library
		// ----------------------------------------------------------------------
		static MusicEffectID GetEffectID(LPCTSTR name);

		// ----------------------------------------------------------------------
		// Method:		ParseEffect
		// Arguments:	script - script, pointing to the start of the effect
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Adds an effect to the shared library
		// ----------------------------------------------------------------------
		static MusicError ParseEffect(MusicScript &script);

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
		static void Munge(LPCTSTR name, LPCTSTR script);

	protected:

		// ----------------------------------------------------------------------
		// Method:		GetVolume
		// Arguments:	none
		// Returns:		Volume (ranges from 0.0 (silence) to 1.0 
		//				(Calculated each time UpdateLayer() is called)
		// ----------------------------------------------------------------------
		MusicValue GetVolume() const {return volume;}

		// ----------------------------------------------------------------------
		// Method:		ConvertName
		// Arguments:	name - name of wave file (four letters only)
		// Returns:		Token comprising the four letters of the name
		// Description:	Temporary helper function, left in until the soundmanager
		//				can be updated to use full names
		// ----------------------------------------------------------------------
		DWORD ConvertName(LPCTSTR name);

		// ----------------------------------------------------------------------
		// Method:		ConvertVolume
		// Arguments:	vol - volume ranging from 0.0 to 1.0 
		// Returns:		Volume in SoundManager scale (ranges from -10000 to 0)
		// ----------------------------------------------------------------------
		long ConvertVolume(MusicValue vol);

		// ----------------------------------------------------------------------
		// Method:		ConvertPan
		// Arguments:	pan - panning, ranging from -1.0 to 1.0 
		// Returns:		Volume in SoundManager scale (ranges from -10000 to 10000)
		// ----------------------------------------------------------------------
		long ConvertPan(MusicValue pan);

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
		void PlaySound(MusicValue uberVolume, MusicValue time, MusicValue beat,
					   MusicWaveID wave, MusicValue vol, MusicEffectID effect);

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
		SoundHandle LoopSound(MusicValue uberVolume,
							  MusicWaveID wave,
							  MusicValue vol,
							  MusicValue pan);

		// ----------------------------------------------------------------------
		// Method:		UpdateLoop
		// Arguments:	uberVolume  - overall soundtrack volume
		//				handle		- handle identifying loop
		//				vol			- new volume for loop
		//				pan			- new position for loop
		// Returns:		Nothing
		// Description:	Updates the volume and stereo position for a loop
		// ----------------------------------------------------------------------
		void UpdateLoop(MusicValue uberVolume,
						SoundHandle handle,
						MusicValue vol,
						MusicValue pan);

		// ----------------------------------------------------------------------
		// Method:		StopLoop	
		// Arguments:	handle - handle identifying loop
		// Returns:		Nothing
		// Description:	Stops the loop playing
		// ----------------------------------------------------------------------
		void StopLoop(SoundHandle handle);

		// ----------------------------------------------------------------------
		// Method:		AddControlledSound
		// Arguments:	soundHandle    - handle of the new sound
		//				intendedVolume - volume of the sound, before the layer
		//								 volume has been taken into account
		//				initialPan	   - initial pan setting
		// Returns:		Nothing
		// Description:	Adds a new controlled sound to the list
		// ----------------------------------------------------------------------
		void AddControlledSound(SoundHandle soundHandle,
								MusicValue intendedVolume,
								MusicValue initialPan);

	private:

		// ----------------------------------------------------------------------
		// Method:		QueueSound
		// Arguments:	wave  - id of wave to be played
		//				vol   - volume of sound (unscaled)
		//				pan   - panning of sound
		//				delay - time, in seconds, when sound should play
		// Returns:		Nothing
		// Description:	Places the sound on a queue, to be played later
		// ----------------------------------------------------------------------
		void QueueSound(MusicWaveID wave,
						MusicValue vol,
						MusicValue pan,
						MusicValue delay);

		// ----------------------------------------------------------------------
		// QueueSound
		// Helper class for queuing sounds
		// ----------------------------------------------------------------------
		class QueuedSound
			{
			public:
				// ----------------------------------------------------------------------
				// Method:		QueueSound
				// Arguments:	wave  - id of wave to be played
				//				vol   - volume of sound (unscaled)
				//				pan   - panning of sound
				//				delay - time, in seconds, when sound should play
				// Returns:		Nothing
				// Description:	Constructor
				// ----------------------------------------------------------------------
				QueuedSound(MusicWaveID wave,
							MusicValue vol,
							MusicValue pan,
							MusicValue delay)
							: id(wave), 
							  volume(vol),
							  panning(pan),
							  playTime(delay) {}
				

				// ----------------------------------------------------------------------
				// Method:		~QueueSound
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Destructor
				// ----------------------------------------------------------------------
				~QueuedSound() {}

				// ----------------------------------------------------------------------
				// Method:		IsReady
				// Arguments:	time - current time in seconds
				// Returns:		true if sound is now ready to play
				// ----------------------------------------------------------------------
				bool IsReady(MusicValue time) {return (time + musicTimerResolution)>= playTime; }

				// ----------------------------------------------------------------------
				// Method:		GetWave
				// Arguments:	None
				// Returns:		Id of wave
				// ----------------------------------------------------------------------
				MusicWaveID GetWave() {return id;}

				// ----------------------------------------------------------------------
				// Method:		GetVolume
				// Arguments:	None
				// Returns:		Unscaled volume of wave
				// ----------------------------------------------------------------------
				MusicValue GetVolume() {return volume;}

				// ----------------------------------------------------------------------
				// Method:		GetPan
				// Arguments:	None
				// Returns:		Stereo position of wave
				// ----------------------------------------------------------------------
				MusicValue GetPan() {return panning;}

				// ----------------------------------------------------------------------
				// Method:		GetPlayTime
				// Arguments:	None
				// Returns:		Time at which sound should be triggered
				// ----------------------------------------------------------------------
				MusicValue GetPlayTime() {return playTime;}

			private:

				// ----------------------------------------------------------------------
				// Attributes
				// ----------------------------------------------------------------------

				MusicWaveID id;
				MusicValue volume;
				MusicValue panning;
				MusicValue playTime;

			};

		// ----------------------------------------------------------------------
		// ControlledSound
		// Helper class for maintaining sounds after they have been started
		// ----------------------------------------------------------------------
		class ControlledSound
			{
			public:
				// ----------------------------------------------------------------------
				// Method:		ControlledSound
				// Arguments:	soundHandle    - handle of the new sound
				//				intendedVolume - volume of the sound, before the layer
				//								 volume has been taken into account
				//				initialPan	   - initial pan setting
				// Returns:		Nothing
				// Description:	Constructor
				// ----------------------------------------------------------------------
				ControlledSound(SoundHandle soundHandle,
								MusicValue intendedVolume,
								MusicValue initialPan);

				// ----------------------------------------------------------------------
				// Method:		~ControlledSound
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Destructor - lets go of the sound
				// ----------------------------------------------------------------------
				~ControlledSound();

				// ----------------------------------------------------------------------
				// Method:		IsFinished
				// Arguments:	None
				// Returns:		TRUE if sound has finished
				// ----------------------------------------------------------------------
				BOOL IsFinished()
					{return theMusicSoundManager ? 
						theMusicSoundManager -> FinishedControlledSound(handle) : TRUE;
				}

				// ----------------------------------------------------------------------
				// Method:		GetHandle
				// Arguments:	None
				// Returns:		handle to stored sound
				// ----------------------------------------------------------------------
				SoundHandle GetHandle() {return handle;}

				// ----------------------------------------------------------------------
				// Method:		GetPan
				// Arguments:	None
				// Returns:		position of sound
				// ----------------------------------------------------------------------
				MusicValue GetPan() {return pan;}

				// ----------------------------------------------------------------------
				// Method:		GetVolume
				// Arguments:	None
				// Returns:		Volume of the sound, before the layer volume has been 
				//				taken into account
				// ----------------------------------------------------------------------
				MusicValue GetVolume() {return volume;}


			private:

				// ----------------------------------------------------------------------
				// Member attributes
				// ----------------------------------------------------------------------

				// ----------------------------------------------------------------------
				// handle
				// Individual handle to sound's representation in manager
				// ----------------------------------------------------------------------
				SoundHandle handle;

				// ----------------------------------------------------------------------
				// volume
				// Intended volume of the sample, before being scaled by the layer
				// ----------------------------------------------------------------------
				MusicValue volume;

				// ----------------------------------------------------------------------
				// pan
				// pan setting at which this sample was started
				// ----------------------------------------------------------------------
				MusicValue pan;

			};


		// ----------------------------------------------------------------------
		// Member attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Queue of sounds awaiting playing
		// ----------------------------------------------------------------------
		std::vector<QueuedSound *> queue;
		typedef std::vector<QueuedSound *>::iterator queueIterator;

		// ----------------------------------------------------------------------
		// Sounds currently playing
		// ----------------------------------------------------------------------
		std::vector<ControlledSound*> controlled;
		typedef std::vector<ControlledSound*>::iterator controlledIterator;

	//	typedef std::set<ControlledSound *>::reverse_iterator controlledReverseIterator;

	protected:
		// ----------------------------------------------------------------------
		// Protected attributes
		// Easier to expose these to derived layers
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// currentVolume
		// Volume, ranging from 0.0 to 1.0.  This can be accessed within a
		// layer's action script, as 'volume'
		// ----------------------------------------------------------------------
		MusicValue volume;

		// ----------------------------------------------------------------------
		// initialVolume
		// Initial value is stored to enable restarting of soundtrack
		// ----------------------------------------------------------------------
		MusicValue initialVolume;

	private:

		// ----------------------------------------------------------------------
		// Static attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Array of available waves
		// ----------------------------------------------------------------------

		static std::vector<MusicWave *> waves;
		typedef std::vector<MusicWave *>::iterator wavesIterator;

		// ----------------------------------------------------------------------
		// Array of named effects
		// ----------------------------------------------------------------------
		static std::vector<MusicEffect *> effects;
		typedef std::vector<MusicEffect *>::iterator effectsIterator;

		// ----------------------------------------------------------------------
		// totalLayers
		// static reference count of existing layers, maintained by the
		// constructor and destructor.  When this returns to zero, the lists of
		// waves and effects will be deleted
		// ----------------------------------------------------------------------
		static int totalLayers;

	};

#endif

