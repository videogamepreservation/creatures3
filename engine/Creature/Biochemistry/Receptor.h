/*********************************************************************
* File:     Receptor.h
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifndef _RECEPTOR
#define _RECEPTOR

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../PersistentObject.h"

//========================= Receptor struct ==========================//

// Contains data about the chemical responsiveness of one receptor site

enum receptorflags {							// Flag values for .Effect
	RE_REDUCE = 1,								// if set, nominal will be REDUCED when chemical
												// exists, default is for nominal to be RAISED
	RE_DIGITAL=2,								// If set, nominal will be changed by GAIN if
												// ANY signal present; otherwise, nominal is
												// changed by signal x gain. (Use RE_DIGITAL to
												// produce fixed or large effects when triggered
												// by potentially small amounts of chemical)
};

struct Receptor : public PersistentObject {
	CREATURES_DECLARE_SERIAL(Receptor)
public:
	Receptor();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

public:
	// Locus ID
	int IDOrgan;								// organ I'm attached to
	int IDTissue;								// tissue on that organ
	int IDLocus;								// locus on that tissue

	// genetically determined members
	int Chem;									// chemical I'm receptive to (0==none)
	float Threshold;							// how much needs to exist before I respond
	float Nominal;								// base signal - chemical modulates this
	float Gain;									// degree to which chemical modulates signal
	int Effect;								// effect of chemical: 0=lower 1=raise signal

	// dynamically determined members
	float* Dest;								// int to modulate (address of locus)
	bool isClockRateReceptor;
};

#endif

