// -------------------------------------------------------------------------
// Filename:    C2eServices.h
// Class:       None (but see also C2eServices.cpp)
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
// 23Mar99  gtb		Added OutputFormattedDebugString function
// 13Aug99  BenC    Separated debugging stuff into C2eDebug
// -------------------------------------------------------------------------


#ifndef C2ESERVICES_H
#define C2ESERVICES_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eDebug.h"
#include "FlightRecorder.h"
#include "../common/Catalogue.h"

// string localisation
extern Catalogue theCatalogue;

// Logging system
extern FlightRecorder theFlightRecorder;

#endif // C2ESERVICES_H

