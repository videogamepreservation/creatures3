// --------------------------------------------------------------------------------------
// Filename:	PrayChunk.h
// Class:		PrayChunk
// Purpose:		To encapsulate the chunks of data in Pray Files
// Description:
//  The PrayChunks are returned by the PrayManager when asked for. They are managed as
//  a list in the praymanager, and are cached thusly.
//  You should never delete a Chunk. What you should do, is tell the Manager you are
//  Done with the chunk. The manager will then deal with it later on.
//
// History:
//  21Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "PrayChunk.h"
#include "PrayManager.h"

const uint8* PrayChunk::GetData() const
{
	return myPtr;
}

PrayChunk::PrayChunk(uint32 sizeOf, PrayManager* manager)
{
	myManager = manager;
	myCount = 0;
	myPtr = new uint8[sizeOf];
	mySize = sizeOf;
}

const int PrayChunk::GetSize() const
{
	return mySize;
}