// --------------------------------------------------------------------------
// Filename:	Music Track.cpp
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
// 07Apr98	PeterC	Created
// 07May98	PeterC	Altered to have all tracks continuously update
// 08May98	PeterC	Changed format of update declarations
// 15May98  PeterC	Fixed fading bug
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTrack.h"
#include "MusicManager.h"
#include "MusicLayer.h"
#include "MusicLoopLayer.h"
#include "MusicAleotoricLayer.h"
#include "MusicScript.h"

// ----------------------------------------------------------------------
// Method:		MusicTrack
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicTrack::MusicTrack() :
	beatOccurred(false),
	barOccurred(false),
	volume(1.0),
	initialVolume(1.0),
	fadeIn(2.0),
	fadeOut(2.0),
	playStatus(FadingIn),
	beatLength(0.0),
	initialBeatLength(0.0),
	barLength(4.0),
	beatCount(0.0),
	barCount(0.0),
	currentBeat(0.0)
	{

	}

// ----------------------------------------------------------------------
// Method:		~MusicTrack
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicTrack::~MusicTrack()
	{
	// Delete all the layers

	layersIterator it;
	for (it = layers.begin(); it != layers.end(); it++ )
		{
		// First delete the allocated space
		// **** Doing this assignment for debugging purposes
		delete (*it);
		}

	// Now clear out the entries
	layers.clear();
	}

// ----------------------------------------------------------------------
// Method:		PreParse
// Arguments:	script - script, pointing to the start of the track
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Allocates the name and variables for the track, allowing
//				other layers to refer to it
// ----------------------------------------------------------------------
MusicError MusicTrack::PreParse(MusicScript &script)
	{
	// First of all, we have the track declaration.  Skip past this
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

	// Whenever we find a layer, we're going to have to PreParse that
	// too

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

				// Skip past the brace
				script.Advance();

				// Have we finished the layer?
				if (indents == 0)
					{
					// Yes - return safely
					return MUSIC_OK;
					}
				break;
				}
			case (MusicScript::String):
				{
				// We're only interested if it's a layer or a variable

				// Is this a loop layer declaration?
				if (strcmp(script.GetCurrentToken(),"LoopLayer")==0)
					{
					// Create a new loop layer, and add it to this tracks
					// list
					MusicLoopLayer *layer = new MusicLoopLayer;

					// Attempt to find this layer's name and variables
					MusicError error = layer -> PreParse(script);

					// If there was an error, bail out
					if (error != MUSIC_OK)
						{
						delete layer;
						return error;
						}

					// Add to this tracks array of layers
					layers.push_back(layer);

					}
				else
					{
					// Is this an aleotoric layer declaration?
					if (strcmp(script.GetCurrentToken(),"AleotoricLayer")==0)
						{
						// Create a new loop layer, and add it to this tracks
						// list
						MusicAleotoricLayer *layer = new MusicAleotoricLayer;

						// Attempt to find this layer's name and variables
						MusicError error = layer -> PreParse(script);

						// If there was an error, bail out
						if (error != MUSIC_OK)
							{
							delete layer;
							return error;
							}

						// Add to this tracks array of layers
						layers.push_back(layer);

						}
					else
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
							// String was nothing we were interested in
							// skip past it
							script.Advance();
							}

						}
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
// Arguments:	script  - script, pointing to the start of the track
//				manager - music manager owning this track
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to parse the track from the script
// ----------------------------------------------------------------------
MusicError MusicTrack::Parse(MusicScript &script,
							 MusicManager &manager)
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
			MusicError error=ParseInitialise(script,manager,*this);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}
				
		// Update{Action,..}
		if ( strcmp( script.GetCurrentToken(), "Update") == 0)
			{
			MusicError error=ParseUpdate(script,manager,*this);

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


		// Volume(Float value)
		if ( strcmp( script.GetCurrentToken(), "Volume") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(volume);

			if (error != MUSIC_OK)
				{
				return error;
				}


			// Store the initial setting, so that the track can
			// be reset
			initialVolume = volume;

			foundSomething = true;
			
			}

		// BeatLength(Value seconds)
		if ( strcmp( script.GetCurrentToken(), "BeatLength") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(beatLength);

			if (error != MUSIC_OK)
				{
				return error;
				}


			// Store the initial setting, so that the track can
			// be reset
			initialBeatLength = beatLength;

			foundSomething = true;
			
			}

		// BarLength(Value beats)
		if ( strcmp( script.GetCurrentToken(), "BarLength") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(barLength);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;
			
			}

		// FadeIn(Value time)
		if ( strcmp( script.GetCurrentToken(), "FadeIn") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(fadeIn);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}

		// FadeOut(Value time)
		if ( strcmp( script.GetCurrentToken(), "FadeOut") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(fadeOut);

			if (error != MUSIC_OK)
				{
				return error;
				}

			foundSomething = true;

			}

		// AleotoricLayer(String name) { ... }
		// LoopLayer(String name) { ... }
		if (   strcmp( script.GetCurrentToken(), "AleotoricLayer") == 0
			|| strcmp( script.GetCurrentToken(), "LoopLayer") == 0)
			{
			script.Advance();

			// Find the name of the layer
			std::string layerName;
			MusicError error = script.ParseArgument(layerName);
			if (error != MUSIC_OK)
				{
				return error;
				}

			// Locate the preparsed layer this refers to
			MusicLayer *layer = GetLayer ( layerName.data() );

			if (!layer)
				{
				return MUSIC_SYNTAX_ERROR;
				}

			// Now try and parse the layer, trapping any errors
			// (Parse is a virtual function, so the appropriate
			// version will be called)
			error = layer -> Parse ( script, manager, *this);

			if (error != MUSIC_OK)
				{
				return error;
				}

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

	// Must have compiled properly if we got this far
	return MUSIC_OK;

	}

// ----------------------------------------------------------------------
// Method:		Update
// Arguments:	uberVolume  - overall music volume
//				time		- current time (in seconds)
// Returns:		Nothing
// Description:	Updates the track, taking into account overall volume
//				This will run any action script the track has
// ----------------------------------------------------------------------
void MusicTrack::Update(MusicValue uberVolume, MusicValue time)
	{
	// Don't do anything if we're already done
	if (playStatus == Finished)
		{
		return;
		}

	// Handle the low level update - this will reset the counter
	// to the next update, and perform any action scripts if necessary

	MusicUpdatable::Update(time);

	// Do we need to update the beat / bar counters ?
	if ( HasTempo() )
		{
		// Has another beat occurred ?
		if (( time + musicTimerResolution) > nextBeat)
			{
			beatCount = beatCount + 1;

			// Flag this so that the outside world can pick
			// up beat events
			beatOccurred = true;

			// Is there any sense of bar length
			if (barLength > 0.0)
				{
				currentBeat += 1.0;

				// Have we passed the bar boundary ?
				if (currentBeat >= barLength)
					{
					// Point back to the beginning of the bar
					currentBeat = 0.0;

					barCount += 1.0;

					// Flag this so that the outside world can pick
					// up bar events
					barOccurred = true;
					}
				}

			// Set the time when the next beat should happen
			// **** Must be sure to have beat lengths higher
			// **** than the resolution
			nextBeat += beatLength;

			}

		}

	// Before we update the layers, scale the volume according to the
	// overall track volume

	MusicValue currentVolume = volume * uberVolume;

	// ... and any fades going on

	if (playStatus == FadingIn)
		{
		ASSERT(fadeIn);

		// Have we finished yet?
		if (time >= fadeEnd)
			{
			playStatus = Playing;
			}
		else
			{
			// Scale the volume in proportion to the amount of time
			// that we've been fading
			currentVolume *= (MusicValue) 1.0 - ( (fadeEnd - time) / fadeDuration);
			}
		}
	else
		{
		if (playStatus == FadingOut)
			{
			ASSERT(fadeOut);

			// Have we finished yet?
			if (time >= fadeEnd)
				{
				// Pack up and go home
				playStatus = Finished;
				StopPlaying();
				return;
				}
			else
				{
				// Scale the volume in proportion to the amount of time
				// that we've been fading
				currentVolume *=  (fadeEnd - time) / fadeDuration;
				}
			}

		}


	// Now update all the layers

	layersIterator it;
	for ( it = layers.begin();it != layers.end();it++)
		{
		(*it) -> Update( currentVolume, time, beatLength, beatCount);
		}

	return;
	}

// ----------------------------------------------------------------------
// Method:		GetVariable
// Arguments:	varName - name of the variable
//				lvalue  - true if value can be written to
// Returns:		Pointer to variable, or NULL if not defined
// Description:	Searches through list of named / pre-defined variables
// ----------------------------------------------------------------------
MusicValue *MusicTrack::GetVariable(LPCTSTR varName, bool lvalue)
	{
	// Several variables for the track are read only

	if (strcmp(varName,"BarCount")==0 && !lvalue)
		{
		return &barCount;
		}

	if (strcmp(varName,"BarLength")==0 && !lvalue)
		{
		return &barLength;
		}

	if (strcmp(varName,"CurrentBeat")==0 && !lvalue)
		{
		return &currentBeat;
		}

	// The following are read / write
	if (strcmp(varName,"Volume")==0)
		{
		return &volume;
		}

	if (strcmp(varName,"BeatLength")==0)
		{
		return &beatLength;
		}


	// Now see if the name matches any of the user defined
	// variables
	return (MusicVariableContainer::GetVariable(varName,lvalue));

	}

// ----------------------------------------------------------------------
// Method:		BeginPlaying
// Arguments:	uberVolume  - overall music volume
//				time		- current time (in seconds)
// Returns:		Nothing
// Description:	Reset state, and commence playing
// ----------------------------------------------------------------------
void MusicTrack::BeginPlaying(MusicValue uberVolume, MusicValue time)
	{
	// Restore all the writable pre-defined variables
	volume = initialVolume;
	beatLength = initialBeatLength;

	// Reset all the user defined variables
	Reset();

	// Run the initialisation script, if defined
	Initialise();

	// If a fade in time has been specified, set it in motion
	if (fadeIn > 0.0)
		{
		playStatus = FadingIn;
		fadeEnd = time + fadeIn;

		// Store the length of fade
		fadeDuration = fadeIn;
		}
	else
		{
		// Otherwise start at full volume
		playStatus = Playing;
		}

	// Reset all the counters
	beatCount = 0.0;
	barCount = 0.0;
	currentBeat = 0.0;

	// Does this have a tempo ?
	if (beatLength > 0.0)
		{
		// Yes - start with a bar and beat event
		barOccurred = true;
		beatOccurred = true;
		}
	else
		{
		barOccurred = false;
		beatOccurred = false;
		}

	// Flag when the next beat should occur
	nextBeat = time + beatLength;

	// Start next time update is called
	UpdateImmediately(time);

	// Now start all of the layers at zero volume (we're
	// fading in)

	layersIterator it;
	for (it = layers.begin();it != layers.end();it++)
		{
		(*it) -> BeginPlaying( 0.0, time);
		}

	}

// ----------------------------------------------------------------------
// Method:		BeginFadingOut
// Arguments:	time		- current time (in seconds)
// Returns:		Nothing
// Description:	Starts the sound track fading out, using the default 
//				fade for the trac
// ----------------------------------------------------------------------
void MusicTrack::BeginFadingOut(MusicValue time)
	{
	// If it's already fading out or finished, we
	// don't need to do anything
	if (playStatus == FadingOut || playStatus == Finished)
		{
		return;
		}

	// Was a valid fadeout specified
	if (fadeOut<= 0.0)
		{
		// No - just cut out completely
		playStatus = Finished;
		StopPlaying();
		return;
		}

	// Was this in the middle of fading in?
	if (playStatus == FadingIn)
		{
		// Shouldn't be able to fade if there is no time specified
		ASSERT ( fadeIn );

		// How far through fading in were we ?
		MusicValue fadeRatio = (MusicValue) 1.0 - ( (fadeEnd - time) / fadeIn );

		// Place this fade an equivalent distance from the end
		fadeEnd = time + fadeRatio * fadeOut;

		}
	else
		{
		// Do a complete fade
		fadeEnd = time + fadeOut;

		}

	// Store the length of fade
	fadeDuration = fadeOut;

	playStatus = FadingOut;

	}

// ----------------------------------------------------------------------
// Method:		BeginFadingOut
// Arguments:	time		- current time (in seconds)
//				duration	- length of fade
// Returns:		Nothing
// Description:	Starts the sound track fading out, overriding the default
//				fade length
// ----------------------------------------------------------------------
void MusicTrack::BeginFadingOut(MusicValue time, MusicValue duration)
	{
	// If it's already fading out or finished, we
	// don't need to do anything
	if (playStatus == FadingOut || playStatus == Finished)
		{
		return;
		}

	// Was a valid fadeout specified
	if (duration<= 0.0)
		{
		// No - just cut out completely
		playStatus = Finished;
		StopPlaying();
		return;
		}

	// Was this in the middle of fading in?
	if (playStatus == FadingIn)
		{
		// Shouldn't be able to fade if there is no time specified
		ASSERT ( fadeIn );

		// How far through fading in were we ?
		MusicValue fadeRatio = (MusicValue) 1.0 - ( (fadeEnd - time) / fadeIn );

		// Place this fade an equivalent distance from the end, using the 
		// overidden fade duration
		fadeEnd = time + fadeRatio * duration;

		}
	else
		{
		// Do a complete fade, using the overidden fade duration
		fadeEnd = time + duration;

		}

	// Store the length of fade
	fadeDuration = duration;

	playStatus = FadingOut;

	}

// ----------------------------------------------------------------------
// Method:		StopPlaying
// Arguments:	None
// Returns:		Nothing
// Description:	Stops any outstanding sounds / loops
// ----------------------------------------------------------------------
void MusicTrack::StopPlaying()
	{
	// Iterate through each layer, stopping it dead
	layersIterator it;
	for (it = layers.begin();it != layers.end();it++)
	{
	(*it) -> StopPlaying();
	}

	// Just to be sure...
	playStatus = Finished;
	}

// ----------------------------------------------------------------------
// Method:		GetLayer
// Arguments:	layerName - name of the variable
// Returns:		Pointer to layer, or NULL if not defined
// Description:	Searches through list of named layers attached to a track
// ----------------------------------------------------------------------
MusicLayer *MusicTrack::GetLayer(LPCTSTR layerName)
	{
	// Iterate through all the layers until a match is found
	layersIterator it;
	for (it = layers.begin();it != layers.end();it++)
		{
		if ((*it)->MatchName(layerName))
			{
			return (*it);
			}
		}

	// We didn't find a match
	return NULL;
	}

// ----------------------------------------------------------------------
// Method:		HasBeatOccurred
// Arguments:	None
// Returns:		true if a new beat has fallen since we last checked
//				Note - the act of checking resets the flag
// ----------------------------------------------------------------------
bool MusicTrack::HasBeatOccurred()
	{
	bool temp = beatOccurred;
	beatOccurred = false;
	return (temp);
	}

// ----------------------------------------------------------------------
// Method:		HasBarOccurred
// Arguments:	None
// Returns:		true if a new bar has been started since we last checked
//				Note - the act of checking resets the flag
// ----------------------------------------------------------------------
bool MusicTrack::HasBarOccurred()
	{
	bool temp = barOccurred;
	barOccurred = false;
	return (temp);

	}

