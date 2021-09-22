#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MessageQueue.h"
#include "../CreaturesArchive.h"
#include "../App.h"
#include "../World.h"
#include <algorithm>

void MessageQueue::WriteMessage(	AgentHandle const & from,
					AgentHandle const& to,
					int msg,
					CAOSVar const &p1, CAOSVar const &p2,
					unsigned delay )
{
	if( delay )
		myDelayedQueue.insert( Message( from, to, msg, p1, p2, theApp.GetWorld().GetWorldTick() + delay ) );
	else
		myImmediateQueue.push_back( Message( from, to, msg, p1, p2, delay ) );
}

//Utilities for removing messages refering to particular AgentHandles
struct RefersToAgent
{
	RefersToAgent( AgentHandle& agentHandle ) : myAgentHandle( agentHandle ) {}

	bool operator()( Message const &message )
	{
		return message.GetFrom() == myAgentHandle || message.GetTo() == myAgentHandle;
	}

	AgentHandle myAgentHandle;
};

void MessageQueue::RemoveMessagesAbout( AgentHandle& o )
{
	RefersToAgent functor(o);
	std::deque<Message> de;
	std::multiset<Message,CompareMessageTimes> ms;
	for(std::deque<Message>::iterator it = myImmediateQueue.begin();
		it != myImmediateQueue.end();
		it++)
		if (functor(*it) == false)
			de.push_back(*it);
	de.swap(myImmediateQueue);
	for(std::multiset<Message,CompareMessageTimes>::iterator it =
		myDelayedQueue.begin();
		it != myDelayedQueue.end();
		it++)
		if (functor(*it) == false)
			ms.insert(*it);
	ms.swap(myDelayedQueue);
}

bool MessageQueue::ReadMessage( Message &message )
{
	if( myImmediateQueue.size() )
	{
		message = myImmediateQueue.front();
		myImmediateQueue.pop_front();
		return true;
	}
	else if( myDelayedQueue.size() && 
		myDelayedQueue.begin()->GetTime() <= theApp.GetWorld().GetWorldTick() )
	{
		message = *myDelayedQueue.begin();
		myDelayedQueue.erase( myDelayedQueue.begin() );
		return true;
	}
	return false;
}

CreaturesArchive &operator<<( CreaturesArchive &ar, Message const &message )
{
	ar << message.myFrom << message.myTo << message.myMsg << message.myP1 << message.myP2 << message.myTime;
	return ar;
}

CreaturesArchive &operator>>( CreaturesArchive &ar, Message &message )
{
	ar >> message.myFrom >> message.myTo >> message.myMsg >> message.myP1 >> message.myP2 >> message.myTime;
	return ar;
}

CreaturesArchive &operator<<( CreaturesArchive &ar, MessageQueue const &messageQueue )
{
	ar << messageQueue.myDelayedQueue << messageQueue.myImmediateQueue;
	return ar;
}

CreaturesArchive &operator>>( CreaturesArchive &ar, MessageQueue &messageQueue )
{
	ar >> messageQueue.myDelayedQueue >> messageQueue.myImmediateQueue;
	return ar;
}
