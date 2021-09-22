#include "CAOSVar.h"
#include "../Agents/Agent.h"

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


// Global
CAOSVar INTEGERZERO;

// virtual
void CAOSVar::Get( const int type, PolyVar& data ) const
{
	_ASSERT(myType == type);
	data = myData;
}

// virtual
void CAOSVar::Set( const int type, const PolyVar& data )
{
	switch( type )
	{
	case typeString:
		if( myType == typeAgent )
		{
			myHandle = NULLHANDLE;
		}
		if( myType != typeString )
			myData.pv_string = new std::string( *(data.pv_string) );
		else
			*(myData.pv_string) = *(data.pv_string);
		myType = typeString;
		break;
	case typeAgent:
		if( myType == typeString )
			delete myData.pv_string;
		myHandle = NULLHANDLE;
		myType = typeAgent;
		break;
	case typeInteger:
		if( myType == typeAgent )
		{
			myHandle = NULLHANDLE;
		}
		if( myType == typeString )
			delete myData.pv_string;
		myData.pv_int = data.pv_int;
		myType = typeInteger;
		break;
	case typeFloat:
		if( myType == typeAgent )
		{
			myHandle = NULLHANDLE;
		}
		if( myType == typeString )
			delete myData.pv_string;
		myData.pv_float = data.pv_float;
		myType = typeFloat;
		break;
	}
}


// Copy constructor
CAOSVar::CAOSVar(const CAOSVar& var)
{
	myType = typeInteger;
	if (var.myType != typeAgent)
	{
		Set(var.myType, var.myData);
		myBecomeZero = var.myBecomeZero;
	}
	else
	{
		myType = typeAgent;
		myHandle = var.myHandle;
		myBecomeZero = var.myBecomeZero;
	}

}
	
// Assignment
CAOSVar& CAOSVar::operator=(const CAOSVar& var)
{
	// Check for self-assignment
	if (&var != this)
	{
		if (var.myType != typeAgent)
			Set(var.myType, var.myData);
		else
		{
			myType = typeAgent;
			myHandle = var.myHandle;
		}
		myBecomeZero = var.myBecomeZero;
	}
	return *this;
}



// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool CAOSVar::Write(CreaturesArchive &ar) const
{
	ar << myType;
	switch( myType )
	{
	case typeString:
		ar << *(myData.pv_string);
		break;
	case typeInteger:
		ar << myData.pv_int;
		break;
	case typeFloat:
		ar << myData.pv_float;
		break;
	case typeAgent:
		ar << myHandle;
		break;
	}
	ar << myBecomeZero;
	return true;
}

bool CAOSVar::Read(CreaturesArchive &ar)
{
	int32 version = ar.GetFileVersion();
	if(version >= 3)
	{

		int type;
		ar >> type;

		PolyVar poly;
		std::string hack;

		switch( type )
		{
		case typeString:
			ar >> hack;
			poly.pv_string = &hack;
			Set( type, poly );
			break;
		case typeInteger:
			ar >> poly.pv_int;
			Set( type, poly );
			break;
		case typeFloat:
			ar >> poly.pv_float;
			Set( type, poly );
			break;
		case typeAgent:
			ar >> myHandle;
			myType = typeAgent;
			break;
		}
		ar >> myBecomeZero;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

bool CAOSVar::Write(std::ostream &stream) const
{
	stream << myType;
	switch( myType )
	{
	case typeString:
		stream << *(myData.pv_string);
		break;
	case typeInteger:
		stream << myData.pv_int;
		break;
	case typeFloat:
		stream << myData.pv_float;
		break;
	case typeAgent:
		// *** GUY ***
		break;
	}
	return true;
}

bool CAOSVar::Read(std::istream &stream)
{
	int type;
	stream >> type;

	PolyVar poly;
	std::string hack;

	switch( type )
	{
	case typeString:
		stream >> hack;
		poly.pv_string = &hack;
		Set( type, poly );
		break;
	case typeInteger:
		stream >> poly.pv_int;
		Set( type, poly );
		break;
	case typeFloat:
		stream >> poly.pv_float;
		Set( type, poly );
		break;
	case typeAgent:
		// *** GUY ***
		break;
	}
	return true;
}
