// -------------------------------------------------------------------------
// Filename:    BasicException.h
// Class:       BasicException
// Purpose:
// Description:
//
// Very basic (!) exception class.
// Really just a base class to form the basis of an exception system.
// See CAOSMachine class for a reasonable example.
//
// NOTES:
// - I haven't made this class derive from std::exception because I wanted
// to keep the c2e exception handling separate from the std C++ lib errors,
// which, I think, are really more fundamental errors that I shouldn't
// have to deal with. Let the very top-level framework deal with that crap.
//
// - Maybe should provide some message formatting and localization
// support...
//
// Usage:
//
//
// History:
// 08Feb99 Initial version BenC
// -------------------------------------------------------------------------

#ifndef BASICEXCEPTION_H
#define BASICEXCEPTION_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>

class BasicException
{
public:
	BasicException( const char* msg )
		{ myMessage = msg; }
	BasicException()
		{ myMessage = ""; }

	virtual const char* what() const
		{ return myMessage.c_str(); }

protected:
	std::string myMessage;
};

#endif // BASICEXCEPTION_H




