#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "VelocityVariable.h"
#include "../Map/Map.h"



void VelocityVariable::Get( const int type, PolyVar& data ) const
{
	if( type == typeFloat )
		data.pv_float = myFloatReference;
	else if( type == typeInteger )
		data.pv_int = Map::FastFloatToInteger(myFloatReference);
	else
		throw TypeErr("VelocityVariable::Get");
}


void VelocityVariable::Set( const int type, const PolyVar& data )
{
	if( type == typeFloat )
	{
		myFloatReference = data.pv_float;
		myStoppedReference = false;
	}
	else if( type == typeInteger )
	{
		myFloatReference = (float)data.pv_int;
		myStoppedReference = false;
	}
	else
		throw TypeErr("VelocityVariable::Set");
}

