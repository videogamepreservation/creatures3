#ifndef __vocab_h
#define __vocab_h

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include <string>
#include "../../common/C2eTypes.h"
#include "Biochemistry/BiochemistryConstants.h"

class Agent;

// vocabulary slot structure - holds recognisable words, pronouncable equivalents and
// degree of learning
class Vocab {
public:
	std::string platonicWord;	// (input form (true spelling))
	std::string	outWord;		// (output form (mis-spelled))
	float learnedStrength;		// how well this word is known

	
	void DoReallyTerribleInfantSpeak();
	void DoReasonablyGoodInfantSpeak();
	void HearWord(std::string wordHeard);

	Vocab();
	void InitBabyWord(const std::string& text);
	void InitWord(const std::string& text, int age, bool perfect);
};

#endif //__vocab_h