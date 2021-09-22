#ifndef CREATURE_HISTORY_H
#define CREATURE_HISTORY_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "LifeEvent.h"
#include "../../Agents/AgentHandle.h"

#include <vector>

class CreatureHistory
{
public:
	CreatureHistory();

	void AddEvent(const LifeEvent::EventType& eventType,
			 const std::string& relatedMoniker1,
			 const std::string& relatedMoniker2,
			 bool executeScript = true);

	int Count() const { return myLifeEvents.size(); };

	LifeEvent* GetLifeEvent(int i);

	int FindLifeEvent(const LifeEvent::EventType& eventType, int fromIndex);
	int FindLifeEventReverse(const LifeEvent::EventType& eventType, int fromIndex);

	friend CreaturesArchive &operator<<( CreaturesArchive &archive, CreatureHistory const &creatureHistory );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, CreatureHistory &creatureHistory );

	std::string myMoniker;
	std::string myName;
	int myGender;
	int myGenus;
	int myVariant;
	int myCrossoverMutationCount;
	int myCrossoverCrossCount;

	std::vector<LifeEvent> myLifeEvents;
};


#endif // CREATURE_HISTORY_H
