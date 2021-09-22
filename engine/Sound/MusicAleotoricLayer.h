// --------------------------------------------------------------------------
// Filename:	Music Aleotoric Layer.h
// Class:		MusicAleotoricLayer
// Purpose:		
//
// Description:
//
// An aleotoric layer provides a means of randomly selecting a wave, given
// a number of criteria.  A layer specifies a number of voices, and for each
// gives a number of conditions that must be satisfied in order to be played
//
// Each update, the layer's action script (if defined) is run, and then all
// the voices are considered.  Those whose conditions are all satisfied are
// "put in a hat" and one is picked and played.  For each voice, an effect,
// the volume and a delay until the next sound can be played can be specified.
// If none are specified the layer default is instead used.
//
// Note that even if the delay specifies that no sound can be played, the
// layer's action script will always run.
//
// History:
// 3Apr98	PeterC	Created
// 07May98	PeterC  Prevented Update requesting next update time
// --------------------------------------------------------------------------

#ifndef _MUSIC_ALEOTORIC_LAYER_H

#define _MUSIC_ALEOTORIC_LAYER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicLayer.h"
#include "MusicExpression.h"
#include <vector>

class MusicAleotoricLayer : public MusicLayer
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicAleotoricLayer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicAleotoricLayer();

		// ----------------------------------------------------------------------
		// Method:		~MusicAleotoricLayer
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicAleotoricLayer();

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
						    MusicValue beatCount);

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

	private:

		// ----------------------------------------------------------------------
		// Member classes
		// ----------------------------------------------------------------------

		class Voice
			{
			public:
				// ----------------------------------------------------------------------
				// Method:		Voice
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Default Constructor
				// ----------------------------------------------------------------------
				Voice();

				// ----------------------------------------------------------------------
				// Method:		~Voice
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Default Destructor
				// ----------------------------------------------------------------------
				~Voice();

				// ----------------------------------------------------------------------
				// Method:		Parse
				// Arguments:	script  - script, pointing to the start of the voice
				//				manager - manager owning track
				//				track   - track owning layer
				//				layer   - layer owning voice	
				// Returns:		Error code, or MUSIC_OK if successful
				// Description:	Attempts to Parse the voice from the script
				// ----------------------------------------------------------------------
				MusicError Parse(MusicScript &script,
								 MusicManager &manager,
								 MusicTrack &track,
								 MusicLayer &layer);

				// ----------------------------------------------------------------------
				// Method:		IsApplicable
				// Arguments:	None
				// Returns:		true if all the voice's play conditions have been met
				// ----------------------------------------------------------------------
				bool IsApplicable() ;

				// ----------------------------------------------------------------------
				// Method:		CalculateDetails
				// Arguments:	wave   - to hold id of wave to be played
				//				effect - to hold effect to be used
				//				delay  - to hold time to elapse until next voice
				//						 (0.0 -> default for layer)
				//				volume - to hold volume at which note is to be played
				//
				//				effect,delay and volume should all be set to the layer's
				//				default's before being called
				//
				// Returns:		Nothing
				// Description:	Retrieves details of voice when it is about to be played
				//				Also activates any update action the voice might have,
				//				before calculating the delay and volume
				// ----------------------------------------------------------------------
				void CalculateDetails(MusicWaveID &wave,
									  MusicEffectID &effect,
									  MusicValue &delay,
									  MusicValue &vol);

			private:

				class Condition
					{
					public:
						// ----------------------------------------------------------------------
						// Method:		Condition
						// Arguments:	None
						// Returns:		Nothing
						// Description:	Default Constructor
						// ----------------------------------------------------------------------
						Condition() {}

						// ----------------------------------------------------------------------
						// Method:		~Condition
						// Arguments:	None
						// Returns:		Nothing
						// Description:	Default Destructor
						// ----------------------------------------------------------------------
						~Condition() {}

						// ----------------------------------------------------------------------
						// Method:		Parse
						// Arguments:	script  - script, pointing to the start of the condition
						//				manager - manager owning track
						//				track   - track owning layer
						//				layer   - layer owning voice	
						// Returns:		Error code, or MUSIC_OK if successful
						// Description:	Attempts to parse the condition from the script
						// ----------------------------------------------------------------------
						MusicError Parse(MusicScript &script,
										 MusicManager &manager,
										 MusicTrack &track,
										 MusicLayer &layer);

						// ----------------------------------------------------------------------
						// Method:		Test
						// Arguments:	None
						// Returns:		Nothing
						// Description:	Default Destructor
						// ----------------------------------------------------------------------
						bool Test() const;

					private:

						// ----------------------------------------------------------------------
						// Member attributes (Voice::Condition)
						// ----------------------------------------------------------------------
						
						// ----------------------------------------------------------------------
						// test
						// Expression evaluated and compared with min/max
						// ----------------------------------------------------------------------
						MusicExpression test;

						// ----------------------------------------------------------------------
						// min
						// minimum value of test
						// ----------------------------------------------------------------------
						MusicValue min;

						// ----------------------------------------------------------------------
						// max
						// maximum value of test
						// ----------------------------------------------------------------------
						MusicValue max;
					};

				// ----------------------------------------------------------------------
				// Member attributes (Voice) 
				// ----------------------------------------------------------------------

				// ----------------------------------------------------------------------
				// waveType
				// wave associated with the voice
				// ----------------------------------------------------------------------
				MusicWaveID waveType;

				// ----------------------------------------------------------------------
				// effectType
				// Effect sound should go through (MUSIC_NO_EFFECT if this should be
				// the default effect for the layer)
				// ----------------------------------------------------------------------
				MusicEffectID effectType;

				// ----------------------------------------------------------------------
				// interval
				// Time to elapse before next sound should play (if NULL,
				// this will be the default for the layer)
				// ----------------------------------------------------------------------
				MusicExpression *interval;

				// ----------------------------------------------------------------------
				// volume
				// Volume sound should be played at (if NULL,
				// this will be the default for the layer)
				// ----------------------------------------------------------------------
				MusicExpression *volume;

				// ----------------------------------------------------------------------
				// conditions
				// Array of conditions
				// ----------------------------------------------------------------------
				std::vector <Condition *> conditions;
				typedef std::vector<Condition*>::iterator conditionsIterator;

				// ----------------------------------------------------------------------
				// update
				// Action to be performed if this voice is activated
				// (if NULL, no action)
				// ----------------------------------------------------------------------
				MusicAction *update;

			};

		// ----------------------------------------------------------------------
		// Attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// interval
		// default time to elapse before next sound is played (in seconds)
		// This can be accessed by MusicActions as 'interval'
		// ----------------------------------------------------------------------
		MusicValue interval;

		// ----------------------------------------------------------------------
		// initialInterval
		// Initial value is stored to enable restarting of soundtrack
		// ----------------------------------------------------------------------
		MusicValue initialInterval;

		// ----------------------------------------------------------------------
		// nextVoice
		// Time (in seconds) at which the next voice should be played
		// ----------------------------------------------------------------------
		MusicValue nextVoice;

		// ----------------------------------------------------------------------
		// effect
		// default effect that voices should be played through
		// ----------------------------------------------------------------------
		MusicEffectID effect;

		// ----------------------------------------------------------------------
		// voices
		// Array of available voices
		// ----------------------------------------------------------------------
		std::vector <Voice *> voices;
		typedef std::vector <Voice *>::iterator voicesIterator;

		// ----------------------------------------------------------------------
		// availableVoices
		// Array of pointers to available voices.  This is created when the
		// layer is parsed in, and is equal in size to the total number of voices
		// ----------------------------------------------------------------------
		Voice **availableVoices;

		// ----------------------------------------------------------------------
		// lastChoice
		// Need to keep track of the last voice played, to avoid unnecessary 
		// repetition
		// ----------------------------------------------------------------------
		Voice *lastChoice;

		// ----------------------------------------------------------------------
		// beatSynch
		// Number of beats between updated (0.0 if not synchronised to tempo)
		// ----------------------------------------------------------------------
		MusicValue beatSynch;

		// ----------------------------------------------------------------------
		// nextBeat
		// Next beat at which an update is needed (if beat synchronised)
		// ----------------------------------------------------------------------
		MusicValue nextBeat;



	};

#endif
