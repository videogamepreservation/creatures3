// --------------------------------------------------------------------------
// Filename:	stub_MidiModule.h
// Class:		MidiModule
// Purpose:		
// History:
// --------------------------------------------------------------------------

#ifndef STUB_MIDI_MODULE_H
#define STUB_MIDI_MODULE_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>


//#define MULTI_TO_WIDE( x,y )  MultiByteToWideChar( CP_ACP, \
//        MB_PRECOMPOSED, y, -1, x, _MAX_PATH );


class MidiModule
{
public :
	MidiModule();
	~MidiModule();

//	bool StartUp(LPDIRECTSOUND directSoundObject,
//						 HWND window);

	bool PlayMidiFile(std::string& file);

	void StopPlaying();
	void SetVolume(long volume);

//	IDirectMusicPerformance* CreatePerformance();
//	IDirectMusicLoader* CreateLoader();
//	IDirectMusicSegment* LoadMidiSegment(WCHAR midiFileName[]);
//	HRESULT FreeDirectMusic();
	void Mute(bool mute);
private:
};

#endif // MIDI_MODULE_H
