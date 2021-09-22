#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "DrawableObject.h"
#include "../PersistentObject.h"

DrawableObject::DrawableObject()
		:myScreenPosition(0,0), 
		myCameraShyFlag(false),
		myPlane(0),
		myAmIASpriteFlag(false),
		myAmIALineFlag(false),
		myAmIACameraFlag(false)
{
	;
}


// new serialization stuff
bool DrawableObject::Write( CreaturesArchive &ar ) const
{
	ar << myScreenPosition;
	ar << myCurrentBound;
	ar << myAmIACameraFlag;
	ar << myAmIALineFlag;
	ar << myAmIASpriteFlag;
	return true;
}

bool DrawableObject::Read( CreaturesArchive &ar )
{
	int32 version = ar.GetFileVersion();

	if(version >= 3)
	{

		ar >> myScreenPosition;
		ar >> myCurrentBound;
		ar >> myAmIACameraFlag;
		ar >> myAmIALineFlag;
		ar >> myAmIASpriteFlag;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}