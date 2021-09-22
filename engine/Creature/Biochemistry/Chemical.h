/*********************************************************************
* Now:      Chemical.h (gtb, Dec 2, 1998)
* (Was:      Physio.h)
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifndef _Chemical_H
#define _Chemical_H	 

struct Chemical {
	float Concentration;	// how much there is of this chem (0-255)
	float decayRate;		// how fast this chem decays in abscence of reactions

public:
	Chemical();				// constructor
};

#endif