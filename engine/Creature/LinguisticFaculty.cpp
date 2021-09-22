////////////////////////////////////////////////////////////////////////////////
// Filename:	LinguisticFaculty.cpp
// Class:		LinguisticFaculty
//
// Description: Handles speach
//
// Author:		Adapted C2 code by David Bhowmik
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "LinguisticFaculty.h"
#include "../App.h"
#include "../Stimulus.h"
#include "Creature.h"
#include "SensoryFaculty.h"
#include "MotorFaculty.h"
#include "../World.h"
#include "Brain/Brain.h"
#include "LifeFaculty.h"
#include "Biochemistry/BiochemistryConstants.h"
#include "../Display/ErrorMessageHandler.h"


CREATURES_IMPLEMENT_SERIAL(LinguisticFaculty)


// default attention nudges for heard words
const float defaultNounNudge = 0.7f;
const float defaultVerbNudge = 0.7f;




// ---------------------------------------------------------------------------
// Function:	LinguisticFaculty
// Description:	constructor
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       LinguisticFaculty
// Description: 
// ------------------------------------------------------------------------
LinguisticFaculty::LinguisticFaculty()
{
	myVocab.resize(noOfTypes-1);	// do not include other creatures names
	myVocab[VERB].resize(NUMACTIONS);
	myVocab[NOUN].resize(SensoryFaculty::GetNumCategories());
	myVocab[DRIVE].resize(NUMDRIVES);
	myVocab[SPECIAL].resize(12);
	myVocab[QUALIFIER].resize(5);
	myVocab[PERSONAL].resize(1);
	myVocab[NICEDRIVE].resize(NUMDRIVES);

	myVoiceFileHasBeenInitialised = false;
	myStackCount = 0;
}



// ---------------------------------------------------------------------------
// Function:	LinguisticFaculty
// Description:	desstructor
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       LinguisticFaculty
// Description: 
// ------------------------------------------------------------------------
LinguisticFaculty::~LinguisticFaculty()
{

}



// ---------------------------------------------------------------------------
// Function:	Init
// Description:	Initialised vocab with baby words
// Arguments:	Creature faculty belongs to
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Init
// Class:       LinguisticFaculty
// Description: 
// Arguments:   AgentHandle a = 
// ------------------------------------------------------------------------
void LinguisticFaculty::Init(AgentHandle a) 
{
	myCreature = a;
	Creature& c = myCreature.GetCreatureReference();

	// load default vocabulary randomly from the appropriate catalogue file
	// (choice depends on type of creature):
	int genus = c.GetClassifier().Genus();
	std::string vocabCatalogueString =
		genus==1 ? "Default Norn Speak" :
		genus==2 ? "Default Grendel Speak" :
		genus==3 ? "Default Ettin Speak" :
		genus==4 ? "Default Geat Speak" : 
		"Default Speak";

	// std::string voiceFile =???

	const int NUM_WORD_VARIANTS = 4;
	for (int i=0; i<noOfTypes-1; i++) 
	{
		for (int o=0; o<myVocab[i].size(); o++) 
		{
			myVocab[i][o].InitBabyWord(
				theCatalogue.Get(vocabCatalogueString, (noOfTypes-1)*Rnd(0,NUM_WORD_VARIANTS-1) + i));
		}
	}
}



// ---------------------------------------------------------------------------
// Function:	SetWord
// Description:	Sets the word the creature is to know, does not know how to say 
//				it perfectly unles perfect == true
// Arguments:	type of word, word id, the word to learn, whether it knows it
//				perfectly from the start
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    SetWord
// Class:       LinguisticFaculty
// Description: 
// Arguments:   int type = 
//              int id = 
//              std::string s = 
//              bool perfect = 
// ------------------------------------------------------------------------
void LinguisticFaculty::SetWord(int type, int id, std::string s, bool perfect) 
{
	std::transform( s.begin(), s.end(), s.begin(), tolower );	
	myVocab[type][id].InitWord(s, myCreature.GetCreatureReference().Life()->GetAge(), perfect);
}



// ---------------------------------------------------------------------------
// Function:	GetWord
// Description:	returns the word - how the creature says it
// Arguments:	word type and id
// Returns:		word how the crature says it
// ---------------------------------------------------------------------------
std::string LinguisticFaculty::GetWord(int type, int id) const 
{
	return myVocab[type][id].outWord;
}



// ---------------------------------------------------------------------------
// Function:	GetWordStrength
// Description:	returns the word strength
// Arguments:	word type and id
// Returns:		float strength
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    GetWordStrength
// Class:       LinguisticFaculty
// Description: 
// Arguments:   int type = 
//              int id = 
// Returns:     float = 
// ------------------------------------------------------------------------
float LinguisticFaculty::GetWordStrength(int type, int id) const 
{
	return myVocab[type][id].learnedStrength;
}


// ---------------------------------------------------------------------------
// Function:	ClearWordStrength
// Description:	blanks the word strength so others can be learned
// Arguments:	word type and id
// Returns:		
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    ClearWordStrength
// Class:       LinguisticFaculty
// Description: 
// Arguments:   int type = 
//              int id = 
// ------------------------------------------------------------------------
void LinguisticFaculty::ClearWordStrength(int type, int id)  
{
	myVocab[type][id].learnedStrength = 0.0f;
}

// ---------------------------------------------------------------------------
// Function:	GetPlatonicWord
// Description:	returns word how the creature should say it
// Arguments:	word type and id
// Returns:		word in question
// ---------------------------------------------------------------------------
std::string LinguisticFaculty::GetPlatonicWord(int type, int id) const 
{
	return myVocab[type][id].platonicWord;
}



// ---------------------------------------------------------------------------
// Function:	KnownWord
// Description:	returns whether the word is known by the creature even if it
//				cant say it properly
// Arguments:	wordtype and id
// Returns:		whether the word is known
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    KnownWord
// Class:       LinguisticFaculty
// Description: 
// Arguments:   int type = 
//              int id = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool LinguisticFaculty::KnownWord(int type, int id) const 
{
	// strength must be greater than 0 else uninitialised baby word
	// or unlearnt word that should be dumped

	return myVocab[type][id].learnedStrength > 0.0f;
}



// ---------------------------------------------------------------------------
// Function:	LearnVocab
// Description:	learns all words properly
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    LearnVocab
// Class:       LinguisticFaculty
// Description: 
// ------------------------------------------------------------------------
void LinguisticFaculty::LearnVocab() 
{
	int i;
	int pointerCat = SensoryFaculty::GetCategoryIdOfClassifier(&thePointer.GetPointerAgentReference().GetClassifier());

    for (i=0; i<myVocab[VERB].size(); i++) 
	{
		// default action word
		std::string word = theCatalogue.Get("Creature Actions", i);
		std::transform( word.begin(), word.end(), word.begin(), tolower );
		myVocab[VERB][i].InitWord(word, 
			myCreature.GetCreatureReference().Life()->GetAge(), true);
    }
    for (i=0; i<myVocab[NOUN].size(); i++) 
	{	// default object name (if nameing pointer make sure not already named
		if(i == pointerCat)
		{
			std::string word = thePointer.GetPointerAgentReference().GetName();
			std::transform( word.begin(), word.end(), word.begin(), tolower );
			myVocab[NOUN][i].InitWord(word, 
				myCreature.GetCreatureReference().Life()->GetAge(), true);
		}
		else
		{
			std::string word = SensoryFaculty::GetCategoryName(i);
			std::transform( word.begin(), word.end(), word.begin(), tolower );
			myVocab[NOUN][i].InitWord(word, 
				myCreature.GetCreatureReference().Life()->GetAge(), true);
		}
    }
    for (i=0; i<myVocab[DRIVE].size(); i++)
	{		// default drive name
		std::string word = theCatalogue.Get("Creature Drives", i);
		std::transform( word.begin(), word.end(), word.begin(), tolower );	
		myVocab[DRIVE][i].InitWord(word, 
			myCreature.GetCreatureReference().Life()->GetAge(), true);
    }
	for (i=0; i<myVocab[SPECIAL].size(); i++) 
	{
		// default specials
		std::string word = theCatalogue.Get("Learnt Specials", i);
		std::transform( word.begin(), word.end(), word.begin(), tolower );	
		myVocab[SPECIAL][i].InitWord(word,
			myCreature.GetCreatureReference().Life()->GetAge(), true);
    }
	for (i=0; i<myVocab[QUALIFIER].size(); i++)
	{
		// default qualifiers
		std::string word = theCatalogue.Get("Learnt Qualifiers", i);
		std::transform( word.begin(), word.end(), word.begin(), tolower );	
		myVocab[QUALIFIER][i].InitWord(word,
			myCreature.GetCreatureReference().Life()->GetAge(), true);
    }
    for (i=0; i<myVocab[PERSONAL].size(); i++) 
	{
		// default personal name (dont if already have one)
		if(myVocab[PERSONAL][i].learnedStrength == 0.0f)
		{
			std::string word = theCatalogue.Get("Learnt Personals", i);
			std::transform( word.begin(), word.end(), word.begin(), tolower );	
			myVocab[PERSONAL][i].InitWord(word, 
				myCreature.GetCreatureReference().Life()->GetAge(), true);
		}
    }
	for (i=0; i<myVocab[NICEDRIVE].size(); i++)
	{		
		// default drive name
		std::string word = theCatalogue.Get("Learnt Nice Drives", i);
		std::transform( word.begin(), word.end(), word.begin(), tolower );	
		myVocab[NICEDRIVE][i].InitWord(word, 
			myCreature.GetCreatureReference().Life()->GetAge(), true);
    }
}



// ---------------------------------------------------------------------------
// Function:	Update
// Description:	Processes sentences qued to be spoken or ques a sentence
//				either what it is doing, its need, or opinion of others
// Arguments:	None
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Update
// Class:       LinguisticFaculty
// Description: 
// ------------------------------------------------------------------------
void LinguisticFaculty::Update() 
{
	Creature& c = myCreature.GetCreatureReference();

	if (c.Life()->GetWhetherAlert() || c.Life()->GetWhetherZombie()) 
	{
		if(mySentenceStack.size() !=0)
		{
			// speak qued sentence
			if(myStackCount-- == 0)	// have time delay betwen sentences
			{
				Say(mySentenceStack.front().sentence);
				if(mySentenceStack.front().shout)
					Shout(mySentenceStack.front().sentence,
					mySentenceStack.front().verb, 
					mySentenceStack.front().noun);

					mySentenceStack.pop_front();

					if(mySentenceStack.size() != 0)
						myStackCount = mySentenceStack.front().delay; // set delay for next sentence
				
			}
		}
		else if (!Rnd(120)) 
		{
			// que a new sentence
			float odds = RndFloat();
			AgentHandle nearest = c.Sensory()->GetNearestCreatureOrPointer();

			if((odds > 0.7f && nearest.IsValid()) || (odds > 0.5f && nearest.IsInvalid())) 
				SayWhatDoing(INTERMEDIATE_DELAY_OFFSET);
			else if((odds > 0.4f && nearest.IsValid()) || nearest.IsInvalid())
				ExpressNeed();
			else
				ExpressOpinion(nearest);
		}
	}
}



// ---------------------------------------------------------------------------
// Function:	SayWhatDoing
// Description:	ques a sentence stating what the creature is doing
// Arguments:	delay to speaking
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    SayWhatDoing
// Class:       LinguisticFaculty
// Description: 
// Arguments:   int delay = 
// ------------------------------------------------------------------------
void LinguisticFaculty::SayWhatDoing(int delay) {
	Creature& c = myCreature.GetCreatureReference();

	int decisionId = c.Motor()->GetCurrentDecisionId();
	if (decisionId<0 || decisionId>=myVocab[VERB].size())
		return;	// nothing to say
    

	int attentionId = NONE;

	// get outpu words & order words for local dialect

	std::string replySyntax = TranslateToLocal("pvn");
	std::string whatToSay;

	for(int w = 0; w != replySyntax.length(); w++)
	{
		const char *replyWord = replySyntax.c_str()+w;
		switch(*replyWord)
		{
		case 'v':
			// add verb
			whatToSay += myVocab[VERB][decisionId].outWord + " ";
			break;
		case 'n':
			// add noun	
			if (DoesThisScriptRequireAnItObject(decisionId)) 
			{
				attentionId = c.Motor()->GetCurrentAttentionId();
				if (attentionId>=0 || attentionId<myVocab[NOUN].size())
					whatToSay += myVocab[NOUN][attentionId].outWord + " ";
			}
			break;
		case 'p':
			// add own name
			if  (!Rnd(2))
			{
				whatToSay = myVocab[PERSONAL][ME].outWord + " ";

				// say own name better
				if(myVocab[PERSONAL][ME].platonicWord != theCatalogue.Get("Learnt Personals", ME))
					myVocab[PERSONAL][ME].HearWord(myVocab[PERSONAL][ME].platonicWord);
			}
			break;
		}
	}

	// remove space at end of sentence
	whatToSay.resize(whatToSay.length()-1);

	// shout so other creatures will learn words
	StackSentence(whatToSay, true, NORMAL_DELAY_OFFSET, decisionId, attentionId);
}



// ---------------------------------------------------------------------------
// Function:	ExpressNeed
// Description:	ques sentence about a drive state
// Arguments:	drive to talk about, delay till speak
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    ExpressNeed
// Class:       LinguisticFaculty
// Description: 
// Arguments:   int requestedDriveToExpress = 
//              int delay = 
// ------------------------------------------------------------------------
void LinguisticFaculty::ExpressNeed(int requestedDriveToExpress, int delay)
{
	Creature& c = myCreature.GetCreatureReference();



	int driveToExpress;
	float driveLevelToExpress = -999.0f;


	float illnessLevel = 0.2f;
	for (int i=FIRST_ANTIGEN; i<=LAST_ANTIGEN; i++) 
	{
		float level = c.GetBiochemistry()->GetChemical(i);
		if (level > illnessLevel) 
			illnessLevel = level;
	}


	std::string drive;
	std::string qualifier;
	if(illnessLevel > 0.2f)
	{
		qualifier = GetQualifier((illnessLevel-0.2f)/0.8f);
		drive = myVocab[SPECIAL][ILL].outWord + " ";   // "adam ill"
	}
	else
	{
   		if(requestedDriveToExpress == NONE)
		{
			 // Establish most pressing need:
			
			for (int i=0; i!=NUMDRIVES; i++) 
			{
				float driveLevel = c.GetDriveLevel(i);
				if (driveLevel > driveLevelToExpress) 
				{
					driveLevelToExpress = driveLevel;
					driveToExpress = i;
				}
			}
			
		}
		else
		{
			 // get level of drive to talk about:
			driveToExpress = requestedDriveToExpress;
			driveLevelToExpress = c.GetDriveLevel(driveToExpress);
		}


		// Say how pressing this drive is:
		if (driveLevelToExpress<0.25f && requestedDriveToExpress==NONE) 
		{ 
			// Nothing particularly pressing:
			drive = myVocab[SPECIAL][HAPPY].outWord + " ";		// "adam happy"
		} 
		else if(driveLevelToExpress<0.25f) 
		{
			// say requested drive to talk about is ok
			qualifier = GetQualifier((0.25f-driveLevelToExpress)/0.25f);
			drive = myVocab[NICEDRIVE][driveToExpress].outWord + " ";		// "adam well"
		}
		else
		{
			// say bad thing about particular drive
			qualifier = GetQualifier((driveLevelToExpress-0.25f)/0.75f);			
			drive = myVocab[DRIVE][driveToExpress].outWord + " ";   // "adam ow"

			// injury???
		}
	}


	// order words for local dialect

	std::string replySyntax = TranslateToLocal("pqd");
	std::string whatToSay;
	for(int w = 0; w != replySyntax.length(); w++)
	{
		const char *replyWord = replySyntax.c_str()+w;
		switch(*replyWord)
		{
		case 'd':
			whatToSay += drive;
			break;
		case 'q':
			whatToSay += qualifier;	// spaces already added if used
			break;
		case 'p':
			// add own name
			whatToSay += myVocab[PERSONAL][ME].outWord + " ";
			// say own name better
			if(myVocab[PERSONAL][ME].platonicWord != theCatalogue.Get("Learnt Personals", ME))
				myVocab[PERSONAL][ME].HearWord(myVocab[PERSONAL][ME].platonicWord);
			break;
		}
	}

	// remove space at end of sentence
	whatToSay.resize(whatToSay.length()-1);

	// shout so other creatures can respond
	StackSentence(whatToSay, true, delay); 	
}



// ---------------------------------------------------------------------------
// Function:	ExpressOpinion
// Description:	ques sentence about creatures opinion about a another creature
//				or the pointer
// Arguments:	creature or pointer to talk about, dealy you wish this to be 
//				said after
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    ExpressOpinion
// Class:       LinguisticFaculty
// Description: 
// Arguments:   AgentHandle creatureOrPointer = 
//              int delay = 
// ------------------------------------------------------------------------
void LinguisticFaculty::ExpressOpinion(AgentHandle creatureOrPointer, int delay)
{
	if(creatureOrPointer.IsInvalid())
		return;

	Creature& c = myCreature.GetCreatureReference();


	float opinion;
	float moodOpinion;
	c.Sensory()->GetOpinionOfCreature(creatureOrPointer, opinion, moodOpinion);

	bool aboutGrendel = creatureOrPointer.IsCreature() && 
		creatureOrPointer.GetAgentReference().GetGenus()==2;
	


	// calculate opinion
	std::string opinionStr;
	
	if(opinion > 0.1f && (c.GetGenus()!=2 || 
		(c.GetGenus()==2 && aboutGrendel)))
	{
		// good opinion


		if(moodOpinion < 0.1f)	
		{
			// bad mood effecting behaviour say why
			if(c.GetDriveLevel(ANGER) > c.GetDriveLevel(FEAR))
				ExpressNeed(ANGER, delay);
			else
				ExpressNeed(FEAR, delay);
	
			return;
		}
		else
		{
			if(opinion > 0.7f  && c.GetGenus()!=2)
				opinionStr = myVocab[SPECIAL][LOVE].outWord + " ";
			else
				opinionStr = myVocab[SPECIAL][LIKE].outWord + " ";
		}
	}
	else if(opinion < -0.1f)
	{
		// bad opinion

		if(moodOpinion > -0.1f)
		{
			// good mood effecting behaviour say why
			if(c.GetDriveLevel(SEXDRIVE) > c.GetDriveLevel(LONELINESS))
				ExpressNeed(SEXDRIVE, delay);
			else
				ExpressNeed(LONELINESS, delay);
	
			return;
		}
		else
		{
			if(opinion < -0.7f)
				opinionStr = myVocab[SPECIAL][HATE].outWord + " ";
			else
				opinionStr = myVocab[SPECIAL][DISLIKE].outWord + " ";
		}
	}	
	else
		return;	// no opinion
	

	

	std::string replySyntax = TranslateToLocal("poc");
	std::string whatToSay;
	for(int w = 0; w != replySyntax.length(); w++)
	{
		const char *replyWord = replySyntax.c_str()+w;
		switch(*replyWord)
		{
		case 'o':
			whatToSay += opinionStr;
			break;
		case 'c':
			// add name of agent spoken about
			if(creatureOrPointer.IsCreature())
			{
				std::string platonic = creatureOrPointer.GetCreatureReference().Linguistic()->GetPlatonicWord(PERSONAL, ME);
			
				std::string name = creatureOrPointer.GetCreatureReference().Linguistic()->GetWord(PERSONAL, ME);

				// dont use personal name use creature type
				if(name.empty() || platonic == theCatalogue.Get("Learnt Personals", ME)
					|| platonic == myVocab[PERSONAL][ME].platonicWord)
					name = myVocab[NOUN][SensoryFaculty::GetCategoryIdOfAgent(creatureOrPointer)].outWord;

				whatToSay += name + " ";
			}
			else if(creatureOrPointer.IsPointerAgent())
				whatToSay += myVocab[NOUN][SensoryFaculty::GetCategoryIdOfAgent(thePointer)].outWord + " ";

			break;
		case 'p':
			// add own name
			whatToSay += myVocab[PERSONAL][ME].outWord + " ";
			// say own name better
			if(myVocab[PERSONAL][ME].platonicWord != theCatalogue.Get("Learnt Personals", ME))
				myVocab[PERSONAL][ME].HearWord(myVocab[PERSONAL][ME].platonicWord);
			break;
		}
	}


	// remove space at end of sentence
	whatToSay.resize(whatToSay.length()-1);
	// shout so other creatures can respond
	StackSentence(whatToSay, true, delay); 	
}



// ---------------------------------------------------------------------------
// Function:	StackSentence
// Description:	ques sentence 
// Arguments:	sentence to que, whether others will hear, verb and noun 
//				mentioned
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    StackSentence
// Class:       LinguisticFaculty
// Description: 
// Arguments:   std::string sentence = 
//              bool shout = 
//              int delay = 
//              int verb = 
//              int noun = 
// ------------------------------------------------------------------------
void LinguisticFaculty::StackSentence(std::string sentence, bool shout,  int delay, int verb, int noun)
{
	if (mySentenceStack.size() > 3)	// have a limit
		return;
	
	SentenceToSpeak s;
	s.sentence = sentence;
	s.noun = noun;
	s.verb = verb;
	s.shout = shout;
	s.delay = delay;

	// ensure a delay before speaking
	if(mySentenceStack.size()==0)
		myStackCount = delay;

	mySentenceStack.push_back(s);
}



// ---------------------------------------------------------------------------
// Function:	Say
// Description:	says a sentence
// Arguments:	sentence to say
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Say
// Class:       LinguisticFaculty
// Description: 
// Arguments:   std::string string = 
// ------------------------------------------------------------------------
void LinguisticFaculty::Say(std::string string)
{
	Creature& c = myCreature.GetCreatureReference();

	int32 delay = 0;

	bool creaturesAreDumb = theApp.GetWorld().GetGameVar("engine_dumb_creatures").GetInteger()==1;

	if (!creaturesAreDumb) 
	{
		if ((!myVoiceFileHasBeenInitialised) || (myLifeStageForMyCurrentVoice < c.Life()->GetAge())) 
		{
			int genus = c.GetClassifier().Genus();


			// New style voice initialisation
			if (!c.GetVoice().ReadData(c.GetClassifier().Genus(), c.GetClassifier().Species(), c.Life()->GetAge()))
				return;
			myVoiceFileHasBeenInitialised = true;
			myLifeStageForMyCurrentVoice = c.Life()->GetAge();

		}
		if (c.GetVoice().BeginSentence(string)) 
		{
			uint32 sound;
			while (c.GetVoice().GetNextSyllable(sound,delay)) 
			{
				c.SoundEffect(sound,delay);
			}
		}
		delay=c.GetVoice().GetSentenceDelay();
	}

	// p1 - string spoken
	// p2 - agent speaking (can't use FROM, as the same message is sent from the agent
	//      help which can't set FROM)
	CAOSVar p1, p2;
	p1.SetString(string);
	p2.SetAgent(myCreature);
	theAgentManager.ExecuteScriptOnAllAgentsDeferred(SCRIPT_MAKE_SPEECH_BUBBLE,
		myCreature, p1, p2);

	
}



// ---------------------------------------------------------------------------
// Function:	Shout
// Description:	lets other creatures hear a sentence
// Arguments:	sentence to say, noun and verb spoke so others can hear
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Shout
// Class:       LinguisticFaculty
// Description: 
// Arguments:   std::string sentence = 
//              int verb = 
//              int noun = 
// ------------------------------------------------------------------------
void LinguisticFaculty::Shout(std::string sentence, int verb, int noun)
{
	Stimulus s;
	s.stimulusType = Stimulus::typeSHOU;
	s.incomingSentence = sentence;
	s.fromAgent = this->myCreature.GetAgentReference();
	// to NOT stim amount cos will nudge brain
	s.nounIdToStim = noun;			
	s.verbIdToStim = verb;
	s.Process(false);
}



// ---------------------------------------------------------------------------
// Function:	HearSentence
// Description:	hears a sentences shouted
// Arguments:	sspeaker, sentence, verb and noun to learn
// Returns:		None
// ---------------------------------------------------------------------------
void LinguisticFaculty::HearSentence(AgentHandle speakerAgent, std::string 
								sentenceSpoken, int learnVerb, int learnNoun)
{
	if(speakerAgent.IsInvalid())
		return;

	if(sentenceSpoken.length() == 0 ) 
		return ;
	
	Creature& c = myCreature.GetCreatureReference();
	

	// dont talk to yourself
	if(speakerAgent.IsCreature())
	{
		Creature& speakerCreature = speakerAgent.GetCreatureReference();
		if(&speakerCreature == &c) return;
	}


	// PREPROCESS SENTENCE STRING 

	// make sentence lowercase
	std::transform( sentenceSpoken.begin(), sentenceSpoken.end(), sentenceSpoken.begin(), tolower );	


	// discard leading spaces:
	sentenceSpoken.erase(0, sentenceSpoken.find_first_not_of(" "));
	// discard trailing spaces
	sentenceSpoken.erase(sentenceSpoken.find_last_not_of(" ")+1, sentenceSpoken.length());
	// if sentenceSpoken has no content return:
	if (sentenceSpoken.length()==0)
        return;
	// ensure terminating space
	sentenceSpoken += " ";
	// compress multiple spaces into one:
	while (true) 
	{
		int indexOfDoubleSpace = sentenceSpoken.find_first_of("  ");
		if (indexOfDoubleSpace = std::string::npos) break;
		sentenceSpoken.erase(indexOfDoubleSpace, 1);
	}





	// see if this is a sentence of the format 
	//"learn verb|noun|drive|qualifier|special|personal <number> <word>"
	// and if so learn words
	if( ParseAsLearnSentence( sentenceSpoken ) ) return;






	// PARSE SENTENCE FOR RESPONCE

	int wordsHeard[noOfTypes];	// referance to platonic word
	std::string localSyntax;


	
	if (!ParseSentence(speakerAgent, sentenceSpoken, localSyntax, wordsHeard, learnVerb, learnNoun))
	{
		// HANDLE INCOMPREHENSABLE SENTENCE

		// learn noun at it/agent help object

		int attentionId = NONE;
		std::string agentName = sentenceSpoken.substr(0, sentenceSpoken.length()-1);
		if ((!speakerAgent.IsCreature() && !speakerAgent.IsPointerAgent()) ||
			(speakerAgent.IsCreature() &&  
			agentName == SensoryFaculty::GetCategoryName(SensoryFaculty::GetCategoryIdOfAgent(speakerAgent)))) 
		{
			// not the pointer or a creature, so the speaking thing
			// is agent help talking - we learn that agent's noun
			attentionId = SensoryFaculty::GetCategoryIdOfAgent(speakerAgent);
			if (attentionId >= 0 && attentionId < myVocab[NOUN].size())
			{
				// norn learns entire text as new word
				myVocab[NOUN][attentionId].HearWord(sentenceSpoken.substr(0, sentenceSpoken.size() - 1));
				StackSentence(myVocab[NOUN][attentionId].outWord);
			}
		}
	    c.Sensory()->Stimulate(speakerAgent, STIM_GOBBLEDYGOOK, -1);
		return;
	}


	std::string syntax = TranslateToBase(localSyntax);
	
	int nounToNudge = NONE;
	int verbToNudge = NONE;
	float amountToNudgeNoun = defaultNounNudge;
	float amountToNudgeVerb = defaultVerbNudge;


	// handle sentence semantics

	if(syntax == "p")
	{
		// your name said without reference to noun or verb
		// then look at agent who spoke
		if(speakerAgent.IsPointerAgent())
	        nounToNudge = SensoryFaculty::GetCategoryIdOfAgent(speakerAgent);
		else if(speakerAgent.IsCreature())
		{
			nounToNudge = myCreature.GetCreatureReference().Sensory()->
						PayAttentionToCreature(speakerAgent);
		}
		// wake them up!
		if (c.Life()->GetWhetherAsleep())
			c.Life()->SetWhetherAsleep(false);
	}
	else if(syntax == "n")
	{
		// statement about object from anything
        nounToNudge = wordsHeard[NOUN];

		// if agent help or still learning word or object is my current focus say so
		if((!speakerAgent.IsCreature() && !speakerAgent.IsPointerAgent()) || // agentHelp
			speakerAgent.IsPointerAgent() ||
			myVocab[NOUN][nounToNudge].outWord != myVocab[NOUN][nounToNudge].platonicWord ||
			nounToNudge == c.Motor()->GetCurrentAttentionId())
		{
			StackSentence(myVocab[NOUN][wordsHeard[NOUN]].outWord);
		}
	}
	else if(syntax == "c")
	{
		// another creature name spoken - pay attention to creature of name
		wordsHeard[NOUN] = myCreature.GetCreatureReference().Sensory()->
		PayAttentionToCreature(wordsHeard[CREATURENAME]);
		nounToNudge = wordsHeard[NOUN];
	}
	else if((syntax == "pg" || syntax == "gp" || syntax == "g") &&
		speakerAgent.IsPointerAgent())
	{		
	    // YES rewards creature as if he'd been patted (do not pick up explicit knowledge)
		c.Sensory()->Stimulate(speakerAgent, STIM_POINTERYES, -1);
	}
	else if((syntax == "pb" || syntax == "bp" || syntax == "b") &&
		speakerAgent.IsPointerAgent())
	{
		// NO punishes creature - like slap (do not pick up explicit knowledge)
		c.Sensory()->Stimulate(speakerAgent, STIM_POINTERNO, -1);
	}
	else if( syntax == "pv" || syntax == "vnp" ||		// directed commands
			syntax == "1vnp" || syntax == "2vnp" ||	// qualified directed commands
			syntax == "pvn" || syntax == "pvc" ||		// directed commands 
			syntax == "1vcp" || syntax == "2vcp" ||	// qualified directed commands 
			((syntax == "vn" || syntax == "vc" || syntax == "v" ||
			syntax == "1vc" || syntax == "2vc" ||
			syntax == "1vn" || syntax == "2vn") 
			&& speakerAgent.IsPointerAgent()))	// undirected commands from player
	{
	
		AgentHandle creatureOrPointer;
		
		if(syntax == "vc" || syntax == "pvc" || syntax == "1vcp" || syntax == "2vcp" ||
			syntax == "1vc" || syntax == "2vc")
		{				
			// pay attention to a creature 
			wordsHeard[NOUN] = myCreature.GetCreatureReference().Sensory()->
			PayAttentionToCreature(wordsHeard[CREATURENAME]);

			CreatureCollection &creatures = theAgentManager.GetCreatureCollection();
			creatureOrPointer = creatures[wordsHeard[CREATURENAME]];
			nounToNudge = wordsHeard[NOUN];
		}
		else if(wordsHeard[NOUN] == SensoryFaculty::GetCategoryIdOfAgent(thePointer))
		{
			// pointer special case
			creatureOrPointer = thePointer;
			nounToNudge = wordsHeard[NOUN];
		}
		else if(wordsHeard[NOUN] != NONE)
		{
			// pay attention to an object
			nounToNudge = wordsHeard[NOUN];
		}
		else if(!(speakerAgent.IsPointerAgent() && wordsHeard[SPECIAL] == LOOK))
		{
			// noun not mentioned i.e. "pv" or "v"
			// verb 'look' may also be SPECIAL LOOK - in which case handled seperately
			// else if no noun presume current attention object
			nounToNudge = c.Motor()->GetCurrentAttentionId();
		}
		
		if(creatureOrPointer.IsValid() && c.GetGenus()!=2)
		{
			// if asking to do bad thing and like creature - refuse (not grendels)
			for(int i=0; i < theCatalogue.GetArrayCountForTag("Bad Action Script"); i++)
			{
				if(atoi(theCatalogue.Get("Bad Action Script",i)) == wordsHeard[VERB])
				{
					float opinion;
					float moodOpinion;
					c.Sensory()->GetOpinionOfCreature(creatureOrPointer, opinion, moodOpinion);

					if(opinion > 0.1f)
						ExpressOpinion(creatureOrPointer);	// say contradiction if so
					break;
				}
			}	
		}
		// if asking to do good thing and dilike creature - refuse 
		for(int i=0; i < theCatalogue.GetArrayCountForTag("Good Action Script"); i++)
		{
			if(atoi(theCatalogue.Get("Good Action Script",i)) == wordsHeard[VERB])
			{
				float opinion;
				float moodOpinion;
				c.Sensory()->GetOpinionOfCreature(creatureOrPointer, opinion, moodOpinion);

				if(opinion < -0.1f)
					ExpressOpinion(creatureOrPointer);	// say contradiction if so
				break;
			}
		}



			
	
		verbToNudge = GetNeuronIdFromScriptOffset(wordsHeard[VERB]);


		if(syntax.find("1") != -1)
		{
			// maybe
			amountToNudgeNoun = 0.3f;
			amountToNudgeVerb = 0.3f;
		}
		else if(syntax.find("2") != -1)
		{
			//definately
			amountToNudgeNoun = 1.1f;
			amountToNudgeVerb = 1.1f;
		}
		else	
		{
			// unqualified
			amountToNudgeNoun = 0.9f;
			amountToNudgeVerb = 0.9f;
		}

	}
	else if((syntax == "pd" || syntax == "dp" || syntax == "ph" || syntax == "hp" ||
		syntax == "d" || syntax == "h") && speakerAgent.IsPointerAgent())
	{
		// questions about my state or broadcast questions about state
		ExpressNeed(wordsHeard[DRIVE]);
	}
	else if((syntax == "pz" || syntax == "zp" ||
		syntax == "z")  && speakerAgent.IsPointerAgent())
	{
		// questions about my state or broadcast questions about state
		ExpressNeed(wordsHeard[NICEDRIVE]);
	}
	else if( (syntax == "cd" || syntax == "d") && speakerAgent.IsCreature())
	{
		// statments about a creatures state 
		// creature may give responce to asker
		// responce should contain creatures name else will be in 
		// the same form sa SayWhatDoing() i.e vn
		Creature& speakerCreature = speakerAgent.GetCreatureReference();

		if(speakerAgent.IsCreature() && 5%Rnd(1, 5) == 0)	// dont respond always
			return;

		// get what id do in this situation
		Brain::KnowledgeAction action = GetCreatureOwner().GetCreatureReference().
				GetBrain()->GetKnowledge(wordsHeard[DRIVE]);
	
		// I dont know fuck
		if(action.attentionId==-1 || action.decisionId==-1 || action.strength <= 0.001)
				return;


		std::string replySyntax = TranslateToLocal("1vnc");
		std::string reply;
		for(int w = 0; w != replySyntax.length(); w++)
		{
			const char *replyWord = replySyntax.c_str()+w;
			switch(*replyWord)
			{
			case '1':
			case '2':
				// qualifier to knowledge
				reply += action.strength < 0.05 ? myVocab[SPECIAL][MAYBE].outWord+ " ":
				(action.strength > 0.8 ? myVocab[SPECIAL][DEFINATELY].outWord + " ": "");
				break;
			case 'v':
				reply += myVocab[VERB][GetScriptOffsetFromNeuronId(action.decisionId)].outWord + " "; 
				break;
			case 'n':
				reply += myVocab[NOUN][action.attentionId].outWord + " ";
				break;
			case 'c':
				// dont say "me" as anothers name
				std::string platonic = speakerCreature.Linguistic()->GetPlatonicWord(PERSONAL, ME);
				if(platonic != theCatalogue.Get("Learnt Personals", ME))
					reply += speakerCreature.Linguistic()->GetWord(PERSONAL, ME) + " "; 

				break;
			}
		}		
		// remove space at end of sentence
		reply.resize(reply.length()-1);

		StackSentence(reply, true, REPLY_TO_CREATURE_DELAY_OFFSET); // shout so creature mentioned can respond
	}
	else if((syntax == "poc" || syntax == "coc" || syntax == "cop" || syntax=="oc" || syntax =="op") && 
		(speakerAgent.IsCreature() || speakerAgent.IsPointerAgent()))
	{
		// statments about a creature or self
		// respone about creature spoken about or about speaker if spoken about self

		if(speakerAgent.IsCreature() && 7%Rnd(1, 7) == 0)	// dont say always
			return;

		int delay = NORMAL_DELAY_OFFSET;
		if(speakerAgent.IsCreature())
			delay = REPLY_TO_CREATURE_DELAY_OFFSET;

		// say opinion of crature
		CreatureCollection &creatures = theAgentManager.GetCreatureCollection();
		if(wordsHeard[CREATURENAME] != -1)
		{
			ExpressOpinion(creatures[wordsHeard[CREATURENAME]], delay);
			wordsHeard[NOUN] = myCreature.GetCreatureReference().Sensory()->
				PayAttentionToCreature(wordsHeard[CREATURENAME]);
		}
		else
		{
			// "op" 
			ExpressOpinion(speakerAgent, delay);
		}
		nounToNudge = wordsHeard[NOUN];

	}
	else if((syntax == "on" || syntax == "nop" ||syntax == "noc" || syntax == "con") && 
		wordsHeard[NOUN] == SensoryFaculty::GetCategoryIdOfAgent(thePointer) &&
		(speakerAgent.IsCreature() || speakerAgent.IsPointerAgent()))
	{
		// statments about hand
		// respone about creature spoken about or about speaker if spoken about self

		if(speakerAgent.IsCreature() && 7%Rnd(1, 7) == 0)	// dont say always
			return;

		if(syntax == "on" || syntax == "con" || syntax == "nop" )
		{
			// say opinion of pointer
			ExpressOpinion(thePointer);
		}
		else
		{
			// say opinion of creature
			CreatureCollection &creatures = theAgentManager.GetCreatureCollection();
			ExpressOpinion(creatures[wordsHeard[CREATURENAME]]);
			wordsHeard[NOUN] = myCreature.GetCreatureReference().Sensory()->
				PayAttentionToCreature(wordsHeard[CREATURENAME]);
		}
	
		nounToNudge = wordsHeard[NOUN];

	}
	else if( (syntax == "nvp" || syntax == "cvp" || syntax == "vp") ||
			((syntax == "nv" || syntax == "cv") && speakerAgent.IsPointerAgent()))
	{
		// statemnts about event regarding me or broadcast statments
		// [n][c]vp
	}

	
	//	COMMANDS NOT DIRECTED AT ME
	//  either from pointer without creature name and another creature is selected
	//  or with differant creature specified in sentence
	//	(syntax == "vn" || syntax == "vc" || syntax == "nv" || syntax == "cv" ||
	//	syntax == "nvn" || syntax == "cvc" || syntax == "nvc" || syntax == "cvn" ||
	//	syntax == "vnc")



	// HANDLE SPECIALS
	if(speakerAgent.IsPointerAgent())
	{
		// handle special words only available from pointer
 		// yes, no can only be said by pointer else it is creatue expressing need
		// special llok & what can only be said by pointer
	
		switch ( wordsHeard[SPECIAL])
		{
			case LOOK:               
				// special LOOK overrides verb 'look' if said by pointer 
				// without mentioning an object of attention (n or c)
				// attention may have been specified in sentence syntax i.e "look noun|creature"
				{
					AgentHandle a = thePointer.GetPointerAgentReference().IsTouching(0, 0);
					
					if(nounToNudge == NONE)
					{
						// shift attn to obj under pointer if no attention object was mentioned
						if(a.IsValid())
							nounToNudge = c.Sensory()->PayAttentionToAgent(a);
						else 	
							nounToNudge = SensoryFaculty::GetCategoryIdOfAgent(thePointer);
					}
					else
					{
						// pointing at thing specifically asked to look at (by noun)
						// currently looking at another agent of same category
						// then shift attention to one under pointer
						if(a.IsValid())
						{
							if(nounToNudge == SensoryFaculty::GetCategoryIdOfAgent(a)  && 
								c.Sensory()->GetKnownAgent(nounToNudge) != a)
								c.Sensory()->PayAttentionToAgent(a);
						}
					}
				}
				break;

			case WHAT:                // force creature to say what
				SayWhatDoing();                     // it is doing
				break;
		}
	}


	




	// STIM AND NUDGE BRAIN

	
    // otherwise send STIM_POINTERWORD or STIM_CREATUREWORD to fire the sensory neu &
    // update boredom etc. (note: these stimuli must NOT have any nounStim values)
	if (speakerAgent.IsPointerAgent())
		c.Sensory()->Stimulate(speakerAgent, STIM_POINTERWORD, -1);
	else if (speakerAgent.IsCreature())
			c.Sensory()->Stimulate(speakerAgent, STIM_CREATUREWORD, -1);



	// Modulate the amount of nudge according to how well recognised the words are
	// (so that newly learned words have less impact than familiar ones)


	if (nounToNudge!=NONE) 
	{
		amountToNudgeNoun *= myVocab[NOUN][nounToNudge].learnedStrength;
		c.GetBrain()->SetInput("noun", nounToNudge, amountToNudgeNoun);
	}
	if (verbToNudge!=NONE) 
	{
		amountToNudgeVerb *= myVocab[VERB][verbToNudge].learnedStrength;
		c.GetBrain()->SetInput("verb", verbToNudge, amountToNudgeVerb);
	}
	
}




// ---------------------------------------------------------------------------
// Function:	ParseSentence
// Description:	set syntax of sentence
// Arguments:	sspeaker, sentence, verb and noun to learn
// Returns:		None
// ---------------------------------------------------------------------------
bool LinguisticFaculty::ParseSentence(AgentHandle speakerAgent,
	std::string sentenceSpoken, std::string &syntax, int wordsHeard[noOfTypes],
	int learnVerb, int learnNoun)
{								  

    int parserIndex = 0;
	bool recognisedAtLeastOneWord = false;
	syntax="";
	for (int i=0; i<noOfTypes; i++)
		wordsHeard[i] = NONE;


	do
	{
		if(speakerAgent.IsPointerAgent() && wordsHeard[SPECIAL]==NONE)
		{
			// handle special LOOK seperatly cos may be same word as
			// VERB LOOK in which case it will not be picked up as special
			// and we want to respond specially to this
			std::string &lookOutWord =  myVocab[SPECIAL][LOOK].outWord;
			std::string &lookPlatonicWord =  myVocab[SPECIAL][LOOK].platonicWord;
			const std::string &outWordMatch = sentenceSpoken.substr(parserIndex, lookOutWord.length());
			const std::string &platonicWordMatch = sentenceSpoken.substr(parserIndex, lookPlatonicWord.length());
			
			
			
			if (lookOutWord == outWordMatch || lookPlatonicWord == platonicWordMatch)
				wordsHeard[SPECIAL] = LOOK;

		}

		int wordType, wordHeard, wordSize;
		bool wordUnderstood;

		bool match;
		int endPos = 0;
		int len = sentenceSpoken.length();
	
		if(sentenceSpoken.find_first_of(" ", parserIndex)==parserIndex) 
			parserIndex++;	// remove leading spaces


		do {
			if(parserIndex+endPos+1 >= len)
				endPos = len-1;
			else
				endPos = sentenceSpoken.find_first_of(" ", parserIndex+endPos+1 );
	

			std::string word = sentenceSpoken.substr(parserIndex, endPos-parserIndex);
			match = GetWordType(word, wordUnderstood, wordType, wordHeard, wordSize);
		} while(!match  && endPos+1 < len && endPos != -1);

		if(match)
		{
			// allow only the first word of each type to be heard
			// unless a creature name always hear last 
			if(wordsHeard[wordType] == NONE || wordType == CREATURENAME)
			{
				// haven't already got a word of this type

				wordsHeard[wordType] = wordHeard;
				

				switch(wordType)
				{
					case CREATURENAME:
						syntax += "c";
						break;
					case VERB:
						syntax += "v";
						break;
					case NOUN:
						syntax += "n";
						break;
					case NICEDRIVE:
						syntax += "z";
						wordsHeard[DRIVE] = wordHeard;	// only hear one drive type word
						break;
					case DRIVE:
						syntax += "d";
						wordsHeard[NICEDRIVE] = wordHeard; 	// only hear one drive type word
						break;
					case SPECIAL:
						switch(wordHeard)
						{
						case YES:
							syntax += "g";
							break;
						case NO:
							syntax += "b";
							break;
						case HAPPY:						
						case ILL:
							syntax += "h";
							break;
						case MAYBE:
							syntax += "1";
							break;
						case DEFINATELY:
							syntax += "2";
						case LIKE:
						case DISLIKE:
						case LOVE:
						case HATE:
							syntax += "o";
							break;
						}
						break;

					case QUALIFIER:
						// ignore
						break;
					case PERSONAL:
						syntax += "p";
						break;
					default:
						// handle creature names
						break;
				}
			}

			recognisedAtLeastOneWord = true;
		
			// if word heard but not pronouncable yet then learn
			if(!wordUnderstood)
				myVocab[wordType][wordHeard].HearWord(myVocab[wordType][wordHeard].platonicWord);

			
			parserIndex = endPos+1;
		}		
		else
			parserIndex++;
		
	}
	while(parserIndex > 0 && parserIndex+1 < sentenceSpoken.size());






	// learn all words explicitly mentioned to learn that have NOT been picked up
	if(speakerAgent.IsCreature())
	{
		Creature &speakerCreature = speakerAgent.GetCreatureReference();
	
		// learn word if speaker knows it better

		if(learnVerb != NONE && learnVerb != wordsHeard[VERB] &&
			speakerCreature.Linguistic()->GetWordStrength(VERB, learnVerb) >
			myVocab[VERB][learnVerb].learnedStrength)
		{	
			myVocab[VERB][learnVerb].HearWord(
				speakerAgent.GetCreatureReference().Linguistic()->GetWord(VERB, learnVerb));
		}		
		if(learnNoun != NONE && learnNoun != wordsHeard[NOUN] &&
			speakerCreature.Linguistic()->GetWordStrength(NOUN, learnNoun) >
			myVocab[NOUN][learnNoun].learnedStrength)
		{	
			myVocab[NOUN][learnNoun].HearWord(
				speakerAgent.GetCreatureReference().Linguistic()->GetWord(NOUN, learnNoun));
		}

	}



	return recognisedAtLeastOneWord;
}




// ---------------------------------------------------------------------------
// Function:	GetWordType
// Description:	matches first part of sentence agains a word the creature can 
//				say	or a word it is learning to say or another creatres name
// Arguments:	sentence to look in, fills in - whether
//				the word was understood type and id of word and the size
//				of the matched word
// Returns:		If match was found
// ---------------------------------------------------------------------------
bool LinguisticFaculty::GetWordType(std::string wordToMatch,
		bool &wordUnderstood, int &wordType, int &wordHeard, int &wordSize)
{
	wordUnderstood = false;
	wordType = NONE;
	wordHeard = NONE;
	wordSize = 0;
	// check against lexicon
	for (int i=0; i!=noOfTypes-1; i++) 
	{
		for (int o=0; o<myVocab[i].size(); o++) 
		{
			// don't recognise "me" as a name
			if(i==PERSONAL && o==ME && myVocab[i][o].platonicWord==theCatalogue.Get("Learnt Personals", ME))
				continue;

			if(KnownWord(i, o))	// has vocab entry
			{
				// compare agains how creature says word
				std::string vocabWordToCompareAgainst =  myVocab[i][o].outWord;
			
				if(	vocabWordToCompareAgainst != "" && wordToMatch==vocabWordToCompareAgainst)
				{
					wordType = i;
					wordHeard = o;
					wordUnderstood = true;
					wordSize = vocabWordToCompareAgainst.length();
					return true;
				}
			
				// compare against how it should say word
				vocabWordToCompareAgainst =  myVocab[i][o].platonicWord;
				if(	vocabWordToCompareAgainst != "" && wordToMatch==vocabWordToCompareAgainst) 
				{				
					wordType = i;
					wordHeard = o;
					wordUnderstood = false;
					wordSize = vocabWordToCompareAgainst.length();
					return true;
				}

			}
		}
	}



	// check against creature names

	CreatureCollection &creatures = theAgentManager.GetCreatureCollection();
	int noCreatures = creatures.size();
	AgentHandle me = myCreature.GetAgentReference();

	for(int c = 0; c != noCreatures; c++)
	{
		if(creatures[c]!=me)
		{
			if (creatures[c].GetCreatureReference().Linguistic()->KnownWord(PERSONAL, ME))
			{

				// ignore "me" as a name
				if(creatures[c].GetCreatureReference().Linguistic()->GetPlatonicWord(PERSONAL, ME) 
					==theCatalogue.Get("Learnt Personals", ME))
					continue;

				// compare against how speaker says its own name
				std::string vocabWordToCompareAgainst =
				creatures[c].GetCreatureReference().Linguistic()->GetWord(PERSONAL, ME);
			
				
				if(	vocabWordToCompareAgainst != "" && wordToMatch==vocabWordToCompareAgainst)
				{
					wordType = CREATURENAME;
					wordHeard = c;
					wordUnderstood = true;
					wordSize = vocabWordToCompareAgainst.length();
					return true;	// can not learn this yet
				}

				// compare against how it should say its own name
				vocabWordToCompareAgainst =
					creatures[c].GetCreatureReference().Linguistic()->GetPlatonicWord(PERSONAL, ME);
				if (vocabWordToCompareAgainst != "" && wordToMatch==vocabWordToCompareAgainst) 
				{				
					wordType = CREATURENAME;
					wordHeard = c;
					wordUnderstood = true;
					wordSize = vocabWordToCompareAgainst.length();
					return true;	// can not learn this yet
				}
			}
		}
	}


	return false;	// unknown word
}




// ---------------------------------------------------------------------------
// Function:	ParseAsLearnSentence
// Description:	command to learn word
// Arguments:	sentence string
// Returns:		whether it was a sentence of this kind
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    ParseAsLearnSentence
// Class:       LinguisticFaculty
// Description: 
// Arguments:   std::string const &str  = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool LinguisticFaculty::ParseAsLearnSentence( std::string const &str )
{
	// see if this is a sentence of the format 
	//"learn verb|noun|drive|qualifier|special|personal <number> <word>"


	//Lets put each word in a list
	std::vector< std::string > wordList;
	int start = str.find_first_not_of( " " );
	while( start != std::string::npos )
	{
		int end = str.find_first_of( " ", start );
		wordList.push_back( str.substr( start, end - start ) ); //This should work even if end == npos
		start = str.find_first_not_of( " ", end );
	}

	if( wordList.size() < 4 )
		return false;

	if( wordList[0] != "learn" && wordList[0] != "perfect")
		return false;

	bool perfect = wordList[0] == "perfect";

	int type;
	if( wordList[1] == "noun" )
		type = NOUN;
	else if( wordList[1] == "verb" )
		type = VERB;
	else if( wordList[1] == "drive" )
		type = DRIVE;
	else if( wordList[1] == "qualifier" )
		type = QUALIFIER;
	else if( wordList[1] == "special" )
		type = SPECIAL;
	else if( wordList[1] == "personal" )
		type = PERSONAL;
	else if( wordList[1] == "nice_drive" )
		type = NICEDRIVE;
	else
		return false;

	int i;
	int id = 0;
	for( i = 0; i < wordList[2].size(); ++i )
	{
		if( !isdigit( wordList[2][i] ) )
			return false;
		id = id * 10 + wordList[2][i] - '0';
	}
	if( id >= myVocab[type].size() )
		return false;

	std::string word = wordList[3];

	for( i = 4; i < wordList.size(); ++i )
		word += " " + wordList[i];

	if (perfect)
		SetWord(type, id, word, true);
	else
		myVocab[type][id].HearWord( word );

	StackSentence(myVocab[type][id].outWord, false, LEARNING_DELAY);

	return true;
}




std::string LinguisticFaculty::GetQualifier(float level)
{
	std::string whatToSay;

	int qualifier = (int)(level*(float)myVocab[QUALIFIER].size());
	if(qualifier > 0)
	{
		whatToSay.append(myVocab[QUALIFIER][qualifier-1].outWord);
		whatToSay.append(" ");
	}
	return whatToSay;
}



// ---------------------------------------------------------------------------
// Function:	TranslateToBase
// Description:	converts a localised sytax string in a base sytax string 
//				understood by the engine
// Arguments:	sytnax string
// Returns:		translated syntax string
// ---------------------------------------------------------------------------	
std::string LinguisticFaculty::TranslateToBase(std::string syntax)
{

	// if translation catalogues dont exist bail out
	if(!theCatalogue.TagPresent("Base Syntax") || !theCatalogue.TagPresent("Localised Syntax"))
		return syntax;


	int arraySize = theCatalogue.GetArrayCountForTag("Localised Syntax");

	for(int i = 0; i != arraySize; i++)
	{
		if(theCatalogue.Get("Localised Syntax", i) == syntax)
		{
			// found match
			if(theCatalogue.GetArrayCountForTag("Base Syntax")-1 < i)
				return syntax;	// mismatch in translation catalogues -- error
			else
				return  theCatalogue.Get("Base Syntax", i);
		}
	}

	return syntax;
}



// ---------------------------------------------------------------------------
// Function:	TranslateToLocal
// Description:	converts a base sytax string in a localised sytax string 
// Arguments:	syntax string
// Returns:		translated syntax string
// ---------------------------------------------------------------------------	
std::string LinguisticFaculty::TranslateToLocal(std::string syntax)
{

	// if translation catalogues dont exist bail out
	if(!theCatalogue.TagPresent("Base Syntax") || !theCatalogue.TagPresent("Localised Syntax"))
		return syntax;


	int arraySize = theCatalogue.GetArrayCountForTag("Base Syntax");

	for(int i = 0; i != arraySize; i++)
	{
		if(theCatalogue.Get("Base Syntax", i) == syntax)
		{
			// found match
			if(theCatalogue.GetArrayCountForTag("Localised Syntax")-1 < i)
				return syntax;	// mismatch in translation catalogues -- error
			else
				return  theCatalogue.Get("Localised Syntax", i);
		}
	}

	return syntax;
}




// ---------------------------------------------------------------------------
// Function:	Write
// Description:	class file storage
// Arguments:	arhive
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Write
// Class:       LinguisticFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool LinguisticFaculty::Write(CreaturesArchive &archive) const {
	base::Write( archive );

	Creature& c = myCreature.GetCreatureReference();
	
	for	(int i=0; i<noOfTypes-1; i++) {
		archive << (int)(myVocab[i].size());
		for (int o=0; o<myVocab[i].size(); o++) {
			archive << myVocab[i][o].platonicWord;
			archive << myVocab[i][o].outWord;
			archive << myVocab[i][o].learnedStrength;
			// archive << myVoiceFile;
			archive << myVoiceFileHasBeenInitialised;
		}
	}

	archive << myStackCount;

	return true;
}


// ---------------------------------------------------------------------------
// Function:	Read
// Description:	class file storage
// Arguments:	arhive
// Returns:		None
// ---------------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Read
// Class:       LinguisticFaculty
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool LinguisticFaculty::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		if(!base::Read( archive ))
			return false;
		
		for	(int i=0; i<noOfTypes-1; i++) {
			int n;
			archive >> n;
			myVocab[i].resize(n);
			for (int o=0; o<n; o++) {
				archive >> myVocab[i][o].platonicWord;
				archive >> myVocab[i][o].outWord;
				archive >> myVocab[i][o].learnedStrength;
				// archive >> myVoiceFile;
				archive >> myVoiceFileHasBeenInitialised;
			}
		}

		archive >> myStackCount;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}
