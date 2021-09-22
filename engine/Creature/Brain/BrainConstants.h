#ifndef BrainConstants_H
#define BrainConstants_H

const int NUM_SVRULE_VARIABLES=8;
typedef float SVRuleVariables[NUM_SVRULE_VARIABLES];

const int MAX_LOBES=255;
const int MAX_TRACTS=255;
const int MAX_DENDRITES_PER_TRACT=255*255;
const int MAX_NEURONS_PER_LOBE=255*255;

const int MAX_INSTINCTS = 255;		// max # pending instincts allowed


#endif//BrainConstants_H