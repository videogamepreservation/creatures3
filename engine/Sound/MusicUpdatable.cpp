// --------------------------------------------------------------------------
// Filename:	Music Updatable.cpp
// Class:		MusicUpdatable
// Purpose:		Base class for objects that can have an update script
//
// Description:
//
// Objects of this type can have an optional update script attached, which
// can be given an optional update rate.
//
// History:
// 23Apr98	PeterC	Created
// 07May98	PeterC  Prevented Update requesting next update time
// 08May98	PeterC	Changed format of update declarations
//					Added Initialisation scripts
// --------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicUpdatable.h"
#include "MusicAction.h"

#include "MusicManager.h"
#include "MusicTrack.h"
#include "MusicLayer.h"
#include "MusicScript.h"

#include "MusicGlobals.h"

#include <float.h>

// ----------------------------------------------------------------------
// Method:		~MusicUpdatable
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicUpdatable::~MusicUpdatable()
	{
	delete update;
	delete initialisation;
	}

// ----------------------------------------------------------------------
// Method:		UpdateImmediately
// Arguments:	time - current time (seconds elapsed since
//					   start of track)
// Returns:		Nothing
// Description:	Prepares to update immediately
// ----------------------------------------------------------------------
void MusicUpdatable::UpdateImmediately(MusicValue time)
	{
	nextUpdate = nextScriptUpdate = time;
	}

// ----------------------------------------------------------------------
// Method:		Update
// Arguments:	time		- current time (seconds elapsed since
//							  start of track)
// Returns:		Nothing
// Description:	Executes the update script (if applicable)
// ----------------------------------------------------------------------
void MusicUpdatable::Update(MusicValue time)
	{
	// Reset the time for the next update to be when hell freezes over
	nextUpdate = FLT_MAX;

	// Is there an update script?
	if (update)
		{
		// Does it operate periodically ?
		if (scriptRate > 0.0)
			{
			// Yes - is it due yet
			if ( ( time + musicTimerResolution )>= nextScriptUpdate)
				{
				// Yes - execute it
				update -> Perform();

				// Now flag when it should next be executed
				// **** This will not be strictly accurate, but will
				// **** prevent the timer getting out of step
				nextScriptUpdate += scriptRate;
				}
			}
		else
			{
			// Script should execute whenever object is updated
			update -> Perform();
			}
		}

	}

// ----------------------------------------------------------------------
// Method:		ParseWithScope
// Arguments:	script - script, pointing to the start of the update
//				manager - music manager
//				track   - optional track
//				layer   - optional layer
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Parses the update script, taking scope into account
//				Updates are of the form:
//				Update { Action { ...,...} Rate ( Float ) }
// ----------------------------------------------------------------------
MusicError MusicUpdatable::ParseWithScope(MusicScript &script,
										  MusicManager *manager,
										  MusicTrack *track,
										  MusicLayer *layer)
	{
	// Get rid of any update previously stored
	delete update;
	update = NULL;

	// Shouldn't been called if this didn't start with 'Update'
	ASSERT ( strcmp( script.GetCurrentToken() , "Update" ) == 0 );

	script.Advance();

	update = new MusicAction;

	// Now parse the action details, trapping errors
	return update -> Parse(script, manager, track, layer);

	}

// ----------------------------------------------------------------------
// Method:		ParseUpdateRate
// Arguments:	script - script, pointing to the start of the update
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Parses the update rate (the gap between executions of
//				the update script)
// ----------------------------------------------------------------------
MusicError MusicUpdatable::ParseUpdateRate(MusicScript &script)
	{
	// Should never be called unless we have "UpdateRate" now
	ASSERT (strcmp ( script.GetCurrentToken(),"UpdateRate") == 0 );

	script.Advance();

	// Update rate is defined by a single argument between brackets
	return ( script.ParseArgument(scriptRate) );
	}

// ----------------------------------------------------------------------
// Method:		ParseInitialiseWithScope
// Arguments:	script - script, pointing to the start of the initialiser
//				manager - music manager
//				track   - optional track
//				layer   - optional layer
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Parses the initial script, taking scope into account
// ----------------------------------------------------------------------
MusicError MusicUpdatable::ParseInitialiseWithScope(MusicScript &script,
												    MusicManager *manager,
												    MusicTrack *track,
												    MusicLayer *layer)
	{
	// Get rid of any initialisation previously defined
	delete initialisation;
	initialisation = NULL;

	// Shouldn't been called if this didn't start with 'Initialise'
	ASSERT ( strcmp( script.GetCurrentToken() , "Initialise" ) == 0 );

	script.Advance();

	initialisation = new MusicAction;

	// Now parse the action details, trapping errors
	return initialisation -> Parse(script, manager, track, layer);

	}

// ----------------------------------------------------------------------
// Method:		Initialise
// Arguments:	Nones
// Returns:		Nothing
// Description:	Runs the (optional) initialisation script
// ----------------------------------------------------------------------
void MusicUpdatable::Initialise()
	{
	if (initialisation)
		{
		initialisation -> Perform();
		}
	}
