// --------------------------------------------------------------------------
// Filename:	Music Effect.cpp
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
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicEffect.h"
#include "MusicScript.h"
#include "MusicTypes.h"

// ----------------------------------------------------------------------
// Method:		MusicEffect
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicEffect::MusicEffect() : 
	stages(NULL),
	currentStage(NULL),
	currentBeat(0.0)
	{

	}

// ----------------------------------------------------------------------
// Method:		~MusicEffect
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicEffect::~MusicEffect()
	{
	// Deleting the head of the chain of stages will delete
	// all elements behind it
	delete stages;
	}

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script - script, pointing to the start of the effect
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to Parse the effect from the script
// ----------------------------------------------------------------------
MusicError MusicEffect::Parse(MusicScript &script)
	{
	// Should always begin with "Effect" ...

	std::string token(script.GetCurrentToken());

	if (strcmp(token.data(),"Effect")!=0)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// ...followed by an open bracket
	script.Advance();

	if (script.GetCurrentType() != MusicScript::StartArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Next comes the effect's name
	script.Advance();

	if (script.GetCurrentType() != MusicScript::String)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	SetName(script.GetCurrentToken());

	// Then a closed bracket
	script.Advance();

	if (script.GetCurrentType() != MusicScript::EndArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Now we should have a series of stage declarations
	// enclosed by braces

	script.Advance();

	if (script.GetCurrentType() != MusicScript::StartSection)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Keep reading in stages until we reach a closing brace
	while (strcmp(script.GetCurrentToken(),"}") != 0 )
		{
		// Attempt to Parse a new stage from the current script
		Stage *stage = new Stage;

		MusicError error = stage -> Parse(script);

		if (error == MUSIC_OK) 
			{
			// Stage successfully Parsed - link it into the list
			if (stages)
				{
				stages -> Add( stage);
				}
			else
				{
				// Nothing yet in the list - make this the first element
				stages = stage;
				}
			}
		else
			{
			// Clean up, and go home
			delete stage;

			return error; 
			}
		}

	// Must have had a closing brace if we got here.  Skip past it,
	// and return safely
	script.Advance();

	return MUSIC_OK;

	}

// ----------------------------------------------------------------------
// Method:		BeginReadingStages
// Arguments:	beat - length of a beat in seconds (delays may be tempo
//					   related)
// Returns:		true if there are any stages to read
// Description:	Prepares to read the stages in sequence
// ----------------------------------------------------------------------
bool MusicEffect::BeginReadingStages(MusicValue beat)
	{
	// Point to the head of the list
	currentStage = stages;

	// Store the current beat duration
	currentBeat = beat;

	// is there anything to read?
	return (stages != NULL );
	}

// ----------------------------------------------------------------------
// Method:		ReadStage
// Arguments:	volume - to contain ratio of stage volume to overall 
//						 sound volume
//				pan    - to contain pan position (-1.0 to 1.0)
//				delay  - time elapsed until next stage
// Returns:		true if there is a stage to read
// Description:	Calculates the settings for this stage, and moves on
//				to the next stage in the list
// ----------------------------------------------------------------------
bool MusicEffect::ReadStage(MusicValue &volume,
			   MusicValue &pan,
			   MusicValue &delay)
	{
	// Have we reached the end of the list yet?
	if (currentStage)
		{
		// No - return the values for this stage, passing along the
		// beat duration set by BeginReadingStages()
		currentStage -> ReadSettings (currentBeat, volume, pan, delay);

		// Move onto the next stage, if there is one
		currentStage = currentStage->GetNext();

		// Flag that we have values to return
		return true;
		}
	else
		{
		// Nothing left in the list
		return false;
		}

	}


// ----------------------------------------------------------------------
// Stage - member class of effect
// 
// Stages are kept in a linked list
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Method:		Stage
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
//				By default, a stage will be centred, full volume, and
//				have a delay of one beat
// ----------------------------------------------------------------------
MusicEffect::Stage::Stage() :
		pan(0.0),
		randomPan(0.0),
		volume(1.0),
		randomVolume(0.0),
		delay(1.0),
		randomDelay(0.0),
		tempoDelay(true),
		nextStage(NULL)
	{

	}

// ----------------------------------------------------------------------
// Method:		~Stage
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
//				Deletes all elements after this in the linked list.
//				Delete the entire list by deleting the first element
// ----------------------------------------------------------------------
MusicEffect::Stage::~Stage()
	{
	// Delete any entries behind this one
	delete nextStage;
	}

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script - script, pointing to the start of the effect
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to Parse the effect from the script
// ----------------------------------------------------------------------
MusicError MusicEffect::Stage::Parse(MusicScript &script)
	{
	// First Token should always be "Stage" ...
	std::string token(script.GetCurrentToken());

	if (strcmp(token.data(),"Stage")!=0)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// ...followed by an open brace
	script.Advance();

	if (script.GetCurrentType() != MusicScript::StartSection)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Now keep interpretting separate sections until we find 
	// a closing brace
	while (script.GetCurrentType() != MusicScript::EndSection)
		{
		// Should have one of the strings first
		const char descriptors[4][12] =
			{
			"Pan",
			"Volume",
			"Delay",
			"TempoDelay",
			};

		enum Descriptor
			{
			PanDescriptor = 0,
			VolumeDescriptor,
			DelayDescriptor,
			TempoDelayDescriptor,
			};

		if (script.GetCurrentType() != MusicScript::String)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		std::string descName(script.GetCurrentToken());

		// Identify the string
		int descriptorType=-1;
		for (int i=0;i<4 && descriptorType == -1;i++)
			{
			if (strcmp(descName.data(),descriptors[i])==0)
				{
				descriptorType = i;
				}
			}

		// Make sure we found something valid
		if (descriptorType == -1)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		// Should now have an open bracket
		script.Advance();

		if (script.GetCurrentType() != MusicScript::StartArgument)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		script.Advance();

		// Next we should have either a single value, or a random
		// range "Random(min,max)"
		
		MusicValue value;
		MusicValue randomSize = 0.0;

		if (script.GetCurrentType() == MusicScript::Constant)
			{
			// Single value - random range stays as zero
			value = script.GetCurrentValue();
			script.Advance();
			}
		else
			{
			// Must be the "Random" case
			if (script.GetCurrentType() != MusicScript::String)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			std::string token(script.GetCurrentToken());

			if (strcmp(token.data(),"Random") != 0 )
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			// Should have an open bracket
			if (script.GetCurrentType() != MusicScript::StartArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			// Now we should have the bottom of the floating point range
			if (script.GetCurrentType() != MusicScript::Constant)
				{
				return MUSIC_SYNTAX_ERROR;
				}
			
			value = script.GetCurrentValue();

			script.Advance();

			// Arguments are separated by a comma
			if (script.GetCurrentType() != MusicScript::Separator)
				{
				return MUSIC_SYNTAX_ERROR;
				}
			
			script.Advance();

			// Now the upper value in the range
			if (script.GetCurrentType() != MusicScript::Constant)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// We only want to store the size of the interval,
			// and not the upper value itself
			randomSize = script.GetCurrentValue() - value;

			script.Advance();

			// "Random" section rounded off by a closing bracket
			if (script.GetCurrentType() != MusicScript::EndArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			}

		// Descriptor section rounded off by a closing bracket
		if (script.GetCurrentType() != MusicScript::EndArgument)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		script.Advance();

		// Now assign the value / random range to the appropriate
		// setting

		switch (descriptorType)
			{
			case (PanDescriptor):
				{
				pan = value;
				randomPan = randomSize;
				break;
				};
			case (VolumeDescriptor):
				{
				volume = value;
				randomVolume = randomSize;
				break;
				};
			case (DelayDescriptor):
				{
				delay = value;
				randomDelay = randomSize;
				tempoDelay = false;
				break;
				};
			case (TempoDelayDescriptor):
				{
				delay = value;
				randomDelay = randomSize;
				tempoDelay = true;
				break;
				};
			default:
				{
				// This should never happen
				ASSERT(FALSE);
				return MUSIC_SYNTAX_ERROR;
				}
			}

		}

	// To have escaped from the while loop, we must have had a
	// closing bracket.  Skip past this to leave us pointing to
	// the next section

	script.Advance();

	return (MUSIC_OK);

	}

// ----------------------------------------------------------------------
// Method:		ReadSettings
// Arguments:	beat	    - length of a beat in seconds (delays may be 
//					          tempo related)
//				stageVolume - to contain ratio of stage volume to overall 
//						      sound volume
//				stagePan    - to contain pan position (-1.0 to 1.0)
//				stageDelay  - time elapsed until next stage
// Returns:		Nothing
// Description:	Calculates the settings for this stage
// ----------------------------------------------------------------------
void MusicEffect::Stage::ReadSettings(MusicValue beat,
				  MusicValue &stageVolume,
				  MusicValue &stagePan,
				  MusicValue &stageDelay) const
	{
	// Give the volume a random offset if defined
	stageVolume = volume + MusicRandom( randomVolume );

	// Same for the pan setting...
	stagePan = pan + MusicRandom( randomPan );

	// ... and for the delay
	stageDelay = delay + MusicRandom( randomDelay );

	// Is the delay tempo based?
	if (tempoDelay)
		{
		// Yes - the delay is in beats, not seconds, so
		// multiply by the length of a beat
		stageDelay *= beat;
		}
	}

// ----------------------------------------------------------------------
// Method:		Add
// Arguments:	next - pointer to the stage to be added
// Returns:		Nothing
// Description:	Adds the new stage to the end of the list.  If this
//				stage is not the last in the list, then the link
//				is passed down the list recursively
// ----------------------------------------------------------------------
void MusicEffect::Stage::Add(Stage *next)
	{
	// If this link is already at the end of the list, attach here,
	// otherwise pass it down the list
	if (!nextStage)
		{
		nextStage = next;
		}
	else
		{
		nextStage -> Add (next);
		}
	}

