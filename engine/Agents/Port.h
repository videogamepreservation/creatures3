// -------------------------------------------------------------------------
// Filename:    Port.h
// Class:       Port
// Purpose:     Base class for agent communication ports
// Description:
//
// Authors:	BenC, Robert
// -------------------------------------------------------------------------

#ifndef PORT_H
#define PORT_H

// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#include "../../common/C2eTypes.h"
#include "../PersistentObject.h"
#include <string>

class Agent;


class Port : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( Port )
public:
	Port::Port( AgentHandle& owner, const char* name, const char* desc, const Vector2D& relativePosition)
		:myOwner(owner),
		 myName(name),
		 myDescription(desc),
		 myRelativePosition(relativePosition)
	{}

	Port() {};
	virtual ~Port() {}

	void Trash()
	{
		myOwner = NULLHANDLE;
	}
	const char* GetName()
	{
		return myName.c_str();
	}
	void SetName( const char* name )
	{
		myName = name;
	}
	const char* GetDescription()
	{
		return myDescription.c_str();
	}
	void SetDescription( const char* desc )
	{
		myDescription = desc;
	}
	AgentHandle& GetOwner()
	{
		return myOwner;
	}
	Vector2D GetRelativePosition() const
	{ 
		return myRelativePosition; 
	}	
	Vector2D GetWorldPosition() const;

	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);
private:
	std::string	myName;
	std::string	myDescription;
	AgentHandle	myOwner;
	Vector2D myRelativePosition;
};

#endif // PORT_H
