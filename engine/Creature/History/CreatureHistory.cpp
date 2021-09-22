#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "CreatureHistory.h"
#include "../../World.h"
#include "../../App.h"
#include "../Creature.h"
#include "../LifeFaculty.h"
//#include "../../Display/Window.h"

CreatureHistory::CreatureHistory()
	: myGender(-1), myGenus(-1), myVariant(-1),
	  myCrossoverMutationCount(-1), myCrossoverCrossCount(-1)
{
}

void CreatureHistory::AddEvent(const LifeEvent::EventType& eventType,
			 const std::string& relatedMoniker1,
			 const std::string& relatedMoniker2,
			 bool executeScript /* = true */)
{
	// don't store life events for the null moniker
	if (myMoniker.empty())
		return;

	AgentHandle creature = theAgentManager.FindCreatureAgentForMoniker(myMoniker);
//	if (eventType == LifeEvent::typeExported)
//		ASSERT(!creature.IsValid());
	if (eventType == LifeEvent::typeBorn) //Doesn't check for typeImported anymore in case it is a related (but dead) creature.
		ASSERT(creature.IsValid());

	LifeEvent event;
	event.myEventType = eventType;

	event.myWorldTick = theApp.GetWorld().GetWorldTick();
	if (creature.IsCreature())
	{
		event.myAgeInTicks = creature.GetCreatureReference().Life()->GetTickAge();
		event.myLifeStage = creature.GetCreatureReference().Life()->GetAge();
	}
	else
	{
		event.myAgeInTicks = (uint32)-1;
		event.myLifeStage = -1;
	}
//	event.myRealWorldTime = GetRealWorldTime();
	event.myRealWorldTime = (uint32)time(NULL);

	event.myRelatedMoniker1 = relatedMoniker1;
	event.myRelatedMoniker2 = relatedMoniker2;
	
	event.myWorldName = theApp.GetWorld().GetWorldName();
	event.myWorldUniqueIdentifier = theApp.GetWorld().GetUniqueIdentifier();

	// add new event to container
	myLifeEvents.push_back(event);
	int newEventNo = myLifeEvents.size() - 1;
#ifdef _DEBUG
	int spareCapacity = myLifeEvents.capacity() - myLifeEvents.size();
#endif

	// send event
	if( executeScript )
	{
		CAOSVar p1, p2;
		p1.SetString(myMoniker);
		p2.SetInteger(newEventNo);
		theAgentManager.ExecuteScriptOnAllAgentsDeferred(SCRIPT_LIFE_EVENT, NULLHANDLE, p1, p2);
	}
}

CreaturesArchive &operator<<( CreaturesArchive &archive, CreatureHistory const &creatureHistory )
{
	archive << creatureHistory.myMoniker;
	archive << creatureHistory.myName;
	archive << creatureHistory.myGender;
	archive << creatureHistory.myGenus;
	archive << creatureHistory.myVariant;
	archive << creatureHistory.myLifeEvents;
	archive << creatureHistory.myCrossoverMutationCount;
	archive << creatureHistory.myCrossoverCrossCount;

	return archive;
}

CreaturesArchive &operator>>( CreaturesArchive &archive, CreatureHistory &creatureHistory )
{
	archive >> creatureHistory.myMoniker;
	archive >> creatureHistory.myName;
	archive >> creatureHistory.myGender;
	archive >> creatureHistory.myGenus;
	archive >> creatureHistory.myVariant;
	archive >> creatureHistory.myLifeEvents;
	archive >> creatureHistory.myCrossoverMutationCount;
	archive >> creatureHistory.myCrossoverCrossCount;

	return archive;
}

LifeEvent* CreatureHistory::GetLifeEvent(int i)
{
	if (i < 0 || i >= myLifeEvents.size())
		return NULL;

	return &myLifeEvents[i];
}

int CreatureHistory::FindLifeEvent(const LifeEvent::EventType& eventType, int fromIndex)
{
	if (fromIndex < -1)
		fromIndex = -1;
	for (int i = fromIndex + 1; i < myLifeEvents.size(); ++i)
	{
		if (myLifeEvents[i].myEventType == eventType)
			return i;
	}
	return -1;
}

int CreatureHistory::FindLifeEventReverse(const LifeEvent::EventType& eventType, int fromIndex)
{
	if (fromIndex == -1 || fromIndex > myLifeEvents.size())
		fromIndex = myLifeEvents.size();
	for (int i = fromIndex - 1; i >= 0; --i)
	{
		if (myLifeEvents[i].myEventType == eventType)
			return i;
	}
	return -1;
}
