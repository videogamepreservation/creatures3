#ifndef SkeletonConstants_H
#define SkeletonConstants_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


enum {NORTH,SOUTH,EAST,WEST};	// dirns objs can face (S=towards camera)
enum {LEFT,RIGHT};				// eg. which of an obj's feet is 'down'
enum {MALE=1, FEMALE=2};		// Sex of creature in 'species' part of classifier, or in genome

enum bodypartnames {			// index constants for following tables
	PART_HEAD,
	PART_BODY,
	PART_LTHIGH,
	PART_LSHIN,
	PART_LFOOT,
	PART_RTHIGH,
	PART_RSHIN,
	PART_RFOOT,
	PART_LHUMERUS,
	PART_LRADIUS,
	PART_RHUMERUS,
	PART_RRADIUS,
	PART_TAILROOT,
	PART_TAILTIP,
	PART_LEFT_EAR,
	PART_RIGHT_EAR,
	PART_HAIR,

	NUMPARTS				// << number of entries in list >>
};

// Indexes into Pose$ of each limb
enum {
	_DIRN,
	_HEAD,
	_BODY,
	_LTHIGH,
	_LSHIN,
	_LFOOT,
	_RTHIGH,
	_RSHIN,
	_RFOOT,
	_LHUMERUS,
	_LRADIUS,
	_RHUMERUS,
	_RRADIUS,
	_TAILROOT,
	_TAILTIP
};


enum expressions
{
    EXPR_NORMAL = 0,
    EXPR_HAPPY,
    EXPR_SAD,
    EXPR_ANGRY,
    EXPR_SURPRISE,
    EXPR_SLEEPY,
	/* EXPR_VERY_SLEEPY,
	EXPR_VERY_HAPPY,
	EXPR_MISCHEVIOUS,
	EXPR_SCARED,
	EXPR_ILL,
	EXPR_HUNGRY,*/

    EXPR_COUNT
};

enum earSets
{
	EARS_NORMAL,
	EARS_DROOPY,
	EARS_ANGRY,
	EARS_PRICKED,

	EAR_SET_COUNT
};



#define BODY_DATA_EXTENSION	"ATT"

#endif//SkeletonConstants_H
