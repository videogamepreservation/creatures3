/*********************************************************************
* File:     Reaction.h
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifndef _REACTION
#define _REACTION

//========================= CReaction struct =============================//

// reactions are in the form:
// n C1 + n C2 ---rate---> n C3 + n C4
// Where	C# is a chemical (or 0 if none),
// 			n is a proportion of that chemical involved
// 			rate is the reaction rate.
// Eg. 2A + 1B -> 2C + 1D

// The number of moles available for a reaction is determined by the lower of the concentrations
// of the reactants, after dividing by their proportions. For example in the reaction 
// 2H + O -> W, when 20 moles of H are available and 15 moles of O: 20/2 = 10 moles of 2H
// are available, plus 15 moles of O; only 10 moles are available for reaction (since 5 Os are
// surplus).
// The actual amount of reactants that react each tick depends on the probability of collision
// between them, and therefore their concentration. This is handled using a 'decay' data type
// showing the fraction of the available moles (10 in the above example) that will react this
// tick (slow rates == every n ticks).
// Once a number of moles have reacted, that same number of moles is available for producing the
// products. However, these too have given proportions (A + B -> 2C + D), so for simplicity
// the number of moles of each product is the number of moles of reacted material multiplied by
// the proportional values.
// Example: 2A + B -> C + 2D, where conc of A=20, B=15:
// 20/2 = 10 moles of 2A and 15 moles of B are available, therefore the lesser (10 moles)
// is available for reaction. The reaction rate determines that 3 moles will react, thus
// the concentration of A is reduced by 2*3=6 moles, and the concentration of B is reduced by
// 3 moles. There are 3 moles of material available, and so the concentration of C is raised
// by 3 and that of D is raised by 2*3=6 moles.

// Available styles of reaction:
// A -> B							- transmutation (exponential)
// A -> nothing						- decay
// A -> 2A							- amplification
// A + B -> C						- combination
// A + B -> C + D					- combination with two products
// A + B -> A + C					- transmutation due to an ENZYME (A is conserved but required)
// A -> A + B						- generation in response to the presence of A
// nothing -> A						- produce A at a constant rate (~ 16 moles per n ticks)

struct Reaction {
	float propR1;				// proportion of reactant1 (2A + 3B...)
	int R1;					// reactant1 chemical (0==none)
	float propR2;				// proportion of reactant2
	int R2;					// reactant2 chemical (0==none)

	float Rate;					// reaction rate (decay data type)

	float propP1;				// amount of product1 produced
	int P1;					// product1	chemical (0==none)
	float propP2;				// amount of product2 produced
	int P2;					// product2	chemical (0==none)
};

#endif