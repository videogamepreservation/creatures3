// -------------------------------------------------------------------------
// Filename:    Port.cpp
// Class:       Port
// Purpose:     Base class for agent communication ports
// Description:
//
// Authors:	BenC, Robert
// -------------------------------------------------------------------------


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Port.h"
#include "Agent.h"

CREATURES_IMPLEMENT_SERIAL( Port )


Vector2D Port::GetWorldPosition() const
{
	return myOwner.GetAgentReference().GetPosition()+myRelativePosition;
}


//virtual
bool Port::Write(CreaturesArchive &ar) const
{
	ar << myName << myDescription;
	ar << myOwner;
	ar << myRelativePosition;
	return true;
}


//virtual
bool Port::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();
	if(version >= 3)
	{
		ar >> myName >> myDescription;
		ar >> myOwner;
		ar >> myRelativePosition;
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}
