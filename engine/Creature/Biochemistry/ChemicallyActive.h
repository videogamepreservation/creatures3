#ifndef _ChemicallyActive_H
#define _ChemicallyActive_H

/*********************************************************************
* ChemicallyActive base class.
* All objects that wish to support chemical receptor and/or emitter sites
* must derive from this class. It provides a pair of virtual functions that
* must be overloaded in the child class to return the addresses of any
* potential chemically active int values (loci), given their ID.
* Each locus is associated with a particular ‘organ’ (creature, brain...)
* a particular ‘object’ in that organ (lobe in that brain...) and a
* particular int or frac member variable in that object (lobe threshold...).
* Any int or frac may be the source of data controlling a chemical emitter,
* or the destination for the output of a chemical receptor.
*********************************************************************/
class ChemicallyActive {
public:
	// return address of given emitter/receptor locus, or NULL if not recognised.
	// Overload this in derived classes to return addresses of their potential loci
	virtual float* GetLocusAddress(int type,			// is this an EMITTER or RECEPTOR locus?
								int organ,				// Creature, brain, whatever
								int tissue, 			// tisse containing locus (eg. lobe#)
								int locus) = 0;		// locus on object (eg. .Activity)
};

#endif //_ChemicallyActive_H