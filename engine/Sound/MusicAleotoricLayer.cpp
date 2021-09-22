// --------------------------------------------------------------------------
// Filename:	Music Aleotoric Layer.cpp
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
// 21Apr98	PeterC	Created
// 05May98	PeterC	Flagged nextVoice for update whenever Update is called
//					Force layer to constantly update until an applicable note
//					is found
//					Prevented unnecessary repetition of notes
// 08May98	PeterC	Changed format of update declarations
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicAleotoricLayer.h"
#include "MusicScript.h"
#include "MusicAction.h"
#include "MusicManager.h"

// ----------------------------------------------------------------------
// Method:		MusicAleotoricLayer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicAleotoricLayer::MusicAleotoricLayer() : 
	interval(1.0),
	initialInterval(1.0),
	beatSynch(0.0),
	nextBeat(0.0),
	effect(MUSIC_NO_EFFECT),
	availableVoices(NULL),
	lastChoice(NULL)
	{

	}

// ----------------------------------------------------------------------
// Method:		~MusicAleotoricLayer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicAleotoricLayer::~MusicAleotoricLayer()
	{
	// Yes - clear out the store of voices
	voicesIterator it;
	for (it = voices.begin(); it != voices.end(); it++ )
		{
		Voice* voice = (*it);
		// First delete the allocated space
		delete (*it);
		}

	// Now clear out the entries
	voices.clear();

	// Now remove the temporary array
	delete [] availableVoices;

	// Stop any remaining sounds
	StopPlaying();
	}

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script - script, pointing to the start of the layer
//				manager - music manager owning this layer
//				track  - track owning layer
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to parse the layer from the script
// ----------------------------------------------------------------------
MusicError MusicAleotoricLayer::Parse(MusicScript &script,
									  MusicManager &manager,
									  MusicTrack &track)
	{
	// Now an opening brace, and then the fun really starts
	if (script.GetCurrentType() != MusicScript::StartSection)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Keep reading in details until we reach a closing brace
	while (script.GetCurrentType() != MusicScript::EndSection)
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
			MusicError error=ParseInitialise(script,manager,track,*this);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}
				
		// Update{Action,..}
		if ( strcmp( script.GetCurrentToken(), "Update") == 0)
			{
			MusicError error=ParseUpdate(script,manager,track,*this);

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

		// Effect(String name)
		if ( strcmp( script.GetCurrentToken(), "Effect") == 0)
			{
			script.Advance();

			// Should have an open bracket
			if (script.GetCurrentType() != MusicScript::StartArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// Now should have the name of the effect
			script.Advance();

			if (script.GetCurrentType() != MusicScript::String)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// Make sure an effect exists with this name
			effect = GetEffectID( script.GetCurrentToken() );

			if (effect == MUSIC_NO_EFFECT)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			// Should have an closing bracket
			if (script.GetCurrentType() != MusicScript::EndArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// Point to the next descriptor
			script.Advance();

			foundSomething = true;

			}

		// Interval(Float value)
		if ( strcmp( script.GetCurrentToken(), "Interval") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(interval);

			if (error != MUSIC_OK)
				{
				return error;
				}


			// Store the initial setting, so that the layer can
			// be reset
			initialInterval = interval;

			foundSomething = true;

			}

		// BeatSynch(Float value)
		if ( strcmp( script.GetCurrentToken(), "BeatSynch") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(beatSynch);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}


		// Volume(Float value)
		if ( strcmp( script.GetCurrentToken(), "Volume") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(volume);

			if (error != MUSIC_OK)
				{
				return error;
				}


			// Store the initial setting, so that the layer can
			// be reset
			initialVolume = volume;

			foundSomething = true;
			
			}

		// Voice(String name)
		if ( strcmp( script.GetCurrentToken(), "Voice") == 0)
			{
			// Create a new voice and try to parse it
			Voice *voice = new Voice;
		
			// Provide it with access to the variables within the manager,
			// the track and the current layer
			MusicError error = voice -> Parse(script, manager, track, *this);

			if (error != MUSIC_OK)
				{
				return error;
				}

			// Add the new voice to the list
			voices.push_back(voice);

			foundSomething = true;

			}

		// we didn't find anything we recognised
		if (!foundSomething)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		}

	// Skip over the final }
	script.Advance();


	// Just to be safe
	delete availableVoices;

	// When we're determining which voices can be played, we'll need
	// to build up a list.  We'll create an array big enough to hold all
	// of them now, to avoid having to keep recreating and deleting it
	// later
	availableVoices = new Voice * [ voices.size() ];

	// Must have compiled properly if we got this far
	return MUSIC_OK;

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
void MusicAleotoricLayer::Update(MusicValue uberVolume,
							     MusicValue time,
							     MusicValue beat,
							     MusicValue beatCount)
	{
	// Update the lower levels of the layer, performing any
	// update scripts, and resetting the nextUpdate variable

	MusicLayer::Update(uberVolume, time, beat, beatCount);

	// Are we ready to play another note ?
	// (This is if either suitable delay has passed or (if this is beat
	// synchronised) the next relevant beat has occurred

	if (   (beatSynch == 0.0 && (time + musicTimerResolution) >= nextVoice)
		|| (beatSynch > 0.0  && beatCount >= nextBeat) )
		{

		// Iterate through all the voices, finding which ones
		// currently satisfy their conditions, and storing them in a list
		int totalAvailable = 0;

		voicesIterator it;
		for (it = voices.begin();it != voices.end();it++)
			{
			Voice *currentVoice = (*it);
			if (currentVoice -> IsApplicable() )
				{
				availableVoices[totalAvailable++] = currentVoice;
				}
			}

		// Did we find any voices 
		if (totalAvailable)
			{
			int choice;

			// pick one at random from the available voices,
			// avoiding repetition where possible
			do
				{
				choice = rand() % totalAvailable;
				} while (totalAvailable>1 && availableVoices[choice] == lastChoice);

			// Calculate the various play factors, providing
			// the layer settings as a default (Default to maximum
			// volume)
			MusicWaveID wave;
			MusicEffectID fx = effect;
			MusicValue gap = interval;

			// Play all sounds at maximum volume, unless otherwise
			// specified
			MusicValue vol = 1.0;
			availableVoices[choice]->CalculateDetails(wave,fx,gap,vol);

			// Play the sound, scaling the volume and applying effects
			PlaySound(uberVolume, time, beat, wave, vol, fx);

			// Try to avoid following up with the same note
			lastChoice = availableVoices[choice];


			// Record the delay until the next note

			// **** Interval is ignored if beat synchronised
			nextVoice += gap;

			}
		else
			{
			// No voices available.  Try again next update
			nextVoice += musicTimerResolution;
			}

		// Are we synchronising with the beat counter ?
		if (beatSynch>0.0)
			{
			// Yes - we must have had another beat

			// Flag the next beat we're looking for
			nextBeat += beatSynch;
			}
		}

	}

// ----------------------------------------------------------------------
// Method:		GetVariable
// Arguments:	varName - name of the variable
//				lvalue  - true if value can be written to
// Returns:		Pointer to variable, or NULL if not defined
// Description:	Searches through list of named / pre-defined variables
// ----------------------------------------------------------------------
MusicValue *MusicAleotoricLayer::GetVariable(LPCTSTR varName,
											 bool lvalue)
	{
	// Interval is the only pre-defined value here - check for it
	if (strcmp(varName,"Interval")==0)
		{
		return &interval;
		}

	// Now see if the name matches any of the pre-defined/user
	// variables for the layer
	return (MusicLayer::GetVariable(varName,lvalue));

	}

// ----------------------------------------------------------------------
// Method:		BeginPlaying
// Arguments:	uberVolume  - overall music volume
//				time		- current time (in seconds)
// Returns:		Nothing
// Description:	Reset state, and commence playing
// ----------------------------------------------------------------------
void MusicAleotoricLayer::BeginPlaying(MusicValue uberVolume, MusicValue time)
	{
	// Restore the interval to its initial setting
	interval = initialInterval;

	// We should play the next voice immediately
	nextVoice = time;

	// Regardless of time, tracks always begin at beat 0
	nextBeat = 0.0;

	// Now handle any other layer initialisation
	MusicLayer::BeginPlaying( uberVolume, time);
	}

// ----------------------------------------------------------------------
// Member classes
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Method:		Voice
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicAleotoricLayer::Voice::Voice() :
	update(NULL),
	effectType(MUSIC_NO_EFFECT),
	interval(NULL),
	volume(NULL),
	waveType(MUSIC_NO_WAVE)
	{
	
	}

// ----------------------------------------------------------------------
// Method:		~Voice
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicAleotoricLayer::Voice::~Voice()
	{
	// If an update action was defined, remove it
	if(update)
	delete update;

	// Similarly, remove the interval and volume expressions
	if(interval)
	delete interval;

	if(volume)
	delete volume;
	
	// Delete all the conditions within the array
	conditionsIterator it;
	for (it = conditions.begin();it != conditions.end();it++)
		{
		delete (*it);
		}

	// Now remove the entries themselves
	conditions.clear();
	}

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script  - script, pointing to the start of the voice
//				manager - manager owning track
//				track   - track owning layer
//				layer   - layer owning voice	
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to Parse the voice from the script
// ----------------------------------------------------------------------
MusicError MusicAleotoricLayer::Voice::Parse(MusicScript &script,
											 MusicManager &manager,
											 MusicTrack &track,
											 MusicLayer &layer)
	{
	// Should first of all just have "Voice"
	if (strcmp(script.GetCurrentToken(),"Voice") != 0)
		{
		return MUSIC_SYNTAX_ERROR;
		}
	
	script.Advance();

	// Now an opening brace, and then the fun really starts
	if (script.GetCurrentType() != MusicScript::StartSection)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Keep reading in details until we reach a closing brace
	while (script.GetCurrentType() != MusicScript::EndSection)
		{
		// We definitely should have a string next
		if (script.GetCurrentType() != MusicScript::String)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		// Want to make sure we find something to parse, or
		// we'll keep going round this loop forever
		bool foundSomething = false;

		// Update{Action}
		if ( strcmp( script.GetCurrentToken(), "Update") == 0)
			{
			script.Advance();

			// Create a new action, and attempt to parse it
			update = new MusicAction;
			MusicError error=update->Parse(script,manager,track,layer);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}

		// Note that we can safely continue checking whether or not
		// we just had a variable - if we reach the '}' none of the
		// following conditions will fire

		// Effect(String name)
		if ( strcmp( script.GetCurrentToken(), "Effect") == 0)
			{
			script.Advance();

			// Read in the name between parenthesis
			std::string effectName;
			MusicError error = script.ParseArgument(effectName);

			if (error != MUSIC_OK)
				{
				return error;
				}

			// Make sure an effect exists with this name
			effectType = layer.GetEffectID( effectName.data() );

			if (effectType == MUSIC_NO_EFFECT)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			foundSomething = true;

			}

		// Interval(Expression)
		if ( strcmp( script.GetCurrentToken(), "Interval") == 0)
			{
			script.Advance();

			// Should have an open bracket
			if (script.GetCurrentType() != MusicScript::StartArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			// Should have an expression next
			interval = new MusicExpression;
			MusicError error = interval->Parse(script, manager, track, layer);

			// Did it parse successfully
			if (error != MUSIC_OK)
				{
				return error;
				}

			// Make sure that there's a closing bracket
			if (script.GetCurrentType() != MusicScript::EndArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// Point to the next descriptor
			script.Advance();

			foundSomething = true;

			}

		// Volume(Expression)
		if ( strcmp( script.GetCurrentToken(), "Volume") == 0)
			{
			script.Advance();

			// Should have an open bracket
			if (script.GetCurrentType() != MusicScript::StartArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			script.Advance();

			// Should have an expression next
			volume = new MusicExpression;
			MusicError error = volume->Parse(script, manager, track, layer);

			// Did it parse successfully
			if (error != MUSIC_OK)
				{
				return error;
				}

			script.Advance();

			// Make sure that there's a closing bracket
			if (script.GetCurrentType() != MusicScript::EndArgument)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// Point to the next descriptor
			script.Advance();

			foundSomething = true;
			
			}

		// Wave(String name)
		if ( strcmp( script.GetCurrentToken(), "Wave") == 0)
			{
			script.Advance();

			std::string waveName;
			MusicError error = script.ParseArgument(waveName);

			if (error != MUSIC_OK)
				{
				return error;
				}

			// Make sure an effect exists with this name
			//
			// Since a complete list of waves is never given
			// in the script, the waves list is built up whenever
			// a new wave is encountered

			waveType = layer.GetWaveID( waveName.data() );

			if (waveType == MUSIC_NO_WAVE)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			foundSomething = true;

			}

		// Condition(Expression test, Float min, Float max)
		if ( strcmp( script.GetCurrentToken(), "Condition") == 0)
			{

			// Create a new conditon, and attempt to parse it
			Condition *condition = new Condition;
			MusicError error = condition -> Parse(script, manager, track, layer);

			if (error != MUSIC_OK)
				{
				// Clean up and bail out
				delete condition;
				return error;
				}

			// Add the new condition to the list of conditions
			conditions.push_back(condition);

			foundSomething = true;

			}


		// we didn't find anything we recognised
		if (!foundSomething)
			{
			return MUSIC_SYNTAX_ERROR;
			}

		}

	// Skip over the closing }
	script.Advance();

	// Must have compiled properly if we got this far
	return MUSIC_OK;


	}

// ----------------------------------------------------------------------
// Method:		IsApplicable
// Arguments:	None
// Returns:		true if all the voice's play conditions have been met
// ----------------------------------------------------------------------
bool MusicAleotoricLayer::Voice::IsApplicable() 
	{
	// We need to iterate through each of the conditions
	// If any are invalid, the voice is not applicable
	conditionsIterator it;
	for (it = conditions.begin();it != conditions.end();it++)
		{
		if (!(*it)-> Test() )
			{
			return false;
			}
		}

	// All (if any) must have evaluated to true
	return true;
	}

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
void MusicAleotoricLayer::Voice::CalculateDetails(MusicWaveID &wave,
					  MusicEffectID &effect,
					  MusicValue &delay,
					  MusicValue &vol)
	{
	// Perform any defined update action
	if (update)
		{
		update->Perform();
		}

	// There should always be a wave defined
	wave = waveType;

	// If an effect was defined, copy it, otherwise keep the default
	if (effectType != MUSIC_NO_EFFECT)
		{
		effect = effectType;
		}

	// If an interval was specified, evaluate it, otherwise keep the
	// default
	if (interval)
		{
		delay = interval -> Evaluate();
		}

	// If an volume was specified, evaluate it, otherwise keep the
	// default
	if (volume)
		{
		vol = volume -> Evaluate();
		}

	}

// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script  - script, pointing to the start of the condition
//				manager - manager owning track
//				track   - track owning layer
//				layer   - layer owning voice	
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to parse the condition from the script
// ----------------------------------------------------------------------
MusicError MusicAleotoricLayer::Voice::Condition::Parse(MusicScript &script,
													    MusicManager &manager,
													    MusicTrack &track,
													    MusicLayer &layer)
	{
	// Should not be able to get here without 'Condition' being declared
	ASSERT( strcmp (script.GetCurrentToken(), "Condition" ) == 0 );

	script.Advance();

	// Next an opening bracket
	if (script.GetCurrentType() != MusicScript::StartArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Now the test expression - trap any parsing errors
	MusicError error = test.Parse(script, manager, track, layer);

	if (error != MUSIC_OK)
		{
		return error;
		}

	// Now should have a comma
	if (script.GetCurrentType() != MusicScript::Separator)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Then the minimum value for the test

	if (script.GetCurrentType() != MusicScript::Constant)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	min = script.GetCurrentValue();

	script.Advance();

	// Now another comma
	if (script.GetCurrentType() != MusicScript::Separator)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Then the maximum value for the test

	if (script.GetCurrentType() != MusicScript::Constant)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	max = script.GetCurrentValue();

	script.Advance();

	// Then a closing bracket
	if (script.GetCurrentType() != MusicScript::EndArgument)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Leave pointing to the next item
	script.Advance();

	// Haven't found any errors
	return MUSIC_OK;

	}

// ----------------------------------------------------------------------
// Method:		Test
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
bool MusicAleotoricLayer::Voice::Condition::Test() const
	{
	// evaluate the test expression, and see if the result lies
	// within acceptable bounds

	MusicValue result = test.Evaluate();

	return (result >= min && result <= max);
	}

