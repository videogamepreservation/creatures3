#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "HistoryHandlers.h"
#include "../World.h"
#include "../App.h"
#include "../Creature/Creature.h"
#include "../Creature/LifeFaculty.h"
#include "../Creature/LinguisticFaculty.h"

#include "../Display/SharedGallery.h"

void HistoryHandlers::Command_HIST( CAOSMachine& vm )
{
	static CommandHandler subcmds [] = 
	{
		SubCommand_HIST_EVNT,
		SubCommand_HIST_UTXT,
		SubCommand_HIST_NAME,
		SubCommand_HIST_WIPE,
		SubCommand_HIST_FOTO,
	};
	int thiscmd;
	thiscmd = vm.FetchOp();
	(subcmds[thiscmd])( vm );
}

void HistoryHandlers::SubCommand_HIST_EVNT( CAOSMachine& vm )
{
	CreatureHistory& history = CreatureHistoryHelper(vm);
	int eventType = vm.FetchIntegerRV();
	
	std::string relatedMoniker1, relatedMoniker2;
	vm.FetchStringRV(relatedMoniker1);
	vm.FetchStringRV(relatedMoniker2);

	history.AddEvent((LifeEvent::EventType)eventType, relatedMoniker1, relatedMoniker2);
}

void HistoryHandlers::SubCommand_HIST_UTXT( CAOSMachine& vm )
{
	LifeEvent& event = LifeEventHelper(vm);

	std::string newValue;
	vm.FetchStringRV(newValue);

	event.myUserText = newValue;
}

void HistoryHandlers::SubCommand_HIST_NAME( CAOSMachine& vm )
{
	CreatureHistory& history = CreatureHistoryHelper(vm);

	std::string newValue;

	vm.FetchStringRV(newValue);

	if(history.myName != newValue)
	{
		history.myName = newValue;

		// Tell creature agent to learn its name
		AgentHandle creature = theAgentManager.FindCreatureAgentForMoniker(history.myMoniker);

		if (creature.IsValid())
		{
			creature.GetCreatureReference().Linguistic()->ClearWordStrength(LinguisticFaculty::PERSONAL, LinguisticFaculty::ME);
			creature.GetCreatureReference().Linguistic()->SetWord(LinguisticFaculty::PERSONAL, LinguisticFaculty::ME, history.myName, false);
		}

	}
}

void HistoryHandlers::SubCommand_HIST_WIPE( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV(moniker);

	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	int state = historyStore.GetOutOfWorldState(moniker);

	// only wipe history if there isn't any anyway,
	// if the creature is fully dead, is exported,
	// or is a genome that is no longer referenced
	// and was never made into a creature
	if (state == 0 || state == 6 || state == 4 || state == 7)
	{
		CreatureHistory& history = historyStore.GetCreatureHistory(moniker);
		for (int i = 0; i < history.Count(); ++i)
		{
			// attic any photos
			LifeEvent* event = history.GetLifeEvent(i);
			ASSERT(event);
			if (!event->myPhoto.empty())
			{
				FilePath filePath(event->myPhoto + ".s16", IMAGES_DIR);
				bool inUse = SharedGallery::QueryGalleryInUse(filePath);
				
				if (inUse)
					vm.ThrowRunError(CAOSMachine::sidCreatureHistoryPhotoStillInUse);
				
				theApp.GetWorld().MarkFileForAttic(filePath);

				event->myPhoto = "";
			}
		}

		// wipe history
		historyStore.WipeCreatureHistory(moniker);
	}
	else
		vm.ThrowRunError( CAOSMachine::sidOnlyWipeTrulyDeadHistory );
}

void HistoryHandlers::SubCommand_HIST_FOTO( CAOSMachine& vm )
{
	LifeEvent& event = LifeEventHelper(vm);

	std::string newValue;
	vm.FetchStringRV(newValue);

	if (!event.myPhoto.empty())
	{
		FilePath filePath(event.myPhoto + ".s16", IMAGES_DIR);
		bool inUse = SharedGallery::QueryGalleryInUse(filePath);
		
		if (inUse)
			vm.ThrowRunError(CAOSMachine::sidCreatureHistoryPhotoStillInUse);
		
		theApp.GetWorld().MarkFileForAttic(filePath);
	}

	event.myPhoto = newValue;
}

int HistoryHandlers::IntegerRV_HIST( CAOSMachine& vm )
{
	static IntegerRVHandler subhandlers [] =
	{
		IntegerRV_HIST_COUN,
		IntegerRV_HIST_TYPE,
		IntegerRV_HIST_WTIK,
		IntegerRV_HIST_TAGE,
		IntegerRV_HIST_RTIM,
		IntegerRV_HIST_CAGE,
		IntegerRV_HIST_GEND,
		IntegerRV_HIST_GNUS,
		IntegerRV_HIST_VARI,
		IntegerRV_HIST_FIND,
		IntegerRV_HIST_FINR,
		IntegerRV_HIST_SEAN,
		IntegerRV_HIST_TIME,
		IntegerRV_HIST_YEAR,
		IntegerRV_HIST_DATE,
		IntegerRV_HIST_MUTE,
		IntegerRV_HIST_CROS,
	};
	int thisrv;
	thisrv = vm.FetchOp();
	return (subhandlers[thisrv])( vm );
}

int HistoryHandlers::IntegerRV_HIST_COUN( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV(moniker);

	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();

	if (!historyStore.IsThereCreatureHistory(moniker))
		return 0;

	return historyStore.GetCreatureHistory(moniker).Count();
}

LifeEvent& HistoryHandlers::LifeEventHelper( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV(moniker);
	int eventNo = vm.FetchIntegerRV();
	LifeEvent* lifeEvent = theApp.GetWorld().GetHistoryStore().GetCreatureHistory(moniker).GetLifeEvent(eventNo);
	if (!lifeEvent)
		vm.ThrowRunError( CAOSMachine::sidLifeEventIndexOutofRange );			

	return *lifeEvent;
}

CreatureHistory& HistoryHandlers::CreatureHistoryHelper( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV(moniker);
	CreatureHistory& history = theApp.GetWorld().GetHistoryStore().GetCreatureHistory(moniker);
	return history;

}

int HistoryHandlers::IntegerRV_HIST_TYPE( CAOSMachine& vm )
{
	return (int)(LifeEventHelper(vm).myEventType);
}

int HistoryHandlers::IntegerRV_HIST_WTIK( CAOSMachine& vm )
{
	return LifeEventHelper(vm).myWorldTick;
}

int HistoryHandlers::IntegerRV_HIST_TAGE( CAOSMachine& vm )
{
	return LifeEventHelper(vm).myAgeInTicks;
}

int HistoryHandlers::IntegerRV_HIST_RTIM( CAOSMachine& vm )
{
	return LifeEventHelper(vm).myRealWorldTime;
}

int HistoryHandlers::IntegerRV_HIST_CAGE( CAOSMachine& vm )
{
	return LifeEventHelper(vm).myLifeStage;
}

int HistoryHandlers::IntegerRV_HIST_GEND( CAOSMachine& vm )
{
	return CreatureHistoryHelper(vm).myGender;
}

int HistoryHandlers::IntegerRV_HIST_GNUS( CAOSMachine& vm )
{
	// number 1 (norn) to 4 (geat), so we need to add one
	// from the count in the genetics file which uses 0 to 3
	return CreatureHistoryHelper(vm).myGenus + 1;
}

int HistoryHandlers::IntegerRV_HIST_VARI( CAOSMachine& vm )
{
	return CreatureHistoryHelper(vm).myVariant;
}

int HistoryHandlers::IntegerRV_HIST_FIND( CAOSMachine& vm )
{
	CreatureHistory& history = CreatureHistoryHelper(vm);

	int type = vm.FetchIntegerRV();
	int from = vm.FetchIntegerRV();

	int foundEvent = history.FindLifeEvent((LifeEvent::EventType)type, from);

	return foundEvent;
}

int HistoryHandlers::IntegerRV_HIST_FINR( CAOSMachine& vm )
{
	CreatureHistory& history = CreatureHistoryHelper(vm);

	int type = vm.FetchIntegerRV();
	int from = vm.FetchIntegerRV();

	int foundEvent = history.FindLifeEventReverse((LifeEvent::EventType)type, from);

	return foundEvent;
}

int HistoryHandlers::IntegerRV_HIST_SEAN(CAOSMachine& vm)
{
	int worldTick = vm.FetchIntegerRV();
	return theApp.GetWorld().GetSeason(false, worldTick);
}

int HistoryHandlers::IntegerRV_HIST_TIME(CAOSMachine& vm)
{
	int worldTick = vm.FetchIntegerRV();
	return theApp.GetWorld().GetTimeOfDay(false, worldTick);
}

int HistoryHandlers::IntegerRV_HIST_YEAR(CAOSMachine& vm)
{
	int worldTick = vm.FetchIntegerRV();
	return theApp.GetWorld().GetYearsElapsed(false, worldTick);
}

int HistoryHandlers::IntegerRV_HIST_DATE(CAOSMachine& vm)
{
	int worldTick = vm.FetchIntegerRV();
	return theApp.GetWorld().GetDayInSeason(false, worldTick);
}

int HistoryHandlers::IntegerRV_HIST_MUTE( CAOSMachine& vm )
{
	return CreatureHistoryHelper(vm).myCrossoverMutationCount;
}

int HistoryHandlers::IntegerRV_HIST_CROS( CAOSMachine& vm )
{
	return CreatureHistoryHelper(vm).myCrossoverCrossCount;
}

void HistoryHandlers::StringRV_HIST( CAOSMachine& vm, std::string& str )
{
	static StringRVHandler substrings [] =
	{
		StringRV_HIST_MON1,
		StringRV_HIST_MON2,
		StringRV_HIST_UTXT,
		StringRV_HIST_WNAM,
		StringRV_HIST_WUID,
		StringRV_HIST_NAME,
		StringRV_HIST_NEXT,
		StringRV_HIST_PREV,
		StringRV_HIST_FOTO,
	};
	int thisst;
	thisst = vm.FetchOp();
	(substrings[thisst])( vm, str );
}


void HistoryHandlers::StringRV_HIST_MON1( CAOSMachine& vm, std::string& str )
{
	LifeEvent& lifeEvent = LifeEventHelper(vm);
	str = lifeEvent.myRelatedMoniker1;
}

void HistoryHandlers::StringRV_HIST_MON2( CAOSMachine& vm, std::string& str )
{
	str = LifeEventHelper(vm).myRelatedMoniker2;
}

void HistoryHandlers::StringRV_HIST_UTXT( CAOSMachine& vm, std::string& str )
{
	str = LifeEventHelper(vm).myUserText;
}

void HistoryHandlers::StringRV_HIST_WNAM( CAOSMachine& vm, std::string& str )
{
	str = LifeEventHelper(vm).myWorldName;
}

void HistoryHandlers::StringRV_HIST_WUID( CAOSMachine& vm, std::string& str )
{
	str = LifeEventHelper(vm).myWorldUniqueIdentifier;
}

void HistoryHandlers::StringRV_HIST_NAME( CAOSMachine& vm, std::string& str )
{
	str = CreatureHistoryHelper(vm).myName;
}

void HistoryHandlers::StringRV_HIST_NEXT( CAOSMachine& vm, std::string& str )
{
	std::string otherMoniker;
	vm.FetchStringRV(otherMoniker);

	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	str = historyStore.Next(otherMoniker);
}

void HistoryHandlers::StringRV_HIST_PREV( CAOSMachine& vm, std::string& str )
{
	std::string otherMoniker;
	vm.FetchStringRV(otherMoniker);

	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	str = historyStore.Previous(otherMoniker);
}

void HistoryHandlers::StringRV_HIST_FOTO( CAOSMachine& vm, std::string& str )
{
	str = LifeEventHelper(vm).myPhoto;
}

int HistoryHandlers::IntegerRV_OOWW( CAOSMachine& vm )
{
	std::string moniker;
	vm.FetchStringRV(moniker);

	HistoryStore& historyStore = theApp.GetWorld().GetHistoryStore();
	return historyStore.GetOutOfWorldState(moniker);
}

