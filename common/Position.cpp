#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../engine/Display/Position.h"
#include "../engine/CreaturesArchive.h"



CreaturesArchive &operator<<( CreaturesArchive &archive, const Position &position )
{
	archive << position.myX << position.myY;
	return archive;
}

CreaturesArchive &operator>>( CreaturesArchive &archive, Position &position )
{
	archive >> position.myX >> position.myY;
	return archive;
}
