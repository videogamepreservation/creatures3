// --------------------------------------------------------------------------
// Filename:	MidiModule.cpp
// Class:		MidiModule
// Purpose:		This plays a midi file.  The midi filename must not include 
//				the whole path and must be extensionless.
//				
//				
// History:
// ------- 
// 24Mar98	Alima		Created. 
//							
//		
// --------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "MidiModule.h"
#include "..\display\system.h"
#include "..\display\ErrorMessageHandler.h"
#include "..\App.h"
#include <wchar.h>
#include "../../common/RegistryHandler.h"


MidiModule::MidiModule()
:myPerformance(NULL),
myDirectMusic(NULL),
myLoader(NULL),
mySegment(NULL),
mySegmentState(NULL),
myMuteFlag(0)
{


}

MidiModule::MidiModule(LPDIRECTSOUND directSoundObject,
						 HWND window)
:myPerformance(NULL),
myDirectMusic(NULL),
myLoader(NULL),
mySegment(NULL),
mySegmentState(NULL),
myMuteFlag(0)
{
	StartUp(directSoundObject,window);
}

bool MidiModule::StartUp(LPDIRECTSOUND directSoundObject,
						 HWND window)
{
	//	Before making any calls to DirectMusic, you have to 
	// initialize COM as follows:

	// should be calling ex version!!!
	if (FAILED(CoInitialize(NULL )))
	{
		// Terminate the application.
		return false;
	}   // Else full speed ahead!



	if(!CreatePerformance())
		return false;

    if (FAILED(myPerformance->Init(&myDirectMusic, directSoundObject, window)))
		return false;

		myVolume = DSBVOLUME_MAX;

	// find the best place for it
	// set your volume according to any registry entry

/*	int32 volume=0;
	std::string testValue = "Volume";
	std::string testKey = theRegistry.DefaultKey() + "\\ParentData";

	if(theRegistry.DoesValueExist(testKey,
								testValue,
								sizeof(int32),
								(uint8*)&volume,
								HKEY_LOCAL_MACHINE))
	{
		// volume attenuates in hundredths of decibels
		// some changes are not noticeable
		if(volume < 25)
		{
			myVolume = -100;
		}
		else if(volume < 50)
		{
			myVolume = 0;
		}
		else if(volume < 75)
		{
			myVolume = 1000;
		}
		else
		{
		myVolume = 10000;
		}
	}
	
	else
	{
		myVolume = DSBVOLUME_MAX;
	}
		
*/

		SetVolume(myVolume);
	
	
//	if (FAILED(myPerformance->AddPort(NULL )))
	//	return false;

	// Now you need to add a port to the performance.
	// Calling AddPortwith a NULL parameter automatically
	// adds the default port (normally the Microsoft Software 
	// Synthesizer) with one channel group, and assigns PChannels 
	// 0-15 to the group's MIDI channels.



IDirectMusicPort* pPort = NULL;
DMUS_PORTPARAMS dmos;DMUS_PORTCAPS dmpc;

GUID guidSynthGUID;
HRESULT hr = S_OK;

if ( !SUCCEEDED(myDirectMusic->GetDefaultPort(&guidSynthGUID)))
{
    return false;
}
 
ZeroMemory(&dmos, sizeof(dmos));
dmos.dwSize = sizeof(DMUS_PORTPARAMS);

dmos.dwChannelGroups = 1;
dmos.dwValidParams = DMUS_PORTPARAMS_CHANNELGROUPS; 

if( !SUCCEEDED(myDirectMusic->CreatePort(guidSynthGUID,            &dmos,
            &pPort,            NULL)))
{
    return false;
} 

ZeroMemory(&dmpc, sizeof(dmpc));dmpc.dwSize = sizeof(DMUS_PORTCAPS); 

if( !SUCCEEDED(pPort->GetCaps(&dmpc)))
{
	// no default synth

    if (pPort) pPort->Release();
    
	if (!pPort)
	{
		for (DWORD index = 0; hr == S_OK; index++)   
		{
        ZeroMemory(&dmpc, sizeof(dmpc));
        dmpc.dwSize = sizeof(DMUS_PORTCAPS); 
        hr = myDirectMusic->EnumPort(index, &dmpc);
		if(hr == S_OK)
        {  
			if ( (dmpc.dwClass == DMUS_PC_OUTPUTCLASS) )           
			{
                CopyMemory(&guidSynthGUID, &dmpc.guidPort,
                        sizeof(GUID)); 
                ZeroMemory(&dmos, sizeof(dmos));
                dmos.dwSize = sizeof(DMUS_PORTPARAMS);
                dmos.dwChannelGroups = 1;
                dmos.dwValidParams = DMUS_PORTPARAMS_CHANNELGROUPS; 
                hr = myDirectMusic->CreatePort(guidSynthGUID, 
                        &dmos, &pPort, NULL);              
				break;         
			}
        }    
		}
		if (hr != S_OK) 
		{  
			if (pPort) pPort->Release();
        return false;
    }
	}
}

	pPort->Activate(TRUE);
	myPerformance->AddPort(pPort);

	myPerformance->AssignPChannelBlock(0, pPort, 1);

//	The original reference to the port can now be released. This call doesn't remove the port from the performance.

if (pPort)
 pPort->Release();



	
	//   DMUS_PORTPARAMS     portParams;
	  // ZeroMemory(&portParams, sizeof(portParams));

	  // portParams.dwSize = sizeof(portParams);



// END TESTING!!!

	if(!CreateLoader())
		return false;


return true;
}

// In order to load any object from disk, you first need 
// to create the DirectMusicLoader object. This is done 
// just as for any other COM object, as shown in the following 
// sample function:
IDirectMusicLoader* MidiModule::CreateLoader()
{
   
 
    if (FAILED(CoCreateInstance(
            CLSID_DirectMusicLoader,
            NULL,
            CLSCTX_INPROC, 
            IID_IDirectMusicLoader,
            (void**)&myLoader
        )))
    {
        myLoader = NULL;
    }

	// we only need to do this once:  set the search directory for midi
	// files
	char buf[200];
	theApp.GetDirectory(SOUNDS_DIR,buf);

	WCHAR wszDir[_MAX_PATH];

  
	std::string path(buf);
	MULTI_TO_WIDE(wszDir, path.data());


    HRESULT hr = myLoader->SetSearchDirectory(GUID_DirectMusicAllTypes,
																wszDir,
																FALSE);



    if (FAILED(hr)) 
    {
        return NULL;
    }
 
//	myLoader->EnableCache(CLSID_DirectMusicSegment, FALSE);
    return myLoader;
}


MidiModule::~MidiModule()
{

	FreeDirectMusic();
}


//	The central object of any DirectMusic application is the performance,
//	which manages the playback of segments. It is created by using the 
//	COM CoCreateInstance function, as in the following sample function:

IDirectMusicPerformance* MidiModule::CreatePerformance()
{
 
    if (FAILED(CoCreateInstance(
            CLSID_DirectMusicPerformance,
            NULL,
            CLSCTX_INPROC, 
            IID_IDirectMusicPerformance,
            (void**)&myPerformance
        )))
    {
        myPerformance = NULL;
    }
 
    return myPerformance;

}



IDirectMusicSegment* MidiModule::LoadMidiSegment(WCHAR  midiFileName[])
{
	if(!myLoader)
		return NULL;

	// If there is any music playing, stop it. This is 
    // not really necessary, because the music will stop when
    // the instruments are unloaded or the performance is
    // closed down.
	if(myPerformance)
		 myPerformance->Stop( NULL, NULL, 0, 0 );

	// Before loading a new segment, clean up any existing one.
	// Then pass a file name to the LoadMIDISegment function.
	if(mySegment)
	{
		// Unload instruments - this will cause silence.
		// CloseDown unloads all instruments, so this call is also not 
		// strictly necessary.
		mySegment->SetParam(GUID_Unload, -1, 0, 0, NULL);
 
		// Release the segment.
		mySegment->Release();
	}


// In this step you will implement a function, 
// LoadMIDISegment, that takes a pointer to the IDirectMusicLoader
// created in the last step and uses it to create 
// a segment object encapsulating the data from a MIDI file.

    DMUS_OBJECTDESC ObjDesc; 
    IDirectMusicSegment* tempSegment = NULL;
 
	

// You then describe the object to be loaded,
// in a DMUS_OBJECTDESC structure:
    ObjDesc.guidClass = CLSID_DirectMusicSegment;
    ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
	
    wcscpy( ObjDesc.wszFileName, midiFileName );
    ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
 
// Now load the object and query it for the IDirectMusicSegment
// interface. This is done in a single call to 
// IDirectMusicLoader::GetObject. Note that loading the object
// also initializes the tracks and does everything else necessary 
// to get the MIDI data ready for playback.
    myLoader->GetObject(&ObjDesc,
            IID_IDirectMusicSegment, (void**) &mySegment);

	if(!mySegment)
		return NULL;
 
// To ensure that the segment plays as a standard MIDI file,
// you now need to set a parameter on the band track. 
// Use the IDirectMusicSegment::SetParam <dmref_1b1f.htm> method 
// and let DirectMusic find the track, by passing -1 (or 0xFFFFFFFF) 
// in the dwGroupBits method parameter.
    mySegment->SetParam(GUID_StandardMIDIFile,
            -1, 0, 0, (void*)myPerformance);
 
// This step is necessary because DirectMusic handles program changes and 
// bank selects differently for standard MIDI files than it does for MIDI content
// authored specifically for DirectMusic. The GUID_StandardMIDIFile 
// parameter must be set before the instruments are downloaded. 
// The next step is to download the instruments. This is necessary even for 
// playing a simple MIDI file, because the default software synthesizer needs 
// the DLS  data for the General MIDI instrument set. If you skip this step,
// the MIDI file will play silently. Again, you call SetParam on the segment, 
// this time specifying the GUID_Download parameter:
    mySegment->SetParam(GUID_Download, -1, 0, 0, (void*)myPerformance);
 


// Note that there's no harm in requesting the download even though this might
// already have been done in a previous call to the LoadMIDISegment function.
// A redundant request is simply ignored. Eventually you have to unload the instruments,
// but that can wait until you're ready to shut down DirectMusic.
// The function now returns a pointer to the segment, which is ready to be played.
//	mySegment = tempSegment;
    return mySegment;
 
} // End of LoadSegment()


//Before exiting, the program must unload the instruments,
// release all the objects that have been created, and dereference 
// COM (remember, every call to CoInitialize must have a matching call
// to CoUninitialize).
//The following function performs the necessary cleanup:
HRESULT MidiModule::FreeDirectMusic()
{
    // If there is any music playing, stop it. This is 
    // not really necessary, because the music will stop when
    // the instruments are unloaded or the performance is
    // closed down.
	if(myPerformance)
		 myPerformance->Stop( NULL, NULL, 0, 0 );
 
	if(mySegment)
	{
		// Unload instruments - this will cause silence.
		// CloseDown unloads all instruments, so this call is also not 
		// strictly necessary.
		mySegment->SetParam(GUID_Unload, -1, 0, 0, (void*)NULL);
 
		// Release the segment.
		mySegment->Release();
		mySegment = NULL;
	}

	if(myPerformance)
	{
 
		// CloseDown and Release the performance object.
		myPerformance->CloseDown();
		myPerformance->Release();
		myPerformance = NULL;
	}
 
	if(myLoader)
	{
		// Release the loader object.
		myLoader->Release();
		myLoader = NULL;
	}
	// for some reason this causes the engine to crash witrh an 
	// access violation???

    // Release COM.
  //  CoUninitialize();
 
    return S_OK;
}


bool MidiModule::PlayMidiFile(std::string& file)
{
	if(file.empty() || myMuteFlag)
		return false;

///	SetVolume(-10000);

	if(file != myCurrentFile || myCurrentFile.empty())
	{
	//  DO NOT USE THE FULL PATH AS THIS HAS ALREADY BEEN
	// ESTABLISHED DURING CONSTRUCTION
	WCHAR wszDir[_MAX_PATH];
	// check the file exists first - you won't catch me with
	// that old chestnut oh no!
	char buf[200];
	theApp.GetDirectory(SOUNDS_DIR,buf);

	// we expect the filename to be extensionless
	std::string path(buf);

	path+=file + ".mid";

	int attributes = GetFileAttributes(path.data());

	if(attributes ==-1)
	{
		ErrorMessageHandler::Show("sound_error", 0, "MidiModule::PlayMidiFile", path.c_str());
		return false;
	}


	path=file + ".mid";

	// must change the string to wide characters

    MULTI_TO_WIDE(wszDir, path.data());

	LoadMidiSegment(wszDir);

	if (mySegment)
	{
	mySegment->SetRepeats(4000000);

    myPerformance->PlaySegment(mySegment, 0, 0, &mySegmentState);
	myCurrentFile = file;
	return true;
	}

	}

	return false;


}


void MidiModule::StopPlaying()
{
		// If there is any music playing, stop it. This is 
    // not really necessary, because the music will stop when
    // the instruments are unloaded or the performance is
    // closed down.
	if(myPerformance)
		 myPerformance->Stop( NULL, NULL, 0, 0 );

	// Before loading a new segment, clean up any existing one.
	// Then pass a file name to the LoadMIDISegment function.
	if(mySegment)
	{
		// Unload instruments - this will cause silence.
		// CloseDown unloads all instruments, so this call is also not 
		// strictly necessary.
		mySegment->SetParam(GUID_Unload, -1, 0, 0, NULL);
 
		// Release the segment.
	//	mySegment->Release();
	}

}


void MidiModule::SetVolume(long volume)
{
	// volume attenuates in hundredths of decibels
	// some changes are not noticeable
	if(volume < 25)
	{
		myVolume = -100;
	}
	else if(volume < 50)
	{
		myVolume = 0;
	}
	else if(volume < 75)
	{
		myVolume = 1000;
	}
	else
	{
	myVolume = 10000;
	}

	HRESULT res =0;
	if(myPerformance)
	{
		res = myPerformance->SetGlobalParam( GUID_PerfMasterVolume,
										&volume,
										sizeof(long));
		if(res != DD_OK)
		{
			int alima = 0;
		}
	}
}

void MidiModule::Mute(bool mute)
{
	myMuteFlag = mute;
}