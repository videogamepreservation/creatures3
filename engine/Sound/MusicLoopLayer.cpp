// --------------------------------------------------------------------------
// Filename:	Music Loop Layer.cpp
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
// 23Apr98	PeterC	Created
// 08May98	PeterC	Changed format of update declarations
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicLoopLayer.h"

#include "MusicManager.h"
#include "MusicTrack.h"
#include "MusicScript.h"

// ----------------------------------------------------------------------
// Method:		MusicLoopLayer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicLoopLayer::MusicLoopLayer() :
	waveType (MUSIC_NO_WAVE),
	handle (NO_SOUND_HANDLE),
	pan(0.0),
	initialPan(0.0),
	loopUpdate(0.0),
	loopRate(0.2f)
	{

	}

// ----------------------------------------------------------------------
// Method:		~MusicLoopLayer
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicLoopLayer::~MusicLoopLayer()
	{
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
MusicError MusicLoopLayer::Parse(MusicScript &script,
				 MusicManager &manager,
				 MusicTrack &track)
	{
	// First an opening brace, and then the fun really starts
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

		// Pan(Float value)
		if ( strcmp( script.GetCurrentToken(), "Pan") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(pan);

			if (error != MUSIC_OK)
				{
				return error;
				}


			// Store the initial setting, so that the layer can
			// be reset
			initialPan = pan;

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

		// Rate(Float value)
		if ( strcmp( script.GetCurrentToken(), "Rate") == 0)
			{
			script.Advance();

			MusicError error = script.ParseArgument(loopRate);

			if (error != MUSIC_OK)
				{
				return error;
				}

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

			waveType = GetWaveID( waveName.data() );

			if (waveType == MUSIC_NO_WAVE)
				{
				return MUSIC_SYNTAX_ERROR;
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
// Arguments:	uberVolume  - overall soundtrack volume
//				time		- current time (seconds elapsed since
//							  start of track)
//				beat		- current length of a beat in seconds
//				beatCount   - total beats elapsed
// Returns:		Nothing
// Description:	Updates the layer, taking into account overall volume
// ----------------------------------------------------------------------
void MusicLoopLayer::Update(MusicValue uberVolume,
						    MusicValue time,
						    MusicValue beat,
						    MusicValue beatCount)
	{
	// Update the lower levels of the layer, performing any
	// update scripts, and resetting the nextUpdate variable

	MusicLayer::Update(uberVolume, time, beat, beatCount);

	// Are we ready to update the loop yet ?
	if ( (time + musicTimerResolution) >= loopUpdate)
		{
		// Make sure we've got a valid handle
		if (handle != NO_SOUND_HANDLE)
			{
			// Play the sound at the full volume for the layer
			UpdateLoop(uberVolume, handle, 1.0, pan);
			}

		// Determine when we'll need to update the loop again
		loopUpdate += loopRate;
		}

	}


// ----------------------------------------------------------------------
// Method:		GetVariable
// Arguments:	varName - name of the variable
//				lvalue  - true if value can be written to
// Returns:		Pointer to variable, or NULL if not defined
// Description:	Searches through list of named / pre-defined variables
// ----------------------------------------------------------------------
MusicValue *MusicLoopLayer::GetVariable(LPCTSTR varName, bool lvalue)
	{
	// Volume is the only pre-defined value here - check for it
	if (strcmp(varName,"Pan")==0)
		{
		return &pan;
		}

	// Now see if the name matches any of the user-defined variables
	return (MusicLayer::GetVariable(varName,lvalue));
	
	}

// ----------------------------------------------------------------------
// Method:		BeginPlaying
// Arguments:	uberVolume  - overall music volume
//				time		- current time (in seconds)
// Returns:		Nothing
// Description:	Reset state, and commence playing
// ----------------------------------------------------------------------
void MusicLoopLayer::BeginPlaying(MusicValue uberVolume, MusicValue time)
	{
	// Was anything already playing ?
	if (handle != NO_SOUND_HANDLE)
		{
		StopLoop(handle);
		handle = NO_SOUND_HANDLE;
		}

	// Reset the state of any layer / user-defined variables
	MusicLayer::BeginPlaying(uberVolume, time);

	// Restore pan to its initial value
	pan = initialPan;

	// Start the loop going in silence - Update() will set its correct
	// volume
	handle = LoopSound( uberVolume, waveType, GetVolume(), pan);

	// Make sure the loop gets updated immediately
	loopUpdate = 0.0;
	}

// ----------------------------------------------------------------------
// Method:		StopPlaying
// Arguments:	None
// Returns:		Nothing
// Description:	Stops any outstanding sounds / loops
// ----------------------------------------------------------------------
void MusicLoopLayer::StopPlaying()
	{
	// Stop any sounds attached to the layer itself (this is unnecessary,
	// but will minimise future error)
	MusicLayer::StopPlaying();

	if (handle != NO_SOUND_HANDLE)
		{
		StopLoop (handle);
		handle = NO_SOUND_HANDLE;
		}
	}

