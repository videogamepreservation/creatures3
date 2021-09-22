#ifdef _MSC_VER
#pragma warning(disable: 4786 4503)
#endif

#include "voice.h"
#include "../File.h"
#include "../C2eServices.h"

#include <string>
#ifndef C2E_OLD_CPP_LIB
#include <sstream>
#include <ios>
#endif // C2E_OLD_CPP_LIB

CREATURES_IMPLEMENT_SERIAL(Voice)

Voice::Voice()
{
	myCurrentSentence="";
	myCurrentVoiceName = "";
	myCurrentPosition=1;
	myCurrentDelay=0;

	int i,j;	
	for ( i=0;i<3;i++) {
		for( j=0;j<SPEECH_SYMBOLS;j++) {
			mySets[i][j]=0;
		}
	}

	for (i=0;i<32;i++) {
		mySounds[i]=0;
		myTimes[i]=1;
	}
}

int32 Voice::GetOutput(int32 a, int32 b, int32 c)
{
	DWORD set=mySets[0][a] & mySets[1][b] & mySets[2][c];

	// calculate mask
	
	//  0 -  3	Silences
	//  4 -  6	Sentence starts
	//  7 - 10	Word starts
	// 11 - 13	Sentence ends
	// 14 - 17	Word ends
	// 18 - 31	Mid word syllables

	if (b==26) {
		// Middle is a space - this MUST be a silence
		set &= 0xf;
	}

	if (a==26) {
		// word start
		set &= 0x000007f0;
	}

	if (c==26) {
		// word end
		set &= 0x0003f800;
	}

	if (a<26 && b<26 && c<26) {
		// Mid word
		set &= 0xfffc0000;
	}

	if (set==0) {
		// flag the default
		return (32);
	}

	int positions[32], total=0;
	for (int count=0;count<32;count++) {
		if (set%2) {
			positions[total]=count;
			total++;
		}
		set>>=1;
	}
	int32 output=positions[(a+b+c)%total];
	return(output);
}

void Voice::GetSound(char a, char b, char c, DWORD &sound, int &time)
{
	char group[3];

	group[0]=a;
	group[1]=b;
	group[2]=c;

	for (int i=0;i<3;i++) {
		switch(group[i]) {
			case '.':
			case ' ':
				// No longer distinguish between sentences starts and spaces
				group[i]=26;
				break;
			default:	// letter
				group[i]-='a';
				break;
		}
	}

	int output=GetOutput(group[0],group[1],group[2]);

	if (output>=32 || output<0) {
		// pick a default

 		if (group[1]==26) {
			// Space - some type of silence

			output=(group[0]+group[1]+group[2])%4;
		} else {
			if (group[0]==26) {
				// word start 

				output=4+(group[0]+group[1]+group[2])%7;
			} else {
				if (group[2]==26) {
					// word end
					output=10+(group[0]+group[1]+group[2])%7;
				} else {
					// Mid word
					output=18+(group[0]+group[1]+group[2])%14;
				}
			}
		}

	}
	sound=mySounds[output];
	time=myTimes[output];
}


// C2E NOTE, TODO: Should sort out the whole Unicode/non-Unicode,
// TCHAR/char stuff properly some time...
// For now, just get it compiling!
bool Voice::BeginSentence(std::string sentence)
{
	if (myCurrentVoiceName == "")
		return false;


	int i;
	std::string text( sentence );

	for( i=0; i<text.length(); ++i )
		tolower( text[i] );

	// strip out anything which is not a letter or space
	// and place '.' at each end of the sentence
	std::string stripped=".";

//	TCHAR read, last_written=0;
	char read, last_written=0;
	int cInt=0;
	
	for (i=0; i<text.length(); i++) {
		read=text[i];

		if ((read==' ' && last_written!=0 && last_written!=' ')
			|| (read>='a' && read<='z'))
		{
			stripped+=read;
			last_written=read;
		}

	}

	stripped+=".";

	// Set up data to be read by GetNextSyllable()
	myCurrentSentence=stripped;
	myCurrentPosition=1;
	myCurrentDelay=0;


	// Is there anything to play?
	return (stripped.length()>2);
}


bool Voice::GetNextSyllable(uint32 &sound, int32 &delay)
{
	sound=0;
	delay=0;

	do
	{
	
		if (myCurrentPosition>=myCurrentSentence.length()-1)
		{
			return(FALSE);
		}

		int time=0;
		GetSound(myCurrentSentence[myCurrentPosition-1],
				 myCurrentSentence[myCurrentPosition],
				 myCurrentSentence[myCurrentPosition+1],
				 sound,time);


		delay=myCurrentDelay;
		myCurrentDelay+=time;

		myCurrentPosition++;

		if (myCurrentPosition<myCurrentSentence.length()-1)
		{

			char a=myCurrentSentence[myCurrentPosition-1],
				 b=myCurrentSentence[myCurrentPosition],
				 c=myCurrentSentence[myCurrentPosition+1];

			if (a>='a' && a<='z' && b>='a' && b<='z'
				&& c>='a' && c<='z')
			{
				myCurrentPosition++;
			}
		}

	} while (sound==0);

	return(TRUE);

}

int32 Voice::GetSentenceDelay()
{
	return(myCurrentDelay);
}

bool Voice::ReadData(std::string& thisVoice)
{

	// Read the voice in, failing if need be :)
	if (!theCatalogue.TagPresent( thisVoice ))
		return false;
	std::string theLanguage = theCatalogue.Get( thisVoice, 0 );
	if (!theCatalogue.TagPresent( theLanguage ))
		return false;

	// We have got it :)

	// Read the language in first...
	int i,j;
	for(i=0;i<3;i++)
		for(j=0;j<SPEECH_SYMBOLS;j++)
		{
			std::string langnum = theCatalogue.Get( theLanguage, (i * SPEECH_SYMBOLS) + j );
#ifndef C2E_OLD_CPP_LIB
			std::stringstream input(langnum);
			input >> std::hex >> mySets[i][j];
#else
			sscanf( langnum.c_str(), "%x", &mySets[i][j] );
#endif
		}
	// Now read the phoneme&gap pairs. All 32 of them :):)

	for(i=0;i<32;i++)
	{
		mySounds[i] = *((DWORD*)theCatalogue.Get( thisVoice, 1 + (i*2) ));
		std::string time = theCatalogue.Get( thisVoice, 2 + (i*2) );
#ifndef C2E_OLD_CPP_LIB
		std::stringstream inputt(time);
		inputt >> myTimes[i];
#else
		sscanf( time.c_str(), "%d", &myTimes[i] );
#endif
	}

	myCurrentVoiceName = thisVoice;
	return true;
}

bool Voice::ReadData(uint8 genus, uint8 gender, uint8 age)
{
	// Descent method is as follows...
	// Drop through age
	// Switch Gender
	// Drop through genus
	// Then go to "DefaultVoice"

	std::string GenusNames[4] = { "Norn", "Grendel", "Ettin", "Geat" };
	std::string GenderNames[2] = { "Male", "Female" };
	std::string AgeNames[8] = { "Embryo", "Baby", "Child", "Adolescent", "Youth", "Adult", "Old", "Senile" };

	// All we have to do here is...
	// Cascade... :):):)

	if (gender == 2)
	{
		// Invert Gender Names
		GenderNames[0] = "Female";
		GenderNames[1] = "Male";
	}

	for(int thisGenus = genus; thisGenus > 0; thisGenus--)
	{
		std::string voiceName = GenusNames[thisGenus-1] + " ";
		for(int thisGender = 0; thisGender < 2; thisGender++)
		{
			std::string voiceNameGender = voiceName + GenderNames[thisGender] + " ";
			for(int thisAge = age; thisAge > -1; thisAge--)
			{
				std::string voiceNameGenderAge = voiceNameGender + AgeNames[thisAge];
				if (ReadData(voiceNameGenderAge))
					return true;
			}
		}
	}
	
	// Hmm, they all failed, so let's read the default voice in...
	std::string defVoice = "DefaultVoice";
	return ReadData(defVoice);	
}
/*
void Voice::ReadData(LPCTSTR name)
{
	File file(name);
	
	for (int i=0;i<32;i++) {
		file.Read(&mySounds[i],sizeof(DWORD));

		// This is a kludge to combat an error that has crept into
		// the voice files - all voices seem to contain a 1 instead of
		// a 0 for "silence"
		if (i<4)
			mySounds[i] = 0;
		file.Read(&myTimes[i],sizeof(int));
	}

	for (i=0; i<3; i++) {
		for (int j=0; j<27; j++) {
			file.Read(&mySets[i][j],sizeof(DWORD));
		}
	}
}
*/


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool Voice::Write(CreaturesArchive &archive) const {

	int i,j;
	for (i=0;i<3;i++) {
		for (j=0;j<SPEECH_SYMBOLS;j++) {
			archive << mySets[i][j];
		}
	}

	for (i=0;i<32;i++) {
		archive << mySounds[i];
		archive << myTimes[i];
	}
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Voice::Read(CreaturesArchive &archive)
	{

	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		int i,j;

		for (i=0;i<3;i++) {
			for (j=0;j<SPEECH_SYMBOLS;j++) {
				archive >> mySets[i][j];
			}
		}

		for (i=0;i<32;i++) {
			archive >> mySounds[i];

			// This is a kludge to combat an error that has crept into
			// the voice files - all voices seem to contain a 1 instead of
			// a 0 for "silence"
			if (i<4)
				mySounds[i] = 0;
			archive >> myTimes[i];
		}
	} 
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}
