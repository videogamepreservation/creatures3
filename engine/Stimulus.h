#ifndef __stimulus
#define __stimulus

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../common/C2eTypes.h"
#include "CreaturesArchive.h"
#include "Classifier.h"

#include "Creature/Biochemistry/BiochemistryConstants.h"


class Agent;
class Creature;



#include "Caos/AutoDocumentationTable.h"

extern TableSpec ourStimulusNumbers[];
extern int sizeOurStimulusNumbers;



enum builtinstimuli {
	// from other objects
	STIM_DISAPPOINT,			//00 issue to disappoint (punish) a creature for a pointless action (eg. failed to activate)

	// passive senses
	STIM_POINTERPAT,			//01 user has patted creature
	STIM_CREATUREPAT,			//02 another creature has patted you
	STIM_POINTERSLAP,			//03 user has slapped creature
	STIM_CREATURESLAP,			//04 another creature has slapped you

	// active senses
	STIM_APPROACHING_DEPRECATED,//05 object is approaching 
	STIM_RETREATING_DEPRECATED,	//06 object is retreating
	STIM_BUMP,					//07 creature has hit a wall
	STIM_NEWOBJ_DEPRECATED,		//08 new object has come into view

	// language
	STIM_GOBBLEDYGOOK,			//09 heard unrecognised speech
	STIM_POINTERWORD,			//10 heard user speak
	STIM_CREATUREWORD,			//11 heard creature speak


	// Actions - stimuli to emit during/after each action (eg. increase tiredness).
	// Emit these from action scripts
	STIM_QUIESCENT,				//12 emit PERIODICALLY while quiescent
	STIM_ACTIVATE1,				//13 emit AFTER activation
	STIM_ACTIVATE2,
	STIM_DEACTIVATE,
	STIM_APPROACH,				//16 emit PERIODICALLY while watching (after approach phase over)
	STIM_RETREAT,				//17 emit AFTER retreat
	STIM_GET,					//18 emit AFTER get
	STIM_DROP,
	STIM_EXPRESSNEED,			//20 emit AFTER say need
	STIM_REST,					//21 emit after resting but before falling asleep
	STIM_SLEEP,					//22 emit PERIODICALLY while sleeping (later in REST action)
	STIM_TRAVWESTEAST,			//23 emit PERIODICALLY while walking west OR east
	STIM_PUSH,			        //24 emit AFTER being pushed
    STIM_HIT,                   //25 emit AFTER being hit
	STIM_EAT,                   //26 emit AFTER eating
	STIM_AC6,

	// Involuntary actions - stimuli to emit during each invol action
	// (eg.get tired when sneeze, get warmer when shiver)
	STIM_INVOL0,				//28
	STIM_INVOL1,
	STIM_INVOL2,
	STIM_INVOL3,
	STIM_INVOL4,
	STIM_INVOL5,
	STIM_INVOL6,
	STIM_INVOL7,				//35

	// Stimuli added for Creatures2.
	STIM_APPROACHING_EDGE_DEPRECATED,//36 emit AFTER moving towards an edge object
	STIM_RETREATING_EDGE_DEPRECATED, //37 emit AFTER moving away from an edge object
	STIM_FALLING_DEPRECATED,     //38 emit whilst falling under gravity
	STIM_IMPACT,                //39 emit after a collision
    STIM_POINTERYES,            //40 emit after user spoken verb YES
    STIM_CREATUREYES,           //41 emit after creature spoken verb YES
    STIM_POINTERNO,             //42 emit after user spoken verb NO
    STIM_CREATURENO,            //43 emit after creature spoken verb NO
    STIM_AGGRESSION,            //44 emit after performing a HIT
    STIM_MATE,                  //45 emit after mating

	STIM_OPPSEX_TICKLE,
	STIM_SAMESEX_TICKLE,

	STIM_GO_NOWHERE,
	STIM_GO_IN,
	STIM_GO_OUT,				// 50
	STIM_GO_UP,
	STIM_GO_DOWN,
	STIM_GO_LEFT,
	STIM_GO_RIGHT,


	STIM_REACHED_PEAK_OF_SMELL0,	// 55
	STIM_REACHED_PEAK_OF_SMELL1,
	STIM_REACHED_PEAK_OF_SMELL2,	
	STIM_REACHED_PEAK_OF_SMELL3,	
	STIM_REACHED_PEAK_OF_SMELL4,	
	STIM_REACHED_PEAK_OF_SMELL5,	// 60
	STIM_REACHED_PEAK_OF_SMELL6,	
	STIM_REACHED_PEAK_OF_SMELL7,
	STIM_REACHED_PEAK_OF_SMELL8,
	STIM_REACHED_PEAK_OF_SMELL9,	
	STIM_REACHED_PEAK_OF_SMELL10,	// 65
	STIM_REACHED_PEAK_OF_SMELL11,
	STIM_REACHED_PEAK_OF_SMELL12,	
	STIM_REACHED_PEAK_OF_SMELL13,
	STIM_REACHED_PEAK_OF_SMELL14,

	STIM_REACHED_PEAK_OF_SMELL15,	// 70	
	STIM_REACHED_PEAK_OF_SMELL16,
	STIM_REACHED_PEAK_OF_SMELL17,
	STIM_REACHED_PEAK_OF_SMELL18,
	STIM_REACHED_PEAK_OF_SMELL19,
	STIM_WAIT,						// 75

	// general-purpose stimuli for use by objects
	STIM_DISCOMFORT,
	STIM_EATEN_PLANT,	
	STIM_EATEN_FRUIT,
	STIM_EATEN_FOOD,
	STIM_EATEN_ANIMAL,				// 80
	STIM_EATEN_DETRITUS,
	STIM_CONSUME_ALCHOHOL,
	STIM_DANGER_PLANT,
	STIM_FRIENDLY_PLANT,
	STIM_PLAY_BUG,					// 85
	STIM_PLAY_CRITTER,
	STIM_HIT_CRITTER,
	STIM_PLAY_DANGER_ANIMAL,
	STIM_ACTIVATE_BUTTON,
	STIM_ACTIVATE_MACHINE,			// 90
	STIM_GOT_MACHINE,
	STIM_HIT_MACHINE,
	STIM_GOT_CREATURE_EGG,
	STIM_TRAVELLED_IN_LIFT,
	STIM_TRAVELLED_THROUGH_META_DOOR,	// 95
	STIM_TRAVELLED_THROUGH_INTERNAL_DOOR,
	STIM_PLAYED_WITH_TOY,
	STIM_DROP_ALL,
	NUMSTIMULI,					//<<<< number of entries in list >>>>
};


class Genome;


// Bitfields used in Stimulus::bitFlags...
enum ssbits {

	MODULATE = 1,			// 1 if Significance, Intensity & all Drive
							// adjustments should be modulated by
							// msg Param2, to arrive at total effect (ie.
							// effect is variable in intensity).
	SPAREFLAG0= 2,			// was IPRANGE
	IFASLEEP = 4,			// stimulus still gets through, even if asleep
							// (without this flag, stim will be ignored)
							// IFASLEEP stimuli will be attenuated when
							// creature is asleep
	SPAREFLAG1 = 8,
	TRAINING_OFF_FOR_0 = 16,
	TRAINING_OFF_FOR_1 = 32,
	TRAINING_OFF_FOR_2 = 64,
	TRAINING_OFF_FOR_3 = 128
};
const int stimTrainingOffFlags[4] = {16,32,64,128};



// Stimulus data structure
struct Stimulus {
	int predefinedStimulusNo;
	enum StimulusType {
		typeINVALID = -1,
		typeSHOU = 0,
		typeSIGN = 1,
		typeTACT = 2,
		typeWRIT = 3
	} stimulusType;

	int fromScriptEventNo;
	AgentHandle fromAgent;
	AgentHandle toCreature;		// who it's for

	float nounStim;					// amount to nudge significance neurone by
	float verbStim;					
	int nounIdToStim;				// if -1 then use ID of fromAgent
	int verbIdToStim;

	std::string incomingSentence;
	byte bitFlags;					// bit record of features (see below)

	int chemicalsToAdjust[4];			// up to 4 chemicals to emit into bloodstream (0==unused)
	float adjustments[4];			// how much of each chemical to emit

	float strengthMultiplier;
	bool forceNoLearning;

	Stimulus();						// Null constr need do nothing
	void InitFromGenome(Genome& g);
	void Process(bool setNoun = true);
	
	static inline int StimChemToBioChem(int stimChem)
	{
		// convert chemicals to stimulate to thier biochemistry equivalents
		// on writing firstDriveChemical = 148
		// drives chemicals start at stim chemical 0 and wrap around the chemical
		// list excluding 0 (unused).
		// i.e.
		// chemical 0 = stim chemical 255 
		// chemical 148-255 = stim chemical 0-107 
		// chemical 1-147 = stim chemical 108-254
		static int chemical_1_Offset = 255-STIMTOBIOCHEMOFFSET;
		return (stimChem == 255 ? 0 : 
		(stimChem <= chemical_1_Offset ? 
			stimChem+STIMTOBIOCHEMOFFSET : 
			stimChem-chemical_1_Offset));
	}

	bool Write(CreaturesArchive &archive) const;
	bool Read(CreaturesArchive &archive);
};
#endif
