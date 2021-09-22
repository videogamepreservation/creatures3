#ifndef __geneticshdr
#define __geneticshdr


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../PersistentObject.h"
#include "../Token.h"

#include <string>

typedef unsigned char byte;

class GenomeStore;


#define NUM_BEHAVIOUR_VARIANTS 8


// GP fn to read a byte ptr as if it was a dword* (reading 4-byte tokens)
#define TokenAt(bptr) (*(TOKEN*) (bptr) )

// Gene marker codes (4-byte tokens) in a form where they can be inserted into a
// byte array to represent a genome. 
// for example:
// byte Genome[] = { GENE,3,123,4,55,GENE,3,122,3,22,ENDGENOME };
const int DNA3TOKEN =		Tok('d','n','a','3');	// Start of gen file marker.
const int GENETOKEN =		Tok('g','e','n','e');	// Start of gene marker.
const int ENDGENOMETOKEN =	Tok('g','e','n','d');	// End of genome marker.

// Offsets into gene header for each datum
enum geneheaderoffsets {
	GH_TYPE = 4,				// BRAINGENE etc.
	GH_SUB,						// G_LOBE etc.
	GH_ID,						// ID# (for gene editor title tracking)
	GH_GEN,						// generation# (for clones)
	GH_SWITCHON,				// switch-on time
	GH_FLAGS,					// mutability flags
	GH_MUTABILITY,				// breadth of mutability
	GH_VARIANT,					// variant

	GH_LENGTH					// number of bytes in header, INCLUDING start marker
};

// Offsets into the Header Gene (gene(0)). These are fields that get written to during
// conception
enum headeroffsets {
	GO_GENUS = GH_LENGTH,		// 1st field is genus (not written to)
	GO_MUM,						// mum's moniker (32 bytes)
	GO_DAD = GO_MUM+32,			// dad's moniker (32 bytes
};

// Mutability bit flags
enum mutflags {
	MUT = 1,					// 1 if gene allows mutations
	DUP = 2,					// 1 if gene may be duplicated
	CUT = 4,					// 1 if gene may be excised
	LINKMALE = 8,				// 1 if gene must only express in males
	LINKFEMALE = 16,			// 1 if gene must only express in females
	MIGNORE = 32				// 1 if gene must be carried but not expressed.
};

// Crossover and mutation parameters
const int LINKAGE = 50;			// AVERAGE # genes between crossover points. (range = 1 to
								// LINKAGE*2). Big numbers imply higher likelihood that nearby
								// genes remain linked in offspring; small number imply offspring
								// that are more even mixtures of parental genes. (Dylan: was 20)

const int CUTERRORRATE = 80;	// Probability that a crossover will lead to a gene duplication
								// or omission (1/n errors per crossover). (Total cutting errors
								// per genome also depends on number of crossovers and hence
								// the value of LINKAGE)

const int MUTATIONRATE = 4800;	// Probability that a codon will mutate (1/n per codon)

// Gene TYPE numbers
enum {
	BRAINGENE = 0,			// brain genes (CBrain class)
	BIOCHEMISTRYGENE,		// Physiology genes (emitter, receptors, reactions & half-lives)
	CREATUREGENE,			// Creature genes (sensory stimuli, appearance, etc.)
	ORGANGENE,
	// ...add other types here

	NUMGENETYPES,			// total number of types defined above
};

const int INVALIDGENE = -1;
const int INVALIDSUBGENE = -1;


// Gene SUBTYPE numbers...
// BRAINGENE subtypes
enum {
	G_LOBE = 0,				// define a brain lobe & its cells
    G_BORGAN = 1,           // configure its organ characteristics.
	G_TRACT = 2,			// define a brain tract & its dendrites (new in C3)

	NUMBRAINSUBTYPES		// total number of subtypes defined	above
};

// BIOCHEMISTRYGENE subtypes
enum {
	G_RECEPTOR,				// define a chemical receptor
	G_EMITTER,				// define a chemical emitter
	G_REACTION,				// define a chemical reaction site
	G_HALFLIFE,				// define natural decay rates for all chemicals
	G_INJECT,				// define initial concentration of a chemical
	G_NEUROEMITTER,

	NUMBIOCHEMSUBTYPES		// total number of subtypes defined	above
};

// ORGANGENE subtypes
enum {
	G_ORGAN,				// define an organ.
	NUMORGANSUBTYPES
};

// CREATUREGENE subtypes
enum {
	G_STIMULUS = 0,			// define a built-in sensory stimulus
	G_GENUS,				// define GENUS of creature
	G_APPEARANCE,			// define sprites & attachment table for a body region
	G_POSE,					// define a pose string
	G_GAIT,					// define a walking gait
	G_INSTINCT,				// hard-wire an instinctive reaction
	G_PIGMENT,				// define redness, greenness or blueness of skin
    G_PIGMENTBLEED,         // define skin pigment effects
	G_EXPRESSION,			// define drive links to facial expressions

	NUMCREATURESUBTYPES		// total number of subtypes defined	above
};



// Constants for G_APPEARANCE genes - body regions that genes control
// eg.
//		GENE,CREATUREGENE,G_APPEARANCE,
//		REGION_ARMS,2,							// set arms to use variant 2
// NOTE: if no gene exists for arms &/or tail, then no such body region will be created.
// Otherwise, absence of a gene means "use variant 0"
enum bodyregions {			// regions of body that are controlled by a single gene
//+++ The first 2 regions are remmed out until articulated head parts are needed
//+++	REGION_EARS,			// should ears go with head?
//+++	REGION_EYES,
	REGION_HEAD,
	REGION_BODY,
	REGION_LEGS,			// both thighs, shins and feet
	REGION_ARMS,			// both upper and lower arms
	REGION_TAIL,			// both parts of tail
	REGION_HAIR,			// hair can now be genetically specified

	NUMREGIONS				// << number of entries in list >>
};

const int NUMCREATUREGENI = 4;

// Gene-switching flags. Pass one of these to GetGeneType() to override the default
// behaviour of switching genes on when their time is right
enum geneswitchoverrides {
	// DEFAULT: Switch on if gene is timed to go off at this myAge
	SWITCH_AGE=0,
	// Gene will be switched on EVERY time genome is scanned
	// e.g. to re-read such things as APPEARANCE or HEADER genes, during later scans
	SWITCH_ALWAYS,
	// switch on if myAge=0, regardless of what switch-on time is stored in the gene header.
	// This is useful to force eg. LOBE genes to switch on during Embryology, even if
	// the gene header has mutated
	SWITCH_EMBRYO,

    // Age of gene is younger or equal to creature age.
    SWITCH_UPTOAGE,
};



class GenomeInitFailedException {
protected:
	std::string myMessage;
public:
	const char* what() {
		return myMessage.c_str();
	}
	GenomeInitFailedException(int tag_offset, const char* messageFrom);
};


//////////////////////// Genome class ///////////////////////////

class Genome : public PersistentObject {
	CREATURES_DECLARE_SERIAL(Genome)	// serialisable so that exported creatures save their genomes
public:

	inline int GetAge() {
		return myAge;
	}
	inline void SetAge(int age){
		myAge = age;
	}
	inline int GetSex() {
		return mySex;
	}
	inline std::string GetMoniker() {
		return myMoniker;
	}

	inline void SetMoniker(const std::string& newMoniker) {
		myMoniker = newMoniker;
	}

	inline byte* GetGenePointer() {
		return myGenePointer;
	}
	inline void AdjustGenePointerBy(int i) {
		myGenePointer += i;
	}
	inline void SetGenePointer(byte* newGenePointer) {
		myGenePointer = newGenePointer;
	}

	byte GetGenus();


protected:
 	byte* myGenes;				// genome data
	byte* myStoredPosition;
	byte* myStoredPosition2;
	byte* myGenePointer;		// ptr to next codon to read

	int myLength;				// total # bytes in genome
	int myCrossMaxLength;			// used during child copying
	std::string myMoniker;		// unique moniker
	int mySex;					// express sex-linked genes of this gender only (1=male 2=female)
	int myVariant;				// (new for C2e) express genes of this variant only. (0=all, 1-NUM_BEHAVIOUR_VARIANTS=behav)
	byte myAge;					// age of creature (0-255) - switches on correct genes
	bool myEndHasBeenReached;
    byte myGeneAge;

public:
	int myCrossoverMutationCount;
	int myCrossoverCrossCount;

public:
//	void InsertExtn( byte *data, int numdata );

	///// genome construction ///


	void Init();					// initialise genome

	Genome();						// null constr
	Genome(char filename[],			// this constructor by dave bhowmik
				 int   sex,			// express 1=male 2=female sex-linked genes
				 byte  age, 		// age of creature (determines which genes switch on)
				 int variant = 1);	// variant
	Genome(GenomeStore& store,
			   int storeIndex,
			   int   sex,			// express 1=male 2=female sex-linked genes
			   byte  age, 		// age of creature (determines which genes switch on)
			   int variant);		// variant (is processed like sex)

	~Genome();						// destr

	void ReadFromFile(std::string filename,
  				 int   sex,			// express 1=male 2=female sex-linked genes
				 byte  age, 		// age of creature (determines which genes switch on)
				 int variant,		// variant (is processed like sex)
				 std::string moniker);
	void WriteToFile(std::string filename);
	void DeclareUnverifiedParents();

	void Cross(std::string newMoniker, Genome* mum, Genome* dad,
		byte MumChanceOfMutation, byte MumDegreeOfMutation,
		byte DadChanceOfMutation, byte DadDegreeOfMutation);

	///// gene expression /////

	void Reset();					// Reset myGenePointer to point at start of genome
	void Store();					// Store a point temporarily in genome.
	void Restore();					// Return to previously stored point in genome.
	void Store2();					// Store a point temporarily in genome.
	void Restore2();					// Return to previously stored point in genome.
	bool GetGeneType(				// Find the NEXT gene of the given type & subtype.
				  int type,
				  int subtype,
				  int numsubs,
				  int flag = SWITCH_AGE,	// special switch-on condition (dflt is switch if correct age)
				  int endtype = INVALIDGENE,
				  int endSubType = INVALIDSUBGENE,
				  int otherEndType = INVALIDGENE);

	int  CountGeneType(				// Count the number of genes of the given type 
				  int type,
				  int subtype,
				  int numsubs);


	int GetCodon(int min, int max); // Extract a codon from the NEXT position in a gene
	inline int GetCodonLessThan(int maxValue) {
		return GetCodon(0, maxValue-1);
	}

	TOKEN GetToken();				// copy a four-byte token (eg. moniker) from gene

	// gtb Jan 99, new functions to read codons as different primitive types:
	inline char GetChar() {				return (char)GetByte();					}
	inline byte GetByte() {				return GetCodon(0,255);					}
	inline int GetByteWithInvalid() {
		int b = GetCodon(0,255);
		return b==255 ? -1 : b;
	}
	inline bool GetBool() {				return GetByte()!=0;					}
	inline float GetFloat() {			return ((float)GetByte())/255;			}
	inline float GetSignedFloat() {		return (((float)GetCodon(0,248))/124.0f)-1.0f;}
	inline int GetInt() {
		int highByte = GetByte();
		int lowByte = GetByte();
		return highByte*256 + lowByte;
	}

    byte GetGeneAge() {	return myGeneAge;	}
    byte Generation();
	bool Genome::TestCodonExtn(void);

	//////////////////////////////////////////////////////////////////////////
// Exceptions
//////////////////////////////////////////////////////////////////////////
class GenomeException: public BasicException
{
public:
	GenomeException(std::string what, uint16 line):
	BasicException(what.c_str()),
	lineNumber(line){;}

	uint16 LineNumber(){return lineNumber;}
private:

	uint16 lineNumber;

};

private:

	void Genome::CrossLoop(Genome* mum, Genome* dad,
		byte MumChanceOfMutation, byte MumDegreeOfMutation,
		byte DadChanceOfMutation, byte DadDegreeOfMutation);

	bool GetStart();	// Find the NEXT gene start marker. Set myGenePointer to point to its TYPE codon
	bool TimeToSwitchOn(byte switchontime,			// is it Ok for this gene to switch on now?
						int flag);

	void Terminate();				// Helper for Cross(): add end-of-genome marker
	bool NextMarker();				// Find the NEXT gene start marker.
	int GeneID();					// get unique header ID of this gene
	bool FindGene(int id);			// Find 1st gene in genome whose header ID = n

	void CopyGene(Genome* src,		// Copy next gene from src genome to my genome, mutating
		byte ParentChanceOfMutation, byte ParentDegreeOfMutation);

	 // Copy one codon from src genome to my genome, mutating
	void CopyCodon(Genome* src, int mutate, byte Mutability,
		byte ParentChanceOfMutation, byte ParentDegreeOfMutation);
};



/***************************** NOTES **********************************

GENE STRUCTURE



MUTATION
Mutations must ONLY happen inside the data area of a gene, never in the start marker or
the TYPE or SUBTYPE entries. Therefore, the mutate() function must scan the genome and
count how many genes there are, then pick one of those genes at random and find its
length, then pick a data codon within the gene and mutate it. This ensures the integrity 
of the genome. New genes can still be added to the system by a combination of cutting error
and mutation, but no existing genes can have their type changed, or be cut in two.

CROSSING OVER
Crossing over must ONLY happen at gene boundaries. Also, a +/-1 gene cutting error should be
allowed. Thus genes can be deleted, and new genes added during crossing over. When a gene
is replicated, it has no significant effect until and unless one of the pair gets mutated.
It could speed things up if I automatically introduce a mutation in any duplicated genes.


DEVELOPMENT AND SENESCENCE
Give each gene a 'switching on time'. Then run the gene expressor several times during the
life of a creature (at startup, maybe). This allows me to add genes that don't switch on
until, say, adolescence or old age, thus allowing for sexual maturation and senescence.
(Note: switch-on times are mutable, allowing evolution of different life histories).
It would even be feasible (just in case) for me to allow child and adult 'appearance' genes!

Also, define body poses using genes - then babies, adolescents and adults could have different
walking postures, etc. (could babies crawl, or otherwise be unable to reach high things?).
In principle, this would also allow young & old body sprites!?

INSTINCT genes can also be switched on. This allows maturity to control eg. sexual behaviour
(attraction to opposite sex), language development, etc.

MUTATIONS
Have a single FLAGS byte in the gene header. Bits in here determine whether the gene may be
duplicated, excised and/or mutated during crossing-over

GENE HEADER
Each gene starts with the following header information (all bytes):-

- 'g','e','n','e' start marker
- gene type (BRAINGENE, etc.)
- gene sub-type (G_LOBE, etc.)
- gene ID# (ignore this here, it is used by the Gene Editor so that it can associate
  			gene captions & annotations with the correct original or cloned gene)
- Generation# (Used by the gene editor to detect genes that have been cloned by cutting errors.
			This program should increment this value in any cloned (duplicated) gene)
- Switch-on-time (Used to determine whether this gene should be expressed now. Zero means
			express only at conception; other values allow genes to be switched on later in life)
- mutability flags (Used to control whether a gene can be duplicated or lost during crossing-over
			and whether it may become mutated)
After this come the gene-specific data.


***********************************************************************/


#endif



