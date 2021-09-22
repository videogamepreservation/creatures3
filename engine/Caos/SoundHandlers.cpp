// Filename:    SoundHandlers.h
// Class:       SoundHandlers
// Purpose:     Routines to implement sound commands/values in CAOS.
// Description:
//
// Usage:
//
// History: 31Mar99 Alima Created
// -------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SoundHandlers.h"
#include "CAOSMachine.h"
#include "../Agents/Agent.h"
#include "../Sound/Soundlib.h"
#include "../App.h"
#include "../World.h"
#include "../Map/Map.h"
// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning( disable : 4786 4503)
#endif

#ifndef _WIN32
#include "../unix/FileFuncs.h"
#endif

// Macro to make four-character constants
// Assumes little-endian by default.
#ifdef C2E_BIG_ENDIAN		// eg Mac
	#define Tok(a,b,c,d) (((a)<<24) | ((b)<<16) | ((c)<<8) | (d))
#else
	#define Tok(a,b,c,d) (((d)<<24) | ((c)<<16) | ((b)<<8) | (a))
#endif



void SoundHandlers::Command_RMSC( CAOSMachine& vm )
{
	std::string str;
	int x,y;
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();
	
	vm.FetchStringRV( str );

	int roomID;
	if(theApp.GetWorld().GetMap().GetRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}



void SoundHandlers::Command_MMSC( CAOSMachine& vm )
{
	std::string str;
	int x,y;

	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	vm.FetchStringRV( str );

	int roomID;
	if(theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetMetaRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}


void SoundHandlers::StringRV_RMSC( CAOSMachine& vm, std::string &str)
{
	int x,y;

	str="";
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().GetRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}



void SoundHandlers::StringRV_MMSC( CAOSMachine& vm, std::string &str)
{
	int x,y;

	str.empty();
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().GetMetaRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}


void SoundHandlers::Command_MIDI( CAOSMachine& vm )
{
	std::string str;

	vm.FetchStringRV( str );

	if (theSoundManager) {
		if(str.empty())
		{
		theSoundManager->StopMidiPlayer();
		}
		else
		{
		CheckSoundFileExists(vm, str + ".mid");
		theSoundManager->PlayMidiFile(str);	
		}
	}
}


void SoundHandlers::Command_RCLR( CAOSMachine& vm )
{
	int x,y;

	std::string str("none");
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetRoomTrack(roomID,str);
	}
	else
	{	
		vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}



void SoundHandlers::Command_MCLR( CAOSMachine& vm )
{
	int x,y;

	std::string str("none");
	
	x = vm.FetchIntegerRV();
	y = vm.FetchIntegerRV();

	int roomID;
	if(theApp.GetWorld().GetMap().GetMetaRoomIDForPoint(Vector2D(x,y),roomID))
	{
		theApp.GetWorld().GetMap().SetMetaRoomTrack(roomID,str);
	}
	else
	{
		
	vm.ThrowRunError( CAOSMachine::sidFailedToGetMetaRoomLocation );
	}
}


void SoundHandlers::CheckSoundFileExists( CAOSMachine& vm, const std::string& file )
{
	FilePath filepath( file, SOUNDS_DIR );
	if (FileExists(filepath.GetFullPath().c_str()) == false)
		vm.ThrowRunError( CAOSMachine::sidSoundFileNotFound, file.c_str());
}

void SoundHandlers::Command_SNDE( CAOSMachine& vm )
{ 
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	CheckSoundFileExists(vm, str + ".wav");

	vm.GetTarg().GetAgentReference().SoundEffect(Tok( str[0], str[1], str[2], str[3] ));
	
}

void SoundHandlers::Command_SNDQ( CAOSMachine& vm )
{
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	int delay = vm.FetchIntegerRV();
	CheckSoundFileExists(vm, str + ".wav");

	vm.GetTarg().GetAgentReference().SoundEffect(Tok( str[0], str[1], str[2], str[3] ),delay);
}



void SoundHandlers::Command_SNDC( CAOSMachine& vm )
{ 
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	CheckSoundFileExists(vm, str + ".wav");
	vm.GetTarg().GetAgentReference().ControlledSound(Tok( str[0], str[1], str[2], str[3] ));
}

void SoundHandlers::Command_SNDL( CAOSMachine& vm )
{
	vm.ValidateTarg();

	std::string str;
	vm.FetchStringRV( str );
	CheckSoundFileExists(vm, str + ".wav");
	vm.GetTarg().GetAgentReference().ControlledSound(Tok( str[0], str[1], str[2], str[3] ),true);
}

void SoundHandlers::Command_STPC( CAOSMachine& vm )
{ 
	vm.ValidateTarg();

	vm.GetTarg().GetAgentReference().StopControlledSound();
}
	

void SoundHandlers::Command_FADE( CAOSMachine& vm ) 
{
		vm.ValidateTarg();

	vm.GetTarg().GetAgentReference().StopControlledSound(TRUE);
}


void SoundHandlers::Command_VOLM(CAOSMachine& vm )
{	
	int volume = vm.FetchIntegerRV();
	if (theSoundManager) {
		theSoundManager->SetVolumeOnMidiPlayer(volume);
		theSoundManager->SetVolume(volume);
	}
}
