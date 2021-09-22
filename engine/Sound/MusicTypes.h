// --------------------------------------------------------------------------
// Filename:	Music Types.h
//
// Purpose:
//
// Declares basic types used by music manager
//
//
// History:
//
// 2Apr98	PeterC	Created
// --------------------------------------------------------------------------

#ifndef _MUSIC_TYPES_H

#define _MUSIC_TYPES_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

typedef float MusicValue;

// Waves and effects are kept in an array, and referred to by ID for efficiency
typedef int MusicWaveID;

#define MUSIC_NO_WAVE -1

typedef int MusicEffectID;

#define MUSIC_NO_EFFECT -1

// **** Temporary
typedef int SoundHandle;

#define NO_SOUND_HANDLE -1

#endif
