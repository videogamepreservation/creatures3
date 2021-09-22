// -------------------------------------------------------------------------
// Filename:    C2eServices.cpp
// Class:       None (but see also C2eServices.h)
// Purpose:     Provide basic services without creating massive dependencies
// Description:
//
// This file provides basic services for c2e such as logging and string
// localisation objects. The reason they are defined here rather than, say,
// being attached to the App class is to reduce dependencies. Systems can
// use this class without having to interface with any other systems. This
// should help to avoid the nasty C1/C2 style rats nest of #includes.
//
// Currently provides:
//
// theCatalogue, localised stringtable object
// theFlightRecorder, logging object
//
// Usage:
//
//
// History:
// 22Feb99	BenC	Initial version
// 13Aug99  BenC    Separated debugging stuff into C2eDebug
// -------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "C2eServices.h"

// globals
Catalogue theCatalogue;
FlightRecorder theFlightRecorder;

