#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Message.h"
#include "Caos/AutoDocumentationTable.h"

TableSpec ourMessageNumbers[] =
{
	TableSpec("Message Numbers"),
	TableSpec("Number", "Name", "Description"),

	TableSpec("0", "Activate 1", "Calls the @#Activate 1@ script.  If the message is from a creature, and the permissions set with @#BHVR@ disallow it, then the script is not executed."),
	TableSpec("1", "Activate 2", "Calls the @#Activate 2@ script.  The permissions set with @#BHVR@ are checked first."),
	TableSpec("2", "Deactivate", "Calls the @#Deactivate@ script.  The permissions set with @#BHVR@ are checked first."),
	TableSpec("3", "Hit", "Calls the @#Hit@ script.  If the message is from a creature, and the permissions set with @#BHVR@ disallow it, then the message is not sent."),
	TableSpec("4", "Pickup", "The agent is picked up by the agent that the message was @#FROM@.  The permissions set with @#BHVR@ are checked first."),
	TableSpec("5", "Drop", "If the agent is being carried, then it is dropped."),
	TableSpec("12", "Eat", "Calls the @#Eat@ script.  The permissions set with @#BHVR@ are checked first."),
	TableSpec("13", "Hold Hands", "Causes a creature to hold hands with the pointer."),
	TableSpec("14", "Stop Holding Hands", "Causes a creature to stop holding hands with the pointer.  Since messages take a tick to be procesed, calling @#NOHH@ is quicker than using this message."),
};

int dummyMessageNumbers = AutoDocumentationTable::RegisterTable(ourMessageNumbers, sizeof(ourMessageNumbers));

