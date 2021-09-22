#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "../Message.h"

#include <deque>
#include <set>

class CreaturesArchive;


struct CompareMessageTimes
{
	bool operator()( Message const & msg1, Message const & msg2 )
	{
		return msg1.GetTime() < msg2.GetTime();
	}
};

class MessageQueue
{
public:
	void WriteMessage(	AgentHandle const& from,
						AgentHandle const& to,
						int msg,
						CAOSVar const &p1, CAOSVar const & p2,
						unsigned delay);

	void RemoveMessagesAbout( AgentHandle& o );

	bool ReadMessage( Message &message );

	friend CreaturesArchive &operator<<( CreaturesArchive &ar, MessageQueue const &message );
	friend CreaturesArchive &operator>>( CreaturesArchive &ar, MessageQueue &message );

private:
	std::deque< Message > myImmediateQueue;
	std::multiset< Message, CompareMessageTimes > myDelayedQueue;
};
CreaturesArchive &operator<<( CreaturesArchive &ar, MessageQueue const &message );
CreaturesArchive &operator>>( CreaturesArchive &ar, MessageQueue &message );

#endif // MESSAGEQUEUE_H
