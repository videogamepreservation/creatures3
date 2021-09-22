// --------------------------------------------------------------------------
// Filename:	Music Track.h
// Class:		MusicTrack
// Purpose:		Individual soundtrack
//
// Description:
//
// A MusicTrack is an individual soundtrack, consisting of a number of
// MusicLayers.  Tracks have an update rate associated with them, and can
// also specify a tempo (updates per beat, beats per bar) according to this.
//
// A fade in and fade out time can be specified, and it is possible to fade
// one soundtrack out, while fading another in.
//
// History:
// 7Apr98	PeterC	Created
// 07May98	PeterC	Altered to have all tracks continuously update
// 15May98  PeterC	Fixed fading bug
// --------------------------------------------------------------------------

#ifndef _MUSIC_TRACK_H

#define _MUSIC_TRACK_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"

#include "MusicUpdatable.h"



class MusicTrack : public MusicUpdatable
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicTrack
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicTrack();

		// ----------------------------------------------------------------------
		// Method:		~MusicTrack
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicTrack();

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
		// Arguments:	script  - script, pointing to the start of the track
		//				manager - music manager owning this track
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to parse the track from the script
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
						 MusicManager &manager);

		// ----------------------------------------------------------------------
		// Method:		Update
		// Arguments:	uberVolume  - overall music volume
		//				time		- current time (in seconds)
		// Returns:		Nothing
		// Description:	Updates the track, taking into account overall volume
		//				This will run any action script the track has
		// ----------------------------------------------------------------------
		void Update(MusicValue uberVolume, MusicValue time);

		// ----------------------------------------------------------------------
		// Method:		GetVariable
		// Arguments:	varName - name of the variable
		//				lvalue  - true if value can be written to
		// Returns:		Pointer to variable, or NULL if not defined
		// Description:	Searches through list of named / pre-defined variables
		// ----------------------------------------------------------------------
		MusicValue *GetVariable(LPCTSTR varName, bool lvalue = false);

		// ----------------------------------------------------------------------
		// Method:		BeginPlaying
		// Arguments:	uberVolume  - overall music volume
		//				time		- current time (in seconds)
		// Returns:		Nothing
		// Description:	Reset state, and commence playing
		// ----------------------------------------------------------------------
		void BeginPlaying(MusicValue uberVolume, MusicValue time);

		// ----------------------------------------------------------------------
		// Method:		BeginFadingOut
		// Arguments:	time		- current time (in seconds)
		//				duration	- (optional) length of fade
		// Returns:		Nothing
		// Description:	Starts the sound track fading out, using the default 
		//				fade for the track, or overiding it
		// ----------------------------------------------------------------------
		void BeginFadingOut(MusicValue time);
		void BeginFadingOut(MusicValue time, MusicValue duration);

		// ----------------------------------------------------------------------
		// Method:		StopPlaying
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Stops any outstanding sounds / loops
		// ----------------------------------------------------------------------
		void StopPlaying();

		// ----------------------------------------------------------------------
		// Method:		IsFinished
		// Arguments:	None
		// Returns:		true when if track has matched its end condition, or
		//				has finished fading
		// ----------------------------------------------------------------------
		bool IsFinished() const {return playStatus == Finished;}

		// ----------------------------------------------------------------------
		// Method:		HasTempo
		// Arguments:	None
		// Returns:		true if track has any fixed 'beat'
		// ----------------------------------------------------------------------
		bool HasTempo() const {return beatLength > 0.0;}
		
		// ----------------------------------------------------------------------
		// Method:		HasBeatOccurred
		// Arguments:	None
		// Returns:		true if a new beat has fallen since we last checked
		//				Note - the act of checking resets the flag
		// ----------------------------------------------------------------------
		bool HasBeatOccurred();

		// ----------------------------------------------------------------------
		// Method:		HasBarOccurred
		// Arguments:	None
		// Returns:		true if a new bar has been started since we last checked
		//				Note - the act of checking resets the flag
		// ----------------------------------------------------------------------
		bool HasBarOccurred();

		// ----------------------------------------------------------------------
		// Method:		GetLayer
		// Arguments:	layerName - name of the variable
		// Returns:		Pointer to layer, or NULL if not defined
		// Description:	Searches through list of named layers attached to a track
		// ----------------------------------------------------------------------
		MusicLayer *GetLayer(LPCTSTR layerName);

	private:

		// ----------------------------------------------------------------------
		// Member attributes 
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// layers
		// Array of layers contained within track
		// ----------------------------------------------------------------------
		std::vector <MusicLayer *> layers;
		typedef std::vector <MusicLayer *>::iterator layersIterator;

		// ----------------------------------------------------------------------
		// beatOccurred
		// barOccurred
		// Flags used in tracking rhythm events from outside of the track
		// ----------------------------------------------------------------------
		bool beatOccurred;
		bool barOccurred;

		// ----------------------------------------------------------------------
		// volume
		// volume of the track (0.0 = silent, 1.0 = maximum)
		// this is accessible as 'volume' to actions/expressions
		// ----------------------------------------------------------------------
		MusicValue volume;

		// ----------------------------------------------------------------------
		// initialVolume
		// Initial value is stored to enable restarting of soundtrack
		// ----------------------------------------------------------------------
		MusicValue initialVolume;

		// ----------------------------------------------------------------------
		// fadeIn / fadeOut
		// Time taken to fade soundtrack in and out
		// (Given in seconds)
		// ----------------------------------------------------------------------
		MusicValue fadeIn;
		MusicValue fadeOut;

		// ----------------------------------------------------------------------
		// playStatus
		// ----------------------------------------------------------------------
		enum {FadingIn, Playing, FadingOut, Finished} playStatus;

		// ----------------------------------------------------------------------
		// fadeEnd
		// Time (in seconds) at which fade will be complete
		// ----------------------------------------------------------------------
		MusicValue fadeEnd;

		// ----------------------------------------------------------------------
		// fadeDuration
		// This needs to be set whenever a fade begins
		// ----------------------------------------------------------------------
		MusicValue fadeDuration;

		// ----------------------------------------------------------------------
		// All of the following can be accessed by actions and expressions
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// beatLength
		// Length of a beat (in seconds) (a "Beat" message can be sent out)
		// The initial beat length is also stored, to allow the layer to be
		// reset
		// ----------------------------------------------------------------------
		MusicValue beatLength;
		MusicValue initialBeatLength;

		// ----------------------------------------------------------------------
		// BarLength
		// Beats per bar (read only)
		// (a "Bar" message can be sent out
		// ----------------------------------------------------------------------
		MusicValue barLength;

		// ----------------------------------------------------------------------
		// beatCount
		// Total number of beats since start of soundtrack (read only)
		// ----------------------------------------------------------------------
		MusicValue beatCount;

		// ----------------------------------------------------------------------
		// barCount
		// Total number of bars since start of soundtrack (read only)
		// ----------------------------------------------------------------------
		MusicValue barCount;

		// ----------------------------------------------------------------------
		// currentBeat
		// Current beat (within the bar) (read only)
		// ----------------------------------------------------------------------
		MusicValue currentBeat;

		// ----------------------------------------------------------------------
		// nextBeat
		// Time at which next beat should fall (if relevant)
		// ----------------------------------------------------------------------
		MusicValue nextBeat;


	};

#endif
