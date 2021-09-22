// LinguisticFaculty.h: interface for the LinguisticFaculty class.
//
//////////////////////////////////////////////////////////////////////

#ifndef LinguisticFaculty_H
#define LinguisticFaculty_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <deque>
#include "Definitions.h"
#include "Faculty.h"
#include <string>
#include <vector>
#include "voice.h"
#include "Vocab.h"
#include "Creature.h"
#include "MotorFaculty.h"
#include "Brain/BrainScriptFunctions.h"


class Agent;

class Creature;

class LinguisticFaculty : public Faculty {
	CREATURES_DECLARE_SERIAL(LinguisticFaculty)
public:
	typedef Faculty base;

	enum {
		VERB,
		NOUN,
		DRIVE,
		SPECIAL,
		QUALIFIER,
		PERSONAL,
		NICEDRIVE,
		CREATURENAME,	// SPECIAL NOT STORED IN OWN VOCAB(keep this as the last)
		noOfTypes
	};
	enum Specials {
		YES, NO, LOOK, WHAT, HAPPY, LIKE, DISLIKE, LOVE, HATE, MAYBE, DEFINATELY, ILL
	};
	enum Personals {
		ME
	};

	enum {NONE = -1};

	typedef struct 
	{
		std::string sentence;
		int verb;
		int noun;
		bool shout;
		int delay;
	} SentenceToSpeak;

	enum { LEARNING_DELAY = 1, NORMAL_DELAY_OFFSET=4, INTERMEDIATE_DELAY_OFFSET=6, REPLY_TO_CREATURE_DELAY_OFFSET=9};



	// ---------------------------------------------------------------------------
	// Function:	LinguisticFaculty
	// Description:	constructor
	// Arguments:	None
	// Returns:		None
	// ---------------------------------------------------------------------------
	LinguisticFaculty();

	// ---------------------------------------------------------------------------
	// Function:	LinguisticFaculty
	// Description:	desstructor
	// Arguments:	None
	// Returns:		None
	// ---------------------------------------------------------------------------
	virtual ~LinguisticFaculty();

	// ---------------------------------------------------------------------------
	// Function:	Init
	// Description:	Initialised vocab with baby words
	// Arguments:	Creature faculty belongs to
	// Returns:		None
	// ---------------------------------------------------------------------------
	void Init(AgentHandle c);				// initialise vocabulary (and voice)



	// ---------------------------------------------------------------------------
	// Function:	SetWord
	// Description:	Sets the word the creature is to know, does not know how to say 
	//				it perfectly unles perfect == true
	// Arguments:	type of word, word id, the word to learn, whether it knows it
	//				perfectly from the start
	// Returns:		None
	// ---------------------------------------------------------------------------
	void SetWord(int type, int id, std::string s, bool perfect);

	// ---------------------------------------------------------------------------
	// Function:	GetWord
	// Description:	returns the word - how the creature says it
	// Arguments:	word type and id
	// Returns:		word how the crature says it
	// ---------------------------------------------------------------------------
	std::string GetWord(int type, int id) const;

	// ---------------------------------------------------------------------------
	// Function:	GetPlatonicWord
	// Description:	returns word how the creature should say it
	// Arguments:	word type and id
	// Returns:		word in question
	// ---------------------------------------------------------------------------
	std::string GetPlatonicWord(int type, int id) const;

	// ---------------------------------------------------------------------------
	// Function:	GetWordStrength
	// Description:	returns the word strength
	// Arguments:	word type and id
	// Returns:		float strength
	// ---------------------------------------------------------------------------
	float GetWordStrength(int type, int id) const;

	// ---------------------------------------------------------------------------
	// Function:	ClearWordStrength
	// Description:	blanks the word strength so others can be learned
	// Arguments:	word type and id
	// Returns:		
	// ---------------------------------------------------------------------------
	void ClearWordStrength(int type, int id);


	// ---------------------------------------------------------------------------
	// Function:	KnownWord
	// Description:	returns whether the word is known by the creature even if it
	//				cant say it properly
	// Arguments:	wordtype and id
	// Returns:		whether the word is known
	// ---------------------------------------------------------------------------
	bool KnownWord(int type, int id) const;

	// ---------------------------------------------------------------------------
	// Function:	LearnVocab
	// Description:	learns all words properly
	// Arguments:	None
	// Returns:		None
	// ---------------------------------------------------------------------------
	void LearnVocab();					



	// ---------------------------------------------------------------------------
	// Function:	Update
	// Description:	Processes sentences qued to be spoken or ques a sentence
	//				either what it is doing, its need, or opinion of others
	// Arguments:	None
	// Returns:		None
	// ---------------------------------------------------------------------------
	virtual void Update();

	// ---------------------------------------------------------------------------
	// Function:	SayWhatDoing
	// Description:	ques a sentence stating what the creature is doing
	// Arguments:	delay to speaking
	// Returns:		None
	// ---------------------------------------------------------------------------
	void SayWhatDoing(int delay = NORMAL_DELAY_OFFSET);	
	
	// ---------------------------------------------------------------------------
	// Function:	ExpressNeed
	// Description:	ques sentence about a drive state
	// Arguments:	drive to talk about
	// Returns:		None
	// ---------------------------------------------------------------------------
	void ExpressNeed(int requestedDriveToExpress=NONE, int delay = NORMAL_DELAY_OFFSET);

	// ---------------------------------------------------------------------------
	// Function:	ExpressOpinion
	// Description:	ques sentence about creatures opinion about a another creature
	//				or the pointer
	// Arguments:	creature or pointer to talk about
	// Returns:		None
	// ---------------------------------------------------------------------------
	void ExpressOpinion(AgentHandle creatureOrPointer, int delay = NORMAL_DELAY_OFFSET);


	
	// ---------------------------------------------------------------------------
	// Function:	StackSentence
	// Description:	ques sentence 
	// Arguments:	sentence to que, whether others will hear, verb and noun, delay  
	//				mentioned
	// Returns:		None
	// ---------------------------------------------------------------------------
	void StackSentence(std::string sentence, bool shout = false, int delay = NORMAL_DELAY_OFFSET, int verb = NONE, int noun = NONE);

	// ---------------------------------------------------------------------------
	// Function:	Say
	// Description:	says a sentence
	// Arguments:	sentence to say
	// Returns:		None
	// ---------------------------------------------------------------------------
	void Say(std::string str);		

	// ---------------------------------------------------------------------------
	// Function:	Shout
	// Description:	lets other creatures hear a sentence
	// Arguments:	sentence to say, noun and verb spoke so others can hear
	// Returns:		None
	// ---------------------------------------------------------------------------
	void Shout(std::string, int verb = NONE, int noun = NONE);

	// ---------------------------------------------------------------------------
	// Function:	HearSentence
	// Description:	hears a sentences shouted
	// Arguments:	sspeaker, sentence, verb and noun to learn
	// Returns:		None
	// ---------------------------------------------------------------------------
	void HearSentence(AgentHandle speakerAgent, std::string sentence, int learnVerb, 
		int learnNoun);

	// ---------------------------------------------------------------------------
	// Function:	ParseSentence
	// Description:	set syntax of sentence
	// Arguments:	sspeaker, sentence, verb and noun to learn
	// Returns:		None
	// ---------------------------------------------------------------------------
	bool ParseSentence(AgentHandle speakerAgent, std::string stentenceSpoken, 
		std::string &syntax, int wordsHeard[noOfTypes], int learnVerb, int learnNoun);

	// ---------------------------------------------------------------------------
	// Function:	GetWordType
	// Description:	matches first part of sentence agains a word the creature can 
	//				say	or a word it is learning to say or another creatres name
	// Arguments:	sentence to look in, fills in - whether
	//				the word was understood type and id of word and the size
	//				of the matched word
	// Returns:		If match was found
	// ---------------------------------------------------------------------------
	bool GetWordType(std::string wordToMatch, bool &wordUnderstood, 
		int &wordType, int &wordHeard, int &wordSize);
	

	// ---------------------------------------------------------------------------
	// Function:	ParseAsLearnSentence
	// Description:	command to learn word
	// Arguments:	sentence string
	// Returns:		whether it was a sentence of this kind
	// ---------------------------------------------------------------------------	
	bool ParseAsLearnSentence( std::string const &str );



	// ---------------------------------------------------------------------------
	// Function:	TranslateToBase
	// Description:	converts a localised sytax string in a base sytax string 
	//				understood by the engine
	// Arguments:	sytnax string
	// Returns:		translated syntax string
	// ---------------------------------------------------------------------------	
	std::string LinguisticFaculty::TranslateToBase(std::string syntax);

	// ---------------------------------------------------------------------------
	// Function:	TranslateToLocal
	// Description:	converts a base sytax string in a localised sytax string 
	// Arguments:	syntax string
	// Returns:		translated syntax string
	// ---------------------------------------------------------------------------	
	std::string LinguisticFaculty::TranslateToLocal(std::string syntax);

	// ---------------------------------------------------------------------------
	// Function:	Write
	// Description:	class file storage
	// Arguments:	arhive
	// Returns:		None
	// ---------------------------------------------------------------------------
	virtual bool Write(CreaturesArchive &archive) const;

	// ---------------------------------------------------------------------------
	// Function:	Read
	// Description:	class file storage
	// Arguments:	arhive
	// Returns:		None
	// ---------------------------------------------------------------------------
	virtual bool Read(CreaturesArchive &archive);


protected:
	std::string GetQualifier(float level);

	bool myVoiceFileHasBeenInitialised;
	int myLifeStageForMyCurrentVoice;
	// Voice myVoice;

	std::vector< std::vector<Vocab> > myVocab;
	std::deque<SentenceToSpeak> mySentenceStack;
	int myStackCount;

};




#endif//LinguisticFaculty_H
