// --------------------------------------------------------------------------
// Filename:	Music Action.h
// Class:		MusicAction
//
// Purpose:		Store a sequence of instructions
//
// Description:
//
// Actions are a sequence of commands separated by commas.
// Commands are of the form:
//
// LVariable = Expression
//
// where lvariables are any variable on the layer owning the action
//
// History:
// 02Apr98	PeterC	Created
// --------------------------------------------------------------------------

#ifndef _MUSIC_ACTION_H

#define _MUSIC_ACTION_H


#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include "MusicTypes.h"
#include "MusicErrors.h"
#include "MusicExpression.h"


class MusicAction
	{
	public:
		// ----------------------------------------------------------------------
		// Method:		MusicAction
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Constructor
		// ----------------------------------------------------------------------
		MusicAction();

		// ----------------------------------------------------------------------
		// Method:		~MusicAction
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Default Destructor
		// ----------------------------------------------------------------------
		~MusicAction();

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script  - script, pointing to the start of an action
		//				manager - current music manager owning soundtracks
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the action from the script
		//				This should be called for an action Parsed from the
		//				MusicManager itself
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
						   MusicManager &manager)
			{return ParseWithScope(script,&manager);}

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script  - script, pointing to the start of an action
		//				manager - current music manager owning soundtracks
		//				track   - current soundtrack
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the action from the script
		//				This should be called for an action Parsed from a 
		//				MusicTrack
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
						   MusicManager &manager,
						   MusicTrack &track)
			{return ParseWithScope(script,&manager, &track);}

		// ----------------------------------------------------------------------
		// Method:		Parse
		// Arguments:	script   - script, pointing to the start of an action
		//				manager* - current music manager owning soundtracks
		//				track*   - current soundtrack
		//				layer*   - current layer within soundtrack
		//						   * can be reference or pointer
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to Parse the action from the script
		//				This should be called for an action Parsed from a 
		//				MusicLayer
		// ----------------------------------------------------------------------
		MusicError Parse(MusicScript &script,
						   MusicManager &manager,
						   MusicTrack &track,
						   MusicLayer &layer)
   			{return ParseWithScope(script,&manager, &track, &layer);}

		MusicError Parse(MusicScript &script,
						   MusicManager *manager,
						   MusicTrack *track=NULL,
						   MusicLayer *layer=NULL)
   			{return ParseWithScope(script,manager, track, layer);}

		
		// ----------------------------------------------------------------------
		// Method:		Perform
		// Arguments:	None
		// Returns:		Nothing
		// Description:	Performs the action (Note that runtime errors can not
		//				arise)
		// ----------------------------------------------------------------------
		void Perform() const;

	private:

		// ----------------------------------------------------------------------
		// Method:		ParseWithScope
		// Arguments:	script  - script, pointing to the start of an action
		//				manager - current music manager owning soundtracks
		//				track   - current soundtrack (or NULL if this is being
		//						  called from the manager)
		//				layer	- current layer (or NULL if this is being called
		//						  from a track)
		// Returns:		Error code, or MUSIC_OK if successful
		// Description:	Attempts to parse the action from the script
		//				This will take the current scope into account
		// ----------------------------------------------------------------------
		MusicError ParseWithScope(MusicScript &script,
								  MusicManager *manager,
								  MusicTrack *track=NULL,
								  MusicLayer *layer=NULL);

		// ----------------------------------------------------------------------
		// Command - Member class of MusicAction
		// Commands are stored in a linked list
		// ----------------------------------------------------------------------
		class Command
			{
			public:
				// ----------------------------------------------------------------------
				// Method:		Command
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Default Constructor
				// ----------------------------------------------------------------------
				Command();

				// ----------------------------------------------------------------------
				// Method:		~Command
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Default Destructor
				//				Deletes everything behind it in the linked list
				// ----------------------------------------------------------------------
				~Command();

				// ----------------------------------------------------------------------
				// Method:		Add
				// Arguments:	next - pointer to the command to be added
				// Returns:		Nothing
				// Description:	Adds the new command to the end of the list.  If this
				//				command is not the last in the list, then the link
				//				is passed down the list recursively
				// ----------------------------------------------------------------------
				void Add(Command *next);


				// ----------------------------------------------------------------------
				// Method:		Parse
				// Arguments:	script  - script, pointing to the start of a command
				//				manager - current music manager owning soundtracks
				//				track   - current soundtrack
				//				layer   - current layer within soundtrack
				// Returns:		Error code, or MUSIC_OK if successful
				// Description:	Attempts to parse the command from the script
				//				Scoping is taken into account
				// ----------------------------------------------------------------------
				MusicError Parse(MusicScript &script,
								   MusicManager *manager,
								   MusicTrack *track,
								   MusicLayer *layer);

				// ----------------------------------------------------------------------
				// Method:		Perform
				// Arguments:	None
				// Returns:		Nothing
				// Description:	Performs the command (Note that runtime errors can not
				//				arise), followed by any other commands in the list
				// ----------------------------------------------------------------------
				void Perform() const;

			private:

				// ----------------------------------------------------------------------
				// Attributes (MusicAction::Command)
				// ----------------------------------------------------------------------
	
				// ----------------------------------------------------------------------
				// lvalue
				// Value being assigned to (must belong to the owning layer)
				// ----------------------------------------------------------------------
				MusicValue *lvalue;


				// ----------------------------------------------------------------------
				// rvalue
				// Expression to be evaluated and passed to the lvalue
				// ----------------------------------------------------------------------
				MusicExpression rvalue;


				// ----------------------------------------------------------------------
				// nextLink
				// Next command in the linked list
				// ----------------------------------------------------------------------
				Command *nextLink;

			};

		// ----------------------------------------------------------------------
		// Attributes (MusicAction::Command)
		// ----------------------------------------------------------------------

		// ----------------------------------------------------------------------
		// commands
		// Points to the head of a linked list of commands
		// ----------------------------------------------------------------------
		Command *commands;


	};


#endif
