#ifndef MESSAGE_H
#define MESSAGE_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"

class CreaturesArchive;

#include <limits.h>

enum {
		ACTIVATE1 = 0,
		ACTIVATE2 = 1,
		DEACTIVATE = 2,
		HIT = 3,
		PICKUP = 4,
		DROP = 5,

        // C2.
        EAT = 12,

	
		START_HOLD_HANDS = 13,
		STOP_HOLD_HANDS = 14,

		// Please remember! (tie a knot in your pyjamas),
		// update the documentation table in Message.cpp if
		// you add a new message, or change the meaning
		// of any of these

        INVALID = INT_MAX
};

#include "Agents/AgentHandle.h"
#include "Caos/CAOSVar.h"

// Message data structure

class Message
{
public:
	Message() : myTime( 0 ) {}

	Message( AgentHandle const& from, AgentHandle const& to, int msg,
		CAOSVar const &p1, CAOSVar const & p2, unsigned time )
		:
		myFrom( from ), myTo( to ),
		myMsg( msg ),
		myP1( p1 ), myP2( p2 ),
		myTime( time )
		{}

	AgentHandle GetFrom() const {return myFrom;}
	AgentHandle GetTo() const {return myTo;}
	uint32 GetTime() const {return myTime;}
	CAOSVar GetP1() const {return myP1;}
	CAOSVar GetP2() const {return myP2;}
	int GetMsg() const {return myMsg;}
 
	friend CreaturesArchive &operator<<( CreaturesArchive &ar, Message const &message );
	friend CreaturesArchive &operator>>( CreaturesArchive &ar, Message &message );

private:
	AgentHandle	myFrom;					// who it's from
	AgentHandle	myTo;					// who it's for

	int myMsg;							// message number
	CAOSVar myP1;						// paramters
	CAOSVar myP2;

	uint32 myTime;					// Time message should be activated
};

CreaturesArchive &operator<<( CreaturesArchive &ar, Message const &message );
CreaturesArchive &operator>>( CreaturesArchive &ar, Message &message );


#endif // MESSAGE_H
