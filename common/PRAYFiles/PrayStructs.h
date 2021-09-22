// --------------------------------------------------------------------------------------
// Filename:	PrayStructs.h
// Contents:	PrayFileHeader, PrayChunkHeader
// Purpose:		To encapsulate the Pray Files structure
// Description:
//  PrayFileHeader is what is at the start of a prayfile.
//
//  PrayChunkHeader is what heads up each chunk.
//
// History:
//  21Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------

#ifndef PRAYSTRUCTS_H
#define PRAYSTRUCTS_H

struct PrayFileHeader
{
	char majik[4];
};

struct PrayChunkHeader
{
	char type[4];
	char name[128];
	int  size;
	int  usize;
	int  flags;
};

#endif //PRAYSTRUCTS_H