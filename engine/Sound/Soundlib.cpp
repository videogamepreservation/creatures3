#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../common/C2eTypes.h"
#include "MidiModule.h"
#include "soundlib.h"
#include "../general.h"
#include "../App.h"
#include "../C2eServices.h"
#include "../File.h"
#include "../Display/ErrorMessageHandler.h"


#ifdef _MSC_VER
#pragma warning (disable:4786 4503)
#endif

CREATURES_IMPLEMENT_SERIAL( SoundManager)

CREATURES_IMPLEMENT_SERIAL( SoundQueueItem)

CREATURES_IMPLEMENT_SERIAL( CachedSound)



// ----------------------------------------------------------------------
// History:
// 05May98 Peter Chilvers Altered OpenSound and GetWaveSize to handle
//						  Playing from the "munged" music file
//						  Commented out DSBSTATIC flag in PlaySound
// 26May98 Peter Chilvers Moved initialisation into constructor head
//						  Added FadeIn and FadeOut
// 29May98 Peter Chilvers Fixed minimum volume to -5000
// 
// ----------------------------------------------------------------------

CachedSound::CachedSound()
{
	name=0;
	size=0;
	used=0;
	copies=0;
	buffer=NULL;
}

CachedSound::~CachedSound()
{
	if (buffer!=NULL)
	{
		// Sound buffer not released (this was a log statement)
		ASSERT(false);

		delete buffer;
	}
}

bool CachedSound::Write(CreaturesArchive &archive) const
{
	return false;
}

bool CachedSound::Read(CreaturesArchive &archive)
{
	return false;
}

ActiveSample::ActiveSample() :
	pSample(NULL),
	wID(0),
	cloned(NULL),
	fade_rate(0),
	locked(FALSE),
	volume(0)
{
}


// Static variables
// Share the DirectSound object and primary buffers between different sound managers
int	SoundManager::references=0;
LPDIRECTSOUND SoundManager::pDSObject=NULL;
IDirectSoundBuffer	*SoundManager::pPrimary=NULL;

void SoundManager::SetMNGFile(std::string& mng)
{
	if (mungeFile == mng)
		return;
	mungeFile = mng;
	FlushCache();
}

SoundManager::SoundManager() :
	maximum_size(0),
	current_size(0),
	last_used(0),
	sound_initialised(FALSE),
	overall_volume(0),
	target_volume(0),
	current_volume(0),
	myMidiModule(0),
	faded(false)
{

	mungeFile = "music.mng";

	references++;

	// Only set up the direct sound object the first time
	if (references == 1)
		{
		///////////////////////////////////////////////////
		// SET UP DIRECT SOUND (FOR EFFECTS)

		//Create a DirectSound object...
		HRESULT hr;

		if(hr = DirectSoundCreate(NULL, &pDSObject, NULL)!=DS_OK) {
			// Failed to create DS Object
			std::string string = ErrorMessageHandler::Format("sound_error",
										(int)SoundManager::sidFailedToCreateDirectSoundObject,
										std::string("SoundManager::SoundManager")
			);
			throw(SoundManager::SoundException(string,__LINE__));
		}

		// Get a handler to the window
		HWND hWnd;
		hWnd=theApp.GetMainHWND();

		//set the coop level...
		if ((hr=pDSObject->SetCooperativeLevel(hWnd, DSSCL_PRIORITY))!=DS_OK)
			{
			if (hr==DSERR_ALLOCATED)
				{
				  // Failed to set cooperative level. Resources already allocated
				}
			if (hr==DSERR_INVALIDPARAM)
				{
				  // Failed to set cooperative level. Invalid parameter
				}

			std::string string = ErrorMessageHandler::Format("sound_error",
										(int)SoundManager::sidFailedToSetCooperativeLevel,
										std::string("SoundManager::SoundManager")
			);
			throw(SoundManager::SoundException(string,__LINE__));
		}


		DSBUFFERDESC	dsbdesc;
		//setup the DSBUFFERDESC structure
		memset(&dsbdesc, 0 , sizeof(DSBUFFERDESC));

		dsbdesc.dwSize				= sizeof(DSBUFFERDESC);
		dsbdesc.dwFlags				= DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
		
		dsbdesc.dwBufferBytes		= 0;
		
		dsbdesc.lpwfxFormat			= NULL;



		//create the primary sound buffer
		hr=pDSObject->CreateSoundBuffer(&dsbdesc,
										&pPrimary,
										NULL);


		if (hr!=DS_OK)
			{
			// Primary buffer not created
			std::string string = ErrorMessageHandler::Format("sound_error",
										(int)SoundManager::sidPrimaryBufferNotCreated,
										std::string("SoundManager::SoundManager")
			);
			throw(SoundManager::SoundException(string,__LINE__));
			}

		WAVEFORMATEX wvfm;
		
		pPrimary->GetFormat(&wvfm,sizeof(wvfm),NULL);

		wvfm.nChannels=2;
		wvfm.nSamplesPerSec=22050;
		wvfm.nAvgBytesPerSec=88200;
		wvfm.nBlockAlign=4;
		wvfm.wBitsPerSample=16;

		hr=pPrimary->SetFormat(&wvfm);

		if (hr!=DS_OK)
			{
			// Primary buffer could not be set to new format
			pPrimary->GetFormat(&wvfm,sizeof(wvfm),NULL);

			std::string string = ErrorMessageHandler::Format("sound_error",
										(int)SoundManager::sidPrimaryBufferCouldNotBeSetToNewFormat,
										std::string("SoundManager::SoundManager")
			);
			throw(SoundManager::SoundException(string,__LINE__));
			}
		}



	sounds_playing = 0;
	sound_index = 1;

	//initialize the sample slots...
	for(int i=0;i<MAX_ACTIVE_SOUNDS;i++)
	{
		active_sounds[i].wID = 0;
	}


	///////////////////////////////////////////////////
	// what we could also do here is to set up the PRIMARY buffer and play it - this would reduce the startup time
	// when the first secondary buffer is played.

	sound_initialised=TRUE;

	// Mixer is active, but silent

	mixer_suspended=FALSE;

	// only start up the midi if we play midi
	// but do start it up now before
	if(theApp.DoYouOnlyPlayMidiMusic())
	{
		if(!myMidiModule)
		{
			myMidiModule = new MidiModule(pDSObject,theApp.GetMainHWND());
		
		}
	}

}

SoundManager::~SoundManager()
{
		if(myMidiModule)
			delete myMidiModule;

	if (!sound_initialised)
	{
		return;
	}

	StopAllSounds();

	FlushCache();	

	// Only close down if this is the last remaining sound manager

	if (-- references == 0)
		{

		pPrimary ->Release();

		pDSObject->Release();

		}
}

CachedSound *SoundManager::OpenSound(DWORD wave)
{
	if (!sound_initialised)
	{
		return (NULL);
	}

	// Either read from the munged file, or
	// from the sound effects file
	bool munged = wave >= 0xff000000;

	File file;
	if (munged)
		{
		try
		{
			char buf[_MAX_PATH];
			theApp.GetDirectory(SOUNDS_DIR,buf);
			std::string path(buf);
			//path+="Music.mng";
			path += mungeFile;
			file.Open(path);
		
			if(!file.Valid())
			{
				return NULL;
			}
		}
		catch(File::FileException& e)
		{
			ErrorMessageHandler::Show(e, std::string("SoundLib::OpenSound"));
			return NULL;

		}


		// Calculate the index into the file
		DWORD index = wave - 0xff000000;

		// Skip straight to the appropriate offset

		// The header consists of:
		// total voices
		// offset to script, sizeof script
		// offset to wave, sizeof wave
		// etc.

		file.Seek( (3 + index * 2) * sizeof(int), File::Start);

		// Now read in the offset to the start, and the total size
		int offset, size;
		file.Read(&offset,sizeof(int));
		file.Read(&size,sizeof(int));

		// Now point straight to the start of the wave
		file.Seek(offset,File::Start);
		}
	else
	{
		try
		{
			std::string string(BuildFsp(wave,"wav",SOUNDS_DIR));

			// Need to load in sound from 'Sounds' directory
			file.Open(string);
			string +="\n";
		}
		catch(File::FileException& e)
		{
			ErrorMessageHandler::Show(e, std::string("SoundLib::OpenSound"));
			return NULL;
		}
	}

	DSBUFFERDESC	dsbdesc;
	PCMWAVEFORMAT	pcmwf;			// this structure has been superseeded by WAVEFORMATEX - 
	HRESULT			hr;				// but the direct sound documentation example uses PCMWAVEFORMAT

	DWORD			dwLength;

	char			dword_str[5]="xxxx";	// used for comparing strings in file

	//----------------------------------

	// Skip the first sixteen bytes if we're reading the
	// munged version.  This removes a few extra clues about
	// the file structure

	if (!munged)
		{

		//CHECK THAT THE FILE IS OF CORRECT FORMAT (RIFF)....
		file.Read(dword_str, 4);
		if(strcmp(dword_str,"RIFF")!=0)
			return NULL;
		
		file.Read(&dwLength, 4);

		//is it a WAV????
		file.Read(dword_str, 4);
		if(strcmp(dword_str,"WAVE")!=0)
			return NULL;
		
		file.Read(dword_str, 4);
		if(strcmp(dword_str,"fmt ")!=0)
			return NULL;
		}


	//READ THE WAVE HEADER...........
	DWORD header_size;
	file.Read(&header_size,4);
	
	file.Read(&pcmwf.wf.wFormatTag,2);
	file.Read(&pcmwf.wf.nChannels,2);

	file.Read(&pcmwf.wf.nSamplesPerSec,4);
	file.Read(&pcmwf.wf.nAvgBytesPerSec,4);

	file.Read(&pcmwf.wf.nBlockAlign,2);
	file.Read(&pcmwf.wBitsPerSample,2);

	if (header_size>16)
	{
		// Skip past any extra header data
		file.Seek(header_size-16,File::Current);
	}

	file.Read(dword_str, 4);
	if (strcmp(dword_str,"fact")==0)
	{
		// Skip past 'fact' data
		file.Seek(8,File::Current);
		file.Read(dword_str, 4);
	}

	if(strcmp(dword_str,"data")!=0)
		return NULL;

	// calculate the remaining number of bytes in the file
	DWORD	numbytes;	//=file.GetLength()-file.GetPosition();
	file.Read(&numbytes,4);

	//setup the DSBUFFERDESC structure
	memset(&dsbdesc, 0 , sizeof(DSBUFFERDESC));

	dsbdesc.dwSize				= sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags				= DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_GLOBALFOCUS;	//|DSBCAPS_STATIC;		//need default controls (vol, pan, freq)
	
	dsbdesc.dwBufferBytes		= numbytes;
	
	dsbdesc.lpwfxFormat			= (LPWAVEFORMATEX)&pcmwf;


	IDirectSoundBuffer *buffer;

	//create the sound buffer
	hr=pDSObject->CreateSoundBuffer(&dsbdesc,
									&buffer,
									NULL);

	//now copy the sound data to the DirectSound buffer
	if(hr==DS_OK)
	{												
		LPVOID	lpvAudio1, lpvAudio2;
		DWORD	Bytes1, Bytes2;
		//--------------------------------

		//obtain a write pointer
		hr = buffer->Lock(0, numbytes,
						  &lpvAudio1, &Bytes1,
						  &lpvAudio2, &Bytes2,
						  0);

		//if we got a DSERR_BUFFERLOST try again
		if(hr==DSERR_BUFFERLOST)
		{
			buffer->Restore();
			hr = buffer->Lock(0, numbytes,
							  &lpvAudio1, &Bytes1,
							  &lpvAudio2, &Bytes2,
							  0);
		}

		//if now okay copy the buffer...
		if(hr==DS_OK)
		{
			file.Read(lpvAudio1, Bytes1);

			if(lpvAudio2!=NULL)
			{
				//this shouldn't happen (since these are all NEW DirectSound buffers)
				file.Read(lpvAudio2, Bytes2);
			}

			//now unlock the thingy....
			hr=buffer->Unlock(lpvAudio1, Bytes1, lpvAudio2, Bytes2);

			CachedSound *w=new CachedSound;
			w->name=wave;
			w->used=last_used++;
			w->size=numbytes;
			w->copies=0;
			w->buffer=buffer;
			
			return(w);

		}
		else
		{
			switch(hr)
			{
				case(DSERR_BUFFERLOST):
					{
						// DSERR_BUFFERLOST
						break;
					}
				case(DSERR_INVALIDCALL):
					{
						// DSERR_INVALIDCALL
						break;
					}
				case(DSERR_INVALIDPARAM):
					{
						// DSERR_INVALIDPARAM
						break;
					}
				case(DSERR_PRIOLEVELNEEDED):
					{
						// DSERR_PRIOLEVELNEEDED
						break;
					}
				default:
					{
						// Unknown error
						break;
					}
			}

			//everything failed dismally...
			return NULL;
		}

	}
	else
	{
		int catalogue_index = 2;
		switch(hr)
		{
			case DSERR_ALLOCATED:		catalogue_index = 3;
										break;

			case DSERR_BADFORMAT:		catalogue_index = 4;
										break;

			case DSERR_INVALIDPARAM:	catalogue_index = 5;
										break;

			case DSERR_NOAGGREGATION:	catalogue_index = 6;
										break;

			case DSERR_OUTOFMEMORY:		catalogue_index = 7;
										break;
		}

		ErrorMessageHandler::Show("sound_error", catalogue_index, "SoundManager::OpenSound");

		return NULL;									
	}
}

SOUNDHANDLE SoundManager::PlayCachedSound(CachedSound *wave,
				int volume, int pan, BOOL loop)
{
	IDirectSoundBuffer *pDSBuffer=wave->buffer;
	ActiveSample		*pActive;
	DWORD				status, playflags=0;
	HRESULT				result=DS_OK;
	//------------------

	//any spare slots????
	if(!sound_initialised || sounds_playing==MAX_ACTIVE_SOUNDS)
		{
		// Sound buffer full;
		return -1;
		}

	if (sounds_playing>MAX_ACTIVE_SOUNDS)
	{
		// Active sounds overflowed limit
		return -1;
	}

	//is it a valid sample 
	//....

	//find a spare slot
	int i=0;
	while(active_sounds[i].wID!=0 && i<MAX_ACTIVE_SOUNDS)
	{
		i++;
	}

	pActive=&active_sounds[i];

	// Set the sound's basic volume (before overall volume is taken
	// into account)
	pActive -> volume = volume;
	
	//is the bank's sample playing??? if it is we need to duplicate it...
	pDSBuffer->GetStatus(&status);
	
	if(status&DSBSTATUS_PLAYING)
	{
		result=pDSObject->DuplicateSoundBuffer(pDSBuffer, &(pActive->pSample));
		
		pActive->cloned=wave;	//mark as not the original buffer i.e. a copy
		wave->copies++;
	}
	else
	{
		pActive->pSample=pDSBuffer;
		pActive->cloned=NULL;	//mark AS the original buffer

	}

	//is everything okay?????
	if(result==DS_OK)
	{
		//set up flags
		if (loop)
		{
			playflags |= DSBPLAY_LOOPING;
		}

		pDSBuffer=pActive->pSample;

		// Take the overall volume of the manager into account
		volume += current_volume;

		// Can't get quieter than silence
		if (volume < SoundMinVolume)
			{
			volume = SoundMinVolume;
			}

		pDSBuffer->SetVolume(volume);

		if (volume > 0 || volume < SoundMinVolume)
			{
			// "Volume out of range %d\n",volume 
			}

		pDSBuffer->SetPan(pan);

		//play the sample 
		
		result=pDSBuffer->Play(0, 0, playflags);	
		
		if(result==DS_OK)
		{
			//set up the sample
			pActive->wID= ++sound_index;			//need pre increment!

			sounds_playing++;
		}
	}

	// Flag as unlocked (delete when finished)
	active_sounds[i].locked=FALSE;
	active_sounds[i].fade_rate=0;

	return (i);		// return channel number (SOUNDHANDLE)

}

void SoundManager::StopAllSounds()
{
	if (sound_initialised)
	{
		FlushSoundQueue();
		if (sounds_playing)
		{
			for(int i=0;i<MAX_ACTIVE_SOUNDS;i++)
			{
				StopSound(i);
			}
		}
	}
}

void SoundManager::FlushSoundQueue()
{
	std::vector<SoundQueueItem*>::iterator it;
		// Flush existing cache
	for (it = sound_queue.begin(); it != sound_queue.end(); it++)
	{
		//	deallocate wave
		delete (*it);
		(*it)=NULL;
	}

	sound_queue.clear();
	
}

void SoundManager::UpdateSoundQueue()					
{
	// Scan through sound queue backwards, flagging that a tick has
	// passed, and playing sounds whose 'time has come'

//	for (int i=sound_queue.GetSize()-1;i>=0;i--)
	std::vector<SoundQueueItem*>::iterator item;

	for (item = sound_queue.begin(); item != sound_queue.end(); item++)
	{
		if((*item))
			{
			if ((*item)->ticks<=0)
			{
			
				PlaySoundEffect((*item)->wave, 0, (*item)->volume, (*item)->pan);
				delete (*item);
				*item = NULL;
			}
			else
			{
				(*item)->ticks--;
			}
		}
	}
	
}

										

									
void SoundManager::Update()
	{
	const long MANAGER_FADE_RATE = 200;

	if (sound_initialised)
		{
		// Play any delayed sounds that are waiting
		UpdateSoundQueue();

		// Move the played volume towards the target volume
		if (current_volume < target_volume)
			{
			current_volume += MANAGER_FADE_RATE;

			// check we didn't overshoot
			if (current_volume > target_volume)
				{
				current_volume = target_volume;
				}
			}
		else
			{
			if (current_volume > target_volume)
				{
				current_volume -= MANAGER_FADE_RATE;

				// check we didn't overshoot
				if (current_volume < target_volume)
					{
					current_volume = target_volume;
					}
				}
			}


		if (sounds_playing)
			{
			for(int i=0;i<MAX_ACTIVE_SOUNDS;i++)
				{
				if (active_sounds[i].wID)
					{
					DWORD status;
					IDirectSoundBuffer *pDSBuffer=active_sounds[i].pSample;
					pDSBuffer->GetStatus(&status);
					if (!(status&DSBSTATUS_PLAYING))
						{
						if (!active_sounds[i].locked)
							{
							StopSound(i);
							}
						}
					else
						{
						// Is the sound fading?
						if (active_sounds[i].fade_rate)
							{
							// Decrease the sound's basic volume by 
							// the fade rate
							active_sounds[i].volume += active_sounds[i].fade_rate;

							// Calculate the volume of the buffer, taking
							// into account the overall volume of the manager
							long volume = active_sounds[i].volume + current_volume;

							// Is the sound still audible ?
							if (volume<=SoundMinVolume)
								{
								// No - stop it
								StopSound(i);
								}
							else
								{
								// Adjust the buffer to take the overall
								// volume into account
								pDSBuffer->SetVolume(volume);
								
								if (volume > 0 || volume < SoundMinVolume)
									{
									// "Volume out of range %d\n",volume
									}
								}
							}
						else
							{
							// Calculate the volume of the buffer, taking
							// into account the overall volume of the manager
							long volume = active_sounds[i].volume + current_volume;

							// Can't get quieter than silence
							if (volume < SoundMinVolume)
								{
								volume = SoundMinVolume;
								}

							pDSBuffer->SetVolume(volume);

							if (volume > 0 || volume < SoundMinVolume)
								{
								// "Volume out of range %d\n",volume
								}


							}

						}
					}
				}
			}

		}
	}

void SoundManager::StopSound(SOUNDHANDLE handle)
{
	if (sounds_playing && sound_initialised)
	{
		ActiveSample *pActive=&active_sounds[handle];
		if (pActive->wID!=0)
		{
			pActive->wID=0;
			// alima added this
			// when you call stop on a secondary buffer
			// the sample will remember where it stopped
			// and will continue playing from there when you
			// restart it.   
			// but that would be pause sound so I will
			// set it back to the beginning here...
			pActive->pSample->Stop();
			pActive->pSample->SetCurrentPosition(0);


			if (pActive->cloned!=NULL)
			{
				pActive->pSample->Release();
				pActive->cloned->copies--;
			}

			sounds_playing--;
		}
	}
}

void SoundManager::CloseSound(CachedSound *wave)
{
	if (wave==NULL)
	{
		return;
	}

	if (wave->copies>0)
	{
		// stop and remove any copies still playing
		for(int i=0;i<MAX_ACTIVE_SOUNDS;i++)
		{
			if (active_sounds[i].wID && active_sounds[i].cloned==wave)
			{
				StopSound(i);
			}
		}
	}

	if (wave->buffer!=NULL)
	{
		// now remove the original if it is still playing
		for(int i=0;i<MAX_ACTIVE_SOUNDS;i++)
		{
			if (active_sounds[i].wID && active_sounds[i].pSample==wave->buffer)
			{
				StopSound(i);
			}
		}
		wave->buffer->Release();
		wave->buffer=NULL;
	}

	delete wave;

}

BOOL SoundManager::SoundEnabled()					//  Is mixer running?
{
	return(sound_initialised && !mixer_suspended);
}

SOUNDERROR SoundManager::InitializeCache(int size)	//	Set size (K) of cache in bytes
													//  Flushes the existing cache
{
	FlushCache();

	maximum_size=size*1024;		// Fixes size of cache (in bytes)

	current_size=0;
	last_used=0;

	return(NO_SOUND_ERROR);
}

SOUNDERROR SoundManager::FlushCache()	//  Clears all stored sounds from
										//  the sound cache
{
	if (sound_initialised)
	{
		StopAllSounds();

		if (maximum_size>0)
		{	
			// Flush existing cache
		//	for (int i=0;i<sounds.GetSize();i++)
		//	{
			std::set<CachedSound*>::iterator item;

			for (item = sounds.begin(); item != sounds.end(); item++)
			{
			//	deallocate wave
			//	CachedSound *wave=(CachedSound *) sounds[i];
				CloseSound((*item));
			}

			sounds.clear();
		}
	}
	current_size=0;

	return(NO_SOUND_ERROR);
}

SOUNDERROR SoundManager::SuspendMixer()				//  Stop the mixer playing
													//  (Use on KillFocus)
{
	if (mixer_suspended || !sound_initialised)
	{
		return(SOUND_MIXER_SUSPENDED);
	}

	StopAllSounds();	

	mixer_suspended=TRUE;

	return(NO_SOUND_ERROR);

}

SOUNDERROR SoundManager::RestoreMixer()		//  Restart the mixer
											//  After it has been suspended

{
	if (mixer_suspended && sound_initialised)
	{
		mixer_suspended=FALSE;
	}
	return(NO_SOUND_ERROR);
}

SOUNDERROR SoundManager::RemoveFromCache(CachedSound* index)
{
	std::set<CachedSound*>::iterator it = sounds.find(index);

	if(it!= sounds.end())
	{
		current_size-=(*it)->size;
		CloseSound((*it));

		sounds.erase(it);
		return(NO_SOUND_ERROR);
	}
	else
	{
		return(SOUND_NOT_FOUND);
	}
/*
	if (index>sounds.size())
	{
		return(SOUND_NOT_FOUND);
	}
	else
	{
		CachedSound *w=(CachedSound *) sounds[index];

		current_size-=w->size;
		CloseSound(w);

		
		sounds.RemoveAt(index);
		return(NO_SOUND_ERROR);
	}*/
}

SOUNDERROR SoundManager::MakeRoomInCache(int size)
{
	// Is sound too big for cache?
	
	if (maximum_size<size)
	{
		return(SOUNDCACHE_TOO_SMALL);
	}

	// We need to clear enough space for the sound

	BOOL no_space=FALSE;	// TRUE if nothing can be deleted

	while((maximum_size-current_size)<size && !no_space)
	{
		// Repeatedly remove least used sound
		// until there is adequate room

		int age=0x7FFFFFFF;
		int oldest=-1;

		CachedSound * oldestWave = NULL;
		std::set<CachedSound*>::iterator it;

	//	for(int i=0;i<sounds.size();i++)
		for(it = sounds.begin(); it!=sounds.end();it++)
		{
		
			if ((*it)->used<age)
			{
			
				// is this sound playing, or is there an existing clone playing?
				uint32 status;
				(*it)->buffer->GetStatus(&status);
				if (!(status&DSBSTATUS_PLAYING) && (*it)->copies==0)
				{
					age=(*it)->used;
				oldestWave = (*it);
				}
			}
		}

		if (oldestWave)
		{
			RemoveFromCache(oldestWave);
		}
		else
		{
			no_space=TRUE;
		}
	}

	if (no_space)
	{
		return(SOUNDCACHE_TOO_SMALL);
	}
	else
	{
		HRESULT hr=pDSObject->Compact();
		if (hr==DSERR_INVALIDPARAM)
		{
			// DS Compact- Invalid Parameter
		}
		if (hr==DSERR_PRIOLEVELNEEDED)
		{
			// DS Compact- Wrong priority
		}
		return(NO_SOUND_ERROR);
	}

}

int SoundManager::GetWaveSize(DWORD wave)
{
	// Is this one of the munged files ?
	if (wave >= 0xff000000)
		{	
		// Yes - read in its details from the munged file
		File munged;
		try
		{
			char buf[_MAX_PATH];
			theApp.GetDirectory(SOUNDS_DIR,buf);
			std::string path(buf);
			//path+="Music.mng";
			path += mungeFile;
			munged.Open(path);
			

			// Calculate the index into the file
			uint32 index = wave - 0xff000000;

			// Skip straight to the appropriate offset

			// The header consists of:
			// total voices
			// offset to script, sizeof script
			// offset to wave, sizeof wave
			// etc.
			munged.Seek(( (index * 2) + 4 ) * sizeof(int), File::Start);

			int size;
			munged.Read(&size,sizeof(int));

			return size;
			}
		catch(File::FileException& e)
			{
			ErrorMessageHandler::Show(e, std::string("SoundLib::GetWaveSize"));
			return (0);
			}
		}
	else
	{
		File file;
		try
		{
			// Need to load in sound from 'Sounds' directory
			file.Open(std::string(BuildFsp(wave,"wav",SOUNDS_DIR)));
		}
		catch(File::FileException& e)
		{
			ErrorMessageHandler::Show(e, std::string("SoundLib::GetWaveSize"));
			return(0);
		}
		
		return(file.GetSize());
		
	}
}


CachedSound *	SoundManager::EnsureInCache(DWORD wave)			//  Places wave in cache and returns
{

	// Is the sound already in the cache?
	BOOL found=FALSE;
	int i=0;
		
	std::set<CachedSound*>::iterator it;

	for(it = sounds.begin(); it!=sounds.end();it++)
	{

		if ((*it)->name==wave)
		{
			break;//=TRUE;		// we have found the sound at the correct volume
		}
	}

	if (it != sounds.end())
	{
		(*it)->used=last_used++;
		return(*it);
	}

	
	
	// Clear space to load sound (taking .wav header into account)

	if (MakeRoomInCache(GetWaveSize(wave))==NO_SOUND_ERROR)
	{

		CachedSound *new_wave=OpenSound(wave);

		if (new_wave!=NULL)
			{

			// Add to cache
			sounds.insert(new_wave);

			// Increase 
			current_size+=new_wave->size;
			}
		else
			{
			// "Sound not found: %s\n",BuildFsp(wave,"wav")
			}
		return(new_wave);
	}
	else
	{
		
		// "Cache Full: Couldn't play %s\n",BuildFsp(wave,"wav"));
		return(NULL);
	}
}

SOUNDERROR SoundManager::PreLoadSound(DWORD wave)	//  Ensures a sound is loaded into the
																	//  cache
{
	if (mixer_suspended || !sound_initialised)
	{
		return(SOUND_MIXER_SUSPENDED);
	}
	
	CachedSound *w=EnsureInCache(wave);

	if (w==NULL)
	{
		return(SOUND_NOT_FOUND);
	}
	else
	{
		return(NO_SOUND_ERROR);
	}
}

SOUNDERROR SoundManager::PlaySoundEffect(DWORD wave, int ticks, long volume, long pan)		//  Loads in sound or locates within
										//  Sound cache and plays it.
										//  No further control
{
	if (mixer_suspended || !sound_initialised)
	{
		return(SOUND_MIXER_SUSPENDED);
	}

	if (ticks)
	{
		// Queue sound to be played later
		
		SOUNDERROR err;
		err=PreLoadSound(wave);
		if (err==NO_SOUND_ERROR)
		{

			SoundQueueItem *add=new SoundQueueItem;

			add->wave=wave;
			add->ticks=ticks;
			add->volume=volume;
			add->pan=pan;
		
			sound_queue.push_back(add);
		}

		return(err);
	}

	// Place in cache and play now

	CachedSound *w=EnsureInCache(wave);
	if (w!=NULL)
	{
		SOUNDHANDLE handle=PlayCachedSound(w,volume,pan);
		if (handle==-1)
		{
			return(SOUND_NOT_FOUND);
		}
		else
		{
			return(NO_SOUND_ERROR);
		}
		return(NO_SOUND_ERROR);
	}
	else
	{
		return(SOUND_NOT_FOUND);
	}
}

SOUNDERROR SoundManager::StartControlledSound(DWORD wave, SOUNDHANDLE &handle, long volume, long pan, BOOL looped)
{
	if (mixer_suspended || !sound_initialised)
	{
		// --->SOUND_MIXER_SUSPENDED
		return(SOUND_MIXER_SUSPENDED);
	}
	
	CachedSound *w=EnsureInCache(wave);
	if (w!=NULL)
	{
		handle=PlayCachedSound(w,volume,pan,looped);
		if (handle==-1)
		{
			// Too many sounds playing: Couldn't play %s\n",BuildFsp(wave,"wav"))
			return(SOUND_NOT_FOUND);
		}
		else
		{
			// Flag as locked (don't delete until instructed)
			active_sounds[handle].locked=TRUE;
			return(NO_SOUND_ERROR);
		}
	}
	else
	{
		return(SOUND_NOT_FOUND);
	}

}

SOUNDERROR SoundManager::UpdateControlledSound(SOUNDHANDLE handle, long volume, long pan)
{
	if (mixer_suspended)
	{
		return(SOUND_MIXER_SUSPENDED);
	}

	if (handle<0 || handle>=MAX_ACTIVE_SOUNDS)
	{
		return(SOUND_HANDLE_UNDEFINED);
	}

	// Store the basic volume of the sound
	active_sounds[handle].volume = volume;


	// Calculate the volume of the buffer, taking
	// into account the overall volume of the manager
	volume += current_volume;

	// Can't get quieter than silence
	if (volume < SoundMinVolume)
		{
		volume = SoundMinVolume;
		}

	active_sounds[handle].pSample->SetVolume(volume);
	if (volume > 0 || volume < SoundMinVolume)
		{
		// Volume out of range %d\n",volume
		}
	active_sounds[handle].pSample->SetPan(pan);

	return(NO_SOUND_ERROR);
}

BOOL SoundManager::FinishedControlledSound(SOUNDHANDLE handle)
{
	if (mixer_suspended)
	{
		return(TRUE);
	}

	if (handle<0 || handle>=MAX_ACTIVE_SOUNDS)
	{
		return(TRUE);
	}

	if (active_sounds[handle].wID)
	{
		DWORD status;
		IDirectSoundBuffer *pDSBuffer=active_sounds[handle].pSample;
		pDSBuffer->GetStatus(&status);
		if (status&DSBSTATUS_PLAYING)
		{
			return(FALSE);
		}

	}

	return(TRUE);
}

SOUNDERROR SoundManager::StopControlledSound(SOUNDHANDLE handle, BOOL fade)
{

	if (mixer_suspended)
	{
		return(SOUND_MIXER_SUSPENDED);
	}

	if (handle<0 || handle>=MAX_ACTIVE_SOUNDS)
	{
		return(SOUND_HANDLE_UNDEFINED);
	}

	active_sounds[handle].locked=FALSE;

	if (fade)
	{
		// fade out in 15 ticks
		active_sounds[handle].fade_rate=(SoundMinVolume-active_sounds[handle].volume)/15;

		if (active_sounds[handle].fade_rate >= 0)
			{
			// No point in fading, as the sound is already silent
			// Zero fade rate - sound stopped
			StopSound(handle);
			}

	}
	else
	{
	 	StopSound(handle);
	}

	return(NO_SOUND_ERROR);
}

SOUNDERROR SoundManager::SetVolume(long volume)
	{
	if (mixer_suspended)
		{
		return(SOUND_MIXER_SUSPENDED);
		}

	overall_volume = volume;

	// If we're currently audible...
	if (!faded)
		{
		// ... Fade towards the new volume
		target_volume = overall_volume;
		}

	return(NO_SOUND_ERROR);

	}

SOUNDERROR SoundManager::SetTargetVolume(long volume)
	{
	if (mixer_suspended)
		{
		return(SOUND_MIXER_SUSPENDED);
		}

	target_volume = volume;

	return(NO_SOUND_ERROR);

	}

SOUNDERROR SoundManager::FadeOut()
	{
	if (mixer_suspended)
		{
		return(SOUND_MIXER_SUSPENDED);
		}

	// Aim for complete silence
	target_volume = SoundMinVolume;

	// Flag that we are fading out
	faded = true;

	return(NO_SOUND_ERROR);

	}

SOUNDERROR SoundManager::FadeIn()
	{
	if (mixer_suspended)
		{
		return(SOUND_MIXER_SUSPENDED);
		}

	// Aim for maximum volume
	target_volume = overall_volume;

	// Flag that we are fading in
	faded = false;

	return(NO_SOUND_ERROR);

	}


bool SoundManager::PlayMidiFile(std::string& fileName)
{
	if(!pDSObject)
		return false;

	static bool volumeSet = false;

	// choose different options for NT
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	::GetVersionEx(&info);

	if (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) 
	{
		

		if(!myMidiModule)
		{
			myMidiModule = new MidiModule(pDSObject,theApp.GetMainHWND());
		}
	
		if(myMidiModule)
			return myMidiModule->PlayMidiFile(fileName);
	}



	return false;
}

void SoundManager::StopMidiPlayer()
{
	if(myMidiModule)
	{
		myMidiModule->StopPlaying();
	}
}

void SoundManager::SetVolumeOnMidiPlayer(int32 volume)
{
	if(myMidiModule)
	{
		myMidiModule->SetVolume(volume);
	}
}

void SoundManager::MuteMidiPlayer(bool mute)
{
	if(myMidiModule)
	{
		myMidiModule->Mute(mute);
	}
}

bool SoundManager::Write(CreaturesArchive &archive) const
{
	return false;
}

bool SoundManager::Read(CreaturesArchive &archive)
{
	return false;
}