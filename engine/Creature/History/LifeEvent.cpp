#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "LifeEvent.h"
#include "../../CreaturesArchive.h"

CreaturesArchive &operator<<( CreaturesArchive &archive, LifeEvent const &lifeEvent )
{
	archive << (int)lifeEvent.myEventType;

	archive << lifeEvent.myWorldTick;
	archive << lifeEvent.myAgeInTicks;
	archive << lifeEvent.myRealWorldTime;
	archive << lifeEvent.myLifeStage;
	
	archive << lifeEvent.myRelatedMoniker1;
	archive << lifeEvent.myRelatedMoniker2;

	archive << lifeEvent.myUserText;
	archive << lifeEvent.myPhoto;

	archive << lifeEvent.myWorldName;
	archive << lifeEvent.myWorldUniqueIdentifier;

	return archive;
}

CreaturesArchive &operator>>( CreaturesArchive &archive, LifeEvent &lifeEvent )
{
	int intEventType;
	archive >> intEventType;
	lifeEvent.myEventType = (LifeEvent::EventType)intEventType;

	archive >> lifeEvent.myWorldTick;
	archive >> lifeEvent.myAgeInTicks;
	archive >> lifeEvent.myRealWorldTime;
	archive >> lifeEvent.myLifeStage;

	archive >> lifeEvent.myRelatedMoniker1;
	archive >> lifeEvent.myRelatedMoniker2;

	archive >> lifeEvent.myUserText;
	archive >> lifeEvent.myPhoto;

	archive >> lifeEvent.myWorldName;
	archive >> lifeEvent.myWorldUniqueIdentifier;

	return archive;
}

bool LifeEvent::operator==(LifeEvent& other)
{
	return
		(myEventType == other.myEventType) &&
		(myWorldTick == other.myWorldTick) &&
		(myAgeInTicks == other.myAgeInTicks) &&
		(myRealWorldTime == other.myRealWorldTime) &&
		(myLifeStage == other.myLifeStage) &&
		(myRelatedMoniker1 == other.myRelatedMoniker1) &&
		(myRelatedMoniker2 == other.myRelatedMoniker2) &&
//		(myUserText == other.myUserText) && // user text can change, so don't compare it
//		(myPhoto == other.myPhoto) &&  // we don't require photos to be the same, as they can be deleted or retaken
		(myWorldName == other.myWorldName) &&
		(myWorldUniqueIdentifier == other.myWorldUniqueIdentifier);
}

