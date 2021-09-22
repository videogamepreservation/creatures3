// --------------------------------------------------------------------------
// Filename:	Music Effect.h
// Class:		MusicEffect
// Purpose:		Delay effect used in music system
//
// Description:
//
// Effects are highly configurable stereo multi-tap delays that can be linked 
// into tempo, or can be selectively randomised.
//
// An effect consist of a sequence of stages, each of which specifies:
//
// Ratio of stage volume to initial volume of voice
// Pan setting (-1.0 to 1.0)
// Delay until next stage (in seconds, or in beats)
//
// History:
// 02Apr98	PeterC	Created
// --------------------------------------------------------------------------

#ifndef _MUSIC_EFFECT_H

#define _MUSIC_EFFECT_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicNamedItem.h"
#include "MusicErrors.h"
#include "MusicRandom.h"

class MusicScript;

class MusicEffect : public MusicNamedItem
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicEffect
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicEffect();

		// ----------------------------------------------------------------------
		// Method:		~MusicEffect
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicEffect();

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script - script, pointing to the start of the effect
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the effect from the script
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script);

		// ----------------------------------------------------------------------
		// Method:		BeginReadingStages
		// Arguments:	beat - length of a beat in seconds (delays may be tempo
		//					   related)
		// Returns:		true if there are any stages to read
		// Description:	Prepares to read the stages in sequence
		// ----------------------------------------------------------------------
		bool BeginReadingStages(MusicValue beat);

		// ----------------------------------------------------------------------
		// Method:		ReadStage
		// Arguments:	volume - to contain ratio of stage volume to overall 
		//						 sound volume
		//				pan    - to contain pan position (-1.0 to 1.0)
		//				delay  - time elapsed until next stage
		// Returns:		true if there is a stage to read
		// Description:	Calculates the settings for this stage, and moves on
		//				to the next stage in the list
		//				By default, a stage will be centred, full volume, and
		//				have a delay of one beat
		// ----------------------------------------------------------------------
		bool ReadStage(MusicValue &volume,
					   MusicValue &pan,
					   MusicValue &delay);


	private:

		// ----------------------------------------------------------------------
		// Stage - member class of effect
		// 
		// Stages are kept in a linked list
		// ----------------------------------------------------------------------

		class Stage
			{
			public:
				// ----------------------------------------------------------------------
				// Method:		Stage
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Default Constructor
				// ----------------------------------------------------------------------
				Stage();

				// ----------------------------------------------------------------------
				// Method:		~Stage
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Default Destructor
				//				Deletes all elements after this in the linked list.
				//				Delete the entire list by deleting the first element
				// ----------------------------------------------------------------------
				~Stage();

				// ----------------------------------------------------------------------
				// Method:		Parse
				// Arguments:	script - script, pointing to the start of the effect
				// Returns:		Error code, or MUSIC_OK if successful
				// Description:	Attempts to Parse the effect from the script
				// ----------------------------------------------------------------------
				MusicError Parse(MusicScript &script);

				// ----------------------------------------------------------------------
				// Method:		ReadSettings
				// Arguments:	beat   - length of a beat in seconds (delays may be tempo
				//					     related)
				//				volume - to contain ratio of stage volume to overall 
				//						 sound volume
				//				pan    - to contain pan position (-1.0 to 1.0)
				//				delay  - time elapsed until next stage
				// Returns:		Nothing
				// Description:	Calculates the settings for this stage
				// ----------------------------------------------------------------------
				void ReadSettings(MusicValue beat,
							      MusicValue &volume,
							      MusicValue &pan,
							      MusicValue &delay) const;

				// ----------------------------------------------------------------------
				// Method:		Add
				// Arguments:	next - pointer to the stage to be added
				// Returns:		Nothing
				// Description:	Adds the new stage to the end of the list.  If this
				//				stage is not the last in the list, then the link
				//				is passed down the list recursively
				// ----------------------------------------------------------------------
				void Add(Stage *next);

				// ----------------------------------------------------------------------
				// Method:		GetNext
				// Arguments:	None
				// Returns:		Pointer to next link in the chain, or NULL if already
				//				at the end of the link
				// ----------------------------------------------------------------------
				Stage *GetNext() const {return nextStage;}


			private:

				// ----------------------------------------------------------------------
				// Attributes (MusicEffect::Stage)
				// ----------------------------------------------------------------------
	
				// ----------------------------------------------------------------------
				// One constant pan setting (-1.0 = Left to 1.0 = Right)
				// ----------------------------------------------------------------------
				MusicValue pan;

				// ----------------------------------------------------------------------
				// If non-zero, consider the pan to be a random value
				//in the range (pan, pan+randomPan)
				// ----------------------------------------------------------------------
				MusicValue randomPan;
				
				// ----------------------------------------------------------------------
				// Volume of sound (0.0 to 1.0)
				// ----------------------------------------------------------------------
				MusicValue volume;

				// ----------------------------------------------------------------------
				// If non-zero, consider the volume to be a random value
				// in the range (volume, volume+randomVolume)
				// ----------------------------------------------------------------------
				MusicValue randomVolume;

				// ----------------------------------------------------------------------
				// Delay before next sound should be played
				// ----------------------------------------------------------------------
				MusicValue delay;

				// ----------------------------------------------------------------------
				// if non-zero, consider the delay to be a random value
				// in the range (delay, delay+randomDelay)
				// ----------------------------------------------------------------------
				MusicValue randomDelay;

				// ----------------------------------------------------------------------
				// Flags if the delay is given in beats (true) or seconds (false)
				// ----------------------------------------------------------------------
				bool tempoDelay;

				// ----------------------------------------------------------------------
				// Next stage in the linked list
				// ----------------------------------------------------------------------
				Stage *nextStage;

			};

		// ----------------------------------------------------------------------
		// Attributes (MusicEffect)
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// Head of the linked list of stages
		// ----------------------------------------------------------------------
		Stage *stages;
	
		// ----------------------------------------------------------------------
		// Current stage used when reading back
		// ----------------------------------------------------------------------
		Stage *currentStage;

		// ----------------------------------------------------------------------
		// Current beat duration (used by tempo delays)
		// ----------------------------------------------------------------------
		MusicValue currentBeat;

	};

#endif