// --------------------------------------------------------------------------
// Filename:	Music Loop Layer.h
// Class:		MusicLoopLayer
// Purpose:		
//
// Description:
//
// A loop layer simply plays a looped sound constantly.  It has a pre-defined
// variable 'pan', which can be accessed by an action script attached to the
// layer.  This varies from -1.0 (left) to 1.0 (right).  'volume' can
// similarly be changed.
//
// History:
// 07Apr98	PeterC	Created
// 07May98	PeterC  Prevented Update requesting next update time
// --------------------------------------------------------------------------

#ifndef _MUSIC_LOOP_LAYER_H

#define _MUSIC_LOOP_LAYER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicLayer.h"

class MusicLoopLayer : public MusicLayer
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicLoopLayer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicLoopLayer();

		// ----------------------------------------------------------------------
		// Method:		~MusicLoopLayer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicLoopLayer();

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script - script, pointing to the start of the layer
		//				manager - music manager owning this layer
		//				track  - track owning layer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to parse the layer from the script
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
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
						    MusicValue currentBeat);


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
		// Method:		StopPlaying
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Stops any outstanding sounds / loops
		// ----------------------------------------------------------------------
		void StopPlaying();

	private:

		// ----------------------------------------------------------------------
		// Member attributes (Voice) 
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// waveType
		// wave associated with the voice
		// ----------------------------------------------------------------------
		MusicWaveID waveType;

		// ----------------------------------------------------------------------
		// handle
		// Handle to the loop being played
		// ----------------------------------------------------------------------
		SoundHandle handle;

		// ----------------------------------------------------------------------
		// pan
		// pan setting from left (-1.0) to right (1.0).  This is exposed to 
		// MusicActions as the 'pan' variable
		// ----------------------------------------------------------------------
		MusicValue pan;

		// ----------------------------------------------------------------------
		// initialPan
		// Initial value is stored to enable restarting of soundtrack
		// ----------------------------------------------------------------------
		MusicValue initialPan;

		// ----------------------------------------------------------------------
		// loopUpdate
		// Time at which loop should next update its pan / volume
		// ----------------------------------------------------------------------
		MusicValue loopUpdate;

		// ----------------------------------------------------------------------
		// loopRate
		// Interval (in seconds) between loop updates
		// ----------------------------------------------------------------------
		MusicValue loopRate;

	};

#endif
