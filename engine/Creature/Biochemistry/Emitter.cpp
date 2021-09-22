/*********************************************************************
* File:     Emitter.cpp
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Emitter.h"

#define NULL 0

Emitter::Emitter() {
	Chem = Effect = 0;
	bioTickRate = 1.0f;
	bioTick = 0.0f;
	Threshold = Gain = 0.0f;
	Source = NULL;
}

