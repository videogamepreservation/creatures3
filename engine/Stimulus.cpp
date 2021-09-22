#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Stimulus.h"
#include "Creature/Creature.h"
#include "AgentManager.h"
#include "Creature/Genome.h"
#include "Creature/SensoryFaculty.h"




TableSpec ourStimulusNumbers[] =
{
	TableSpec("Stimulus Numbers"),
	TableSpec("Number", "Name", "Description"),

	// from other objects
	TableSpec("0", "Disappoint", "Issue to disappoint (punish) a creature for a pointless action (eg. failed to activate)"),

	// passive senses
	TableSpec("1", "Pointerpat", "User has patted creature"),
	TableSpec("2", "Creaturepat", "Another creature has patted you"),
	TableSpec("3", "Pointerslap", "User has slapped creature"),
	TableSpec("4", "Creatureslap", "Another creature has slapped you"),

	// active senses
	TableSpec("5", "Approaching deprecated", "Object is approaching "),
	TableSpec("6", "Retreating deprecated", "Object is retreating"),
	TableSpec("7", "Bump", "Creature has hit a wall"),
	TableSpec("8", "Newobj deprecated", "New object has come into view"),

	// language
	TableSpec("9", "Gobbledygook", "Heard unrecognised speech"),
	TableSpec("10", "Pointerword", "Heard user speak"),
	TableSpec("11", "Creatureword", "Heard creature speak"),


	// Actions - stimuli to emit during/after each action (eg. increase tiredness).
	// Emit these from action scripts
	TableSpec("12", "Quiescent", "Emit PERIODICALLY while quiescent"),
	TableSpec("13", "Activate1", "Emit AFTER activation"),
	TableSpec("14", "Activate2", "&nbsp;"),
	TableSpec("15", "Deactivate", "&nbsp;"),
	TableSpec("16", "Approach", "Emit PERIODICALLY while watching (after approach phase over)"),
	TableSpec("17", "Retreat", "Emit AFTER retreat"),
	TableSpec("18", "Get", "Emit AFTER get"),
	TableSpec("19", "Drop", "&nbsp;"),
	TableSpec("20", "Expressneed", "Emit AFTER say need"),
	TableSpec("21", "Rest", "Emit after resting but before falling asleep"),
	TableSpec("22", "Sleep", "Emit PERIODICALLY while sleeping (later in REST action)"),
	TableSpec("23", "Travwesteast", "Emit PERIODICALLY while walking west OR east"),
	TableSpec("24", "Push", "Emit AFTER being pushed"),
	TableSpec("25", "Hit", "Emit AFTER being hit"),
	TableSpec("26", "Eat", "Emit AFTER eating"),
	TableSpec("27", "Ac6", "&nbsp;"),

	// Involuntary actions - stimuli to emit during each invol action
	// (eg.get tired when sneeze, get warmer when shiver)
	TableSpec("28", "Invol0", "&nbsp;"),
	TableSpec("29", "Invol1", "&nbsp;"),
	TableSpec("30", "Invol2", "&nbsp;"),
	TableSpec("31", "Invol3", "&nbsp;"),
	TableSpec("32", "Invol4", "&nbsp;"),
	TableSpec("33", "Invol5", "&nbsp;"),
	TableSpec("34", "Invol6", "&nbsp;"),
	TableSpec("35", "Invol7", "&nbsp;"),

	// Stimuli added for Creatures2.
	TableSpec("36", "Approaching edge deprecated", "Emit AFTER moving towards an edge object"),
	TableSpec("37", "Retreating edge deprecated", "Emit AFTER moving away from an edge object"),
	TableSpec("38", "Falling deprecated", "Emit whilst falling under gravity"),
	TableSpec("39", "Impact", "Emit after a collision"),
	TableSpec("40", "Pointeryes", "Emit after user spoken verb YES"),
	TableSpec("41", "Creatureyes", "Emit after creature spoken verb YES"),
	TableSpec("42", "Pointerno", "Emit after user spoken verb NO"),
	TableSpec("43", "Creatureno", "Emit after creature spoken verb NO"),
	TableSpec("44", "Aggression", "Emit after performing a HIT"),
	TableSpec("45", "Mate", "Emit after mating"),

	TableSpec("46", "Oppsex tickle", "&nbsp;"),
	TableSpec("47", "Samesex tickle", "&nbsp;"),

	TableSpec("48", "Go nowhere", "&nbsp;"),
	TableSpec("49", "Go in", "&nbsp;"),
	TableSpec("50", "Go out", "&nbsp;"),
	TableSpec("51", "Go up", "&nbsp;"),
	TableSpec("52", "Go down", "&nbsp;"),
	TableSpec("53", "Go left", "&nbsp;"),
	TableSpec("54", "Go right", "&nbsp;"),


	TableSpec("55", "Reached peak of smell0", "&nbsp;"),
	TableSpec("56", "Reached peak of smell1", "&nbsp;"),
	TableSpec("57", "Reached peak of smell2", "&nbsp;"),
	TableSpec("58", "Reached peak of smell3", "&nbsp;"),
	TableSpec("59", "Reached peak of smell4", "&nbsp;"),
	TableSpec("60", "Reached peak of smell5", "&nbsp;"),
	TableSpec("61", "Reached peak of smell6", "&nbsp;"),
	TableSpec("62", "Reached peak of smell7", "&nbsp;"),
	TableSpec("63", "Reached peak of smell8", "&nbsp;"),
	TableSpec("64", "Reached peak of smell9", "&nbsp;"),
	TableSpec("65", "Reached peak of smell10", "&nbsp;"),
	TableSpec("66", "Reached peak of smell11", "&nbsp;"),
	TableSpec("67", "Reached peak of smell12", "&nbsp;"),
	TableSpec("68", "Reached peak of smell13", "&nbsp;"),
	TableSpec("69", "Reached peak of smell14", "&nbsp;"),

	TableSpec("70", "Reached peak of smell15", "&nbsp;"),
	TableSpec("71", "Reached peak of smell16", "&nbsp;"),
	TableSpec("72", "Reached peak of smell17", "&nbsp;"),
	TableSpec("73", "Reached peak of smell18", "&nbsp;"),
	TableSpec("74", "Reached peak of smell19", "&nbsp;"),
	TableSpec("75", "Wait", "&nbsp;"),

	// general-purpose stimuli for use by objects
	TableSpec("76", "Discomfort", "&nbsp;"),
	TableSpec("77", "Eaten plant", "&nbsp;"),
	TableSpec("78", "Eaten fruit", "&nbsp;"),
	TableSpec("79", "Eaten food", "&nbsp;"),
	TableSpec("80", "Eaten animal", "&nbsp;"),
	TableSpec("81", "Eaten detritus", "&nbsp;"),
	TableSpec("82", "Consume alchohol", "&nbsp;"),
	TableSpec("83", "Danger plant", "&nbsp;"),
	TableSpec("84", "Friendly plant", "&nbsp;"),
	TableSpec("85", "Play bug", "&nbsp;"),
	TableSpec("86", "Play critter", "&nbsp;"),
	TableSpec("87", "Hit critter", "&nbsp;"),
	TableSpec("88", "Play danger animal", "&nbsp;"),
	TableSpec("89", "Activate button", "&nbsp;"),
	TableSpec("90", "Activate machine", "&nbsp;"),
	TableSpec("91", "Got machine", "&nbsp;"),
	TableSpec("92", "Hit machine", "&nbsp;"),
	TableSpec("93", "Got creature egg", "&nbsp;"),
	TableSpec("94", "Travelled in lift", "&nbsp;"),
	TableSpec("95", "Travelled through meta door", "&nbsp;"),
	TableSpec("96", "Travelled through internal door", "&nbsp;"),
	TableSpec("97", "Played with toy", "&nbsp;"),

};

int sizeOurStimulusNumbers = sizeof(ourStimulusNumbers) / sizeof(TableSpec);

int dummyStimulusNumbers = AutoDocumentationTable::RegisterTable(ourStimulusNumbers, sizeof(ourStimulusNumbers));






Stimulus::Stimulus() {
	predefinedStimulusNo = -1;
    fromAgent = NULLHANDLE;
	toCreature = NULLHANDLE;

	stimulusType = typeINVALID;
	bitFlags = 0;

	verbStim = nounStim = 0.0f;
	verbIdToStim = nounIdToStim = -1;

	for (int o=0; o<4; o++) {
		chemicalsToAdjust[o] = 0;
		adjustments[o] = 0.0f;
	}

	fromScriptEventNo = -1;

	strengthMultiplier = 1.0f;
	forceNoLearning = false;
}


void Stimulus::InitFromGenome(Genome& g) {
	nounStim = g.GetFloat();
	verbIdToStim = g.GetByteWithInvalid();
	verbStim = 0.0f; g.GetFloat();		//gtb!temp! (will be verb stim)

	bitFlags = g.GetCodon(0,255);

	for (int o=0; o<4; o++) {
		chemicalsToAdjust[o] = StimChemToBioChem(g.GetByte());
		adjustments[o] = g.GetSignedFloat();
	}
}


void Stimulus::Process(bool setNoun) {
	// nounID is decided automatically if not already set (speach does not want this set:
	if (fromAgent.IsValid() && stimulusType!=typeWRIT && setNoun)
		nounIdToStim = SensoryFaculty::GetCategoryIdOfAgent(fromAgent);

	// scan creature library
	for (int i=0; i<theAgentManager.GetCreatureCollection().size(); i++) {
		AgentHandle c = theAgentManager.GetCreatureByIndex(i);

		if (stimulusType==typeWRIT ? c==toCreature : 
			stimulusType==typeSHOU ? c.GetAgentReference().CanHear(fromAgent) :
			stimulusType==typeSIGN ? c.GetAgentReference().CanSee(fromAgent) :
			stimulusType==typeTACT ? c.GetAgentReference().CanTouch(fromAgent) : false) {

			if (predefinedStimulusNo>=0)
				c.GetCreatureReference().Sensory()->Stimulate(fromAgent, predefinedStimulusNo, fromScriptEventNo, strengthMultiplier, forceNoLearning);
			else
				c.GetCreatureReference().Sensory()->Stimulate(*this);
		}
	}
}


// IF YOU CHANGE THIS YOU *MUST* UPDATE THE VERSION SEE ::READ!!!!
bool Stimulus::Write(CreaturesArchive &archive) const {

	archive << predefinedStimulusNo;

	archive << (int16)stimulusType;

	archive << fromScriptEventNo;
	archive << fromAgent;
	archive << toCreature;

	archive << nounStim;
	archive << verbStim;
	archive << nounIdToStim;
	archive << verbIdToStim;

	archive << incomingSentence;
	archive << bitFlags;

	for (int i=0; i<4; i++) {
		archive << chemicalsToAdjust[i];
		archive << adjustments[i];
	}

	archive << strengthMultiplier;
	archive << forceNoLearning;

	return true;
}

bool Stimulus::Read(CreaturesArchive &archive) {

	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{
		archive >> predefinedStimulusNo;

		int16 s;
		archive >> s;
		stimulusType = (StimulusType)s;

		archive >> fromScriptEventNo;
		archive >> fromAgent;
		archive >> toCreature;

		archive >> nounStim;
		archive >> verbStim;
		archive >> nounIdToStim;
		archive >> verbIdToStim;

		archive >> incomingSentence;
		archive >> bitFlags;

		for (int i=0; i<4; i++) 
		{
			archive >> chemicalsToAdjust[i];
			archive >> adjustments[i];
		}

		archive >> strengthMultiplier;
		archive >> forceNoLearning;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}
