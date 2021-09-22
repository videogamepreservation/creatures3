/*********************************************************************
* File:     Emitter.h
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifndef _EMITTER
#define _EMITTER

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif



//=========================== Emitter struct ==============================//
// Contains data about how much of which chemical gets emitted when this site
// is active or changes	state

enum emitterflags {								// flags for .Effect member
	EM_REMOVE=1,								// If set, clear source int after response
												//  enables short bursts of chemical without complex code at the locus end
	EM_DIGITAL=2,								// If set, o/p = Gain value if signal>threshold, regardless of
												//  actual signal value (signal triggers emission rather than modulates it)
	EM_INVERT=4,								// Invert locus value (v=255-v) before computing o/p
												// (saves having to have "I am hot" AND "I am cold" etc. loci)
};


struct Emitter {
	// Locus ID
	int IDOrgan;								// organ I'm attached to
	int IDTissue;								// tissue on that organ
	int IDLocus;								// locus on that tissue

	// genetically determined members
	int Chem;									// what to emit
	float Threshold;							// how much needs to exist before I respond
	float bioTickRate;
	float bioTick;
	float Gain;									// fraction: how much to attenuate source value
	int Effect;								// flags controlling response

	// dynamically determined members
	float* Source;								// int to respond to (address of locus)

	Emitter();									// constr
};


#endif