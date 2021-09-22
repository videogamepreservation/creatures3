#ifndef CreatureConstants_H
#define CreatureConstants_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


// The Seven Ages of Norn...
enum {
	AGE_BABY = 0,		// Initial embryological phase: T=0 genes switch on to create brain etc.
	AGE_CHILD,			// instincts for language, etc
	AGE_ADOLESCENT,		// change in response to opposite sex; ovulation starts; etc
	AGE_YOUTH,			// pair-bonding & mating time
	AGE_ADULT,			// more mature relationships
	AGE_OLD,			// no interest in opposite sex; failing faculties
	AGE_SENILE,			// slowly poisoning yourself to death

	NUMAGES
};



// Reflex and involuntary actions
// These actions invoke scripts whenever their chemoreceptor recommends them more highly
// than any other voluntary action.
// Use them for, eg. flinching from pain, laying eggs when pregnancy reaches full term,
// shivering when cold, languishing when very weak...
// After an action has been taken, it may call the LTCY macro command to set its
// Latency timer. This prevents reactivation for a given period, to allow the chemical
// which caused it to subside. Calling LTCY with a random time interval will allow things
// like shivering to be occasional, rather than perpetual actions. (Note that the time
// value in Latency depends on how frequently TaskHandleBrainWave() is called - this
// is currently every 4 ticks, allowing for latencies up to approx 100 seconds)
const int NUMINVOL = 8;						// # involuntary (reflex) actions supported



// Action numbers / offsets into ACTION_LOBE & VERB_LOBE
enum decisionoffsets {
    AC_DEFAULT,         // 0    
    AC_ACTIVATE1,       // 1
    AC_ACTIVATE2,       // 2
    AC_DEACTIVATE,      // 3
    AC_APPROACH,        // 4
    AC_RETREAT,         // 5		// get rid of!!!
    AC_GET,             // 6
    AC_DROP,            // 7
    AC_EXPRESSNEED,     // 8		// get rid of!!!
    AC_REST,            // 9
    AC_TRAVWEST,        //10
    AC_TRAVEAST,        //11
    AC_EAT,             //12
    AC_HIT,             //13

	NUMACTIONS			//<<<< number of entries in list >>>>
};

// Drive numbers / offsets into DRIVE_LOBE
enum driveoffsets {
    PAIN,                   //0
    HUNGER_FOR_PROTEIN,     //1
    HUNGER_FOR_CARB,        //2
    HUNGER_FOR_FAT,         //3
	COLDNESS,				//4
    HOTNESS,                //5
    TIREDNESS,              //6
    SLEEPINESS,				//7
    LONELINESS,				//8
    CROWDEDNESS,		    //9
    FEAR,					//10
    BOREDOM,				//11
    ANGER,					//12
    SEXDRIVE,				//13
    COMFORT,				//14
	UP,						//15
	DOWN,					//16		
	EXIT,					//17
	ENTER,					//18
	WAIT,
	NUMDRIVES				//<<<< number of entries in list >>>>
};

#endif//CreatureConstants_H