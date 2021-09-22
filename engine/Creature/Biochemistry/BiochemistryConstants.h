#ifndef BiochemistryConstants_H
#define BiochemistryConstants_H


// used cos biochemistry chemical numbers do not match 
// chemcical numbers in stimuli gene 
// conversions are performed.
// on writing STIMTOBIOCHEMOFFSET = 148
// STIM chemicals start at stim chemical 0 and wrap around the chemical
// list excluding 0 (unused).
// i.e.
// chemical 0 = stim chemical 255 
// chemical 148-255 = stim chemical 0-107 
// chemical 1-147 = stim chemical 108-254
const int STIMTOBIOCHEMOFFSET = 148;


// from reaction.h
const int MAXREACTIONS = 128;		// # chemical reactions in a Creature
const int MAXREACTANTS = 16;		// # chemical reactants of each chemical in a reaction


// from chemical.h
const int NUMCHEM = 256;			// # chemicals that can exist (numbered 0 to NUMCHEM-1) (0==none)
const int MAXORGANS = 128;			// # organs in a Creature.
// Specialised chemical ranges...
// ... bulk of chemical numbers are freely assignable, but the following are fixed...
const int CHEM_GLYCOGEN = 4;
const int CHEM_ATP = 35;
const int CHEM_ADP = 36;
const int CHEM_PROGESTERONE = 48;
const int CHEM_TIREDNESS = 154;
const int CHEM_SLEEPINESS = 155;
const int CHEM_INJURY = 127;
const int FIRST_SMELL_CHEMICAL = 165;
const int FIRST_ANTIGEN = 82;
const int LAST_ANTIGEN = 89;




#include "../../Caos/AutoDocumentationTable.h"

extern TableSpec ourReceptorLocusNumbers[];
extern int sizeOurReceptorLocusNumbers;

extern TableSpec ourReceptorLocusNumbers[];
extern int sizeOurReceptorLocusNumbers;









/*********************************************************************
* Typedefs.
*********************************************************************/
// Genetic Ids for organs, their objects and their loci.
// Stored in RECEPTOR or EMITTER gene to identify the locus to which
// the receptor or emitter binds (always a byte or frac value)
enum OrganIDs 
{
	ORGAN_BRAIN = 0,				// brain lobes
	ORGAN_CREATURE,					// creature object itself

	ORGAN_ORGAN,					// Any other organ.
	NUM_EMITTER_ORGANS,				// 3w

	ORGAN_REACTION = 3,				// (new in C2e.. to do enzyme like things)
	NUM_RECEPTOR_ORGANS
};
// ORGAN_CREATURE : tissues
enum CreatureTissueIDs 
{			
	TISSUE_SOMATIC,					// things to do with body, appearance, muscles
	TISSUE_CIRCULATORY,				// loci that can have an emitter AND a receptor attached
	TISSUE_REPRODUCTIVE,			// things to do with sex urges and acts
	TISSUE_IMMUNE,					// things to do with disease resistance
	TISSUE_SENSORIMOTOR,			// boredom, air temp, constants & other sensorimotor i/o
	TISSUE_DRIVES,					// receptors for each drive. Can also be used as emitters

	NUMCREATURETISSUES				// (# entries in this enum)
};


const int noOfVariablesAvailableAsLoci=4;


// from biochemistry.h
const int BIOCHEMISTRY_SCHEMA = 2;
const int DEFAULT_HEART_RATE = 25;


// organ.h
/*********************************************************************
* Constants.
*********************************************************************/
const int MAXRECEPTORGROUPS = 255;
#define LOC_TO_LF(f) (myInitialLifeForce*(f))
#define LF_TO_LOC(f) ((f)/myInitialLifeForce)

/*********************************************************************
* Typedefs.
*********************************************************************/
// Receptor/emitter locus IDs
// General constants to supply to GetLocusAddress() functions
// to specify whether ID refers to an emitter or receptor locus.
enum locustype {						
	RECEPTOR = 0,	
	EMITTER = 1
};

enum OrganEmitterLocusIDs {
    // Receptor & Emitter.
	ELOCUS_CLOCKRATE = 0, ELOCUS_RATEOFREPAIR,
    // Emitter only.
	ELOCUS_LIFEFORCE,
};
enum OrganReceptorLocusIDs {
    // Receptor & Emitter.
	RLOCUS_CLOCKRATE = 0, RLOCUS_RATEOFREPAIR,
    // Receptor only.
    RLOCUS_INJURY,
};

const int MAX_NEUROEMITTERS = 128;	// # neuroemitters in a Creature

// from emitter.h
const int MAXEMITTERS = 128;		// # chemical emitters in a Creature
enum CreatureEmitterLocusIDs {		// ORGAN_CREATURE : <tissue> : emitter loci
	// IMPORTANT NOTE: START EACH TISSUE'S LIST OF LOCI WITH A "LOCUS_something = 0"
	// BECAUSE THE LOCI IN EACH TISSUE MUST COUNT FROM 0 UPWARDS
	// Somatic
	LOC_MUSCLES = 0,				// How much energy has been expended on movement this tick
	// circulatory
	////// These loci are defined above, as the same numbers apply to both recep & emit
	// Reproductive
	LOC_FERTILE = 0,				// 255 if male has a sperm or female has an egg available
	LOC_PREGNANT,					// 255 if female has both egg and sperm & so is pregnant
	LOC_E_OVULATE,					// if low, remove any egg/sperm from .Gamete; if high, add one
	LOC_E_RECEPTIVE,				// if >0, female is receptive to incoming sperm & will conceive
	LOC_E_CHANCEOFMUTATION,
	LOC_E_DEGREEOFMUTATION,
	// Immune
	LOC_DEAD = 0,					// >0 if creature is dead (allows post-mortem chemistry)
	// Sensorimotor
	LOC_CONST = 0,					// constant 255 (for regular emitters)
	LOC_ASLEEP,						// 255 if asleep, else 0
	LOC_COLDNESS,					// how far air temp is below blood temperature
	LOC_HOTNESS,					// how far air temp is above blood temperature
	LOC_LIGHTLEVEL,					// how bright the sky is (eg. control sleepiness)
	LOC_CROWDEDNESS,				// how many and how close others of your kind are
	LOC_RADIATION,					// how much radiation is present
    LOC_TIMEOFDAY,					// what time of day is it	
    LOC_SEASON,						// what time of year is it
	LOC_AIRQUALITY,					// how breathable is the air
    LOC_UPSLOPE,                    // how steep is the slope I'm facing
    LOC_DOWNSLOPE,                  // how steep is the slope I'm facing
	LOC_HEADWIND,                   // speed of wind coming towards me
    LOC_TAILWIND,                    // speed of wind coming from behind me.
	LOC_E_INVOLUNTARY0,				// trigger involuntary actions (fits, flinches, etc)
	LOC_E_INVOLUNTARY1,
	LOC_E_INVOLUNTARY2,
	LOC_E_INVOLUNTARY3,
	LOC_E_INVOLUNTARY4,
	LOC_E_INVOLUNTARY5,
	LOC_E_INVOLUNTARY6,
	LOC_E_INVOLUNTARY7,
	LOC_E_GAIT0,						// trigger various walking gaits (0=default, usually no need for a receptor here)
	LOC_E_GAIT1,
	LOC_E_GAIT2,
	LOC_E_GAIT3,
	LOC_E_GAIT4,
	LOC_E_GAIT5,
	LOC_E_GAIT6,
	LOC_E_GAIT7,
	LOC_E_GAIT8,
	LOC_E_GAIT9,
	LOC_E_GAIT10,
	LOC_E_GAIT11,
	LOC_E_GAIT12,
	LOC_E_GAIT13,
	LOC_E_GAIT14,
	LOC_E_GAIT15,
	LOC_E_GAIT16,

	// drives
	////// These loci are defined in receptor.h, as the same numbers apply to both recep & emit
};


// from receptor.h:
const int MAXRECEPTORS = 128;		// # chemical receptors in a Creature
enum CreatureReceptorLocusIDs {			// ORGAN_CREATURE : <tissue> : receptor loci
	// IMPORTANT NOTE: START EACH TISSUE'S LIST OF LOCI WITH A "LOCUS_something = 0"
	// BECAUSE THE LOCI IN EACH TISSUE MUST COUNT FROM 0 UPWARDS
	// Somatic
	LOC_AGE0 = 0,					// if on and currently AGE_BABY, become AGE_CHILD
	LOC_AGE1,						// if on and currently AGE_CHILD, become AGE_ADOLESCENT
	LOC_AGE2,
	LOC_AGE3,
	LOC_AGE4,
	LOC_AGE5,						// if on and currently AGE_ADULT, become AGE_SENILE
	LOC_AGE6,						// if on DIE IMMEDIATELY of old age (only implement receptor if death needs to be forced to occur)

	// circulatory
	LOC_FLOATING_FIRST = 0,			// These IDs are both receptor AND emitter loci.
									// They allow me to attach a receptor directly to an emitter
									// and therefore make one chemical respond to the
									// existence or non-existence of another in a more
									// complex way than Reactions can handle. For instance
	LOC_FLOATING_LAST = 31,			// "produce chem B when chem A exceeds threshold"
	NUM_FLOATING_LOCI,


	// Reproductive
	LOC_OVULATE = 0,				// if low, remove any egg/sperm from .Gamete; if high, add one
	LOC_RECEPTIVE,					// if >0, female is receptive to incoming sperm & will conceive
	LOC_CHANCEOFMUTATION,
	LOC_DEGREEOFMUTATION,
	// Immune
	LOC_DIE = 0,					// if on, creature dies (ill health, poison, starvation...)
	// Sensorimotor
	LOC_INVOLUNTARY0 = 0,			// trigger involuntary actions (fits, flinches, etc)
	LOC_INVOLUNTARY1,
	LOC_INVOLUNTARY2,
	LOC_INVOLUNTARY3,
	LOC_INVOLUNTARY4,
	LOC_INVOLUNTARY5,
	LOC_INVOLUNTARY6,
	LOC_INVOLUNTARY7,
	LOC_GAIT0,						// trigger various walking gaits (0=default, usually no need for a receptor here)
	LOC_GAIT1,
	LOC_GAIT2,
	LOC_GAIT3,
	LOC_GAIT4,
	LOC_GAIT5,
	LOC_GAIT6,
	LOC_GAIT7,
	LOC_GAIT8,
	LOC_GAIT9,
	LOC_GAIT10,
	LOC_GAIT11,
	LOC_GAIT12,
	LOC_GAIT13,
	LOC_GAIT14,
	LOC_GAIT15,
	LOC_GAIT16,
	// Drives
    LOC_DRIVE0=0,					// These loci are both receptors and emitters.
    LOC_DRIVE1,						// Receptors should be attached to each of these loci
    LOC_DRIVE2,						// and used to monitor the levels of the various drive
    LOC_DRIVE3,						// chemicals. The data from here are transferred to the
    LOC_DRIVE4,						// DRIVE_LOBE of the brain before updating it.
    LOC_DRIVE5,						// Emitters can also be attached here so that chemical
    LOC_DRIVE6,						// events may be triggered by drives (e.g. produce a
    LOC_DRIVE7,						// 'stress' chemical whenever certin drives exceed a
    LOC_DRIVE8,						// threshold defined in the emitter.
    LOC_DRIVE9,						// Any code inside Creature that needs to monitor drive
    LOC_DRIVE10,					// levels, eg. for determining facial expressions, etc.
    LOC_DRIVE11,					// should look at these loci, instead of looking at
    LOC_DRIVE12,					// the DRIVE_LOBE neus, as the latter may be unrepresentative
    LOC_DRIVE13,					// due to thresholds, WTA behaviour etc.
    LOC_DRIVE14,
    LOC_DRIVE15,
    LOC_DRIVE16,
    LOC_DRIVE17,					// due to thresholds, WTA behaviour etc.
    LOC_DRIVE18,
    LOC_DRIVE19
};

#endif//BiochemistryConstants_H
