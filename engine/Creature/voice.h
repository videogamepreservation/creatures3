// disable annoying warning in VC when using stl (debug symbols > 255 chars)
#ifdef _MSC_VER
#pragma warning(disable: 4786 4503)
#endif

#ifndef VOICE_H
#define VOICE_H

#include "../PersistentObject.h"

#include <string>


#define SPEECH_SYMBOLS	27

class Voice : public PersistentObject {
	CREATURES_DECLARE_SERIAL(Voice)

private:
	DWORD mySets[3][SPEECH_SYMBOLS];
	DWORD mySounds[32];
	int32 myTimes[32];

	std::string myCurrentVoiceName;

	std::string myCurrentSentence;
	int32 myCurrentPosition;
	int32 myCurrentDelay;
	
	int32 GetOutput(int32 a, int32 b, int32 c);
	void GetSound(char a, char b, char c, DWORD &sound, int &time);

public:
	Voice();


	const std::string& GetCurrentVoice() const 
	{ 
		return myCurrentVoiceName; 
	}

	bool BeginSentence(std::string sentence);
	bool GetNextSyllable(DWORD &sound, int32 &length);
	int32 GetSentenceDelay();

	bool ReadData(uint8 genus, uint8 gender, uint8 age);
	bool ReadData(std::string& thisVoice);
	// ----------------------------------------------------------------------
	// Method:		Write
	// Arguments:	archive - archive being written to
	// Returns:		true if successful
	// Description:	Overridable function - writes details to archive,
	//				taking serialisation into account
	// ----------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;


	// ----------------------------------------------------------------------
	// Method:		Read
	// Arguments:	archive - archive being read from
	// Returns:		true if successful
	// Description:	Overridable function - reads detail of class from archive
	// ----------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);
};
#endif