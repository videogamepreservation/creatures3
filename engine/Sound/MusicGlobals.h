// --------------------------------------------------------------------------
// Filename:	Music Globals.h
//
// Purpose:
//
// Declares any global variables shared by the music manager
//
// History:
//
// 21Apr98	PeterC	Created
// 28Jul98	PeterC	Changed pSound to musicSoundManager
// --------------------------------------------------------------------------

#ifndef _MUSIC_GLOBALS_H

#define _MUSIC_GLOBALS_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MusicTypes.h"

// The Music Engine should have its own separate sound manager, to avoid
// thread clashes
extern class SoundManager *theMusicSoundManager;

// Timer calls at intervals of 100ms
const MusicValue musicTimerResolution = (MusicValue) 0.05;

// Timer resolution in milliseconds
const int musicTimerResolutionMs = 50;

#endif
