// --------------------------------------------------------------------------
// Filename:	Music Action.cpp
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
#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include "MusicAction.h"
#include "MusicScript.h"
#include "MusicLayer.h"
#include "MusicTrack.h"
#include "MusicManager.h"



// ----------------------------------------------------------------------
// Method:		MusicAction
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicAction::MusicAction() : commands(NULL)
	{

	}

// ----------------------------------------------------------------------
// Method:		~MusicAction
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
// ----------------------------------------------------------------------
MusicAction::~MusicAction()
	{
	// Deleting the first element in the linked list
	// will delete all elements behind it
	if(commands)
	delete commands;
	}

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
MusicError MusicAction::ParseWithScope(MusicScript &script,
						  MusicManager *manager,
						  MusicTrack *track,
						  MusicLayer *layer)
	{
	// Should have an opening brace first
	if (script.GetCurrentType() != MusicScript::StartSection)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Now should keep parsing commands until we hit an error or a closing 
	// brace
	while (script.GetCurrentType() != MusicScript::EndSection)
		{
		// Create a new command to add to the linked list
		Command *command = new Command;
		
		// Now attempt to parse the script into it
		MusicError error = command -> Parse( script, manager, track, layer);

		// Did we succeed ?
		if (error == MUSIC_OK)
			{
			// Is there anything in the list yet?
			if (commands)
				{
				// Yes - add this to the list
				commands -> Add( command );
				}
			else
				{
				// No - start the list with this
				commands = command;
				}
			}
		else
			{
			// Bail out with the error message
			return MUSIC_SYNTAX_ERROR;
			}
		}

	// Skip over the final }
	script.Advance();

	// Must have completed succesfully
	return MUSIC_OK;
	}

// ----------------------------------------------------------------------
// Method:		Perform
// Arguments:	None
// Returns:		Nothing
// Description:	Performs the action (Note that runtime errors can not
//				arise)
// ----------------------------------------------------------------------
void MusicAction::Perform() const
	{
	// Are there any commands to perform?
	if (commands)
		{
		// Yes - performing the command at the head of the list
		// will automatically cause the others to follow
		commands -> Perform();
		}
	}

// ----------------------------------------------------------------------
// Command - Member class of MusicAction
// Commands are stored in a linked list
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Method:		Command
// Arguments:	None
// Returns:		Nothing
// Description:	Default Constructor
// ----------------------------------------------------------------------
MusicAction::Command::Command() : 
	nextLink(NULL),
	lvalue(NULL)
	{
	}

// ----------------------------------------------------------------------
// Method:		~Command
// Arguments:	None
// Returns:		Nothing
// Description:	Default Destructor
//				Deletes everything behind it in the linked list
// ----------------------------------------------------------------------
MusicAction::Command::~Command()
	{
	// Delete anything following in the linked list
	delete nextLink;
	}

// ----------------------------------------------------------------------
// Method:		Add
// Arguments:	next - pointer to the command to be added
// Returns:		Nothing
// Description:	Adds the new command to the end of the list.  If this
//				command is not the last in the list, then the link
//				is passed down the list recursively
// ----------------------------------------------------------------------
void MusicAction::Command::Add(Command *next)
	{
	// Is this the last element in the list?
	if (nextLink)
		{
		// No - pass this down the rest of the list
		nextLink -> Add (next);
		}
	else
		{
		// Yes - link the new one straight after this
		nextLink = next;
		}
	}


// ----------------------------------------------------------------------
// Method:		Parse
// Arguments:	script  - script, pointing to the start of a command
//				manager - current music manager owning soundtracks
//				track   - current soundtrack
//				layer   - current layer within soundtrack
// Returns:		Error code, or MUSIC_OK if successful
// Description:	Attempts to parse the command from the script
// ----------------------------------------------------------------------
MusicError MusicAction::Command::Parse(MusicScript &script,
									   MusicManager *manager,
									   MusicTrack *track,
									   MusicLayer *layer)
	{
	// Should start off with an lvalue here.
manager->Update();
	if (script.GetCurrentType() != MusicScript::String)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// This can only be within the scope of the object we are currently 
	// parsing
	if (layer)
		{
		lvalue = layer -> GetVariable( script.GetCurrentToken() , true );
		}
	else
		{
		if (track)
			{
			lvalue = track -> GetVariable( script.GetCurrentToken() , true );
			}
		else
			{
			// Should always have a manager
			lvalue = manager->GetVariable( script.GetCurrentToken() , true );
			}
		}

	// Did we find a valid lvalue ?

	if (!lvalue)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	// Should follow this with an assignment

	script.Advance();

	if (script.GetCurrentType() != MusicScript::Assignment)
		{
		return MUSIC_SYNTAX_ERROR;
		}

	script.Advance();

	// Now attempt to parse the rvalue, taking scope into account
	return rvalue.Parse(script,manager,track,layer);
	}

// ----------------------------------------------------------------------
// Method:		Perform
// Arguments:	None
// Returns:		Nothing
// Description:	Performs the command (Note that runtime errors can not
//				arise), followed by any other commands in the list
// ----------------------------------------------------------------------
void MusicAction::Command::Perform() const
	{
	// Should always have a valid lvalue
	ASSERT(lvalue);

	// Evaluate the rvalue expression, and assign this to the lvalue
	*lvalue = rvalue.Evaluate();

	// Now perform all the others in the list recursively
	if (nextLink)
		{
		nextLink -> Perform();
		}
	
	}


