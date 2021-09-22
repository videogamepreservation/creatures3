// --------------------------------------------------------------------------
// Filename:	Music Updatable.h
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
// 08May98	PeterC	Changed format of update declarations
//					Added initialisation script
// --------------------------------------------------------------------------

#ifndef _MUSIC_UPDATABLE_H

#define _MUSIC_UPDATABLE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicVariableContainer.h"
#include "MusicTypes.h"
#include "MusicGlobals.h"

class MusicAction;
class MusicManager;
class MusicTrack;
class MusicLayer;

class MusicUpdatable : public MusicVariableContainer
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicUpdatable
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicUpdatable() :
		  nextUpdate(0.0),
		  update(NULL),
		  initialisation(NULL),
		  scriptRate(0.0),
		  nextScriptUpdate(0.0) {}


		// ----------------------------------------------------------------------
		// Method:		~MusicUpdatable
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicUpdatable();

		// ----------------------------------------------------------------------
		// Method:		GetNextUpdate
		// Arguments:	None
		// Returns:		time at which next update should occur (in seconds) since
		//				start
		// ----------------------------------------------------------------------
		MusicValue GetNextUpdate() {return nextUpdate; }
		
		// ----------------------------------------------------------------------
		// Method:		IsReady
		// Arguments:	time - current time (in seconds) since start
		// Returns:		true if the object has an event waiting to update
		//				within the next timeslice
		// ----------------------------------------------------------------------
		bool IsReady(MusicValue time)
			{return (time + musicTimerResolution) >= nextUpdate; }

	protected:

		// ----------------------------------------------------------------------
		// Method:		Initialise
		// Arguments:	Nones
		// Returns:		Nothing
		// Description:	Runs the (optional) initialisation script
		// ----------------------------------------------------------------------
		void Initialise();

		// ----------------------------------------------------------------------
		// Method:		UpdateImmediately
		// Arguments:	time - current time (seconds elapsed since
		//					   start of track)
		// Returns:		Nothing
		// Description:	Prepares to update immediately
		// ----------------------------------------------------------------------
		void UpdateImmediately(MusicValue time);

		// ----------------------------------------------------------------------
		// Method:		Update
		// Arguments:	time		- current time (seconds elapsed since
		//							  start of track)
		// Returns:		Nothing
		// Description:	Executes the update script (if applicable)
		// ----------------------------------------------------------------------
		void Update(MusicValue time);

		// ----------------------------------------------------------------------
		// Method:		FlagEvent
		// Arguments:	time - time at which event is to occur
		// Returns:		Nothing
		// Description:	Keeps track of forthcoming events, so that the next
		//				update can be determined
		// ----------------------------------------------------------------------
		void FlagEvent(MusicValue time)
			{if ( time < nextUpdate ) nextUpdate = time; }

		// ----------------------------------------------------------------------
		// Method:		ParseUpdate
		// Arguments:	script - script, pointing to the start of the update
		//				manager - music manager owning this layer
		//				track  - track owning layer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the update script for a layer
		// ----------------------------------------------------------------------
		MusicError ParseUpdate(MusicScript &script,
							   MusicManager &manager,
							   MusicTrack &track,
							   MusicLayer &layer)
			{return ParseWithScope(script, &manager, &track, &layer);}

		// ----------------------------------------------------------------------
		// Method:		ParseUpdate
		// Arguments:	script - script, pointing to the start of the update
		//				manager - music manager owning this track
		//				track  - track owning track
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the update script for a track
		// ----------------------------------------------------------------------
		MusicError ParseUpdate(MusicScript &script,
							   MusicManager &manager,
							   MusicTrack &track)
			{return ParseWithScope(script, &manager, &track);}


		// ----------------------------------------------------------------------
		// Method:		ParseUpdate
		// Arguments:	script - script, pointing to the start of the update
		//				manager - overall music managerr
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the update script for a manager
		// ----------------------------------------------------------------------
		MusicError ParseUpdate(MusicScript &script,
							   MusicManager &manager)
			{return ParseWithScope(script, &manager);}

		// ----------------------------------------------------------------------
		// Method:		ParseUpdateRate
		// Arguments:	script - script, pointing to the start of the update
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the update rate (the gap between executions of
		//				the update script)
		// ----------------------------------------------------------------------
		MusicError ParseUpdateRate(MusicScript &script);

		// ----------------------------------------------------------------------
		// Method:		ParseInitialise
		// Arguments:	script - script, pointing to the start of the initialisation
		//				manager - music manager owning this layer
		//				track  - track owning layer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the initialisation script for a layer
		// ----------------------------------------------------------------------
		MusicError ParseInitialise(MusicScript &script,
							   MusicManager &manager,
							   MusicTrack &track,
							   MusicLayer &layer)
			{return ParseInitialiseWithScope(script, &manager, &track, &layer);}

		// ----------------------------------------------------------------------
		// Method:		ParseInitialise
		// Arguments:	script - script, pointing to the start of the initialisation
		//				manager - music manager owning this track
		//				track  - track owning track
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the initialisation script for a track
		// ----------------------------------------------------------------------
		MusicError ParseInitialise(MusicScript &script,
							   MusicManager &manager,
							   MusicTrack &track)
			{return ParseInitialiseWithScope(script, &manager, &track);}


		// ----------------------------------------------------------------------
		// Method:		ParseInitialise
		// Arguments:	script - script, pointing to the start of the initialisation
		//				manager - overall music managerr
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the initialisation script for a manager
		// ----------------------------------------------------------------------
		MusicError ParseInitialise(MusicScript &script,
							   MusicManager &manager)
			{return ParseInitialiseWithScope(script, &manager);}


	private:

		// ----------------------------------------------------------------------
		// Method:		ParseWithScope
		// Arguments:	script - script, pointing to the start of the update
		//				manager - music manager
		//				track   - optional track
		//				layer   - optional layer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the update script, taking scope into account
		// ----------------------------------------------------------------------
		MusicError ParseWithScope(MusicScript &script,
								  MusicManager *manager,
								  MusicTrack *track = NULL,
								  MusicLayer *layer = NULL);

		// ----------------------------------------------------------------------
		// Method:		ParseInitialiseWithScope
		// Arguments:	script - script, pointing to the start of the initialiser
		//				manager - music manager
		//				track   - optional track
		//				layer   - optional layer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Parses the initial script, taking scope into account
		// ----------------------------------------------------------------------
		MusicError ParseInitialiseWithScope(MusicScript &script,
										    MusicManager *manager,
										    MusicTrack *track = NULL,
										    MusicLayer *layer = NULL);

		// ----------------------------------------------------------------------
		// Attributes
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// nextUpdate
		// Time at which object will next be ready to update (this is calculated 
		// automatically by Update(), which should be called from all derived
		// classes
		// ----------------------------------------------------------------------
		MusicValue nextUpdate;

		// ----------------------------------------------------------------------
		// update
		// optional update script
		// ----------------------------------------------------------------------
		MusicAction *update;

		// ----------------------------------------------------------------------
		// update
		// optional initialisation script
		// ----------------------------------------------------------------------
		MusicAction *initialisation;

		// ----------------------------------------------------------------------
		// scriptRate
		// time between updates of script.  If zero, the script will only update
		// when any other event occurs
		// ----------------------------------------------------------------------
		MusicValue scriptRate;

		// ----------------------------------------------------------------------
		// nextScriptUpdate
		// Time at which script should next update, if scriptRate is non-zero 
		// ----------------------------------------------------------------------
		MusicValue nextScriptUpdate;

};

#endif