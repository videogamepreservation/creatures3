
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CAOSTables.h"
#include "OpSpec.h"

#include "CAOSDescription.h"

#include "CAOSMachine.h"
#include "GeneralHandlers.h" 
#include "AgentHandlers.h"
#include "CompoundHandlers.h"
#include "CreatureHandlers.h"
#include "SoundHandlers.h"
#include "PortHandlers.h"
#include "DebugHandlers.h"
#include "MapHandlers.h"
#include "DisplayHandlers.h"
#include "../../common/PRAYFiles/PrayHandlers.h"
#include "HistoryHandlers.h"

#include "../build.h"
#include "../App.h"

#include "TableSpec.h"
#include "../Agents/AgentConstants.h"

#ifndef C2E_OLD_CPP_LIB
// dubious stringstream use by PACE documenation...
#include <sstream>
#endif

// Categories for ordering the manual better - 
// they will be sorted in this order.
// So please add in alphabetical order!
// enum in CAOSTables.h must be in the same order
std::string ourCategoryText[] =
{
	"No Need to Document",
	"Agents",
	"Brain",
	"Camera",
	"Compounds",
	"Creatures",
	"Debug",
	"Files",
	"Flow",
	"Genetics",
	"History",
	"Input",
	"Map",
	"Motion",
	"Ports",
	"Resources",
	"Scripts",
	"Sounds",
	"Time",
	"Variables",
	"Vehicles",
	"World",
	"Not Implemented"
};

// Extra tables and things we'll need to deliver in a CAOS manual:
// 1. Stimulus numbers
// 4. LOCI table
// 7. Correct body parts for WEAR command.  Max layers as well.

// Values for parameter types:
// a: agent rvalue
// b: byte string
// c: conditional
// d: decimal rvalue (can be integer or float)
// f: float rvalue (decimal rv casted to float)
// i: integer rvalue (decimal rv casted to integer)
// s: string rvalue
// v: variable
// m: message (can be anything)
// #: label
// -: insert a zero placeholder int in the bytecode stream

OpSpec ourSubCommandTable_HIST[] =
{
	OpSpec( 0, "EVNT", "siss", "moniker event_type related_moniker_1 related_moniker_2", categoryHistory, "Triggers a life event of the given type.  Some events are triggered automatically by the engine, some events need triggering from CAOS, others are custom events that you can use for your own purposes.  See @#HIST TYPE@ for details of the event numbers.  All new events made call the @#Life Event@ script."),
	OpSpec( 1, "UTXT", "sis", "moniker event_no new_value", categoryHistory, "For the given life event, sets the user text."),
	OpSpec( 2, "NAME", "ss", "moniker new_name", categoryHistory, "Renames the creature with the given moniker."),
	OpSpec( 3, "WIPE", "s", "moniker", categoryHistory, "Purge the creature history for the given moniker.  Only applies if the genome isn't referenced by any slot, and the creature is fully dead or exported.  Use @#OOWW@ to test this first."),
	OpSpec( 4, "FOTO", "sis", "moniker event_no new_value", categoryHistory, "For the given life event, sets the associated photograph.  Use @#SNAP@ to take the photograph first.<p>If there was already a photograph for the event, then it is automatically marked for the attic as in @#LOFT@, and overwritten with the new photo.  Hence you can use an empty string to clear a photo.  If @#HIST WIPE@ is used to clear the event, the photo is similarly stored in the attic.<p>It is considered an error to send a photograph that is in use (unless cloned with @#TINT@) to the attic.  If this happens, you will get a runtime error.  You should either be confident that no agents are using the photo, or call @#LOFT@ first to test if they are."),
};

OpSpec ourSubIntegerRVTable_HIST[] =
{
	OpSpec( 0, "COUN", "s", "moniker", categoryHistory, "Returns the number of life events that there are for the given moniker.  Returns 0 of there are no events, or the moniker doesn't exist."),
	OpSpec( 1, "TYPE", "si", "moniker event_no", categoryHistory, "For the given life event, returns its type.<p>All histories begin with one of the following four events.  You can read the associated monikers with @#HIST MON1@ and @#HIST MON2@.<br>0 Conceived - a natural start to life, associated monikers are the mother's and father's<br>1 Spliced - created using @#GENE CROS@ to crossover the two associated monikers<br>2 Engineered - from a human made genome with @#GENE LOAD@, the first associated moniker is blank, and the second is the filename<br>14 Cloned - such as when importing a creature that already exists in the world and reallocating the new moniker, when @#TWIN@ing or @#GENE CLON@ing; associated moniker is who we were cloned from<p>The following events happen during a creature's life:<br>3  Born - triggered by the @#BORN@ command, associated monikers are the parents.<br>4 Aged - reached the next life stage, either naturally from the ageing loci or with @#AGES@<br>5 Exported - emmigrated to another world<br>6 Imported - immigrated back again<br>7 Died - triggered naturally with the death trigger locus, or by the @#DEAD@ command<br>8 Became pregnant - the first associated moniker is the child, and the second the father<br>9 Impregnated - first associated moniker is the child, second the mother<br>10 Child born - first moniker is the child, second the other parent<br>15 Clone source - someone was cloned from you, first moniker is whom<p>These events aren't triggered by the engine, but reserved for CAOS to use with these numbers:<br>11 Laid by mother<br>12 Laid an egg<br>13 Photographed<p>Other numbers can also be used for custom life events.  Start with numbers 100 and above, as events below that are reserved for the engine.  You send your own events using @#HIST EVNT@."),
	OpSpec( 2, "WTIK", "si", "moniker event_no", categoryHistory, "Returns the world tick when the life event happened, for the world that the event happened in. "),
	OpSpec( 3, "TAGE", "si", "moniker event_no", categoryHistory, "Returns the age in ticks of the creature when the given life event happened to it.  If the creature was not in the world, wasn't born yet, or was fully dead, then -1 is returned.  If the creature was dead, but its body still in the world, then its age on death is returned.  See also @#TAGE@."),
	OpSpec( 4, "RTIM", "si", "moniker event_no", categoryHistory, "Returns the real world time when the given life event happened.  This is measured in seconds since midnight, 1 January 1970 in UTC.  To display, use @#RTIF@."),
	OpSpec( 5, "CAGE", "si", "moniker event_no", categoryHistory, "Returns the life stage of the creature when the given life event happened."),
	OpSpec( 6, "GEND", "s", "moniker", categoryHistory, "Returns the sex that the creature with the given moniker has or had.  1 for male, 2 for female.  If the creature hasn't been born yet, returns -1."),
	OpSpec( 7, "GNUS", "s", "moniker", categoryHistory, "Returns the genus of the moniker.  This is 1 for Norn, 2 for Grendel, 3 for Ettin by convention."),
	OpSpec( 8, "VARI", "s", "moniker", categoryHistory, "Returns the variant that the creature with the given moniker has or had.  If the creature hasn't been born yet, returns -1."),
	OpSpec( 9, "FIND", "sii", "moniker event_type from_index", categoryHistory, "Searches for a life event of a certain @#HIST TYPE@ for the given moniker.  The search begins at the life event <b>after</b> the from index.  Specify -1 to find the first event.  Returns the event number, or -1 if there is no matching event."),
	OpSpec( 10, "FINR", "sii", "moniker event_type from_index", categoryHistory, "Reverse searches for a life event of a certain @#HIST TYPE@ for the given moniker.  The search begins at the life event <b>before</b> the from index.  Specify -1 to find the last event.  Returns the event number, or -1 if there is no matching event."),
	OpSpec( 11, "SEAN",  "i", "world_tick", categoryTime, "Returns the current season for a given world tick.  This is the same as @#SEAN@.  See also @#WTIK@ and @#HIST WTIK@." ),
	OpSpec( 12, "TIME",  "i", "world_tick", categoryTime, "Returns the time of day for a given world tick.  This is the same as @#TIME@.  See also @#WTIK@ and @#HIST WTIK@." ),
	OpSpec( 13, "YEAR",  "i", "world_tick", categoryTime, "Returns the number of game years elapsed for a given world tick.  This is the same as @#YEAR@.  See also @#WTIK@ and @#HIST WTIK@." ),
	OpSpec( 14, "DATE",  "i", "world_tick", categoryTime, "Returns the day within the current season.  This is the same as @#DATE@.  See also @#WTIK@ and @#HIST WTIK@." ),
	OpSpec( 15, "MUTE", "s", "moniker", categoryHistory, "Returns the number of point mutations the genome received during crossover from its parents."),
	OpSpec( 16, "CROS", "s", "moniker", categoryHistory, "Returns the number of crossover points when the genome was made by splicing its parents genomes."),
};

OpSpec ourSubStringRVTable_HIST[] =
{
	OpSpec( 0, "MON1", "si", "moniker event_no", categoryHistory, "For the given life event, returns the first associated moniker."),
	OpSpec( 1, "MON2", "si", "moniker event_no", categoryHistory, "For the given life event, returns the second associated moniker."),
	OpSpec( 2, "UTXT", "si", "moniker event_no", categoryHistory, "For the given life event, returns the user text."),
	OpSpec( 3, "WNAM", "si", "moniker event_no", categoryHistory, "Returns the name of the world the given life event happened in."),
	OpSpec( 4, "WUID", "si", "moniker event_no", categoryHistory, "Returns the unique identifier of the world the given life event happened in."),
	OpSpec( 5, "NAME", "s", "moniker", categoryHistory, "Returns the name of the creature with the given moniker."),
	OpSpec( 6, "NEXT", "s", "moniker", categoryHistory, "Returns the next moniker which has a history, or an empty string if we're at the end already.  If the specified moniker is an empty string or doesn't have a history, then the first moniker with a history entry is returned, or an empty string if there isn't one."),
	OpSpec( 7, "PREV", "s", "moniker", categoryHistory, "Returns the previous moniker which has a history.  If the specified moniker is an empty string or doesn't have a history, then the last moniker with a history entry is returned, or an empty string if there isn't one."),
	OpSpec( 8, "FOTO", "si", "moniker event_no", categoryHistory, "For the given life event, returns the filename of the associated photograph, or an empty string if there is no photo."),
};

OpSpec ourSubCommandTable_FILE[] =
{
	OpSpec( 0, "OOPE", "isi", "directory filename append", categoryFiles, "Sets the file for the output stream of the current virtual machine - there is a virtual machine for each agent, so this is much the same as setting it for @#OWNR@.  Use @#OUTV@ and @#OUTS@ or various other commands to send text data to the stream.  The filename should include any file extension.<p>You should use @#FILE OCLO@ to close the file, although this happens automatically if you set a new file, if the virtual machine is destroyed, or if the engine exits.<p>Directory is 0 for the current world's journal directory, or 1 for the main journal directory.  Set append to 1 to add to the end of the file, or 0 to replace any existing file."),
	OpSpec( 1, "OCLO", "", "", categoryFiles, "Disconnects anything which is attached to the output stream.  If this is a file, then the file is closed."),
	OpSpec( 2, "OFLU", "", "", categoryFiles, "Flush output stream.  If it is attached to a disk file, this will force any data in the buffer to be written to disk."),
	OpSpec( 3, "IOPE", "is", "directory filename", categoryFiles, "Sets the file for the input stream of the current virtual machine - there is a virtual machine for each agent, so this is much the same as setting it for @#OWNR@.  Use @#INNL@, @#INNI@ and @#INNF@ to get data from the stream, and @#INOK@ to check validity of the stream.  The filename should include any file extension.<p>You should use @#FILE ICLO@ to close the file, although this happens automatically if you set a new file, if the virtual machine is destroyed, or if the engine exits.<p>Directory is 0 for the current world's journal directory, or 1 for the main journal directory."),
	OpSpec( 4, "ICLO", "", "", categoryFiles, "Disconnects anything which is attached to the input stream.  If this is a file, then the file is closed."),
	OpSpec( 5, "JDEL", "is", "directory filename", categoryFiles, "This deletes the file (filename) specified from the journal directory specified. If directory is zero, this is the current world's journal directory, otherwise it is the main journal directory.  It deletes the file immediately, rather than marking it for the attic."),
};

OpSpec ourSubCommandTable_GENE[] =
{
	OpSpec( 0, "CROS", "aiaiaiiiii", "child_agent child_slot mum_agent mum_slot dad_agent dad_slot mum_chance_of_mutation mum_degree_of_mutation dad_chance_of_mutation dad_degree_of_mutation.", categoryGenetics, "Crosses two genomes with mutation, and fills in a child geneme slot.  Mutation variables may be in the range of 0 to 255."),
	OpSpec( 1, "MOVE", "aiai", "dest_agent dest_slot source_agent source_slot", categoryGenetics, "Moves a genome from one slot to another."),
	OpSpec( 2, "KILL", "ai", "agent slot", categoryGenetics, "Clears a genome slot."),
	OpSpec( 3, "LOAD", "ais", "agent slot gene_file", categoryGenetics, "Loads an engineered gene file into a slot.  Slot 0 is a special slot used only for creatures, and contains the moniker they express.  Only the @#NEW: CREA@ command fills it in.  Other slot numbers are used in pregnant creatures, in eggs, or to temporarily store a genome before expressing it with @#NEW: CREA@.  You can use them as general purpose genome stores.<p>The gene file can have any name, and is loaded from the main genetics file.  A new moniker is generated, and a copy of the gene file put in the world directory. You can use * and ? wildcards in the name, and a random matching file will be used.<p>You can also load monikered files from the world genetics directory with this command.  If so, the file is copied and a new moniker generated.  Wildcards are matched first in the main genetics directory, and only if none match is the world genetics directory searched."),
	OpSpec( 4, "CLON", "aiai", "dest_agent dest_slot source_agent source_slot", categoryGenetics, "Clones a genome, creating a new moniker and copying the genetics file."),
};

OpSpec ourSubCommandTable_PRAY[] =
{
	OpSpec( 0, "REFR", "","",categoryResources,"This command refreshes the engine's view of the Resource directory. Execute this if you have reason to believe that the files in the directory may have changed. This forces a @#PRAY GARB@ to happen automatically"),
	OpSpec( 1, "GARB", "i","force",categoryResources,"This command clears the manager's cached resource data. Execute this after a lot of resource accesses (E.g. installing an agent) to clean up the memory used during the process. If you don't do this, excess memory can be held for a while, If the parameter is zero (the most usual) then the manager will only forget resources which are not in use at the moment. If force is non-zero, then the manager will forget all the previously loaded resources. As the resources currently in use go out of scope, they are automatically garbage collected."),
};

OpSpec ourSubIntegerRVTable_PRAY[] =
{
	OpSpec( 0,"COUN", "s","resource_type",categoryResources,"This returns the number of resource chunks which are tagged with the resource type passed in. Resource types are four characters only. Anything over that length will be silently truncated."),
	OpSpec( 1,"AGTI", "ssi","resource_name integer_tag default_value",categoryResources,"This returns the value of the integer tag associated with the named resource. If the resource does not contain such a tag, then the default value specified is returned. This call pairs with @#PRAY AGTS@."),
	OpSpec( 2,"DEPS", "si","resource_name do_install",categoryResources,"This performs a scan of the specified resource, and checks out the dependency data. The primary use for this would be in the preparation for injection of agents. If you pass zero in the do_install parameter, then the dependencies are only checked. If do_install is non-zero, then they are installed also. The return values are as follows:<br>0 = Success<br>-1 = Agent Type not found<br>-2 = Dependency Count not found<br>-3 to -(2 + count) is the dependency string missing<br>-(3+count) to -(2+2*count) is the dependency type missing<br>2*count to 3*count is the category ID for that dependency being invalid<br>1 to count is the dependency failing"),
	OpSpec( 3,"FILE", "sii","resource_name resource_type do_install",categoryResources,"This performs the \"installation\" of one file from the resource files. The resource_type is defined in the agent resource guide. If do_install is zero, the command simply checks if the file install should succeed. Return value is 0 for success, 1 for error."),
	OpSpec( 4,"TEST", "s","resource_name",categoryResources,"This checks for the existence of a chunk, returning zero if it is not found, and a value from 1-3 indicating the cost to load if it is.<br>Return values are currently defined as:<br>0 - Chunk not available at this time<br>1 - Chunk Available, Cached and ready for use<br>2 - Chunk available, on disk uncompressed and fine for loading<br>3 - Chunk available, on disk compressed and ready for loading. <p>Thus the return value could be thought of as the cost of loading, where 1 is no cost, and 3 is high cost."),
	OpSpec( 5,"INJT", "siv","resource_name do_install report_var",categoryResources,"This command injects an agent. The agent must be in the chunk named. If do_install is zero, the command simply checks for the presence of the required scripts and dependencies. If non-zero, it attempts to inject the agent. The report var is a string variable, and is set to the name of the offending script if the injection/check fails. <br>Return is 0 for success, -1 for \"Script not found\" and if injecting, -2 for \"Injection failed\". <br>Return value -3 indicates that a dependency evaluation failed, and in this case, the report var is the return code from @#PRAY DEPS@"),
	OpSpec( 6,"SIZE", "s","resource_name", categoryResources, "The return value for this is the size of the chunk. This can be used to determine chunk information for decisions on time criteria. E.g. a large compressed chunk will take a short while to decompress."),
	OpSpec( 7,"EXPO", "s","chunk_name", categoryResources, "This function exports the target creature. If the creature is exported successfully then it has been removed from the world. Returns value is one of the following:<br>0 for success<br>1 if the creature, or if pregnant any of its offspring, are already on disk in some form.<p>The chunk name should be used to find the creature again to import it.  In Creatures 3, most exported creatures have a chunk name EXPC, and the starter family uses SFAM."),
	OpSpec( 8,"IMPO", "sii","moniker actually_do_it keep_file", categoryResources, "This function imports the creature with the requested moniker. Returns one of the following codes:<br>0 - success<br>1 - couldn't reconcile histories so creature was cloned<br>2 - moniker not found in PRAY system<br>3 - related genome files could not be loaded<p>Set actually_do_it to 1 to try and perform the import, or 0 to perform a query giving just the return value.  You can use the query to test if the creature is available, and if the creature would have to be cloned upon importing, and warn the user.  The new creature is @#TARG@etted after import.  If you set keep file to 1, then the exported file won't be deleted (moved to the porch)."),
	OpSpec( 9,"MAKE", "isisv", "which_journal_spot journal_name which_pray_spot pray_name report_destination", categoryResources, "<b>Please see the documentation accompanying the praybuilder on CDN</b><p>Suffice it to say: return value is zero for success, otherwise non-zero, and report is set to the praybuilder output for you<p>Also, the which_journal_spot is zero for world journal, 1 for global journal. Also the which_pray_spot is zero for \"My Agents\" and 1 for \"My Creatures\""),
};

OpSpec ourSubStringRVTable_PRAY[] =
{
	OpSpec( 0,"PREV", "ss","resource_type last_known",categoryResources,"This returns the name of the resource chunk directly before the named one, given that they are of the same type. If the named resource cannot be found in the list of resources of the type specified, then the first resource of that type is returned. This call pairs with @#PRAY NEXT@."),
	OpSpec( 1,"NEXT", "ss","resource_type last_known",categoryResources,"This returns the name of the resource chunk directly after the named one, given that they are of the same type. If the named resource cannot be found in the list of resources of the type specified, then the last resource of that type is returned. This call pairs with @#PRAY PREV@."),
	OpSpec( 2,"AGTS", "sss","resource_name string_tag default_value",categoryResources,"This returns the value of the string tag associated with the named resource. If the resource does not contain such a tag, then the default value specified is returned. This call pairs with @#PRAY AGTI@."),
};

OpSpec ourSubCommandTable_NEW[] =
{
	OpSpec( 0,"SIMP", "iiisiii", "family genus species sprite_file image_count first_image plane", categoryAgents, "Create a new simple agent, using the specified sprite file. The agent will have image_count sprites available, starting at first_image in the file. The plane is the screen depth to show the agent at - the higher the number, the nearer the camera."),
	OpSpec( 1,"COMP", "iiisiii", "family genus species sprite_file image_count first_image plane", categoryCompound, "Create a new compound agent. The sprite file is for the first part, which is made automatically.  Similarly, image_count and first_image are for that first part.  The plane is the absolute plane of part 1 - the planes of other parts are relative to the first part." ),
	OpSpec( 2,"VHCL", "iiisiii", "family genus species sprite_file image_count first_image plane", categoryVehicles, "Create a new vehicle.  Parameters are the same as @#NEW: COMP@." ),
	OpSpec( 3,"CREA", "iaiii", "family gene_agent gene_slot sex variant", categoryCreatures, "Makes a creature using the genome from the given gene slot in another agent.  You'll want to use @#GENE CROS@ or @#GENE LOAD@ to fill that slot in first.  The gene slot is cleared, as control of that genome is moved to the special slot 0 of the new creature, where it is expressed.  Sex is 1 for male, 2 for female or 0 for random.  The variant can also be 0 for a random value between 1 and 8.  See also @#NEWC@."),
};


OpSpec ourSubCommandTable_MESG[] =
{
	OpSpec( 0,"WRIT", "ai", "agent message_id", categoryAgents, "Send a message to another agent.  The message_id is from the table of @#Message Numbers@; remember that early @#Message Numbers@ differ slightly from @#Script Numbers@.  If used from an install script, then @#FROM@ for the message to @#NULL@ rather than @#OWNR@." ),
	OpSpec( 1,"WRT+", "aimmi", "agent message_id param_1 param_2 delay", categoryAgents, "Send a message with parameters to another agent.  Waits delay ticks before sending the message.  The message_id is from the table of @#Message Numbers@." ),
};

OpSpec ourSubCommandTable_STIM[] =
{
	OpSpec( 0,"SHOU", "if", "stimulus strength", categoryCreatures, "Shout a stimulus to all creatures who can hear @#OWNR@.  The strength is a multiplier for the stimulus.  Set to 1 for a default stimulation, 2 for a stronger stimulation and so on.  It is important you use this, rather than send several stims, as it affects learning.  Set strength to 0 to prevent learning altogether, and send a strength 1 chemical change.  See the table of @#Stimulus Numbers@." ),
	OpSpec( 1,"SIGN", "if", "stimulus strength", categoryCreatures, "Send a stimulus to all creatures who can see @#OWNR@." ),
	OpSpec( 2,"TACT", "if", "stimulus strength", categoryCreatures, "Send a stimulus to all creatures who are touching @#OWNR@." ),
	OpSpec( 3,"WRIT", "aif", "creature stimulus strength", categoryCreatures, "Send stimulus to a specific creature.  Can be used from an install script, but the stimulus will be from @#NULL@, so the creature will react but not learn." ),
};

OpSpec ourSubCommandTable_URGE[] =
{
	OpSpec( 0,"SHOU", "fif", "noun_stim verb_id verb_stim", categoryCreatures, "Urge all creatures who can hear @#OWNR@ to perform the verb_id action on @#OWNR@.  Stimuli can range from -1 to 1, ranging from discourage to encourage." ),
	OpSpec( 1,"SIGN", "fif", "noun_stim verb_id verb_stim", categoryCreatures, "Urge all creatures who can see @#OWNR@ to perform an action on @#OWNR@."),
	OpSpec( 2,"TACT", "fif", "noun_stim verb_id verb_stim", categoryCreatures, "Urge all creatures who are touching @#OWNR@ to perform an action on @#OWNR@."),
	OpSpec( 3,"WRIT", "aifif", "creature noun_id noun_stim verb_id verb_stim", categoryCreatures, "Urge a specific creature to perform a specific action on a specific noun.  A stimulus greater than 1 will <i>force</i> the Creature to perform an action, or to set its attention (mind control!).  Use an id -1 and stim greater than 1 to unforce it."),
};

OpSpec ourSubCommandTable_SWAY[] =
{
	OpSpec( 0,"SHOU", "ifififif", "drive adjust drive adjust drive adjust drive adjust", categoryCreatures, "Stimulate all creatures that can hear @#OWNR@ to adjust four drives by the given amounts."),
	OpSpec( 1,"SIGN", "ifififif", "drive adjust drive adjust drive adjust drive adjust", categoryCreatures, "Stimulate all creatures that can see @#OWNR@ to adjust four drives by the given amounts."),
	OpSpec( 2,"TACT", "ifififif", "drive adjust drive adjust drive adjust drive adjust", categoryCreatures, "Stimulate all creatures that are touching @#OWNR@ to adjust four drives by the given amounts."),
	OpSpec( 3,"WRIT", "aifififif", "creature drive adjust drive adjust drive adjust drive adjust", categoryCreatures, "Stimulate a specific creature to adjust four drives by the given amounts."),
};

OpSpec ourSubCommandTable_ORDR[] =
{
	OpSpec( 0,"SHOU", "s", "speech", categoryCreatures, "Sends a spoken command from target to all creatures that can hear it." ),
	OpSpec( 1,"SIGN", "s", "speech", categoryCreatures, "Sends a spoken command from target to all creatures that can see it." ),
	OpSpec( 2,"TACT", "s", "speech", categoryCreatures, "Sends a spoken command from target to all creatures that are touching it." ),
	OpSpec( 3,"WRIT", "as", "creature speech", categoryCreatures, "Sends a spoken command from target to the specified creature." ),
};

OpSpec ourSubCommandTable_GIDS[] =
{
	OpSpec( 0,"ROOT", "", "", categoryScripts, "Output the family numbers for which there are scripts in the scriptorium.  List is space delimited." ),
	OpSpec( 1,"FMLY", "i", "family", categoryScripts, "Output the genus numbers for which there are scripts in the scriptorium for the given family.  List is space delimited." ),
	OpSpec( 2,"GNUS", "ii", "family genus", categoryScripts, "Output the species numbers for which there are scripts in the scriptorium for the given family and genus.  List is space delimited." ),
	OpSpec( 3,"SPCS", "iii", "family genus species", categoryScripts, "Output the event numbers of scripts in the scriptorium for the given classifier.  List is space delimited." ),
};

// PRT: subcommands:
OpSpec ourSubCommandTable_PRT[] =
{
	OpSpec( 0,"INEW", "issiii", "id name description x y message_num", categoryPorts, "Create a new input port on target. You should number input port ids starting at 0.  The message_num is the message that will be sent to the agent when a signal comes in through the input port. _P1_ of that message will contain the data value of the signal. The position of the port, relative to the agent, is given by x, y." ),
	OpSpec( 1,"IZAP", "i", "id", categoryPorts, "Remove the specified input port." ),
	OpSpec( 2,"ONEW", "issii", "id name description x y", categoryPorts, "Create a new output port on target. You should number input port ids starting at 0.  The port's relative position is given by x, y." ),
	OpSpec( 3,"OZAP", "i", "id", categoryPorts, "Remove the specified output port." ),
	OpSpec( 4,"JOIN", "aiai", "source_agent output_id dest_agent input_id", categoryPorts, "Connect an output port on the source agent to an input port on the destination. An input may only be connected to one output at at time, but an output may feed any number of inputs." ),
	OpSpec( 5,"SEND", "im", "id data", categoryPorts, "Send a signal from the specified output port to all connected inputs.  The data can be any integer."),
	OpSpec( 6,"BANG", "i", "bang_strength", categoryPorts, "Breaks connections randomly with other machines (as if the machine had been 'banged'. Use a bang_strength of 100 to disconnect all ports, 50 to disconnect about half etc."),
	OpSpec( 7,"KRAK", "aii", "agent in_or_out port_index", categoryPorts, "Breaks a specific connection on a machine. If in_or_out is zero, it is an input port whose connection is broken, if it is an output port, then all inputs are disconnected."),
};

OpSpec ourSubIntegerRVTable_PRT[] =
{
	OpSpec( 0,"ITOT", "", "", categoryPorts, "Returns the number of input ports, assuming they are indexed sequentially."),
	OpSpec( 1,"OTOT", "", "", categoryPorts, "Returns the number of output ports, assuming they are indexed sequentially." ),
	OpSpec( 2,"FROM", "i", "inputport", categoryPorts, "Returns the output port index on the source agent, feeding that input port on the @#TARG@ agent.<br>Return values are -ve for error."),
};

OpSpec ourSubStringRVTable_PRT[] =
{
	OpSpec( 0,"NAME", "aii", "agent in_or_out port_index", categoryPorts, "Returns the name of the indexed port (input port if in_or_out is zero, output port if non-zero) on the specified agent. Returns \"\" in error."),
};

OpSpec ourSubAgentRVTable_PRT[] =
{
	OpSpec( 0,"FRMA", "i", "inputport", categoryPorts, "Returns the agent from which the input port is fed. Returns NULLHANDLE if that port does not exist, or is not connected."),
};

// compoundpart subcommands
OpSpec ourSubCommandTable_PAT[] =
{
	OpSpec( 0,"DULL", "isiddi", "part_id sprite_file first_image rel_x rel_y rel_plane", categoryCompound, "Create a dull part for a compound agent.  A dull part does nothing except show an image from the given sprite file.  You should number part ids starting at 1, as part 0 is automatically made when the agent is made.  The dull part's position is relative to part 0, as is its plane.  Use @#PART@ to select it before you change @#POSE@ or @#ANIM@, or use various other commands."),	
	OpSpec( 1,"BUTT", "isiiddibii", "part_id sprite_file first_image image_count rel_x rel_y rel_plane anim_hover message_id option", categoryCompound, "Create a button on a compound agent.  anim_hover is an animation, as in the @#ANIM@ command, to use when the mouse is over the button - when the mouse is moved off, it returns to any previous animation that was going.  message_id is sent when the button is clicked.  option is 0 for the mouse to hit anywhere in the bounding box, 1 to hit only non-transparent pixels.<br>@#_P1_@ of the message is set to the part number of the buttons allowing you to overload your messages by button group and then switch on input value in the script."),
	OpSpec( 2,"TEXT", "isiddiis","part_id sprite_file first_image rel_x rel_y rel_plane message_id font_sprite", categoryCompound, "Creates a text entry part.  Gains the focus when you click on it, or with the @#FCUS@ command.  Sends the message_id when tab or return are pressed - a good place to use @#PTXT@ to get the text out, and to set the focus elsewhere." ),
	OpSpec( 3,"FIXD", "isiddis", "part_id sprite_file first_image rel_x rel_y rel_plane font_sprite", categoryCompound, "Create a fixed text part. The text is wrapped on top of the supplied gallery image. new-line characters may be used.  Use @#PTXT@ to set the text." ),
	OpSpec( 4,"CMRA", "isiddiiiii","part_id overlay_sprite baseimage relx rely relplane viewWidth viewHeight cameraWidth cameraHeight",categoryCompound, "Create a camera with possible overlay sprite whose name may be blank.  Use @#SCAM@ to change the camera's view."),
	OpSpec( 5,"GRPH", "isiddii","part_id overlay_sprite baseimage relx rely relplane numValues",categoryCompound, "Creates a graph part on a compound agent. Use @#GRPL@ to add a line to the graph and @#GRPV@ to add a value to a graph line."),
	OpSpec( 6,"KILL", "i", "part_id", categoryCompound, "Destroys the specified part of a compound agent.  You can't destroy part 0."),
};


OpSpec ourSubCommandTable_BRN[] =
{
	OpSpec( 0,"SETN", "iiif", "lobe_number neuron_number state_number new_value", categoryBrain, "Sets a neuron weight." ),
	OpSpec( 1,"SETD", "iiif", "tract_number dendrite_number weight_number new_value", categoryBrain, "Sets a dendrite weight."),
	OpSpec( 2,"SETL", "iif", "lobe_number line_number new_value", categoryBrain, "Sets a lobe's SV rule float value." ),
	OpSpec( 3,"SETT", "iif", "tract_number line_number new_value", categoryBrain, "Sets a tract's SV rule float value." ),
	OpSpec( 4,"DMPB", "", "", categoryBrain, "Dumps the sizes of the binary data dumps for current lobes and tracts." ),
	OpSpec( 5,"DMPL", "i", "lobe_number", categoryBrain, "Dumps a lobe as binary data." ),
	OpSpec( 6,"DMPT", "i", "tract_number", categoryBrain, "Dumps a tract as binary data." ),
	OpSpec( 7,"DMPN", "ii", "lobe_number neuron_number", categoryBrain, "Dumps a neuron as binary data." ),
	OpSpec( 8,"DMPD", "ii", "tract_number dendrite_number", categoryBrain, "Dumps a dendrite as binary data." ),
};

OpSpec ourSubCommandTable_DBG[] =
{
	OpSpec( 0,"PAWS", "", "", categoryDebug, "This pauses everything in the game. No game driven ticks will occur until a @#DBG: PLAY@ command is issued, so this command is only useful for debugging.  Use @#PAUS@ for pausing of specific agents, which you can use to implement a pause button." ),
	OpSpec( 1,"PLAY", "", "", categoryDebug, "This command undoes a previously given @#DBG: PAWS@ and allows game time to flow as normal." ),
	OpSpec( 2,"TOCK", "", "", categoryDebug, "This command forces a tick to occur. It is useful in external apps to drive the game according to a different clock instead of the game clock." ),
	OpSpec( 3,"FLSH", "", "", categoryDebug, "This flushes the system's input buffers - usually only useful if @#DBG: PAWS@ed." ),	
	OpSpec( 4,"POLL", "", "", categoryDebug, "This takes all of the @#DBG: OUTV@ and @#DBG: OUTS@ output to date and writes it to the output stream." ),
	OpSpec( 5,"OUTV", "d", "value", categoryDebug, "Send a number to the debug log - use @#DBG: POLL@ to retrieve."),
	OpSpec( 6,"OUTS", "s", "value", categoryDebug, "Send a string to the debug log - use @#DBG: POLL@ to retrieve."),
	OpSpec( 7,"PROF", "", "", categoryDebug, "Sends agent profile information to the output stream.  This gives you data about the time the engine spends running the update and message handling code for each classifier.  The data is measured from engine startup, or the point marked with @#DBG: CPRO@.  It's output in comma separated value (CSV) format, so you can load it into a spreadsheet for sorting and summing." ),
	OpSpec( 8,"CPRO", "", "", categoryDebug, "Clears agent profiling information.  Measurements output with @#DBG: PROF@ start here."),
	OpSpec( 9,"HTML", "i", "sort_order", categoryDebug, "Sends CAOS documentation to the output stream.  Sort order is 0 for alphabetical, 1 for categorical."),
	OpSpec( 10,"ASRT", "c", "condition", categoryDebug, "Confirms that a condition is true.  If it isn't, it displays a runtime error dialog."),
	OpSpec( 11, "WTIK", "i", "new_world_tick", categoryDebug, "Changes the world tick @#WTIK@ to the given value.  This should only be used for debugging, as it will potentially leave confusing information in the creature history, and change the time when delayed messages are processed.  Its main use is to jump to different seasons and times of day."),
	OpSpec( 12, "TACK", "a", "follow", categoryDebug, "Pauses the game when the given agent next executes a single line of CAOS code.  This pause is mid-tick, and awaits incoming requests, or the pause key.  Either another DBG: TACK or a @#DBG: PLAY@ command will make the engine carry on.  Any other incoming requests will be processed as normal.  However, the virtual machine of the tacking agent is effectively in mid-processing, so some CAOS commands may cause unpredictable results, and even crash the engine.  In particular, you shouldn't @#KILL@ the tacking agent.  You can see which agent is being tracked with @#TACK@."),
};

OpSpec ourCommandTable[] =
{
	// commands that need special handling...
	OpSpec( "DOIF", GeneralHandlers::Command_DOIF, "-c", (int)specialDOIF, "condition", categoryFlow, "Execute a block of code if the condition is true.  The code block ends at the next @#ELSE@, @#ELIF@ or @#ENDI@.  A condition is composed of one or more comparisons joined by AND or OR.  A comparison compares two values with EQ, NE, GT, GE, LT, LE, or alternatively =, &lt;&gt;, &gt;, &gt;=, &lt;, &lt;=.<br><br>DOIF ov00 GE 5 AND ov00 LT 10<br>--- code block 1 ---<br>ELIF ov00 GE 10 OR ov00 LT 100<br>--- code block 2 ---<br>ELSE<br>--- code block 3 ---<br>ENDI<br><br>Conditions are evaluated simply from left to right, so \"a AND b OR c\" is the same as \"(a AND b) OR c\", not \"a AND ( b OR c )\".<p>Conditional statements may not work correctly with commands overloaded by rvalue." ),
	OpSpec( "ELIF", GeneralHandlers::Command_ELIF, "-c", (int)specialELIF, "condition", categoryFlow, "ELseIF command to follow a @#DOIF@. If the condition in a DOIF is false, each following ELIF command will be evaluated in turn.  Only the first true condition will have its code block executed." ),
	OpSpec( "ELSE", GeneralHandlers::Command_ELSE, "-", (int)specialELSE, "", categoryFlow, "ELSE clause to follow @#DOIF@ and @#ELIF@(s). If nothing else matches, the ELSE block will be executed." ),
	OpSpec( "ENDI", GeneralHandlers::Command_ENDI, "", (int)specialENDI, "", categoryFlow, "Closes a @#DOIF@...@#ELIF@...@#ELSE@... set." ),
	OpSpec( "ENUM", GeneralHandlers::Command_ENUM, "-iii", (int)specialENUM, "family genus species", categoryAgents, "Iterate through each agent which conforms to the given classification, setting @#TARG@ to point to each valid agent in turn. family, genus and/or species can be zero to act as wildcards.  @#NEXT@ terminates the block of code which is executed with each TARG.  After an ENUM, TARG is set to @#OWNR@." ),
	OpSpec( "ESEE", GeneralHandlers::Command_ESEE, "-iii", (int)specialESEE, "family genus species", categoryAgents, "As @#ENUM@, except only enumerates through agents which @#OWNR@ can see.  An agent can see another if it is within @#RNGE@, its @#PERM@ allows it to see through all intervening walls, and for creatures @#ATTR@ @#Invisible@ isn't set.  See also @#STAR@ and @#SEEE@.  In install scripts, when there is no @#OWNR@, @#TARG@ is used instead." ),
	OpSpec( "ETCH", GeneralHandlers::Command_ETCH, "-iii", (int)specialETCH, "family genus species", categoryAgents, "As @#ENUM@, except only enumerates through agents which @#OWNR@ is touching.  Agents are said to be touching if their bounding rectangles overlap.  See also @#TTAR@.  In install scripts, when there is no @#OWNR@, @#TARG@ is used instead." ),
	OpSpec( "EPAS", AgentHandlers::Command_EPAS, "-iii", (int)specialEPAS, "family genus species", categoryVehicles, "Enumerate over owner vehicle's passengers which match the given classifier.  Similar to @#ENUM@."),
	OpSpec( "NEXT", GeneralHandlers::Command_NEXT, "", (int)specialNEXT, "", categoryAgents, "Closes an enumeration loop.  The loop can begin with @#ENUM@, @#ESEE@, @#ETCH@ or @#EPAS@." ),
	OpSpec( "REPS", GeneralHandlers::Command_REPS, "i", (int)specialREPS, "count", categoryFlow, "Loop through a block of code a number of times. Must have a matching @#REPE@ command to close the block." ),
	OpSpec( "REPE", GeneralHandlers::Command_REPE, "", (int)specialREPE, "", categoryFlow, "Closes a @#REPS@ loop." ),
	OpSpec( "LOOP", GeneralHandlers::Command_LOOP, "", (int)specialLOOP, "", categoryFlow, "Begin a LOOP..@#UNTL@ or LOOP..@#EVER@ loop." ),
	OpSpec( "UNTL", GeneralHandlers::Command_UNTL, "c", (int)specialUNTL, "condition", categoryFlow, "Forms the end of a @#LOOP@..UNTL loop. The loop will execute until the condition is met.  See @#DOIF@ for information on the form of the condition." ),
	OpSpec( "EVER", GeneralHandlers::Command_EVER, "", (int)specialEVER, "", categoryFlow, "Forms the end of a @#LOOP@..EVER loop, which just loops forever." ),
	OpSpec( "SUBR", GeneralHandlers::Command_SUBR, "", (int)specialSUBR, "", categoryFlow, "Defines the start of a subroutine. Specify a label after the @#SUBR@ command - the label is case sensitive, and should start with a letter. If this instruction is hit during normal program flow, it works as a @#STOP@ instruction.  See @#GSUB@ and @#RETN@." ),
	OpSpec( "ECON", PortHandlers::Command_ECON, "-a", (int)specialECON, "agent", categoryPorts, "Starts an enumeration across all the agents in a connective system, where agent is any agent within the connective system."),

	// standard commands...
	OpSpec( "GOTO", GeneralHandlers::Command_GOTO, "#", "destination", categoryFlow, "Don't use this command.  It jumps directly to a label defined by @#SUBR@. This command is only here because it is used implicitly by @#DOIF@ blocks. This is a really dangerous command to use manually, because if you jump out of a block of code (eg a @#LOOP@...@#EVER@ block), the stack frame will no longer be correct, and the script will most likely crash. Don't use it!  See @#SUBR@." ),
	OpSpec( "STOP", GeneralHandlers::Command_STOP, "", "", categoryScripts, "Stops running the current script.  Compare @#STPT@." ),
	OpSpec( "GSUB", GeneralHandlers::Command_GSUB, "#", "destination", categoryFlow, "Jumps to a subroutine defined by @#SUBR@. Execution will continue at the instruction after the GSUB when the subroutine hits a @#RETN@ command." ),
	OpSpec( "RETN", GeneralHandlers::Command_RETN, "", "", categoryFlow, "Return from subroutine. Do not use this instruction from inside a block of code (eg a @#LOOP@#..@#EVER@ or @#ENUM@...@#NEXT@ etc...)!  See @#SUBR@ and @#GSUB@." ),
	OpSpec( "NEW:", AgentHandlers::Command_NEW, idSubCommandTable_NEW, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "TARG", CAOSMachine::Command_TARG, "a", "agent", categoryVariables, "This sets the TARG variable to the agent specified." ),
	OpSpec( "KILL", AgentHandlers::Command_KILL, "a", "agent", categoryAgents, "Destroys an agent.  The pointer won't be destroyed.  For creatures, you probably want to use @#DEAD@ first." ),
	OpSpec( "TICK", AgentHandlers::Command_TICK, "i", "tick_rate", categoryAgents, "Start agent timer, calling @#Timer@ script every tick_rate ticks.  Set to 0 to turn off the timer." ),
	OpSpec( "ADDV", GeneralHandlers::Command_ADDV, "vd", "var sum", categoryVariables, "Adds two integers or floats, so var = var + sum." ),
	OpSpec( "SUBV", GeneralHandlers::Command_SUBV, "vd", "var sub", categoryVariables, "Subtracts an integer or float from a variable, so var = var - sub." ),
	OpSpec( "MULV", GeneralHandlers::Command_MULV, "vd", "var mul", categoryVariables, "Multiplies a variable by an integer or float, so var = var * mul." ),
	OpSpec( "DIVV", GeneralHandlers::Command_DIVV, "vd", "var div", categoryVariables, "Divides a variable by an integer or float, so var = var / div.  Uses integer division if both numbers are integers, or floating point division otherwise." ),
	OpSpec( "MODV", GeneralHandlers::Command_MODV, "vi", "var mod", categoryVariables, "Gives the remainder (or modulus) when a variable is divided by an integer, so var = var % mod.  Both values should to be integers."),
	OpSpec( "NEGV", GeneralHandlers::Command_NEGV, "v", "var", categoryVariables, "Reverse the sign of the given integer or float variable, so var = 0 - var." ),
	OpSpec( "ANDV", GeneralHandlers::Command_ANDV, "vi", "var value", categoryVariables, "Peform a bitwise AND on an integer variable, so var = var & value." ),
	OpSpec( "ORRV", GeneralHandlers::Command_ORRV, "vi", "var value", categoryVariables, "Peform a bitwise OR on an integer variable, so var = var | value." ),
	OpSpec( "STIM", CreatureHandlers::Command_STIM, idSubCommandTable_STIM, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "SETV", GeneralHandlers::Command_SETV, "vd", "var value", categoryVariables, "Stores an integer or float in a variable." ),
	OpSpec( "STAR", AgentHandlers::Command_STAR, "iii" ,"family genus species", categoryAgents, "Randomly chooses an agent which matches the given classifier and can be seen by the owner of the script. It then sets @#TARG@ to that agent.  See @#ESEE@ for an explanation of seeing."),
	OpSpec( "RTAR", AgentHandlers::Command_RTAR, "iii", "family genus species", categoryAgents, "Randomly chooses an agent which matches the given classifier, and targets it." ),
	OpSpec( "INST", GeneralHandlers::Command_INST, "", "", categoryScripts, "This command indicates that the following commands should execute in a single tick - ie the script cannot be interrupted by the script 'scheduler'. This can be important for certain tasks which might leave an agent in an undefined (and dangerous) state if interrupted. The INST state is broken either manually, using a @#SLOW@ command, or implictly, if a blocking instruction is encountered (eg @#WAIT@). Blocking instructions force the remainder of the script's timeslice to be discarded." ),
	OpSpec( "SLOW", GeneralHandlers::Command_SLOW, "", "", categoryScripts, "Turn off @#INST@ state." ),
	OpSpec( "TTAR", AgentHandlers::Command_TTAR, "iii" , "family genus species", categoryAgents, "Randomly chooses an agent which matches the given classifier and is touching the owner of the script. It then sets @#TARG@ to that agent.  See @#ETCH@."),
	OpSpec( "WAIT", GeneralHandlers::Command_WAIT, "i", "ticks", categoryScripts, "Block the script for the specified number of ticks. This command does an implicit @#SLOW@." ),
	OpSpec( "ANIM", AgentHandlers::Command_ANIM, "b", "pose_list", categoryAgents, "Specify a list of @#POSE@s such as [1 2 3] to animate the current agent/part.  Put 255 at the end to continually loop.  The first number after the 255 is an index into the animation string where the looping restarts from - this defaults to 0 if not specified.  e.g. [0 1 2 10 11 12 255 3] would loop just the 10, 11, 12 section." ),
	OpSpec( "OVER", AgentHandlers::Command_OVER, "", "", categoryAgents, "Wait until the current agent/part's @#ANIM@ation is over before continuing.  Looping anims stop this command terminating until the animation is changed to a non-looping one." ),
	OpSpec( "POSE", AgentHandlers::Command_POSE, "i", "pose", categoryAgents, "Specify a frame in the sprite file for the target agent/part.  Relative to any index specified by @#BASE@." ),
	OpSpec( "GAIT", CreatureHandlers::Command_GAIT, "i", "gait_number", categoryAgents, "Specifies the current gait for a creature. The gaits are genetically defined. It sets the gait of the creature agent stored in @#TARG@." ),
	OpSpec( "BASE", AgentHandlers::Command_BASE, "i", "index", categoryAgents, "Set the base image for this agent or part.  The index is relative to the first_image specified in the NEW: command.  Future @#POSE@/@#ANIM@ commands and any @#ANIM@ in progress are relative to this new base."),
	OpSpec( "MVTO", AgentHandlers::Command_MVTO, "ff", "x y", categoryMotion, "Move the top left corner of the target agent to the given world coordinates.  Use @#MVFT@ instead to move creatures."),
	OpSpec( "MVSF", AgentHandlers::Command_MVSF, "ff", "x y", categoryMotion, "Move the target agent into a safe map location somewhere in the vicinity of x, y. Only works on autonomous agents - see @#MOVS@.  Works like a safe @#MVFT@ for creatures."),
	OpSpec( "MVBY", AgentHandlers::Command_MVBY, "ff", "delta_x delta_y", categoryMotion, "Move the target agent by relative distances, which can be negative or positive."),
	OpSpec( "MVFT", CreatureHandlers::Command_MVFT, "ff", "x y", categoryCreatures, "Move creature's down foot to position x,y.  Use this instead of @#MVTO@ for creatures." ),
	OpSpec( "ABSV", GeneralHandlers::Command_ABSV, "v", "var", categoryVariables, "Set a variable to its absolute value, so if var is negative var = 0 - var, otherwise var is left alone."),
	OpSpec( "MESG", AgentHandlers::Command_MESG, idSubCommandTable_MESG, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "URGE", CreatureHandlers::Command_URGE, idSubCommandTable_URGE, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "PRT:", PortHandlers::Command_PRT, idSubCommandTable_PRT, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "DELR", MapHandlers::Command_DELR, "i", "room_id", categoryMap, "Deletes the specified room from the map." ),
	OpSpec( "DELM", MapHandlers::Command_DELM, "i", "metaroom_id", categoryMap, "Deletes the specified metaroom from the map."),	
	OpSpec( "PART", AgentHandlers::Command_PART, "i", "part_id", categoryCompound, "Sets the working part number.  Future command such as @#POSE@ and @#ANIM@, amongst others, act on that part of a compound agent."),
	OpSpec( "CABN", AgentHandlers::Command_CABN, "iiii", "left top right bottom", categoryVehicles, "Set a vehicles cabin rectangle.  The cabin is the area in which agents inside the vehicle are kept.  The rectangle is relative to the position of the vehicle.  Default cabin is the bounding rectangle of part 0.  You might want to use @#ATTR@ to set attribute @#Greedy Cabin@, on the vehicle.  This will make it automatically pick up items which are dropped in the cabin." ),
	OpSpec( "DPAS", AgentHandlers::Command_DPAS, "iii", "family genus species", categoryVehicles, "Drop all passengers matching classifier." ),
	OpSpec( "GPAS", AgentHandlers::Command_GPAS, "iiii", "family genus species rect_to_use", categoryVehicles, "Collect all nearby agents matching the classifier as passengers. <br>rect_to_use 0 : Nearby means touching bounding rectangle of agent<br>rect_to_use 1 : Nearby means touching cabin rectangle"),
	OpSpec( "SPAS", AgentHandlers::Command_SPAS, "aa", "vehicle new_passenger", categoryVehicles, "Specified vehicle picks up the specified passenger." ),
	OpSpec( "LOCI", CreatureHandlers::Command_LOCI,  "iiiif", "type organ tissue id new_value", categoryCreatures, "Sets a biochemical locus value.  See @#Receptor Locus Numbers@ and @#Emitter Locus Numbers@"),
	OpSpec( "RMSC", SoundHandlers::Command_RMSC, "iis", "x y track_name", categorySounds, "Associates a music track with the room at the specified coordinates.  This overrides any track specified for the metaroom that the room is in." ),
	OpSpec( "RCLR", SoundHandlers::Command_RCLR, "ii", "x y", categorySounds, "Clear the music for the room at the given location."  ),
	OpSpec( "SNDE", SoundHandlers::Command_SNDE, "s", "sound_file", categorySounds, "Play a sound effect audible as if emitted from target's current location." ),
	OpSpec( "SNDQ", SoundHandlers::Command_SNDQ, "si", "sound_file delay", categorySounds, "As @#SNDE@, only with a delay before playing." ),
	OpSpec( "SNDC", SoundHandlers::Command_SNDC, "s", "sound_file", categorySounds, "Plays a controlled sound effect emitted from the target.  Updates volume and panning as the agent moves." ),
	OpSpec( "SNDL", SoundHandlers::Command_SNDL, "s", "sound_file", categorySounds, "Play a sound effect as in @#SNDC@, only the sound is looped." ),
	OpSpec( "STPC", SoundHandlers::Command_STPC, "", "", categorySounds, "Stops a controlled sound." ),
	OpSpec( "FADE", SoundHandlers::Command_FADE, "", "", categorySounds, "Fade out a controlled sound." ),
	OpSpec( "CHEM", CreatureHandlers::Command_CHEM, "if", "chemical adjustment", categoryCreatures, "Adjusts chemical (0 to 255) by concentration -1.0 to +1.0 in the target creature's bloodstream." ),
	OpSpec( "INJR", CreatureHandlers::Command_INJR, "ii", "organ amount", categoryCreatures, "Injures an organ, -1 to randomly choose the organ, 0 for the body organ." ),
	OpSpec( "APPR", CreatureHandlers::Command_APPR, "", "", categoryCreatures, "Creature approaches the IT agent.  If there is no IT agent, the creature follows the CA smell to find an agent of that category.  The script resumes when it gets there, or if it can't get any further." ),
	OpSpec( "WALK", CreatureHandlers::Command_WALK, "", "", categoryCreatures, "Sets creature walking indefinitely. Chooses a walking gait according to chemo-receptors.  Always means ignore IT and walk in the current direction set by @#DIRN@."  ),
	OpSpec( "TOUC", CreatureHandlers::Command_TOUC, "", "", categoryCreatures, "Make creature reach out to touch the IT agent.  Blocks the script until the creature either reaches the agent, or it's fully stretched and still can't."),
	OpSpec( "SAYN", CreatureHandlers::Command_SAYN, "", "", categoryCreatures, "Creature expresses need, by speaking."),
	OpSpec( "LTCY", CreatureHandlers::Command_LTCY, "iii", "action min max", categoryCreatures, "Sets latency time on involuntary actions to a random value between min and max.  After an involuntary action occurs, the same action will not be able to kick in again until after that many ticks.  Min and max must range between 0 and 255. " ),
	OpSpec( "DREA", CreatureHandlers::Command_DREA, "i", "dream", categoryCreatures, "Set to 1 to make the creature fall asleep and dream, 0 to stop the creature dreaming.  When dreaming, a creature's instincts are processed.  See also @#ASLP@." ),
	OpSpec( "GIDS", GeneralHandlers::Command_GIDS, idSubCommandTable_GIDS, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "MATE", CreatureHandlers::Command_MATE, "", "", categoryCreatures, "Male creature mates with the IT agent - if IT is a female of the same genus!  The female doesn't need to be in reach.  If successful, the sperm is transmitted to the female and there is a chance of conception.  If pregnancy occurs, gene slot 1 of the mother contains the genome of the child."),
	OpSpec( "OUTS", GeneralHandlers::Command_OUTS, "s", "text", categoryFiles, "Sends a string to the output stream.  When running a script, the stream is null and this command does nothing.  For an externally injected command, the data from the stream is returned to the calling process.  For the @#CAOS@ command, the text is returned as a string.  You can use @#FILE OOPE@ to set the stream to a journal file."),
	OpSpec( "OUTX", GeneralHandlers::Command_OUTX, "s", "text", categoryFiles, "This sends the string <i>text</i> to the output stream. However it first transforms any escapes into quoted escapes, and it also quotes the entire string for you.<p>e.g.:<p>outx \"Moooose\\n\"<p>Would produce:<p>\"Moooose\\n\"<p>on the output stream instead of:<p>Moooose<br>"),
	OpSpec( "OUTV", GeneralHandlers::Command_OUTV, "d", "value", categoryFiles, "Similar to @#OUTS@ only outputs a float or integer as a decimal string." ),
	OpSpec( "DELG", GeneralHandlers::Command_DELG, "s", "variable_name", categoryVariables, "Deletes the specified @#GAME@ variable." ),
	OpSpec( "DIRN", CreatureHandlers::Command_DIRN, "i","direction", categoryCreatures, "Change creature to face a different direction. North 0, South 1, East 2, West 3." ),
	OpSpec( "SCRX", GeneralHandlers::Command_SCRX, "iiii", "family genus species event", categoryScripts, "Remove specified script from the scriptorium."),
	OpSpec( "LOCK", CAOSMachine::Command_LOCK, "", "", categoryScripts, "Prevent the current script being interrupted until @#UNLK@.  Normally, events other than timer scripts interrupt (abort) currently running scripts.  You can also use @#INST@ for similar, stronger protection." ),
	OpSpec( "UNLK", CAOSMachine::Command_UNLK, "", "", categoryScripts, "End the @#LOCK@ section."),
	OpSpec( "FRIC", AgentHandlers::Command_FRIC,  "i", "friction", categoryMotion, "Set physics friction percentage, normally from 0 to 100.  Speed is lost by this amount when an agent slides along the floor."),
	OpSpec( "SETS", GeneralHandlers::Command_SETS,  "vs", "var value", categoryVariables, "Sets a variable to a string value." ),
	OpSpec( "ADDS", GeneralHandlers::Command_ADDS,  "vs", "var append", categoryVariables, "Concatenates two strings, so var = var + append." ),
	OpSpec( "PAT:", CompoundHandlers::Command_PAT,  idSubCommandTable_PAT, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "DBG:", DebugHandlers::Command_DBG,  idSubCommandTable_DBG, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "MAPK", MapHandlers::Command_MAPK,  "", "", categoryMap, "Resets the map to be empty."),
	OpSpec( "DOOR", MapHandlers::Command_DOOR,  "iii", "room_id1 room_id2 permiability", categoryMap, "Sets the permiability of the door between two rooms.  This is used for both CAs and physical motion.  See also @#PERM@."),
	OpSpec( "MAPD", MapHandlers::Command_MAPD,  "ii", "width height", categoryMap, "Sets the dimensions of the map.  These are the maximum world coordinates.  Metarooms are rectangles within this area."),
	OpSpec( "ATTR", AgentHandlers::Command_ATTR,  "i", "attributes", categoryAgents, "Set attributes of target.  Sum the values in the @#Attribute Flags@ table to get the attribute value to pass into this command." ),
	OpSpec( "IMSK", AgentHandlers::Command_IMSK,  "i", "mask", categoryInput, "Set the input event mask.  Indicates which types of global input events the agent is interested in, if any. For example, if the flag for \"key up\" events is set here, the agents \"key up\" script will be run every time a key is released.<p>Input event bit flags are <br>1  @#Raw Key Down@<br>2 @#Raw Key Up@<br>4 @#Raw Mouse Move@<br>8 @#Raw Mouse Down@<br>16 @#Raw Mouse Up@<br>32 @#Raw Mouse Wheel@<br>64 @#Raw Translated Char@<p>You can find the script numbers executed by following the links above." ),
	OpSpec( "ACCG", AgentHandlers::Command_ACCG,  "f", "acceleration", categoryMotion, "Set acceleration due to gravity in pixels per tick squared." ),
	OpSpec( "ELAS", AgentHandlers::Command_ELAS,  "i", "elasticity", categoryMotion, "Set the elasticity percentage.  An agent with elasticity 100 will bounce perfectly, one with elasticity 0 won't bounce at all." ),
	OpSpec( "AERO", AgentHandlers::Command_AERO,  "i", "aerodynamics", categoryMotion, "Set aerodynamic factor as a percentage.  The velocity is reduced by this factor each tick." ),
	OpSpec( "CLAC", AgentHandlers::Command_CLAC,  "i", "message", categoryInput, "Set the click action, which is the identifier of the message sent to the agent when it is clicked on, provided attribute @#Activateable@ is set.  Default is activate 1.  Use -1 to prevent it sending a message.  Also overriden by @#CLIK@.  Remember that the early @#Message Numbers@ differ slightly from @#Script Numbers@." ),
	OpSpec( "SETA", GeneralHandlers::Command_SETA,  "va", "var value", categoryVariables, "Stores a reference to an agent in a variable." ),
	OpSpec( "ADDB", MapHandlers::Command_ADDB,  "is", "metaroom_id background_file", categoryMap, "Add a new background to the given metaroom.  Use @#BKGD@ to change the current displayed background."),	
	OpSpec( "META", MapHandlers::Command_META,  "iiii", "metaroom_id camera_x camera_y transition", categoryCamera, "Change the current camera (set with @#SCAM@) to a new meta room.  Moves the top left coordinate of the camera to the given coordinates.<p>Transition can be:<br>0 - no transition effect<br>1 - flip horizontally<br>2 - burst"),	
	OpSpec( "BKGD", MapHandlers::Command_BKGD,  "isi", "metaroom_id background transition", categoryCamera, "Change the current background displayed for the selected camera (with @#SCAM@).  Transition is as for @#META@.  The background must have been specified with the @#ADDM@ or @#ADDB@ command first."),
	OpSpec( "PERM", AgentHandlers::Command_PERM,  "i", "permiability", categoryMap, "Value from 1 to 100. Sets which room boundaries the agent can pass through.  The smaller the @#PERM@ the more it can go through.  @#DOOR@ sets the corresponding room boundary permiability.  Also used for @#ESEE@, to decide what it can see through."),
	OpSpec( "CMRA", DisplayHandlers::Command_CMRA,  "iii", "x y pan", categoryCamera, "Move current camera so top left corner of view is at world coordinate x y. Set pan 0 to jump straight to location, pan 1 to smoothly scroll there (unless in a different meta room)."),
	OpSpec( "CMRP", DisplayHandlers::Command_CMRP,  "iii", "x y pan", categoryCamera, "Centre current camera on world coordinate x y. Set pan 0 to jump straight to location, pan 1 to smoothly scroll there (unless in different meta room), and pan 2 to smoothly scroll only if the destination is already visible." ),
	OpSpec( "CMRT", DisplayHandlers::Command_CMRT,  "i", "pan", categoryCamera, "Centre current camera on target.  Set pan 0 to jump straight to location, pan 1 to smoothly scroll there (unless in different meta room), and pan 2 to smoothly scroll only if the destination is already visible."),
	OpSpec( "DMAP", DisplayHandlers::Command_DMAP,  "i", "debug_map", categoryMap, "Set to 1 to turn the debug map image on, 0 to turn it off. The debug map includes vehicle cabin lines." ),
	OpSpec( "RTYP", MapHandlers::Command_RTYP,  "ii", "room_id room_type", categoryMap, "Sets the type of a room.  The meaning of the types depends on the game.  @#RATE@ also uses the room type." ),
	OpSpec( "PROP", MapHandlers::Command_PROP,  "iif", "room_id ca_index ca_value", categoryMap, "Sets the level of a CA (cellular automata) in a particular room.  There are 16 CAs, and their meaning depends on the game.  The level is between 0 and 1."),
	OpSpec( "RATE", MapHandlers::Command_RATE,  "iifff", "room_type ca_index gain loss diffusion", categoryMap, "Sets various rates for a CA (cellular automata) in a particular type of room.  The values can be from 0 to 1.  Gain is the susceptibility to absorb from agents in the room, and loss is the amount lost to the atmosphere.  The diffusion is the amount it spreads to adjacent rooms."),
	OpSpec( "EMIT", MapHandlers::Command_EMIT,  "if", "ca_index amount", categoryMap, "Target now constantly emits an amount of a CA into the room it is in." ),
	OpSpec( "CACL", MapHandlers::Command_CACL,  "iiii", "family genus species ca_index", categoryMap, "This associates the classification specified with the CA specified. This allows the linking of CA's to classifiers within creatures' brains."),
	OpSpec( "ALTR", MapHandlers::Command_ALTR,  "iif", "room_id ca_index ca_delta", categoryMap, "Directly adjusts the level of a CA in a room.  Specify an identifier of -1 to use the room of the midpoint of the target agent." ),
	OpSpec( "WEAR", CreatureHandlers::Command_WEAR,  "iii", "body_id set_number layer", categoryCreatures, "Sets a layer of clothing on part of the creature.  The set_number is the type of clothing to put on from the overlay file - think of it as an outfit number.  layer 0 is the actual body of the creature, so unless you want to replace the body part itself use a higher layer.  Higher layers are on top of lower ones. e.g. 0 for a face, 1 for measels spots, 2 for a fencing mask.  See also @#BODY@ and @#NUDE@." ),
	OpSpec( "MIDI", SoundHandlers::Command_MIDI,  "s", "midi_file", categorySounds, "Plays a MIDI file.  Set to an empty string to stop the MIDI player." ),
	OpSpec( "LINE", DisplayHandlers::Command_LINE,  "iiiiiiiii", "x1 y1 x2 y2 r g b stipple_on stipple_off", categoryCamera, "Adds a line to target's drawing list.  The line goes between the start and end points (world coordinates) in the specified colour.  Set stipple_on and stipple_off to 0 to draw a solid line, or to the number of pixels to alternate for a stippled line.  To clear all the lines for an agent, call LINE with the start and end points the same."),
	OpSpec( "PLNE", AgentHandlers::Command_PLNE,  "i", "plane", categoryAgents, "Sets the target agent's principal drawing plane.  The higher the value, the nearer the camera.  For compound agents, the principal plane is the one for the automatically made first part.  The plane of other parts is relative to this one."),
	OpSpec( "RNGE", AgentHandlers::Command_RNGE,  "f", "distance", categoryAgents, "Sets the distance that the target can see and hear, and the distance used to test for potential collisions.  See also @#ESEE@, @#OBST@." ),
	OpSpec( "BRN:", CreatureHandlers::Command_BRN,  idSubCommandTable_BRN, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "CHAR", GeneralHandlers::Command_CHAR,  "vii", "string index character", categoryVariables, "Sets a character in a string.  String indices begin at 1." ),
	OpSpec( "VELO", AgentHandlers::Command_VELO,  "ff", "x_velocity y_velocity", categoryMotion, "Set velocity, measured in pixels per tick." ),
	OpSpec( "RPAS", AgentHandlers::Command_RPAS,  "aa", "vehicle passenger", categoryVehicles, "Specified vehicle drops the specified passenger." ),
	OpSpec( "PUPT", AgentHandlers::Command_PUPT,  "iii", "pose x y", categoryAgents, "Set the relative x and y coordinate of the place where target picks agents up, for the given pose.  This pose is measured from the absolute base specified in the NEW: command, rather than the relative base specified by the @#BASE@ command. Pose -1 sets the same point for all poses.  For vehicles use the @#CABN@ command." ),
	OpSpec( "PUHL", AgentHandlers::Command_PUHL,  "iii", "pose x y", categoryAgents, "Set the relative x and y coordinate of the handle that target is picked up by, for the given pose.  This pose is measured from the absolute base specified in the NEW: command, rather than the relative base specified by the @#BASE@ command. Pose -1 sets the same point for all poses." ),
	OpSpec( "CABP", AgentHandlers::Command_CABP,  "i", "plane", categoryVehicles, "Set the plane that vehicle passengers are at.  This is relative to the vehicle's plane."),
	OpSpec( "TRCK", DisplayHandlers::Command_TRCK,  "aiiii", "agent x% y% style transition", categoryCamera, "Camera follows the given agent.  Set to @#NULL@ to stop tracking. x% and y% are percentages (0-100) of the screen size.  They describe a rectangle centred on the screen which the target stays within. <br> Style 0 is brittle - if you move the camera so the target is out of the rectangle, then the tracking is broken.<br>Style 1 is flexible - you can move the camera away from the target.  If you move it back, then tracking resumes.<br>Style 2 is hard - you can't move the camera so the target is out of the rectangle.<br> The transition is the sort of fade to use if the tracking causes a change in meta room.  The values are the same as for the transition in the @#META@ command."),
	OpSpec( "GALL", AgentHandlers::Command_GALL,  "si", "sprite_file first_image", categoryAgents, "Changes the gallery (sprite file) used by an agent.  This works for simple and compound agents (using the current @#PART@).  The current @#POSE@ is kept the same in both galleries."),
	OpSpec( "SWAY", CreatureHandlers::Command_SWAY, idSubCommandTable_SWAY, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "ASLP", CreatureHandlers::Command_ASLP,  "i", "asleep", categoryCreatures, "Make the creature asleep or awake.  1 for asleep, 0 for awake." ),
	OpSpec( "UNCS", CreatureHandlers::Command_UNCS,  "i", "unconscious", categoryCreatures, "Make the creature conscious or unconscious.  0 for conscious, 1 for unconscious."),
	OpSpec( "BODY", CreatureHandlers::Command_BODY,  "ii", "set_number layer", categoryCreatures, "Similar to @#WEAR@, only puts the given set of clothes on every body part." ),
	OpSpec( "NUDE", CreatureHandlers::Command_NUDE,  "", "", categoryCreatures, "Removes all clothes from a creature.  Any changed layer 0 will revert to drawing the body part again.  See @#WEAR@." ),
	OpSpec( "AGES", CreatureHandlers::Command_AGES,  "i", "times", categoryCreatures, "Forces a creature to age the given number of times.  See also @#CAGE@."),
	OpSpec( "DRIV", CreatureHandlers::Command_DRIV, "if", "drive adjustment", categoryCreatures, "Adjusts the level of the given drive by the specified amount - can be positive or negative." ),
	OpSpec( "BHVR", AgentHandlers::Command_BHVR,  "i", "permissions", categoryAgents, "Sets the creature permissions for target.  Sum the entries in the @#Creature Permissions@ table to get the value to use."),
	OpSpec( "FACE", CreatureHandlers::Command_FACE,  "i", "set_number", categoryCreatures, "Sets a facial expression on target creature." ),
	OpSpec( "MCLR", SoundHandlers::Command_MCLR, "ii", "x y", categorySounds, "Clear the music for the metaroom at the given location."  ),
	OpSpec( "MMSC", SoundHandlers::Command_MMSC, "iis", "x y track_name", categorySounds, "Associates a music track with the meta room at the specified coordinates." ),
	OpSpec( "PTXT", CompoundHandlers::Command_PTXT,  "s", "text", categoryCompound, "Set string of current text part.  Use @#PAT: TEXT@ or @#PAT: FIXD@ to make a text part, and @#PART@ to set the current part."),
	OpSpec( "ORDR", CreatureHandlers::Command_ORDR,  idSubCommandTable_ORDR, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "NORN", CreatureHandlers::Command_NORN,  "a", "creature", categoryCreatures, "Chooses the active creature.  Script 120 is executed on that creature, to inform them that they've been selected."),
	OpSpec( "FLTO", AgentHandlers::Command_FLTO, "ff", "screen_x screen_y", categoryMotion, "Move the top left corner of target to either the given screen coordinates, or the given coordinates relative to the agent it is @#FREL@ to.  Useful for floating agents."),
	OpSpec( "VOCB", CreatureHandlers::Command_VOCB, "", "", categoryCreatures, "Learn all vocabulary instantly." ),
	OpSpec( "CLIK", AgentHandlers::Command_CLIK,  "iii", "message_1 message_2 message_3", categoryInput, "Sets a chain of three message ids to cycle through as the agent is clicked on.  Entries of -1 are ignored.  Overriden by @#CLAC@."),
	OpSpec( "MOUS", AgentHandlers::Command_MOUS,  "i", "behaviour", categoryInput, "Defines the behaviour of the mouse button for the default pointer behaviour - see @#PURE@.<br>0 is normal<br>1 means the right button does what the left button does<br>2 means the left button does what the right button does" ),
	OpSpec( "SPNL", CreatureHandlers::Command_SPNL,  "sif", "lobe_moniker neuron_id value", categoryCreatures, "This sets the input of the neuron in the lobe specified to be the value given."),
	OpSpec( "HAIR", CreatureHandlers::Command_HAIR,  "i", "stage", categoryCreatures, "Tidies or ruffles hair.  Positive means tidy, negative untidy.  There can be multiple stages of tidiness or untidiness; the more extreme the value the tidier or untidier."),
	OpSpec( "TRAN", DisplayHandlers::Command_TRAN,  "ii", "transparency part_no", categoryInput, "Sets pixel transparency awareness.  1 for pixel perfect, so transparent parts of the agent can't be clicked.  0 to allow anywhere on the agent rectangle to be clicked.  See also the option parameter on @#PAT: BUTT@ which overrides this."),
	OpSpec( "VOLM", SoundHandlers::Command_VOLM,  "i", "volume", categorySounds, "Set overall the volume of the MIDI and the sound effects."),
	OpSpec( "SAVE", GeneralHandlers::Command_SAVE,  "", "", categoryWorld, "Saves the current world at the start of the next tick.  See also @#QUIT@ for important information about using @#INST@."),
	OpSpec( "LOAD", GeneralHandlers::Command_LOAD,  "s", "world_name", categoryWorld, "Loads the specified world at the start of the next tick.  See also @#QUIT@ for important information about using @#INST@."), 
	OpSpec( "PURE", AgentHandlers::Command_PURE,  "i", "value", categoryInput, "Enables or disables the default clicking and moving behaviour of the pointer.  This default behaviour is to implement @#CLAC@ and @#CLIK@, and to operate ports.  Set to 1 to enable, 0 to disable.  When disabled, use @#IMSK@ to hook mouse events."),
	OpSpec( "DELW", GeneralHandlers::Command_DELW,  "s",  "world_name", categoryWorld, "Deletes the specified world directory."),
	OpSpec( "PAUS", AgentHandlers::Command_PAUS,  "i", "paused", categoryAgents, "Stops the target agent from running - it'll freeze completely, scripts and physics.  Set to 1 to pause, 0 to run.  You might want to use @#WPAU@ with this to implement a pause game option."),
	OpSpec( "STPT", AgentHandlers::Command_STPT, "", "", categoryScripts, "Stops any currently running script in the target agent.  See also @#STOP@." ),
	OpSpec( "QUIT", GeneralHandlers::Command_QUIT, "", "", categoryWorld, "Quits the engine at the start of the next tick, without saving any changes.  Call @#SAVE@ first to make it save the current world.  If doing a sequence like \"SAVE QUIT\" or \"SAVE LOAD menu\", do it in an @#INST@ section.  Otherwise it will sometimes save between the two instructions, meaning it quits (or loads menu) immediately upon reloading."),
	OpSpec( "SCAM", CompoundHandlers::Command_SCAM,  "ai", "compoundagent partNumber", categoryCamera, "Sets the current camera to be used in subsequent camera macro commands.  This uses the given @#TARG@ and the given @#PART@ number.  If you set this to @#NULL@ then the Main Camera will be used.  This is the default setting"),
	OpSpec( "ANMS", AgentHandlers::Command_ANMS, "s","anim_string", categoryAgents, "This is like @#ANIM@, only it reads the poses from a string such as \"3 4 5 255\".  Use this when you need to dynamically construct animations.  Use ANIM in general as it is quicker to execute, although they are the same speed once the animation is underway." ),
	OpSpec( "ZOOM", DisplayHandlers::Command_ZOOM,  "iii", "pixels x y", categoryCamera, "Zoom in on the specified position by a negative amount of pixels or out by positive amount of pixels.  If you send -1 as the x and y coordinates then the camera zooms in on the exising view port centre.  This only applies to remote cameras."),
	OpSpec( "LINK", MapHandlers::Command_LINK,  "iii", "room1 room2 permiability", categoryMap, "Sets the permiability of the link between the rooms specified, creating the link if none exists before.  Set to 0 to close (destroy) the link.  This is used for CAs.  See also @#DOOR@."),
	OpSpec( "FRAT", AgentHandlers::Command_FRAT,  "i", "FrameRate", categoryAgents, "This command sets the frame rate on the @#TARG@ agent. If it is a compound agent, then the part affected can be set with the @#PART@ command. Valid rates are from 1 to 255. 1 is Normal rate, 2 is half speed etc..."),
	OpSpec( "WRLD", GeneralHandlers::Command_WRLD,  "s", "world_name", categoryWorld, "Creates a new world directory for the specified world. "),
	OpSpec( "PRAY", PrayHandlers::Command_PRAY,  idSubCommandTable_PRAY, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "FCUS", CompoundHandlers::Command_FCUS,  "", "", categoryCompound, "Set keyboard focus to the current @#PART@ of the targetted agent.  The part should be a @#PAT: TEXT@.  If you TARG NULL first, then no part will have the focus."),
	OpSpec( "WPAU", GeneralHandlers::Command_WPAU,  "i", "paused", categoryTime, "Stops world ticks from running.  Days, seasons and years won't change and any delayed messages are paused, as are CAs and some sound effects.  Set to 1 to pause, 0 to run.  Use along with @#PAUS@."),
	OpSpec( "FREL", AgentHandlers::Command_FREL, "a", "relative", categoryMotion, "Sets an agent for target to float relative to.  To make target actually float, you need to set attribute @#Floatable@ as well.  Set @#FREL@ to @#NULL@ to make the target float relative to the main camera - this is the default.  Use @#FLTO@ to set the relative position of the top left corner of the floating agent to the top left corner of the agent it is floating relative to."),
	OpSpec( "GRPL", CompoundHandlers::Command_GRPL,  "iiiff", "red green blue min_y max_y", categoryCompound, "Add a line to a graph (previously created with @#PAT: GRPH@). The first line you add will be line 0."),
	OpSpec( "GRPV", CompoundHandlers::Command_GRPV,  "if", "line_index value", categoryCompound, "Add a value to a line on a graph. after you have added a value to each line on the graph, it will be updated by scrolling the current values to the left"),
	OpSpec( "PSWD", GeneralHandlers::Command_PSWD,  "s", "world_name", categoryWorld, "Sets the password of the current world. "),
	OpSpec( "SHOW", AgentHandlers::Command_SHOW,  "i", "visibility", categoryAgents, "Set the parameter to 0 to hide the agent and to 1 to show the agent on camera.  This removes or adds the agent to the main camera and any remote cameras.  A non-shown agent can still be visible to creatures, and can still be clicked on or picked up.  It just doesn't appear on the cameras."),
	OpSpec( "GENE", CreatureHandlers::Command_GENE,  idSubCommandTable_GENE, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "FILE", GeneralHandlers::Command_FILE,  idSubCommandTable_FILE, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "FRMT", CompoundHandlers::Command_FRMT,  "iiiiiii", "left_margin top_margin right_margin bottom_margin line_spacing character_spacing justification", categoryCompound, "Use this command to alter the appearance of the current text part. The line and character spacing values are expressed in number of extra pixels to insert between characters. Values for justification are 0 - Left, 1 - Right, 2 - Center, 4 - Bottom, 8 - Middle (you may add mutually compatible numbers).  The default format values are 8 8 8 8 0 0 0." ),
	OpSpec( "SNAP", DisplayHandlers::Command_SNAP,  "siiiii", "filename x_centre y_centre width height zoom_factor", categoryCamera, "This takes a photograph of the world at a particular place. The zoom parameter should be <= 100. 100 means at original size, 50 means half size etc.  It makes a new image file in the world images directory - you can use it to make agents and parts as with any image file.  Call @#SNAX@ first to check your filename isn't already in use in any images directory.  When you have finished with the file, call @#LOFT@." ),
	OpSpec( "BORN", CreatureHandlers::Command_BORN, "", "", categoryCreatures, "Signals the target creature as having been born - this sends a birth event, and sets the @#TAGE@ ticking." ),
	OpSpec( "HIST", HistoryHandlers::Command_HIST,  idSubCommandTable_HIST, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "NOHH", CreatureHandlers::Command_NOHH,  "", "", categoryAgents, "Tell the creature to immediately stop holding hands with the pointer.  Useful when you are about to teleport a norn, it prevents the pointer from continuosly changing his position back to where it was."),
	OpSpec( "VOIS", AgentHandlers::Command_VOIS, "s", "voice_name", categorySounds, "Sets the @#TARG@ agent's voice to the specified value. The voice name must be valid in the catalogue. If it fails, then \"DefaultVoice\" will be reloaded.  Use @#SEZZ@ to actually say something."),
	OpSpec( "VOIC", AgentHandlers::Command_VOIC, "iii", "genus gender age", categorySounds, "This sets the @#TARG@ agent's voice to the specified creature voice, using standard cascade techniques to select the nearest match.  On failure, \"DefaultVoice\" will be reloaded.  Use @#SEZZ@ to actually say something."),
	OpSpec( "DEAD", CreatureHandlers::Command_DEAD, "", "", categoryCreatures, "Makes the target creature die, triggering @#Die@ script and history events, closing its eyes, and stopping brain and biochemistry updates.  Not to be confused with @#KILL@, which you will have to use later to remove the actual body."),
	OpSpec( "SEZZ", AgentHandlers::Command_SEZZ, "s", "text", categorySounds, "Makes the @#TARG@ agent speak the specified text with voice as set by @#VOIS@ or @#VOIC@. If @#TARG@ is a creature then it will be spoken properly (speech bubble et al)."),
	OpSpec( "WTNT", DisplayHandlers::Command_WTNT, "iiiiii", "index red_tint green_tint blue_tint rotation swap", categoryWorld, "This sets up the world (global) tint table. The index is the number associated with the tint table - (keep it small please) and the r,g,b is the tint level. Rotation and Swap work as for pigment bleed genes."),
	OpSpec( "TNTW", DisplayHandlers::Command_TNTW, "i", "index", categoryWorld, "This tints the @#TARG@ agent with the global tint manager at index.  Specify the @#PART@ first for compound agents.  See also @#TINT@."),
	OpSpec( "TINT", DisplayHandlers::Command_TINT, "iiiii", "red_tint green_tint blue_tint rotation swap", categoryAgents, "This tints the @#TARG@ agent with the r,g,b tint and applies the colour rotation and swap as per pigment bleed genes.  Specify the @#PART@ first for compound agents.  The tinted agent or part now uses a cloned gallery, which means it takes up more memory, and the save world files are larger.  However it also no longer needs the sprite file.  Also, tinting resets camera shy and other properties of the gallery."),
	OpSpec( "RGAM", GeneralHandlers::Command_RGAM,  "", "", categoryWorld, "Refresh all settings that are always read from game variables at start up e.g. the length of a day.  This allows you to change such setting on the fly."),
	OpSpec( "REAF", GeneralHandlers::Command_REAF, "", "", categoryVariables, "Refreshes the catalogue from files on disk, from the main catalogue directory and the world catalogue directory.  These are normally read in at startup, when a new world is read in, or when the PRAY resources system installs a catalogue file.  Use while developing CAOS programs to refresh the catalogue as you add entries." ),
	OpSpec( "NEWC", AgentHandlers::Command_NEWC,  "iaiii", "family gene_agent gene_slot sex variant", categoryCreatures, "This version of @#NEW: CREA@ executes over a series of ticks, helping to prevent the pause caused by the creation of a creature with the @#NEW: CREA@ command. However, it cannot be used in install scripts (e.g. the bootstrap) and so @#NEW: CREA@ should be used for that."),
	OpSpec( "FRSH", DisplayHandlers::Command_FRSH,  "", "", categoryCamera, "Refreshes the main view port."),	
	OpSpec( "BRMI", MapHandlers::Command_BRMI, "ii", "metaroom_base, room_base", categoryCamera, "Sets the Map's Metaroom and Room index bases for adding new rooms/metarooms."),
	OpSpec( "CABW", AgentHandlers::Command_CABW,  "i", "cabin_capacity", categoryVehicles, "Set the capacity or width of the cabin.  This will determine how many passengers the cabin can hold, each passenger will be on a separate plane within the cabin.  Use @#CABP@ to set the plane of the first agent relative to the cabin.  The default width is zero, this means that the cabin will accept any number of passengers and will place them all on the same plane. "),
	OpSpec( "DOCA", MapHandlers::Command_DOCA, "i", "no_of_updates", categoryMap, "Updates all CAs the specified number of times."),
	OpSpec( "PAGE", CompoundHandlers::Command_PAGE,  "i", "page", categoryCompound, "Sets current page for text part. The page number should be equal or greater than zero and less than the number returned by @#NPGS@. Use @#PAT: TEXT@ or @#PAT: FIXD@ to make a text part, and @#PART@ to set the current part."),
	OpSpec( "DONE", CreatureHandlers::Command_DONE,  "", "", categoryCreatures, "Stops the targetted creature doing any involuntary actions."),
	OpSpec( "STRK", GeneralHandlers::Command_STRK, "is", "latency track", categorySounds, "This triggers the music track specified. The track will play for at least latency seconds before being overridden by room or metaroom music."),
	OpSpec( "WDOW", DisplayHandlers::Command_WDOW,  "", "", categoryCamera, "Toggle full screen mode." ),
	OpSpec( "FORF", CreatureHandlers::Command_FORF,  "a", "creature_to_learn_about", categoryCreatures, "Set the friends or foe lobe to learn from the creature." ),
	OpSpec( "LIKE", CreatureHandlers::Command_LIKE,  "a", "creature_state_opinion_about", categoryCreatures, "State a personal opinion about a creature." ),
	OpSpec( "MIRA", AgentHandlers::Command_MIRA,  "i", "on_off", categoryAgents, "Tell the agent to draw the current sprite mirrored (send 1 as a parameter) or normally (send 0 as a parameter)"),
	OpSpec( "ZOMB", CreatureHandlers::Command_ZOMB,  "i", "zombie", categoryCreatures, "Make or undo the creature's zombification factor.  1 makes creatures zombies: in a zombie state creatures won't process any decision scripts but they will respond to @ANIM@s and @POSE@s.  0 umzombifies." ),
	OpSpec( "CABV", AgentHandlers::Command_CABV,  "i", "cabin_room_id", categoryVehicles, "Sets the room number which things in the cabin think they are in.  By default, the value is -1, and the room is the one underneath wherever the agent happens to be.  Make sure you set this if the vehicle ever remotely leaves the room system.  This command effects values returned from @#ROOM@ and @#GRID@.  It won't apply to some aspects of Creatures in the vehicle."),
	OpSpec( "HAND", AgentHandlers::Command_HAND,  "s", "name_for_the_hand", categoryAgents, "Sets the name of the hand. Bt default this is 'hand'."),
	OpSpec( "MANN", DebugHandlers::Command_MANN, "s", "command", categoryDebug, "Outputs help on the given command to the output stream."),
	OpSpec( "HELP", DebugHandlers::Command_HELP, "", "", categoryDebug, "Lists all command names to the output stream."),
	OpSpec( "APRO", DebugHandlers::Command_APRO, "s", "search_text", categoryDebug, "Lists all command names whose help contains the text."),
	OpSpec( "MEMX", DebugHandlers::Command_MEMX, "", "", categoryDebug, "Sends information about the memory allocated to the output stream.  In order, these are the Memory Load (unknown), Total Physical (size in bytes of physical memory), Available Physical (free physical space), Total Page File (maximum possible size of page file), Available Page File (size in bytes of space available in paging file), Total Virtual (size of user mode portion of the virtual address space of the engine), Available Virtual (size of unreserved and uncommitted memory in the user mode portion of the virtual address space of the engine)."),
};


OpSpec ourIntegerRVTable[] =
{
	OpSpec( "FALL", AgentHandlers::IntegerRV_FALL,  "", "", categoryMotion, "Returns 1 if target is moving under the influence of gravity, or 0 if it is at rest." ),
	OpSpec( "TMVT", AgentHandlers::IntegerRV_TMVT, "ff", "x y", categoryMotion, "Test if target can move to the given location and still lie validly within the room system.  Returns 1 if it can, 0 if it can't." ),
	OpSpec( "TMVB", AgentHandlers::IntegerRV_TMVB, "ff", "delta_x delta_y", categoryMotion, "Similar to @#TMVT@ only tests a @#MVBY@." ),
	OpSpec( "TMVF", AgentHandlers::IntegerRV_TMVF, "ff", "x y", categoryMotion, "Test if a creature could move it's down foot to position x,y."),
    OpSpec( "MOVS", AgentHandlers::IntegerRV_MOVS,  "", "", categoryMotion, "Returns the movement status of the target.  <br>0 Autonomous<br>1 Mouse driven<br>2 Floating<br>3 In vehicle<br>4 Carried" ),
	OpSpec( "FRIC", AgentHandlers::IntegerRV_FRIC,  "", "", categoryMotion, "Return physics friction percentage." ),
	OpSpec( "GRID", MapHandlers::IntegerRV_GRID, "ai","agent direction", categoryMap, "Returns the ID of a room adjacent to the agent in the given direction.  A straight line is drawn from the centre of the agent until it hits a room.  Directions are @#LEFT@, @#RGHT@, @#_UP_@, or @#DOWN@. A value of -1 is returned if no room can be found."),
	OpSpec( "WDTH", AgentHandlers::IntegerRV_WDTH,  "", "", categoryAgents, "Returns the width of target." ),
	OpSpec( "HGHT", AgentHandlers::IntegerRV_HGHT,  "", "", categoryAgents, "Returns the height of target." ),
	OpSpec( "UNID", AgentHandlers::IntegerRV_UNID,  "", "", categoryDebug, "Returns unique identifier for target agent.  @#AGNT@ goes the opposite way.  NOTE: This should only be used for external programs to persistently refer to an agent.  Variables can use @#SETA@ to store agent r-values directly for internal use." ),
	OpSpec( "WALL", AgentHandlers::IntegerRV_WALL,  "", "", categoryMotion, "Returns the direction of the last wall the agent collided with.  Directions are @#LEFT@, @#RGHT@, @#_UP_@, or @#DOWN@." ),
	OpSpec( "TICK", AgentHandlers::IntegerRV_TICK,  "", "", categoryAgents, "Returns the current timer rate set by the command TICK." ),
	OpSpec( "CABL", AgentHandlers::IntegerRV_CABL,  "", "", categoryVehicles, "Returns relative position of left side of cabin." ),
	OpSpec( "CABR", AgentHandlers::IntegerRV_CABR,  "", "", categoryVehicles, "Returns relative position of right side of cabin." ),
	OpSpec( "CABB", AgentHandlers::IntegerRV_CABB,  "", "", categoryVehicles, "Returns relative position of bottom side of cabin." ),
	OpSpec( "CABT", AgentHandlers::IntegerRV_CABT,  "", "", categoryVehicles, "Returns relative position of topside of cabin." ),
	OpSpec( "FMLY", AgentHandlers::IntegerRV_FMLY,  "", "", categoryAgents, "Returns family of target.  See also @#GNUS@, @#SPCS@." ),
	OpSpec( "GNUS", AgentHandlers::IntegerRV_GNUS,  "", "", categoryAgents, "Returns genus of target.  See also @#FMLY@, @#SPCS@." ),
	OpSpec( "SPCS", AgentHandlers::IntegerRV_SPCS,  "", "", categoryAgents, "Returns species of target.  See also @#FMLY@, @#GNUS@." ),
	OpSpec( "TOTL", AgentHandlers::IntegerRV_TOTL,  "iii", "family genus species", categoryAgents, "Counts the number of agents in the world matching the classifier." ),
	OpSpec( "TOUC", AgentHandlers::IntegerRV_TOUC,  "aa", "first second", categoryAgents, "Returns 1 if the two specified agents are touching, or 0 if they are not.  Agents are said to be touching if their bounding rectangles overlap." ),
	OpSpec( "SEEE", AgentHandlers::IntegerRV_SEEE,  "aa", "first second", categoryAgents, "Returns 1 if the first agent can see the second, or 0 if it can't.  See @#ESEE@." ),
	OpSpec( "POSE", AgentHandlers::IntegerRV_POSE,  "", "", categoryAgents, "Return the current @#POSE@ of the target agent/part, or -1 if invalid part." ),
	OpSpec( "WNDT", DisplayHandlers::IntegerRV_WNDT,  "", "", categoryCamera, "Returns world coordinates of top of current camera window." ),
	OpSpec( "WNDB", DisplayHandlers::IntegerRV_WNDB,  "", "", categoryCamera, "Returns world coordinates of bottom of current camera window." ),
	OpSpec( "DOOR", MapHandlers::IntegerRV_DOOR,  "ii", "room_id1 room_id2", categoryMap, "Returns the door permiability between two rooms." ),	
	OpSpec( "MAPW", MapHandlers::IntegerRV_MAPW,  "", "", categoryMap, "Returns the total width of the map." ),
	OpSpec( "MAPH", MapHandlers::IntegerRV_MAPH,  "", "", categoryMap, "Returns the total height of the map." ),
	OpSpec( "SEAN", GeneralHandlers::IntegerRV_SEAN,  "", "", categoryTime, "Returns the current season.  This can be<br>0 - spring<br>1 - summer<br>2 - autumn<br>3 - winter<p>The @#GAME@ variable engine_LengthOfSeasonInDays sets the season length.  See also @#HIST SEAN@." ),
	OpSpec( "TIME", GeneralHandlers::IntegerRV_TIME,  "", "", categoryTime, "Returns the time of day.  This can be<br>0 - dawn<br>1 - morning<br>2 - afternoon<br>3 - evening<br>4 - night<p>The @#GAME@ variable engine_LengthOfDayInMinutes sets the day length.  See also @#HIST TIME@." ),
	OpSpec( "YEAR", GeneralHandlers::IntegerRV_YEAR,  "", "", categoryTime, "Returns the number of game years elapsed.  The @#GAME@ variable engine_NumberOfSeasons sets the year length.  See also @#HIST YEAR@." ),
	OpSpec( "CMRX", DisplayHandlers::IntegerRV_CMRX,  "", "", categoryCamera, "Returns the x coordinate of the centre of the current camera." ),
	OpSpec( "CMRY", DisplayHandlers::IntegerRV_CMRY,  "", "", categoryCamera, "Returns the y coordinate of the centre of the current camera." ),
	OpSpec( "DRV!", CreatureHandlers::IntegerRV_DRV,  "", "", categoryCreatures, "Returns the id of the highest drive for the target creature." ),
	OpSpec( "DEAD", CreatureHandlers::IntegerRV_DEAD,  "", "", categoryCreatures, "Returns 1 if target creature is dead, or 0 if alive."  ),
	OpSpec( "INS#", CreatureHandlers::IntegerRV_INS,  "", "", categoryCreatures, "Number of instincts still queued to be processed."  ),
	OpSpec( "DIRN", CreatureHandlers::IntegerRV_DIRN,  "", "", categoryCreatures, "Returns the direction that target creature is facing.  North 0, South 1, East 2, West 3." ),
	OpSpec( "CAGE", CreatureHandlers::IntegerRV_CAGE,  "", "", categoryCreatures, "Returns life stage of target creature.  See also @#AGES@." ),
	OpSpec( "RAND", GeneralHandlers::IntegerRV_RAND,  "ii", "value1 value2", categoryVariables, "Returns a random integer between value1 and value2 inclusive of both values.  You can use negative values, and have them either way round." ),
	OpSpec( "KEYD", GeneralHandlers::IntegerRV_KEYD,  "i", "keycode", categoryInput, "Returns 1 if the specified key is currently pressed down, 0 if not." ),
	OpSpec( "MOPX", GeneralHandlers::IntegerRV_MOPX,  "", "", categoryInput, "Returns x position of mouse in world coordinates." ),
	OpSpec( "MOPY", GeneralHandlers::IntegerRV_MOPY,  "", "", categoryInput, "Returns y position of mouse in world coordinates." ),
	OpSpec( "ADDM", MapHandlers::IntegerRV_ADDM,  "iiiis", "x y width height background", categoryMap, "Creates a new metaroom with the given coordinates.  Specifies the starting background file.  Returns the id of the new metaroom."),
	OpSpec( "ADDR", MapHandlers::IntegerRV_ADDR,  "iiiiiii", "metaroom_id x_left x_right y_left_ceiling y_right_ceiling y_left_floor y_right_floor", categoryMap, "Creates a new room within a metaroom.  Rooms have vertical left and right walls, but potentially sloped floors and ceilings.  The coordinates specify the exact shape.  Returns the id of the new room."),
	OpSpec( "META", MapHandlers::IntegerRV_META,  "", "", categoryCamera, "Returns the metaroom id that the current camera is looking at."),
	OpSpec( "PERM", AgentHandlers::IntegerRV_PERM,  "", "", categoryMap, "Returns the target's map permiability."),
	OpSpec( "ATTR", AgentHandlers::IntegerRV_ATTR,  "", "", categoryAgents, "Return attributes of target."),
	OpSpec( "AERO", AgentHandlers::IntegerRV_AERO,  "", "", categoryMotion, "Returns aerodynamic factor as a percentage."),
	OpSpec( "IMSK", AgentHandlers::IntegerRV_IMSK,  "", "", categoryAgents, "Returns the input event mask."),
	OpSpec( "ELAS", AgentHandlers::IntegerRV_ELAS,  "", "", categoryMotion, "Return the elasticity percentage."),
	OpSpec( "BYIT", CreatureHandlers::IntegerRV_BYIT,  "", "", categoryCreatures, "Returns 1 if the creature is within reach of the IT agent, or 0 if it isn't."),
	OpSpec( "RTYP", MapHandlers::IntegerRV_RTYP,  "i", "room_id", categoryMap, "Returns the type of a room, or -1 if not a valid room id."),
	OpSpec( "FACE", CreatureHandlers::IntegerRV_FACE,  "", "", categoryCreatures, "Returns the front facing pose for the current facial expression.  See the @#FACE@ string rvalue."),
	OpSpec( "PAWS", DebugHandlers::IntegerRV_PAWS,  "", "", categoryDebug, "Returns 1 for debug pawsed, 0 for playing.  See @#DBG: PAWS@."),
	OpSpec( "LEFT", GeneralHandlers::IntegerRV_LEFT,  "", "", categoryMap, "Returns the value of the left constant."),
	OpSpec( "RGHT", GeneralHandlers::IntegerRV_RGHT,  "", "", categoryMap, "Returns the value of the right constant."),
	OpSpec( "_UP_", GeneralHandlers::IntegerRV_UP,  "", "", categoryMap, "Returns the value of the up constant."),
	OpSpec( "DOWN", GeneralHandlers::IntegerRV_DOWN,  "", "", categoryMap, "Returns the value of the down constant."),
	OpSpec( "PLNE", AgentHandlers::IntegerRV_PLNE,  "", "", categoryAgents, "Returns the screen depth plane of the principal part."),
	OpSpec( "ROOM", MapHandlers::IntegerRV_ROOM,  "a", "agent", categoryMap, "Returns the id of the room containing the midpoint of the specified agent."),
	OpSpec( "HIRP", MapHandlers::IntegerRV_HIRP,  "iii", "room_id ca_index directions", categoryMap, "Returns id of the room adjacent to this one with the highest concentration of the given CA.  direction is 0 for left/right, 1 for any direction."),
	OpSpec( "LORP", MapHandlers::IntegerRV_LORP,  "iii", "room_id ca_index directions", categoryMap, "Returns id of the room adjacent to this one with the lowest concentration of the given CA.  direction is 0 for left/right, 1 for any direction."),
	OpSpec( "STRL", GeneralHandlers::IntegerRV_STRL,  "s", "string", categoryVariables, "Returns the length of a string."),
	OpSpec( "CHAR", GeneralHandlers::IntegerRV_CHAR,  "si", "string index", categoryVariables, "Returns a character from a string.  String indicies begin at 1." ),
	OpSpec( "PUPT", AgentHandlers::IntegerRV_PUPT,  "ii", "pose x_or_y", categoryAgents, "Returns the x or y coordinate of the place where target picks agents up for the given pose.  x_or_y is 1 for x, 2 for y."),
	OpSpec( "PUHL", AgentHandlers::IntegerRV_PUHL,  "ii", "pose x_or_y", categoryAgents, "Returns the x or y coordinate of the handle that target is picked up by for the given pose.  x_or_y is 1 for x, 2 for y."),
	OpSpec( "CABP", AgentHandlers::IntegerRV_CABP,  "", "", categoryVehicles, "Returns the plane that passengers of the vehicle are at." ),
	OpSpec( "BVAR", CreatureHandlers::IntegerRV_BVAR,  "", "", categoryCreatures, "Returns the variant number for target creature."),
	OpSpec( "ASLP", CreatureHandlers::IntegerRV_ASLP,  "", "", categoryCreatures, "Returns 1 if the creature is asleep, 0 otherwise." ),
	OpSpec( "UNCS", CreatureHandlers::IntegerRV_UNCS,  "", "", categoryCreatures, "Returns 1 if the creature is unconscious, 0 otherwise." ),
	OpSpec( "ATTN", CreatureHandlers::IntegerRV_ATTN,  "", "", categoryCreatures, "Returns the current focus of attention id." ),
	OpSpec( "DECN", CreatureHandlers::IntegerRV_DECN,  "", "", categoryCreatures, "Returns the current focus of decision id." ),
	OpSpec( "BHVR", AgentHandlers::IntegerRV_BHVR,  "", "", categoryAgents, "Returns the creature permissions for the target agent."),
	OpSpec( "WNDL", DisplayHandlers::IntegerRV_WNDL,  "", "", categoryCamera, "Returns world coordinates of left of current camera window." ),
	OpSpec( "WNDR", DisplayHandlers::IntegerRV_WNDR,  "", "", categoryCamera, "Returns world coordinates of right of current camera window." ),
	OpSpec( "WNDW", DisplayHandlers::IntegerRV_WNDW,  "", "", categoryCamera, "Returns width of current camera window." ),
	OpSpec( "WNDH", DisplayHandlers::IntegerRV_WNDH,  "", "", categoryCamera, "Returns height of current camera window." ),
	OpSpec( "CODF", DebugHandlers::IntegerRV_CODF,  "", "", categoryDebug, "Returns family of script currently being run by target. Returns -1 if not running anything." ),
	OpSpec( "CODG", DebugHandlers::IntegerRV_CODG,  "", "", categoryDebug, "Returns genus of script currently being run by target. Returns -1 if not running anything." ),
	OpSpec( "CODS", DebugHandlers::IntegerRV_CODS,  "", "", categoryDebug, "Returns species of script currently being run by target. Returns -1 if not running anything." ),
	OpSpec( "CODE", DebugHandlers::IntegerRV_CODE,  "", "", categoryDebug, "Returns event script number currently being run by target. Returns -1 if not running anything." ),
	OpSpec( "CODP", DebugHandlers::IntegerRV_CODP,  "", "", categoryDebug, "Returns the offset into the source code of the next instruction to be executed by the target. Use @#SORC@ to get the source code.  Returns -1 if not running anything." ),
	OpSpec( "CREA", CreatureHandlers::IntegerRV_CREA,  "a", "agent", categoryCreatures, "Returns 1 if the agent is a creature, 0 if not."),
	OpSpec( "BASE", AgentHandlers::IntegerRV_BASE, "", "", categoryAgents, "Returns the @#BASE@ image for the current agent/part.  Returns -1 if an invalid part." ),
	OpSpec( "WTIK", GeneralHandlers::IntegerRV_WTIK, "", "", categoryTime, "Returns the number of ticks since the world was first made.  For debugging purposes only you can change this value with @#DBG: WTIK@."),
	OpSpec( "ETIK", GeneralHandlers::IntegerRV_ETIK, "", "", categoryTime, "Returns the number of ticks since the engine was loaded in."),
	OpSpec( "PURE", AgentHandlers::IntegerRV_PURE, "", "", categoryInput, "Returns whether default pointer behaviour is disabled or enabled.  1 if enabled, 0 if disabled."),
	OpSpec( "CATI", CreatureHandlers::IntegerRV_CATI, "iii", "family genus species", categoryAgents, "Return the category id for the given classifier.  The catalogue tag \"Agent Classifiers\" specifies these, and you can have more than 40.  They are tested in order until the first match is found.  39 (or \"unclassified\") is always returned if none match."),
	OpSpec( "NWLD", GeneralHandlers::IntegerRV_NWLD,  "", "", categoryWorld, "Returns the number of world directories." ),
	OpSpec( "PAUS", AgentHandlers::IntegerRV_PAUS,  "", "", categoryAgents, "Returns 1 if the target agent is paused, or 0 otherwise."),
	OpSpec( "GRAP", MapHandlers::IntegerRV_GRAP,  "ff", "x y", categoryMap, "Returns the room id at point x,y on the map. If the point is outside the room system, it returns -1."),
	OpSpec( "LINK", MapHandlers::IntegerRV_LINK,  "ii", "room1 room2", categoryMap, "Returns the permiability of the link between the rooms specified or 0 if no link exists."),
	OpSpec( "PRAY", PrayHandlers::IntegerRV_PRAY,  idSubIntegerRVTable_PRAY, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "VMNR", GeneralHandlers::IntegerRV_VMNR,  "", "", categoryVariables, "Returns the minor version number of the engine."),
	OpSpec( "VMJR", GeneralHandlers::IntegerRV_VMJR,  "", "", categoryVariables, "Returns the major version number of the engine."),
	OpSpec( "WPAU", GeneralHandlers::IntegerRV_WPAU,  "", "", categoryTime, "Returns 1 if world ticks are paused, or 0 otherwise."),
	OpSpec( "VISI", AgentHandlers::IntegerRV_VISI,  "i", "checkAllCameras", categoryAgents, "Checks if the agent, or any of its parts, is on screen and returns 1 if it is or 0 if it is not.  Set to 0 to check if the agent is on the main camera. Set to 1 to check if the agent is on the main camera or any remote cameras"),
	OpSpec( "TYPE", AgentHandlers::IntegerRV_TYPE,  "m", "something", categoryVariables, "Determines the type of a variable. The type is one of the following:<br>0 - integer<br>1 - floating-point<br>2 - string<br>3 - simple agent<br>4 - pointer agent<br>5 - compound agent<br>6 - vehicle<br>7 - creature<br>ERROR codes for agents:<br>-1 - NULL agent handle<br>-2 - Unknown agent - you should <i>never</i> get this"),
	OpSpec( "CLIK", AgentHandlers::IntegerRV_CLIK, "i","which_value", categoryAgents, "This returns the CLIK action of the @#TARG@ object. If the object is in @#CLAC@ mode, then it returns -2, else the return values are as follows:<br>0 -&gt; Current click action number (1,2,3)<br>1 -&gt; First CLIK action.<br>2 -&gt; Second CLIK action.<br>3 -&gt; Third CLIK action"),
	OpSpec( "CLAC", AgentHandlers::IntegerRV_CLAC, "","", categoryAgents, "This returns the CLAC action of the @#TARG@ object. If the TARG is in @#CLIK@ mode, then the return value is -2. Otherwise it is the CLAC action."),
	OpSpec( "BODY", CreatureHandlers::IntegerRV_BODY, "i","bodyPart", categoryCreatures, "Return the set number of the outfit the norn is wearing on the outer most layer or -1 if it is not wearing anything "),
	OpSpec( "STOI", GeneralHandlers::IntegerRV_STOI, "s", "value", categoryVariables, "Converts a string in decimal to an integer.  Characters in the string after an initial number are quietly ignored.  If there is no obvious number then zero is returned." ),
	OpSpec( "TAGE", CreatureHandlers::IntegerRV_TAGE, "", "", categoryCreatures, "Returns the age in ticks since the target creature was @#BORN@.  Ticking stops when the creature dies - see @#DEAD@." ),
	OpSpec( "HIST", HistoryHandlers::IntegerRV_HIST,  idSubIntegerRVTable_HIST, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "RTIM", GeneralHandlers::IntegerRV_RTIM, "", "", categoryTime, "Returns the current real world time.  This is measured in seconds since midnight, 1 January 1970 in UTC. To display, use @#RTIF@."),
	OpSpec( "MOWS", AgentHandlers::IntegerRV_MOWS, "", "", categoryAgents, "Returns whether the lawn was cut last Sunday or not."),
	OpSpec( "OOWW", HistoryHandlers::IntegerRV_OOWW, "s", "moniker", categoryHistory, "Returns the status of the moniker.<p>0 - never existed, or history purged<br>1 - genome referenced by a slot, for example an egg<br>2 - creature made with @#NEW: CREA@<br>3 - creature properly @#BORN@<br>4 - out of world, exported<br>5 - dead, body still exists<br>6 - dead, body @#KILL@ed<br>7 - unreferenced genome"),
	OpSpec( "REAN", GeneralHandlers::IntegerRV_REAN, "s", "catalogue_tag", categoryVariables, "Returns the number of entries in the catalogue for the given tag.  For the same tag, you can @#READ@ values from 0 to one less than REAN returns." ),
	OpSpec( "REAQ", GeneralHandlers::IntegerRV_REAQ, "s", "catalogue_tag", categoryVariables, "Returns 1 if the catalogue tag is present, 0 if not." ),
	OpSpec( "PRT:", PortHandlers::IntegerRV_PRT, idSubIntegerRVTable_PRT, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "GMAP", MapHandlers::IntegerRV_GMAP,  "ff", "x y", categoryMap, "Returns the metaroom id at point x,y on the map. If the point is outside the room system, it returns -1."),
	OpSpec( "DATE", GeneralHandlers::IntegerRV_DATE,  "", "", categoryTime, "Returns the day within the current season, from 0 to @#GAME@ \"engine_LengthOfSeasonInDays\" - 1.  See also @#HIST DATE@." ),
	OpSpec( "LOFT", DisplayHandlers::IntegerRV_LOFT,  "s", "filename", categoryCamera, "Declares that you have finished with a photograph image file taken by @#SNAP@.  If the file is in use in a gallery, this function fails and returns 1.  Otherwise it returns 0.  The file will be marked for the attic, and moved there later." ),
	OpSpec( "SNAX", DisplayHandlers::IntegerRV_SNAX,  "s", "filename", categoryCamera, "Returns 1 if the specified image file exists, or 0 if it doesn't.  Use with @#SNAP@ to find a unique filename to use." ),
	OpSpec( "INNI", GeneralHandlers::IntegerRV_INNI,  "", "", categoryFiles, "Retrieves an integer from the input stream, delimited by white space.  Defaults to 0 if no valid data." ),
	OpSpec( "INOK", GeneralHandlers::IntegerRV_INOK,  "", "", categoryFiles, "Returns 1 if the input stream is good, or 0 if it is bad.  A bad stream could be a non existent file, or the end of file reached.  If the stream has never been opened at all, an error is displayed." ),
	OpSpec( "EXPR", CreatureHandlers::IntegerRV_EXPR,  "", "", categoryCreatures, "Returns the current facial expression index for the creature.  Expressions are as follows:   EXPR_NORMAL = 0, EXPR_HAPPY =1, EXPR_SAD = 2, EXPR_ANGRY= 3, EXPR_SURPRISE = 4, EXPR_SLEEPY = 5, EXPR_VERY_SLEEPY = 6, EXPR_VERY_HAPPY = 7, EXPR_MISCHEVIOUS = 8, EXPR_SCARED = 9, EXPR_ILL = 10, EXPR_HUNGRY = 11"),	
	OpSpec( "DAYT", GeneralHandlers::IntegerRV_DAYT,"" , "", categoryTime, "Returns the current day of the month" ),
	OpSpec( "MONT", GeneralHandlers::IntegerRV_MONT,"" , "", categoryTime, "Returns the month of the year" ),
	OpSpec( "PAGE", CompoundHandlers::IntegerRV_PAGE, "", "", categoryCompound, "Returns the current page for current text part.  See the @#PAGE@ command for more information."),
	OpSpec( "NPGS", CompoundHandlers::IntegerRV_NPGS, "", "", categoryCompound, "Returns the number of available pages for current text part."),
	OpSpec( "ORGN", CreatureHandlers::IntegerRV_ORGN, "", "", categoryCreatures, "Returns the number of organs in target creature."  ),
	OpSpec( "ORGI", CreatureHandlers::IntegerRV_ORGI, "ii", "organ_number data", categoryCreatures, "Returns integer data about the specified organ.  The organ number is from 0 to @#ORGN@ - 1.  The data parameter specifies what information is returned:<p>0 - receptor count<br>1 - emitter count<br>2 - reaction count."  ),
	OpSpec( "FTOI", GeneralHandlers::IntegerRV_FTOI, "f", "number_to_convert", categoryVariables, "Converts a floating-point value into its integer equivalent." ),
	OpSpec( "MUTE", GeneralHandlers::IntegerRV_MUTE, "ii", "andMask eorMask", categorySounds, "This returns (and potentially sets) the mute values for the sound managers in the game. Sensible settings for the parameters are as follows:<p><table border=1><tr><th>andMask</th><th>eorMask</th><th>returns</th></tr><tr><td>0</td><td>3</td><td>3 - Mutes both sound and music</td></tr><tr><td>3</td><td>0</td><td>0 for no mute<br>1 for sound muted<br>2 for music muted<br>3 for both muted<br>Sets nothing</td></tr><tr><td>1</td><td>2</td><td>Returns 2 for music muted, or 3 for both muted<br>Only sets mute on music, leaves sound alone</td></tr></table>"),
	OpSpec( "WOLF", GeneralHandlers::IntegerRV_WOLF, "ii", "kanga_mask eeyore_mask", categoryTime, "Provides various functions to distort space-time and otherwise help with wolfing runs.  Set an AND and an EOR mask, to control the following bits:<p>1 - Display rendering.  Turning it off speeds the game up.<br>2 - Fastest ticks.  The game usually runs at a maximum of 20 frames per second.  If this is set, it instead runs as fast as it can.<br>4 - Refresh display at end of tick.  If set, then the display is updated at the end of the tick, and the flag is cleared.<br>8 - Autokill.  If set, agents which generate run errors are automatically killed, as the command line option.<p>"),
	OpSpec( "RACE", GeneralHandlers::IntegerRV_RACE, "", "", categoryTime, "Returns the time in milliseconds which the last tick took overall.  This differs from @#PACE@ in that on fast machines it will have a minimum of 50 milliseconds.  It accounts for all the time in the tick, including event handling and window processing."),
	OpSpec( "WDOW", DisplayHandlers::IntegerRV_WDOW,  "", "", categoryCamera, "Returns 1 if in full screen mode, or 0 if in windowed mode." ),
	OpSpec( "SCOL", GeneralHandlers::IntegerRV_SCOL, "iibb", "and_mask eor_mask up_speeds down_speeds", categoryTime, "Set an AND and an EOR mask, to control the following scrolling functions:<p>1 - Screen edge nudgy scrolling<br>2 - Keyboard scrolling<br>4 - Middle mouse button screen dragging<br>8 - Mouse wheel screen scrolling<p>The byte strings is used for nudgy and keyboard scrolling.  The defaults are [1 2 4 8 16 32 64] and [0 1 2 4 8 16 32].  They represent the number of pixels scrolled each consecutive tick, as the scrolling gets slower and faster.<p>If you use [] for a byte string, then the string won't be changed at all."),
	OpSpec( "SORQ", GeneralHandlers::IntegerRV_SORQ, "iiii", "family genus species event", categoryScripts, "Returns 1 if the script is in the scriptorium, or if there is a general event script for the entire genus, or family.  Returns 0 if there is no matching script." ),
	OpSpec( "MSEC", GeneralHandlers::IntegerRV_MSEC, "", "", categoryTime, "Returns a time stamp measured in milliseconds.  It is not specified when the time is measured from; the stamp is only guaranteed to be consistent during one session."),
	OpSpec( "DREA", CreatureHandlers::IntegerRV_DREA,  "", "", categoryCreatures, "Returns 1 if the creature is asleep and dreaming, 0 otherwise." ),
	OpSpec( "MIRA", AgentHandlers::IntegerRV_MIRA,  "", "", categoryAgents, "Is the current sprite for this agent mirrored (returns 1) or not (returns 0)"),
	OpSpec( "ZOMB", CreatureHandlers::IntegerRV_ZOMB,  "", "", categoryCreatures, "Returns 1 if the creature is zombified (has its brain to motor link severed), 0 otherwise." ),
	OpSpec( "WNTI", GeneralHandlers::IntegerRV_WNTI, "s", "world", categoryWorld, "This returns the index of the <i>world</i>. If you pass in a world name which is not within the system, -1 is returned."),
	OpSpec( "CABV", AgentHandlers::IntegerRV_CABV,  "", "", categoryVehicles, "Returns the cabin room number."),
	OpSpec( "HEAP", DebugHandlers::IntegerRV_HEAP,  "i", "index", categoryDebug, "Returns heap and garbage collection information.<br>0 - current allocated heap memory (development builds only)<br>1 - total agents, including ones waiting to be garbage collected<br>2 - similar, but just for creatures"),
};


OpSpec ourStringRVTable[] =
{
	OpSpec( "VTOS", GeneralHandlers::StringRV_VTOS, "d", "value", categoryVariables, "Converts an integer or float into a string in decimal." ),
	OpSpec( "SORC", GeneralHandlers::StringRV_SORC, "iiii", "family genus species event", categoryScripts, "Returns the source code for the specified script.  Use the @#GIDS@ commands to find available scripts." ),	
	OpSpec( "BKGD", MapHandlers::StringRV_BKGD, "i", "metaroom_id", categoryCamera, "Returns the name of the background file currently shown by the given camera." ),
	OpSpec( "READ", GeneralHandlers::StringRV_READ, "si", "catalogue_tag offset", categoryVariables, "Returns a string from the catalogue.  This is used for localisation.  offset 0 is the first string after the TAG command in the catalogue file.  See also @#REAN@ and @#WILD@." ),
	OpSpec( "SUBS", GeneralHandlers::StringRV_SUBS, "sii", "value start count", categoryVariables, "Slices up a string, returing the substring starting at position start, with length count.  String indices begin at 1." ), 
	OpSpec( "FACE", CreatureHandlers::StringRV_FACE, "", "", categoryCreatures, "Returns the name of the sprite file for the target creature's face.  Currently automatically gives you the youngest age version of the gallery but soon will work in the following way: If you set the parameter to -1 you will get the name of the file the creature is currently using.  Note that when the creature ages, this file name will change (the @#GALL@ command could be useful here).  If you set the parameter to a particular age then the filename returned will be the gallery that best matches that age.  Use the @#FACE@ integer rvalue to get the pose number for facing forwards."),
	OpSpec( "BUTY", GeneralHandlers::StringRV_BUTY, "", "", categoryNoNeedToDocument, "X"), // probably shouldn't document this :-) hence the X
	OpSpec( "RMSC", SoundHandlers::StringRV_RMSC, "ii", "x y", categorySounds, "Returns the name of the music track played at the room in the given location."),
	OpSpec( "MMSC", SoundHandlers::StringRV_MMSC, "ii", "x y", categorySounds, "Returns the name of the music track played at the metaroom in the given location."),
	OpSpec( "PTXT", CompoundHandlers::StringRV_PTXT, "", "", categoryCompound, "Returns the string of the current text part.  See the @#PTXT@ command for more information."),
	OpSpec( "DBG#", DebugHandlers::StringRV_DBG, "i", "variable", categoryDebug, "Dumps debug information for the virtual machine of target.  Whatever the type of the variable, a string is output.<p>Variable can be:<br> -1 : Whether in INST or not<br> -2 : Whether in LOCK or not<br> -3 : Current TARG of virtual machine<br> -4 : OWNR - should be the same as our outer TARG<br> -5 : FROM - who sent the message which is being run<br> -6 : IT - if a Creature, where their attention was<br> -7 : PART - part number being worked on for compound agents<br> -8 : _P1_ - first parameter of message, if in a message<br> -9 : _P2_ - second parameter of message, if in a mesesage<br> 0 to 99 : Local variables VA00 to VA99" ),
	OpSpec( "DBGA", DebugHandlers::StringRV_DBGA, "i", "variable", categoryDebug, "Dumps debug information for target.  Whatever the type of the variable, a string is output.<p>Variable can be:<br>0 to 99 : agent variables OV00 to OV99<br>-1 : Counter for timer tick" ),
	OpSpec( "WILD", GeneralHandlers::StringRV_WILD, "iiisi", "family genus species tag_stub offset", categoryAgents, "Searches for a catalogue tag based on the given classifier, and returns the string at the given offset.  See also @#READ@.  As an example, with a tag_stub of \"Agent Help\" and a classifier 3 7 11 it would first look for the tag \"Agent Help 3 7 11\".  If that wasn't present, it would go through the wildcards, eventually trying \"Agent Help 0 0 0\", and throwing an error if even that isn't there."),
	OpSpec( "CATX", CreatureHandlers::StringRV_CATX, "i", "category_id", categoryAgents, "Returns the name of the given category.  For example, \"toy\" or \"bad bug\".  The catalogue tag \"Agent Categories\" stores these.  If the id is out of range, CATX returns an empty string." ),
	OpSpec( "WRLD", GeneralHandlers::StringRV_WRLD, "i", "world_index", categoryWorld, "Returns the name of the world specified by world_index which must be in the range 0 to (@#NWLD@-1)." ),
	OpSpec( "CAOS", GeneralHandlers::StringRV_CAOS, "iimmsiiv", "inline state_trans p1 p2 commands throws catches report", categoryScripts, "Executes the specified CAOS commands instantly. The local environment (@#_IT_@ @#VAxx@ @#TARG@ @#OWNR@ etc.) will be promoted to the script's environment if inline is non-zero. If state_trans is non-zero, then @#FROM@ and @#OWNR@ are propogated, if zero, then the script is run orphaned. CAOS returns the output of the script. As you can put multiple scripts through in one call, the output is potentially concatenated. Note that all sets of scripts are executed in the same virtual machine if inline is non-zero, otherwise the virtual machine is reset before each script is passed. The params _p0_ and _p1_ are passed in as the parameters to the script, even when inline.  You can execute \"outv 7 endm scrp 3 7 11 6 outv 3 endm outv 9\", which will make a script 3 7 11 6 and return \"79\". <p>If throws is non-zero then the system will throw exceptions, otherwise it will return \"***\" with report set to the exception sid in the CAOS catalogue TAG. If catches is non-zero then the system will catch any run errors encountered and return them in report, having set the return value to \"###\" first."),
	OpSpec( "RATE", MapHandlers::StringRV_RATE, "ii", "room_type ca_index", categoryMap, "Returns a string containing gain, loss and diffusion rates for that combination of room type and CA." ),
	OpSpec( "EMID", MapHandlers::StringRV_EMID, "", "", categoryMap, "Returns a string containing all the metaroom ids in the world seperated by spaces." ),
	OpSpec( "ERID", MapHandlers::StringRV_ERID, "i", "metaroom_id", categoryMap, "Returns a string containing all the room ids in the specified metaroom separated by spaces.  Returns all rooms in the world if metaroom_id is -1." ),
	OpSpec( "RLOC", MapHandlers::StringRV_RLOC, "i", "room_id", categoryMap, "Returns the location of the specified room as a string formated as follows: xLeft xRight yLeftCeiling yRightCeiling yLeftFloor yRightFloor." ),
	OpSpec( "MLOC", MapHandlers::StringRV_MLOC, "i", "metaroom_id", categoryMap, "Returns the location of the specified metaroom as a string formated as follows: x y width height." ),
	OpSpec( "BKDS", MapHandlers::StringRV_BKDS, "i", "metaroom_id", categoryMap, "Returns a string containing all the background names for the specified metaroom in a comma seperated list." ),
	OpSpec( "PRAY", PrayHandlers::StringRV_PRAY, idSubStringRVTable_PRAY, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "WNAM", GeneralHandlers::StringRV_WNAM, "", "", categoryWorld, "Returns the name of the currently loaded world." ),
	OpSpec( "GNAM", GeneralHandlers::StringRV_GNAM, "", "", categoryVariables, "Returns the game name.  For example \"Creatures 3\"." ),
	OpSpec( "PSWD", GeneralHandlers::StringRV_PSWD, "i", "worldIndex", categoryWorld, "Returns the password for the specified world.  If the world is not password protected the return value will be an empty string." ),
	OpSpec( "GTOS", CreatureHandlers::StringRV_GTOS, "i", "slot", categoryGenetics, "Returns the target's moniker in the given gene variable slot.  This universally unique identifier is the name of a genetics file.  Slot 0 is a creature's actual genome.  Other slots are used in pregnant creatures, eggs and other places." ),
	OpSpec( "WUID", GeneralHandlers::StringRV_WUID, "", "", categoryWorld, "Returns the unique identifier of the currently loaded world." ),
	OpSpec( "HIST", HistoryHandlers::StringRV_HIST, idSubStringRVTable_HIST, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "VOIS", AgentHandlers::StringRV_VOIS, "", "", categorySounds, "This returns the voice name for the @#TARG@ agent."),
	OpSpec( "RTIF", GeneralHandlers::StringRV_RTIF, "is", "real_time format", categoryTime, "Takes a real world time, as returned by @#RTIM@ or @#HIST RTIM@ and converts it to	a localised string for display.  The format string is made up of any text, with the following special codes:<p>%a - Abbreviated weekday name<br>%A - Full weekday name<br>%b - Abbreviated month name<br>%B - Full month name<br>%c - Date and time representation appropriate for locale<br>%d - Day of month as decimal number (01 - 31)<br>%H - Hour in 24-hour format (00 - 23)<br>%I - Hour in 12-hour format (01 - 12)<br>%j - Day of year as decimal number (001 - 366)<br>%m - Month as decimal number (01 - 12)<br>%M - Minute as decimal number (00 - 59)<br>%p - Current locale’s AM/PM indicator for 12-hour clock<br>%S - Second as decimal number (00 - 59)<br>%U - Week of year as decimal number, with Sunday as first day of week (00 - 53)<br>%w - Weekday as decimal number (0 - 6; Sunday is 0)<br>%W - Week of year as decimal number, with Monday as first day of week (00 - 53)<br>%x - Date representation for current locale<br>%X - Time representation for current locale<br>%y - Year without century, as decimal number (00 - 99)<br>%Y - Year with century, as decimal number<br>%z, %Z - Time-zone name or abbreviation; no characters if time zone is unknown<br>%% - Percent sign<p>The # flag may prefix any formatting code, having the following meanings:<br>%#a, %#A, %#b, %#B, %#p, %#X, %#z, %#Z, %#% # flag is ignored. <br>%#c Long date and time representation, appropriate for current locale. For example: Tuesday, March 14, 1995, 12:41:29. <br>%#x Long date representation, appropriate to current locale. For example: Tuesday, March 14, 1995. <br>%#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y Remove leading zeros (if any).<p>You probably want to @#READ@ any formatted string you use from a catalogue file." ),
	OpSpec( "PRT:", PortHandlers::StringRV_PRT, idSubStringRVTable_PRT, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "INNL", GeneralHandlers::StringRV_INNL,  "", "", categoryFiles, "Retrieves a line of text from the input stream." ),
	OpSpec( "GAMN", GeneralHandlers::StringRV_GAMN, "s", "previous", categoryVariables, "Enumerates through game variable names.  Pass in an empty string to find the first one, and then the previous one to find the next.  Empty string is returned at the end." ),
	OpSpec( "FVWM", GeneralHandlers::StringRV_FVWM, "s", "name", categoryFiles, "This returns a guaranteed-safe filename for use in world names, jornal file names, etc."),
	OpSpec( "HAND", AgentHandlers::StringRV_HAND, "", "", categoryAgents, "This returns the name of the hand."),

};


OpSpec ourFloatRVTable[] =
{
	OpSpec( "ACCG", AgentHandlers::FloatRV_ACCG, "", "", categoryMotion, "Returns target's acceleration due to gravity in pixels per tick squared." ),
	OpSpec( "DRIV", CreatureHandlers::FloatRV_DRIV,  "i", "drive", categoryCreatures, "Returns the value (0.0 to 1.0) of the specified drive." ),
	OpSpec( "CHEM", CreatureHandlers::FloatRV_CHEM,  "i", "chemical", categoryCreatures, "Returns concentration (0.0 to 1.0) of chemical (1 to 255) in the target creature's bloodstream." ),
	OpSpec( "PROP", MapHandlers::FloatRV_PROP,  "ii", "room_id ca_index", categoryMap, "Returns the value of a CA in a room."),
	OpSpec( "RNGE", AgentHandlers::FloatRV_RNGE,  "", "", categoryAgents, "Returns the target's range. See @#ESEE@, @#OBST@."),
	OpSpec( "LOCI", CreatureHandlers::FloatRV_LOCI,  "iiii", "type organ tissue id", categoryCreatures, "Reads a biochemical locus value."),
	OpSpec( "MOVX", GeneralHandlers::FloatRV_MOVX,  "", "", categoryInput, "Returns horizontal mouse velocity." ),
	OpSpec( "MOVY", GeneralHandlers::FloatRV_MOVY,  "", "", categoryInput, "Returns vertical mouse velocity." ),
	OpSpec( "DFTX", CreatureHandlers::FloatRV_DFTX,  "", "", categoryCreatures, "Returns X coordinate of creature's down foot." ),
	OpSpec( "DFTY", CreatureHandlers::FloatRV_DFTY,  "", "", categoryCreatures, "Returns Y coordinate of creature's down foot." ),
	OpSpec( "UFTX", CreatureHandlers::FloatRV_UFTX,  "", "", categoryCreatures, "Returns X coordinate of creature's up foot." ),
	OpSpec( "UFTY", CreatureHandlers::FloatRV_UFTY,  "", "", categoryCreatures, "Returns Y coordinate of creature's up foot." ),
	OpSpec( "SIN_", GeneralHandlers::FloatRV_SIN,  "f", "theta", categoryVariables, "Returns sine of theta. Theta should be in degrees." ),
	OpSpec( "COS_", GeneralHandlers::FloatRV_COS,  "f", "theta", categoryVariables, "Returns cosine of theta. Theta should be in degrees." ),
	OpSpec( "TAN_", GeneralHandlers::FloatRV_TAN,  "f", "theta", categoryVariables, "Returns tangent of theta. Theta should be in degrees. Watch out for those nasty discontinuities at 90 and 270." ),
	OpSpec( "ASIN", GeneralHandlers::FloatRV_ASIN,  "f", "x", categoryVariables, "Returns arcsine of x in degrees." ),
	OpSpec( "ACOS", GeneralHandlers::FloatRV_ACOS,  "f", "x", categoryVariables, "Returns arccosine of x in degrees." ),
	OpSpec( "ATAN", GeneralHandlers::FloatRV_ATAN,  "f", "x", categoryVariables, "Returns arctangent of x in degrees." ),
	OpSpec( "SQRT", GeneralHandlers::FloatRV_SQRT,  "f", "value", categoryVariables, "Calculates a square root." ),
	OpSpec( "RELX", AgentHandlers::FloatRV_RELX,  "aa", "first second", categoryMotion, "Returns the relative X distance of the centre point of the second agent from the centre point of the first." ),
	OpSpec( "RELY", AgentHandlers::FloatRV_RELY,  "aa", "first second", categoryMotion, "Returns the relative Y distance of the centre point of the second agent from the centre point of the first." ),
	OpSpec( "POSX", AgentHandlers::FloatRV_POSX,  "", "", categoryAgents, "Returns X position of centre of target."),
	OpSpec( "POSY", AgentHandlers::FloatRV_POSY,  "", "", categoryAgents, "Returns Y position of centre of target."),
	OpSpec( "POSL", AgentHandlers::FloatRV_POSL,  "", "", categoryAgents, "Returns left position of target's bounding box."),
	OpSpec( "POSR", AgentHandlers::FloatRV_POSR,  "", "", categoryAgents, "Returns right position of target's bounding box."),
	OpSpec( "POSB", AgentHandlers::FloatRV_POSB,  "", "", categoryAgents, "Returns bottom position of target's bounding box."),
	OpSpec( "POST", AgentHandlers::FloatRV_POST,  "", "", categoryAgents, "Returns top position of target's bounding box."),
	OpSpec( "OBST", AgentHandlers::FloatRV_OBST,  "i", "direction", categoryMotion, "Returns the distance from the agent to the nearest wall that it might collide with in the given direction. Directions are @#LEFT@, @#RGHT@, @#_UP_@, or @#DOWN@. If the distance to the collsion is greater than @#RNGE@ then a very large number is returned." ),
	OpSpec( "TORX", MapHandlers::FloatRV_TORX,  "i", "room_id", categoryMap, "Returns relative X position of the centre of the given room from target's top left corner."), 
	OpSpec( "TORY", MapHandlers::FloatRV_TORY,  "i", "room_id", categoryMap, "Returns relative Y position of the centre of the given room from target's top left corner."),
#ifdef C2E_OLD_CPP_LIB
// can't use stringstream...
	OpSpec( "PACE", GeneralHandlers::FloatRV_PACE, "", "", categoryTime, "Returns the tick rate satisfaction factor.<br>Factor 1 - ticks are taking the time we would expect them to, which is ????  secs.<br>Factor more than 1 - the engine is running too slowly.<br>Factor less than 1 - the engine is leaving spare processing time.<br>This is averaged over the last ????  ticks.<p>Agents can look at this to adjust the resources they use according to current spare processing time.  For example, if you have a random snowflake generator in winter, you could increase the chance of generation if PACE is low, and decrease the chance if PACE is high.  When you do this remember that computers will be arbitarily faster in the future, so you should place an extra upper limit on the number of snowflakes to stop them filling the whole screen.<p>Note that PACE only measures the time the engine takes for tick processing, not for handling requests from external applications, or adding Windows events to its internal queue.  Because of this, you should aim for a value which is a bit less than 1.<p>Compare @#RACE@." ),
#else
// original stringstream version:
// should really just use constants here anyway!
// The documentation shouldn't change with ingame settings!
	OpSpec( "PACE", GeneralHandlers::FloatRV_PACE, "", "", categoryTime, ((std::ostringstream*)(&(std::ostringstream() << "Returns the tick rate satisfaction factor.<br>Factor 1 - ticks are taking the time we would expect them to, which is " << (float)App::GetWorldTickInterval() / 1000 << " secs.<br>Factor more than 1 - the engine is running too slowly.<br>Factor less than 1 - the engine is leaving spare processing time.<br>This is averaged over the last " << App::ourTickLengthsAgo << " ticks.<p>Agents can look at this to adjust the resources they use according to current spare processing time.  For example, if you have a random snowflake generator in winter, you could increase the chance of generation if PACE is low, and decrease the chance if PACE is high.  When you do this remember that computers will be arbitarily faster in the future, so you should place an extra upper limit on the number of snowflakes to stop them filling the whole screen.<p>Note that PACE only measures the time the engine takes for tick processing, not for handling requests from external applications, or adding Windows events to its internal queue.  Because of this, you should aim for a value which is a bit less than 1.<p>Compare @#RACE@.")))->str() ),
#endif
	OpSpec( "STOF", GeneralHandlers::FloatRV_STOF, "s", "value", categoryVariables, "Converts a string in decimal to a floating point number.  Characters in the string after an initial number are quietly ignored.  If there is no obvious number then zero is returned." ),
	OpSpec( "MTHX", CreatureHandlers::FloatRV_MTHX, "", "", categoryAgents, "This returns the X position of the @#TARG@ creature's mouth attachment point in absolute (map) coordinates."),
	OpSpec( "MTHY", CreatureHandlers::FloatRV_MTHY, "", "", categoryAgents, "This returns the Y position of the @#TARG@ creature's mouth attachment point in absolute (map) coordinates."),
	OpSpec( "FLTX", AgentHandlers::FloatRV_FLTX, "", "", categoryAgents, "This returns the X position of the @#TARG@ object's floating vector."),
	OpSpec( "FLTY", AgentHandlers::FloatRV_FLTY, "", "", categoryAgents, "This returns the Y position of the @#TARG@ object's floating vector."),
	OpSpec( "INNF", GeneralHandlers::FloatRV_INNF,  "", "", categoryFiles, "Retrieves a float from the input stream, delimited by white space.  Defaults to 0.0 if no valid data." ),
	OpSpec( "ORGF", CreatureHandlers::FloatRV_ORGF, "ii", "organ_number data", categoryCreatures, "Returns floating point data about the specified organ.  The organ number is from 0 to @#ORGN@ - 1.  The data parameter specifies what information is returned:<p>0 - Clock rate in updates per tick (as locus)<br>1 - Short term life force as a proportion of intial (as locus)<br>2 - Factor to modulate rate of repair (as locus)<br>3 - Injury to apply (as locus)<br>4 - Initial life force, a million is the largest initial value<br>5 - Short term life force, temporary damage<br>6 - Long term life force, permanent damage<br>7 - Long term rate damage during repair<br>8 - Energy cost to run this organ, calculated from the number of receptors, emitters and reactions<br>9 - Damage done to the organ if no energy is available" ),
	OpSpec( "ITOF", GeneralHandlers::FloatRV_ITOF, "i", "number_to_convert", categoryVariables, "Converts an integer value into its floating-point equivalent."),
	OpSpec( "DISQ", AgentHandlers::FloatRV_DISQ,  "a", "other", categoryAgents, "Returns the square of the distance between the centre points of the target agent, and the other agent.  It is quicker to compare this square against a squared constant directly, or if you need the actual distance use @#SQRT@."),
};

OpSpec ourAgentRVTable[] =
{
	OpSpec( "TARG", 	CAOSMachine::AgentRV_TARG,  "", "", categoryAgents, "Returns current target, on whom many commands act." ),
	OpSpec( "OWNR", CAOSMachine::AgentRV_OWNR,  "", "", categoryAgents, "Returns the agent who's virtual machine the script is running on.  Returns @#NULL@ for injected or install scripts."),
	OpSpec( "FROM", CAOSMachine::AgentRV_FROM,  "", "", categoryAgents, "If we're processing a message, this is the @#OWNR@ who sent the message.  @#NULL@ if the message was sent from an injected script or an install script." ),
	OpSpec( "_IT_", CAOSMachine::AgentRV_IT,  "", "", categoryAgents, "Returns the agent @#OWNR@'s attention was on <i>when the current script was entered</i>.  This is only valid if OWNR is a creature.  Compare @#IITT@." ),
	OpSpec( "PNTR", AgentHandlers::AgentRV_PNTR,  "", "", categoryAgents, "Returns the mouse pointer, which is also known as the hand." ),
	OpSpec( "NORN", AgentHandlers::AgentRV_NORN,  "", "", categoryCreatures, "Returns the creature currently selected by the user." ),
	OpSpec( "NULL", AgentHandlers::AgentRV_NULL, "", "", categoryAgents, "Returns a null agent pointer." ),
	OpSpec( "HELD", AgentHandlers::AgentRV_HELD,  "", "", categoryAgents, "Returns the item currently held by the target.  For vehicles this returns a random carried agent if carrying more than one.  Consider using @#EPAS@ instead." ),
	OpSpec( "CARR", AgentHandlers::AgentRV_CARR,  "", "", categoryAgents, "Returns the the agent currently holding the target, or @#NULL@ if there is none." ),
	OpSpec( "AGNT", AgentHandlers::AgentRV_AGNT,  "i", "unique_id", categoryDebug, "Given a unique identifier, returns the corresponding agent.  Returns @#NULL@ if the agent no longer exists.  @#UNID@ extracts the unique id.  NOTE: This should only be used for external programs to persistently refer to an agent. Variables can use @#SETA@ to store agent r-values directly for internal use." ),
	OpSpec( "TRCK", DisplayHandlers::AgentRV_TRCK,  "", "", categoryCamera, "Returns the agent being tracked by the camera, if any."),
	OpSpec( "HOTS", AgentHandlers::AgentRV_HOTS,  "", "", categoryInput, "Returns the agent nearest the screen under the hotspot of the pointer.  For each agent, @#TRAN@ decides whether this allows for transparent pixels."),
	OpSpec( "TWIN", AgentHandlers::AgentRV_TWIN,  "ai", "original agent_null", categoryAgents, "Clones an agent, and returns the replica.  If agent_null is set to 1 the agents that this agent points to (in OVxx, or VAxx in its running script) are set to NULL in the clone.  If agent_null is 0, then the clone points to the same agents as the original.  When using agent_null 1, you might want to call @#STPT@ first so variables being used mid-script aren't cleared under the agent's nose."),
	OpSpec( "NCLS", AgentHandlers::AgentRV_NCLS,  "aiii", "previous family genus species", categoryAgents, "Finds the next agent in the agent list which also matches the given classifier.  If the previous agent doesn't exist or doesn't match the classifier then the first agent matching it is returned.  If none match the classifier, then @#NULL@ is returned."),
	OpSpec( "PCLS", AgentHandlers::AgentRV_PCLS,  "aiii", "next family genus species", categoryAgents, "Same as @#NCLS@, only cycles the other way."),
	OpSpec( "IITT", CreatureHandlers::AgentRV_IITT, "", "", categoryAgents, "Returns the target creature's current agent of attention.  Compare @#_IT_@."),
	OpSpec( "MTOC", CreatureHandlers::AgentRV_MTOC, "s", "moniker", categoryGenetics, "Returns the creature with the given moniker.  If there is no agent alive with that moniker, then returns @#NULL@.  See also @#MTOA@."),
	OpSpec( "MTOA", CreatureHandlers::AgentRV_MTOA, "s", "moniker", categoryGenetics, "Returns the agent which references the given moniker.  The moniker could be stored in any of the gene slots for that agent, including the special slot 0 for a creature.  If the moniker is not currently used in the game, then returns @#NULL@.  This command can be slow - use @#MTOC@ if possible."),
	OpSpec( "PRT:", PortHandlers::AgentRV_PRT, idSubAgentRVTable_PRT, "subcommand", categoryNoNeedToDocument, "X" ),
	OpSpec( "HHLD", CreatureHandlers::AgentRV_HHLD, "", "", categoryCreatures, "Returns the creature currently holding hands with the pointer agent. NULL if no agent is holding hands."),
	OpSpec( "TACK", DebugHandlers::AgentRV_TACK, "", "", categoryDebug, "Returns the agent currently being @#DBG: TACK@ed."), 
};

#define TEN_VA_DEFS( nameprefix )\
	OpSpec( nameprefix "0", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "1", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "2", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "3", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "4", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "5", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "6", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "7", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "8", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "9", CAOSMachine::Variable_VAnn, "", "", categoryVariables, "X" ),

#define TEN_OV_DEFS( nameprefix )\
	OpSpec( nameprefix "0", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "1", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "2", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "3", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "4", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "5", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "6", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "7", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "8", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "9", AgentHandlers::Variable_OVnn, "", "", categoryVariables, "X" ),

#define TEN_MV_DEFS( nameprefix )\
	OpSpec( nameprefix "0", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "1", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "2", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "3", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "4", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "5", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "6", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "7", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "8", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),\
	OpSpec( nameprefix "9", AgentHandlers::Variable_MVnn, "", "", categoryVariables, "X" ),

OpSpec ourVariableTable[] =
{
	TEN_VA_DEFS( "VA0" )	// VA00..VA09
	TEN_VA_DEFS( "VA1" )	// VA10..VA19
	TEN_VA_DEFS( "VA2" )	// VA20..VA29
	TEN_VA_DEFS( "VA3" )	// VA30..VA39
	TEN_VA_DEFS( "VA4" )	// VA40..VA49
	TEN_VA_DEFS( "VA5" )	// VA50..VA59
	TEN_VA_DEFS( "VA6" )	// VA60..VA69
	TEN_VA_DEFS( "VA7" )	// VA70..VA79
	TEN_VA_DEFS( "VA8" )	// VA80..VA89
	TEN_VA_DEFS( "VA9" )	// VA90..VA99
	// object variables
	TEN_OV_DEFS( "OV0" )	// OV00..OV09
	TEN_OV_DEFS( "OV1" )	// OV10..OV19
	TEN_OV_DEFS( "OV2" )	// OV20..OV29
	TEN_OV_DEFS( "OV3" )	// OV30..OV39
	TEN_OV_DEFS( "OV4" )	// OV40..OV49
	TEN_OV_DEFS( "OV5" )	// OV50..OV59
	TEN_OV_DEFS( "OV6" )	// OV60..OV69
	TEN_OV_DEFS( "OV7" )	// OV70..OV79
	TEN_OV_DEFS( "OV8" )	// OV80..OV89
	TEN_OV_DEFS( "OV9" )	// OV90..OV99
	// Machine variables
	TEN_MV_DEFS( "MV0" )	// MV00..MV09
	TEN_MV_DEFS( "MV1" )	// MV10..MV19
	TEN_MV_DEFS( "MV2" )	// MV20..MV29
	TEN_MV_DEFS( "MV3" )	// MV30..MV39
	TEN_MV_DEFS( "MV4" )	// MV40..MV49
	TEN_MV_DEFS( "MV5" )	// MV50..MV59
	TEN_MV_DEFS( "MV6" )	// MV60..MV69
	TEN_MV_DEFS( "MV7" )	// MV70..MV79
	TEN_MV_DEFS( "MV8" )	// MV80..MV89
	TEN_MV_DEFS( "MV9" )	// MV90..MV99
	OpSpec( "VELX", AgentHandlers::Variable_VELX, "", "", categoryMotion, "Horizontal velocity in pixels per tick - floating point." ),
	OpSpec( "VELY", AgentHandlers::Variable_VELY, "", "", categoryMotion, "Vertical velocity in pixels per tick - floating point." ),
	OpSpec( "GAME", GeneralHandlers::Variable_GAME, "s", "variable_name", categoryVariables, "A game variable is a global variable which can be referenced by name.  <blockquote>eg: SETV GAME \"pi\" 3.142</blockquote>Game variables are stored as part of the world and so will be saved out in the world file. If a script uses a non-existant game variable, that variable will be created automatically (with undefined contents).  Agents, integers, floats and strings can be stored in game variables.  Variable names are case sensitive.  <p>There are some conventions for the variable names:<br> engine_ for Creatures Engine<br> cav_ for Creatures Adventures<br> c3_	for Creatures 3<p>It's important to follow these, as 3rd party developers will just use whatever names they fancy.  @#DELG@ deletes a game variable.  See also the table of engine @#Game Variables@." ),
	OpSpec( "_P1_", CAOSMachine::Variable_P1,  "", "", categoryVariables, "Returns the first parameter sent to a script." ),
	OpSpec( "_P2_", CAOSMachine::Variable_P2,  "", "", categoryVariables, "Returns the second parameter sent to a script." ),
	OpSpec( "AVAR", AgentHandlers::Variable_AVAR, "ai", "agent index", categoryVariables, "This is the OVnn variable of the agent passed in.  It is equivalent to target agent, OVnn, but means that you don't need to target it first :)  You can also use them to implement primitive arrays."),
};


void CAOSDescription::LoadDefaultTables()
{
	myTables.clear();
	
	PushTable(idCommandTable, ourCommandTable, sizeof(ourCommandTable) / sizeof(OpSpec));
	
	PushTable(idIntegerRVTable, ourIntegerRVTable, sizeof(ourIntegerRVTable) / sizeof(OpSpec));
	PushTable(idVariableTable, ourVariableTable, sizeof(ourVariableTable) / sizeof(OpSpec));
	PushTable(idFloatRVTable, ourFloatRVTable, sizeof(ourFloatRVTable) / sizeof(OpSpec));
	PushTable(idStringRVTable, ourStringRVTable, sizeof(ourStringRVTable) / sizeof(OpSpec));
	PushTable(idAgentRVTable, ourAgentRVTable, sizeof(ourAgentRVTable) / sizeof(OpSpec));
	PushTable(idSubCommandTable_NEW, ourSubCommandTable_NEW, sizeof(ourSubCommandTable_NEW) / sizeof(OpSpec));
	PushTable(idSubCommandTable_MESG, ourSubCommandTable_MESG, sizeof(ourSubCommandTable_MESG) / sizeof(OpSpec));
	PushTable(idSubCommandTable_STIM, ourSubCommandTable_STIM, sizeof(ourSubCommandTable_STIM) / sizeof(OpSpec));
	PushTable(idSubCommandTable_URGE, ourSubCommandTable_URGE, sizeof(ourSubCommandTable_URGE) / sizeof(OpSpec));
	PushTable(idSubCommandTable_SWAY, ourSubCommandTable_SWAY, sizeof(ourSubCommandTable_SWAY) / sizeof(OpSpec));
	PushTable(idSubCommandTable_ORDR, ourSubCommandTable_ORDR, sizeof(ourSubCommandTable_ORDR) / sizeof(OpSpec));
	PushTable(idSubCommandTable_GIDS, ourSubCommandTable_GIDS, sizeof(ourSubCommandTable_GIDS) / sizeof(OpSpec));

	PushTable(idSubCommandTable_PRT, ourSubCommandTable_PRT, sizeof(ourSubCommandTable_PRT) / sizeof(OpSpec));

	PushTable(idSubCommandTable_PAT, ourSubCommandTable_PAT, sizeof(ourSubCommandTable_PAT) / sizeof(OpSpec));
	PushTable(idSubCommandTable_DBG, ourSubCommandTable_DBG, sizeof(ourSubCommandTable_DBG) / sizeof(OpSpec));	
	PushTable(idSubCommandTable_BRN, ourSubCommandTable_BRN, sizeof(ourSubCommandTable_BRN) / sizeof(OpSpec));	

	PushTable(idSubCommandTable_PRAY, ourSubCommandTable_PRAY, sizeof(ourSubCommandTable_PRAY) / sizeof(OpSpec));	
	PushTable(idSubIntegerRVTable_PRAY, ourSubIntegerRVTable_PRAY, sizeof(ourSubIntegerRVTable_PRAY) / sizeof(OpSpec));	
	PushTable(idSubStringRVTable_PRAY, ourSubStringRVTable_PRAY, sizeof(ourSubStringRVTable_PRAY) / sizeof(OpSpec));

	PushTable(idSubCommandTable_GENE, ourSubCommandTable_GENE, sizeof(ourSubCommandTable_GENE) / sizeof(OpSpec));	
	PushTable(idSubCommandTable_FILE, ourSubCommandTable_FILE, sizeof(ourSubCommandTable_FILE) / sizeof(OpSpec));	

	PushTable(idSubCommandTable_HIST, ourSubCommandTable_HIST, sizeof(ourSubCommandTable_HIST) / sizeof(OpSpec));	
	PushTable(idSubIntegerRVTable_HIST, ourSubIntegerRVTable_HIST, sizeof(ourSubIntegerRVTable_HIST) / sizeof(OpSpec));	
	PushTable(idSubStringRVTable_HIST, ourSubStringRVTable_HIST, sizeof(ourSubStringRVTable_HIST) / sizeof(OpSpec));

	PushTable(idSubIntegerRVTable_PRT, ourSubIntegerRVTable_PRT, sizeof(ourSubIntegerRVTable_PRT) / sizeof(OpSpec));
	PushTable(idSubStringRVTable_PRT, ourSubStringRVTable_PRT, sizeof(ourSubStringRVTable_PRT) / sizeof(OpSpec));
	PushTable(idSubAgentRVTable_PRT, ourSubAgentRVTable_PRT, sizeof(ourSubAgentRVTable_PRT) / sizeof(OpSpec));

	ASSERT(myTables.size() == DEFAULT_NUMBER_OF_TABLES);

	myCAOSEngineVersion = GetEngineVersion();

	// If this ASSERTs:
	// Make sure you've added a category string for each category type in the enumeration, and vice versa
	ASSERT(categoryMax == sizeof(ourCategoryText) / sizeof(std::string));
	myCategoryText = std::vector<std::string>(ourCategoryText, ourCategoryText + sizeof(ourCategoryText) / sizeof(std::string));

	myScriptNames.resize(256);
	for (int i = 2; i < sizeOurScriptNumbers; ++i)
	{
		TableSpec* spec = ourScriptNumbers + i;
		if (spec->entries.size() > 0)
		{
			int entry = atoi(spec->entries[0].c_str());
			ASSERT(entry >= 0 && entry < myScriptNames.size());
			std::string str = spec->entries[1];
			for (int i = 0; i < str.size(); ++i)
				if (str[i] != '@')
					myScriptNames[entry] += str[i];

		}
	}

#ifdef _DEBUG
	SanityCheck();
#endif // _DEBUG 
}
