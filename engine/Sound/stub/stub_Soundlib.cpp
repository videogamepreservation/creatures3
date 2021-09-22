#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../../common/C2eTypes.h"
#include "../MidiModule.h"
#include "stub_Soundlib.h"
//#include "../General.h"
//#include "../App.h"
//#include "../C2eServices.h"
//#include "../File.h"
//#include "../Display/ErrorMessageHandler.h"


#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

CREATURES_IMPLEMENT_SERIAL( SoundManager)

void SoundManager::SetMNGFile(std::string& mng)
{}

SoundManager::SoundManager()
{}

SoundManager::~SoundManager()
{}


void SoundManager::StopAllSounds()
{}


									
void SoundManager::Update()
	{ }


BOOL SoundManager::SoundEnabled()					//  Is mixer running?
	{ return FALSE; }

SOUNDERROR SoundManager::InitializeCache(int size)
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::FlushCache()
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::SuspendMixer()
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::RestoreMixer()
	{ return(NO_SOUND_ERROR); }



SOUNDERROR SoundManager::PreLoadSound(DWORD wave)
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::PlaySoundEffect(DWORD wave, int ticks, long volume, long pan)
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::StartControlledSound(DWORD wave, SOUNDHANDLE &handle, long volume, long pan, BOOL looped)
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::UpdateControlledSound(SOUNDHANDLE handle, long volume, long pan)
	{ return(NO_SOUND_ERROR); }

BOOL SoundManager::FinishedControlledSound(SOUNDHANDLE handle)
	{ return(TRUE); }

SOUNDERROR SoundManager::StopControlledSound(SOUNDHANDLE handle, BOOL fade)
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::SetVolume(long volume)
	{ return(NO_SOUND_ERROR); }


SOUNDERROR SoundManager::FadeOut()
	{ return(NO_SOUND_ERROR); }

SOUNDERROR SoundManager::FadeIn()
	{ return(NO_SOUND_ERROR); }

bool SoundManager::PlayMidiFile(std::string& fileName)
{ return true; }

void SoundManager::StopMidiPlayer()
{ }

void SoundManager::SetVolumeOnMidiPlayer(int32 volume)
{ }

void SoundManager::MuteMidiPlayer(bool mute)
{ }

// these two are private.
bool SoundManager::Write(CreaturesArchive &archive) const
{ return false; }

bool SoundManager::Read(CreaturesArchive &archive)
{ return false; }

