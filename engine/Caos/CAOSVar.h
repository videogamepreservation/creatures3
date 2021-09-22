#ifndef CAOSVAR_H
#define CAOSVAR_H



#ifdef _MSC_VER
// turn off warning about symbols too long for debugger
#pragma warning (disable : 4786 4503)
#endif // _MSC_VER

#include "../../common/BasicException.h"
#include "../C2eServices.h"	// for debugging macros
#include "../CreaturesArchive.h"
#include <string>
#include "../Agents/AgentHandle.h"

class CAOSVar
{
public:
	CAOSVar();
	~CAOSVar();	// note: NOT virtual, because we want to save space

	// Copy constructor
	CAOSVar(const CAOSVar& var);
	
	// Assignment
	CAOSVar& operator=(const CAOSVar& var);

	// variable types
	
	void BecomeAZeroedIntegerOnYourNextRead() { myBecomeZero = true; };

	enum { 
		typeInteger, 
		typeFloat, 
		typeString, 
		typeAgent
	};

	// exception to throw if Get* called on wrong type.
	class TypeErr : public BasicException
	{ public: TypeErr( const char* msg ) : BasicException(msg) {}; };


	float GetFloat();
	int GetInteger();
	AgentHandle GetAgent();
	void GetString( std::string& str );

	void SetInteger( int i );
	void SetFloat( float f );
	void SetString( const std::string& str );
	void SetAgent( AgentHandle& a );

	int GetType() const { return myBecomeZero?typeInteger:myType; }

	// serialization stuff
	virtual bool Write(CreaturesArchive &ar) const;
	virtual bool Read(CreaturesArchive &ar);

	virtual bool Write( std::ostream &stream) const;
	virtual bool Read( std::istream &stream);

protected:
	union PolyVar
	{
		int pv_int;
		float pv_float;
		std::string* pv_string;
	};

	bool myBecomeZero;

	int myType;
	PolyVar myData;
	AgentHandle myHandle;

	virtual void Get( const int type, PolyVar& data ) const;
	virtual void Set( const int type, const PolyVar& data );
};



inline CAOSVar::CAOSVar()
{
	// default to integer
	myType = typeInteger;
	myData.pv_int = 0;
	myBecomeZero = false;
}


inline CAOSVar::~CAOSVar()
{
	if( myType == typeString )
		delete myData.pv_string;
}



inline float CAOSVar::GetFloat()
{
	if (myBecomeZero)
		SetInteger(0);
	PolyVar norwegian_blue;
	int type( typeFloat );

	Get( type, norwegian_blue );
	return norwegian_blue.pv_float;
}

inline int CAOSVar::GetInteger()
{
	if (myBecomeZero)
		SetInteger(0);
	PolyVar norwegian_blue;
	int type( typeInteger );

	Get( type, norwegian_blue );
	return norwegian_blue.pv_int;
}

inline void CAOSVar::GetString( std::string& str )
{
	if (myBecomeZero)
		SetInteger(0);
	PolyVar norwegian_blue;
	int type( typeString );

	Get( type, norwegian_blue );
	str = *(norwegian_blue.pv_string);
}

inline AgentHandle CAOSVar::GetAgent()
{
	if (myBecomeZero)
		SetInteger(0);
	_ASSERT(myType == typeAgent);
	return myHandle;
}


inline void CAOSVar::SetInteger( int i )
{
	PolyVar norwegian_blue;
	int type = typeInteger;
	norwegian_blue.pv_int = i;
	Set(type, norwegian_blue );
	myBecomeZero = false;
}

inline void CAOSVar::SetFloat( float f )
{
	PolyVar norwegian_blue;
	int type = typeFloat;
	norwegian_blue.pv_float = f;
	Set( type, norwegian_blue );
	myBecomeZero = false;
}

inline void CAOSVar::SetString( const std::string& str )
{
	std::string hack( str);
	PolyVar norwegian_blue;
	int type = typeString;
	norwegian_blue.pv_string = &hack;
	Set( type, norwegian_blue );
	myBecomeZero = false;
}

inline void CAOSVar::SetAgent( AgentHandle& a )
{
	
	if( myType == typeString )
		delete myData.pv_string;
	myType = typeAgent;
	myHandle = a;
	myBecomeZero = false;
}


inline CreaturesArchive &operator<<( CreaturesArchive& ar, CAOSVar const &var )
{
	var.Write( ar );
	return ar;
}

inline CreaturesArchive &operator>>( CreaturesArchive& ar, CAOSVar &var )
{
	var.Read( ar );
	return ar;
}

// Global
extern CAOSVar INTEGERZERO;

#endif // CAOSVAR_H