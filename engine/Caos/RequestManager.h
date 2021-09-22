#ifndef REQUEST_MANAGER_H
#define REQUEST_MANAGER_H

#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"
// TODO: ServerSide should be fwd declaration
#include "../../common/ServerSide.h"

class RequestManager
{
public:
	void HandleIncoming( ServerSide& server );
};

#endif // REQUEST_MANAGER_H
