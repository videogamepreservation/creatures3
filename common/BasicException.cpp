// -------------------------------------------------------------------------
// Filename:    BasicException.cpp
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
// - Maybe this class should use a proper string object
//
// - Maybe should provide some message formatting and localization
// support...  [ ErrorMessageHandler does this now ]
//
// Usage:
//
//
// History:
// 08Feb99 Initial version BenC
// -------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BasicException.h"
