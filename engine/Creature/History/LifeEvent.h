#ifndef LIFE_EVENT_H
#define LIFE_EVENT_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include <string>
#include "../../../common/C2eTypes.h"

class LifeEvent
{
public:
	// please add new values to the end of 
	// this enum, as the values are used in 
	// CAOS scripts
	enum EventType
	{
		typeConcieved, // naturally
		typeSpliced, // artificially
		typeEngineered, // engineered

		typeBorn,
		typeNewLifeStage,
		typeExported,
		typeImported,
		typeDied,

		typeBecamePregnant,
		typeImpregnated,
		typeChildBorn,

		typeLaid, // fired from CAOS, not engine
		typeLaidEgg,// fired from CAOS, not engine
		typePhotographed,// fired from CAOS, not engine

		typeCloned, // another conception
		typeClonedSource,
	};

	EventType myEventType;
	
	uint32 myWorldTick;
	uint32 myAgeInTicks;
	uint32 myRealWorldTime; // seconds since midnight (00:00:00), January 1, 1970 UTC
	int myLifeStage;
	
	std::string myRelatedMoniker1;
	std::string myRelatedMoniker2;

	std::string myUserText;
	std::string myPhoto;

	std::string myWorldName;
	std::string myWorldUniqueIdentifier;

	friend CreaturesArchive &operator<<( CreaturesArchive &archive, LifeEvent const &lifeEvent );
	friend CreaturesArchive &operator>>( CreaturesArchive &archive, LifeEvent &lifeEvent );

	bool operator==(LifeEvent& other);
};


#endif // LIFE_EVENT_H
