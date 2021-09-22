// Genetic functions

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "Genome.h"
#include "GenomeStore.h"

#include <math.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#endif // _WIN32

#include "../General.h"
#include "../C2eServices.h"
#include	"../../common/BasicException.h"
#include "../Display/ErrorMessageHandler.h"
#include "../Maths.h"
#include "SkeletonConstants.h"
#include "../General.h"
#include "../File.h"
#include "../FilePath.h"

#include "../AppConstants.h"

#ifndef _WIN32
#include <sys/stat.h>

#endif

// this must be greater than the maximum possible
// length of a gene including the header
const int ourLongerThanAnyGene = 1024;

CREATURES_IMPLEMENT_SERIAL(Genome)

////////////////// gene construction / reproduction /////////////////////

GenomeInitFailedException::GenomeInitFailedException(int tag_offset, const char* messageFrom) {
	std::string type = theCatalogue.Get("genome_error", tag_offset);
	myMessage = ErrorMessageHandler::Format("genome_error", 3, "GenomeInitFailedException", type.c_str(), messageFrom);
}




// initialise genome
void Genome::Init()
{
	myGenes = NULL;
	myStoredPosition = NULL;
	myLength = 0;
	myMoniker = "";
	myGenePointer = NULL;
	mySex = 1;
	myVariant = 0;
	myAge = 0;
	myEndHasBeenReached = false;
        myGeneAge = 255;
	myCrossoverMutationCount = 0;
	myCrossoverCrossCount = 0;
	myCrossMaxLength = 0;
}


// Null constructor
Genome::Genome()
{
	Init();
}


// destructor - doesn't affect genome data file
Genome::~Genome()
{
	delete myGenes;  
}


// construct a genome from given gene file, or a blank one if moniker is zero
Genome::Genome(GenomeStore& store,
			   int storeIndex,
				 int   sex,			// express 1=male 2=female sex-linked genes
				 byte  age, 		// age of creature (determines which genes switch on)
				 int variant)		// variant (is processed like sex)
{
	std::string moniker = store.MonikerAsString(storeIndex);

	// if moniker is non-zero, read data file...
	if( !moniker.empty() )
	{
		std::string filename = GenomeStore::Filename(moniker);
		ReadFromFile(filename, sex, age, variant, moniker);
	}
}



#ifdef _WIN32
// TODO: check if this fn is still used by vat kit!

// this function by dave bhowmik for sole use of the vat kit... not used by
// the game.
// construct a genome from given gene filename

// DAVE - could this call Genome::ReadFromFile now? - FRANCIS
Genome::Genome(char filename[],		
				 int   sex,			// express 1=male 2=female sex-linked genes
				 byte  age, 		// age of creature (determines which genes switch on)
				 int variant)		// variant (0==all, 1-NUM_BEHAVIOUR_VARIANTS=whatever)
{
	Init();
	mySex = sex;
	myVariant = variant;
	myAge = age;

	OFSTRUCT ofstruct;
	HFILE hFile;
	DWORD bytesRead;

	if((hFile = OpenFile(filename, &ofstruct, OF_READ)) == HFILE_ERROR)
	{
		std::string str = ErrorMessageHandler::Format("genome_error", 0, "Genome::Genome", filename);
		throw GenomeException(str,__LINE__);
	}

	//Look for dna3 marker, reject if not found.
	const int fileHdrLen = 4;
	char fileHdr[fileHdrLen];
	
	ReadFile((HANDLE)hFile, &fileHdr, fileHdrLen, &bytesRead, NULL) ; 


	if (*((TOKEN*)fileHdr) == DNA3TOKEN)
	{
		int iSize = GetFileSize((HANDLE)hFile, NULL) - fileHdrLen;
		myGenes = new byte[iSize];					// allocate file-sized buffer
		ReadFile((HANDLE)hFile, myGenes, iSize, &bytesRead, NULL) ; 

		myLength = bytesRead;				// read stuff into it
	}
	else
	{
		std::string str = ErrorMessageHandler::Format("genome_error", 1, "Genome::Genome", filename);
		throw GenomeException(str,__LINE__);
	}
	_lclose(hFile);
	Reset();									// reset pointers
}

#endif // _WIN32


void Genome::ReadFromFile(std::string filename,
  				 int   sex,			// express 1=male 2=female sex-linked genes
				 byte  age, 		// age of creature (determines which genes switch on)
				 int variant,		// variant (is processed like sex)
				 std::string moniker)		

{
	Init();
	mySex = sex;
	myAge = age;
	myVariant = variant;
	myMoniker = moniker;

	int len;
	struct stat buf;
	stat(filename.data(),&buf); // TODO: Check for errors
	len = buf.st_size;
	FILE* fp = fopen(filename.data() , "rb" );
	if( !fp )
	{
		std::string str = ErrorMessageHandler::Format("genome_error", 0, "Genome::Genome", filename.c_str());
		throw GenomeException(str,__LINE__);
	}
	// Look for dna3 marker, reject if not found.
	TOKEN header;
	fread( &header,4,1,fp );
	if( header != DNA3TOKEN )
	{
		std::string str = ErrorMessageHandler::Format("genome_old_dna", 0, "Genome::Genome", filename.c_str());
		throw GenomeException(str, __LINE__);
		return;
	}

	myLength = len-4;
	myGenes = new byte[myLength];
	fread( myGenes, myLength, 1, fp );
	fclose( fp );

	Reset();		// reset pointers

#ifdef _DEBUG
	{
		// check we've got new format file
		int foo = GO_DAD;
		int bar = GO_MUM;
		int humbug = GO_GENUS;
		byte g = *(myGenePointer + GO_DAD + 32);
		byte e = *(myGenePointer + GO_DAD + 33);
		byte n = *(myGenePointer + GO_DAD + 34);
		byte e2 = *(myGenePointer + GO_DAD + 35);
		ASSERT(g == 'g' && e == 'e' && n == 'n' && e2 == 'e');
		Reset();	
	}
#endif
}

// Set a null byte to show we don't know the parents -
// we either have both parents or none.  Any engineered
// file loaded in counts as no parents.
void Genome::DeclareUnverifiedParents()
{
	Reset();
	*(myGenePointer + GO_MUM) = 0;
	*(myGenePointer + GO_DAD) = 0;
}

// Write a genome file to disc, using moniker as fsp
// now write the all new genomes to the current world genetics folder
void Genome::WriteToFile(std::string filename)
{	
	try
	{
		File file;

		file.Create(filename);

		const int fileHdrLen = 4;
		char *fileHdr = (char *)&DNA3TOKEN;
		
		file.Write(fileHdr,fileHdrLen);
		file.Write(myGenes,myLength);
	}
	catch(File::FileException&)
	{
		std::string str = ErrorMessageHandler::Format("genome_error", 2, "Genome::Write", filename.c_str());
		throw GenomeException(str,__LINE__);
	}
}


// Derive my genotype from both parents, using crossing-over, cutting errors
// and mutations

void Genome::Cross(std::string newMoniker, Genome* mum, Genome* dad,
	byte MumChanceOfMutation, byte MumDegreeOfMutation,
	byte DadChanceOfMutation, byte DadDegreeOfMutation)
{
	myMoniker = newMoniker;

	myCrossoverMutationCount = 0;
	myCrossoverCrossCount = 0;
	CrossLoop(mum, dad, MumChanceOfMutation, MumDegreeOfMutation,
		DadChanceOfMutation, DadDegreeOfMutation);

	// Write mum's & dad's monikers into the Header Gene, so that when it gets
	// expressed, the child will be able to recognise its parents & siblings
	Reset();											// reset pointers

	{
		std::string mumMoniker = mum->myMoniker;
		for (int i = 0; i < 32; ++i)
		{
			byte value;
			// Pad with trailing zeros
			if (i < mumMoniker.size())
				value = mumMoniker[i];
			else
				value = 0;
			*(myGenePointer + GO_MUM + i) = value;
		}

		std::string dadMoniker = dad->myMoniker;
		for (int j = 0; j < 32; ++j)
		{
			byte value;
			if (j < dadMoniker.size())
				value = dadMoniker[j];
			else
				value = 0;
			*(myGenePointer + GO_DAD + j) = value;
		}
	}
}

// Call null constructor only before this
void Genome::CrossLoop(Genome* mum, Genome* dad,
	byte MumChanceOfMutation, byte MumDegreeOfMutation,
	byte DadChanceOfMutation, byte DadDegreeOfMutation)
{
	// Allocate enough space for inherited genome

	// Look at what this code used to do, and grin!
	//int i = mum->myLength;											// find biggest parent genome
	//if (dad->myLength > i)
	//	i = dad->myLength;						// baby will likely be this big
	//i += 50000;												// dylan - make buffer BIG (it's temporary!)

	// Make a sensible length
	// CopyGene now checks we are within this
	int myCrossMaxLength = mum->myLength + dad->myLength;

	// The (ourLongerThanAnyGene * 2) ensures we have space for the
	// last gene and any closing bytes that are written.
	myGenes = new byte[myCrossMaxLength + ourLongerThanAnyGene * 2]; // plenty of space for baby's genome
	myLength = 0;										// no size yet, cos no data

	Genome* src;										// gene being read
	Genome* alt;										// gene not being read
	Genome* swap;										// temp var for ptr swapping
	int cross,g;
	int PrevGeneID=0;                                     // temp var to store previously copied gene ID
	bool bMum;

	mum->Reset();										// reset myGenePointer ptrs in all genes
	dad->Reset();
	Reset();

	if	(Rnd(1)) {										// gene being read is randomly mum/dad
		bMum = true;
		src = mum;										// to start with (first gene is always
		alt = dad;										// sex gene, so must pick randomly!)
	}
	else {
		bMum = false;
		src = dad;
		alt = mum;
	}

	for (;;) {											// stop when detect end-of-genome

		do {

			// pick next cross-over point
			cross = Rnd(10,LINKAGE*2);						// average of n-gene linkage (Dylan: min was 1)

			// for each gene up to next crossover point...
			for	(g=0; g<cross; g++) {						// for n genes
				if	((myGenePointer - myGenes >= myCrossMaxLength) ||  // quit if out of space
					(TokenAt(src->myGenePointer)==ENDGENOMETOKEN)) // quit if reached end
				{
					Terminate();
					return;
				}
				PrevGeneID = src->GeneID();                  // remember what the last gene was

				// copy 1 gene to me, with poss mutations
				if (bMum)
				{
					CopyGene(src, MumChanceOfMutation, MumDegreeOfMutation);
				}
				else
				{
					CopyGene(src, DadChanceOfMutation, DadDegreeOfMutation);
				}
			}

			// Check again that we haven't reached the end of the gene (there's a small chance)
			if	((myGenePointer - myGenes >= myCrossMaxLength)  // quit if out of space
				|| TokenAt(src->myGenePointer)==ENDGENOMETOKEN) // quit if reached end
			{
				Terminate();
				return;
			}

		// Reached potential crossover point, but don't stop if there's no equivalent 
		// allele on the other strand to the one you've just copied - otherwise you
		// can't sync the two strands to ensure a cross without potential loss of a
		// non-cuttable gene. AND do not stop if in the middle of copying a block of
		// identical genes
		} while ((alt->FindGene(src->GeneID())==false) || (PrevGeneID==src->GeneID()));
		
		// Reached workable crossover point. src is pointing to the next uncopied gene;
		// alt is pointing to the identical allele on the other strand - the place to
		// continue copying, unless there's a cutting error.
		myCrossoverCrossCount++;

		// Swap strands
		swap = src;
		src = alt;
		alt = swap;

		// Decide whether to cut or dup a gene:
		// if no error  - continue from where src is pointing (ABC -> def...)
		// if dup		- copy alt version then continue with src (ABCD -> def...)
		// if cut		- skip next gene on src (ABC -> efg...)
		g = 0;
		if	(Rnd(CUTERRORRATE)==0)
			g = Rnd(1,2);								// 0=none 1=dup 2=cut

		// DUP? copy gene from PREV strand before continuing with same gene on new strand
		if	((g==1)&&(alt->myGenePointer[GH_FLAGS]&DUP)) {		// only if gene permits dup
			alt->myGenePointer[GH_GEN]++;						// inc generation number to mark a clone
			// and ensure child gets BOTH parents' versions

			if (bMum)
			{
				CopyGene(alt, DadChanceOfMutation, DadDegreeOfMutation);
			}
			else
			{
				CopyGene(alt, MumChanceOfMutation, MumDegreeOfMutation);
			}
		}

		// CUT? start with next-but-one gene
		else if	((g==2)&&(src->myGenePointer[GH_FLAGS]&CUT)) {	// only if gene permits cut
			if	(!src->NextMarker()) {					// skip it, and quit if no more left
				Terminate();
				return;
			}
		}

		// Now continue copying this strand until the next crossover or end
	}
}

// Helper for Cross(): add end-of-genome marker at *myGenePointer & set myLength to correct value
void Genome::Terminate()
{
	TokenAt(myGenePointer) = ENDGENOMETOKEN;				// add the end-of-genome marker
	myGenePointer += 4;									// tally it
	myLength = myGenePointer - myGenes;						// store length of genome in member
}

// Find the NEXT gene start marker. Set myGenePointer to point to it
// return false if reached end of genome
bool Genome::NextMarker()
{
	int marker;

	myGenePointer++;										// in case I'm still pointing at prev marker
	do {
		marker = TokenAt(myGenePointer++);					// read 4 bytes from genome
		if	(marker==ENDGENOMETOKEN)				// retn false if reached end of genome
			return false;
	} while (marker!=GENETOKEN);					// repeat till found "gene" marker
	myGenePointer--;										// undo the increment & point to start	
	return true;									// myGenePointer points to marker
}

// Given that myGenePointer points to the start marker of a gene, return an integer containing the 
// unique header ID of this gene (drawn from its type, subtype and ID number).
int Genome::GeneID()
{
	return (myGenePointer[GH_TYPE]<<16) | (myGenePointer[GH_SUB]<<8) | myGenePointer[GH_ID];
}

// Find the first gene in this genome whose header ID (see above) = n
// and set myGenePointer to point to the start of that gene.
// OR, return false if I couldn't find it
bool Genome::FindGene(int id)
{
	Reset();												// start at the beginning
	while (id != GeneID()) {								// search till found
		if	(!NextMarker())									// or false if end-of-genome
			return false;
	}
	return true;
}

// Copy next gene from src genome to my genome, mutating occasionally if permitted
void Genome::CopyGene(Genome* src, byte ParentChanceOfMutation, byte ParentDegreeOfMutation)
{
	int countLength = 0;
	int mutate = src->myGenePointer[GH_FLAGS] & MUT;			// determine whether gene is mutable
	byte Mutability = src->myGenePointer[GH_MUTABILITY];

	for	(int i=0; i<GH_SWITCHON; i++)					// copy most of header without mutation
	{
		++countLength;
		*myGenePointer++ = *src->myGenePointer++;
	}
	// switchon time may mutate
	CopyCodon(src, mutate, Mutability, ParentChanceOfMutation, ParentDegreeOfMutation);
	++countLength;
	*myGenePointer++ = *src->myGenePointer++;							// but flags don't

	while ((TokenAt(src->myGenePointer)!=GENETOKEN)&&			// copy rest of gene with poss mutations
		   (TokenAt(src->myGenePointer)!=ENDGENOMETOKEN))
    {
		++countLength;
		CopyCodon(src, mutate, Mutability, ParentChanceOfMutation, ParentDegreeOfMutation);
    }

	if (countLength >= ourLongerThanAnyGene)
	{
		std::string str = ErrorMessageHandler::Format("genome_too_long_gene", 0, "Genome::CopyGene", countLength, ourLongerThanAnyGene);
		throw GenomeException(str,__LINE__);
	}
}


// Copy one codon from src genome to my genome, mutating occasionally if mutable>0
void Genome::CopyCodon(Genome* src, int mutate, byte Mutability,
	byte ParentChanceOfMutation, byte ParentDegreeOfMutation)
{
	byte Codon = *src->myGenePointer++;
	const float fMaxDegreeMinus1 = 127.0;

	if (mutate)
	{
		int ChanceOfMutation = MUTATIONRATE;
		ChanceOfMutation = (ChanceOfMutation * (256 - Mutability)) / 256;
		ChanceOfMutation = (ChanceOfMutation * (256 - ParentChanceOfMutation)) / 256;

		if (Rnd(ChanceOfMutation) == 0)
		{
			//TRACE("ChanceOfMutation = 1 in %d\n", ChanceOfMutation);
			// Determine which bits to mutate.
			double dDegree = 1.0 + (fMaxDegreeMinus1 * 
				((double)(255 - ParentDegreeOfMutation)) / 255.0); 
			// 1.0 -> linear relationship... 32.0. -> tight bell curve.

			double dRandom = (double)RndFloat();

			double dP = pow(dRandom, dDegree);

			// MutationMask must always have at least one bit set.
			byte MutationMask = (byte)(255.0 * dP);
			if (MutationMask == 0x00)
				MutationMask = 0x01;

			// Process mask.
			//byte MutantCodon = Rnd(255);
			//byte NewCodon = Codon ^ (MutantCodon & MutationMask);
			byte NewCodon = Codon ^ MutationMask;

			if (NewCodon != Codon) {
				myCrossoverMutationCount++;
			}
			Codon = NewCodon;
		}
	}

	*myGenePointer++ = Codon;	
}

////////////// gene expression //////////////

// Reset ThisGene & myGenePointer to point at start of genome.
// Call this before attempting to read a different type of gene from the one last read
void Genome::Reset()
{
	myGenePointer = myGenes;
	myEndHasBeenReached = false;
    myGeneAge = 255;
}


void Genome::Store()
{
	myStoredPosition = myGenePointer;
}


void Genome::Restore()
{
	myGenePointer = myStoredPosition;
	myEndHasBeenReached = false;
}


void Genome::Store2()
{
	myStoredPosition2 = myGenePointer;
}


void Genome::Restore2()
{
	myGenePointer = myStoredPosition2;
	myEndHasBeenReached = false;
}



// Extract a codon from the NEXT position in a gene and force it into the range
// given by min & max. Return the value & increment the gene pointer
int Genome::GetCodon(int min, int max)	// force codon into this range (max must be <=255)
{

	int c = *myGenePointer++;						// get codon & incr pointer. codon = 0-255
	if	((c>=min)&&(c<=max)) return c;		// if codon is within expected range, return it

	// Codon has mutated outside of an acceptable range, so must be forced into it by wrapping
	// (but not by truncation, as min & max will be the most probable values then)
	return c % (max-min+1) + min;
}

// copy a four-byte token (eg. moniker) from gene
// Stored in gene so that 1st byte = 1st character. Read into dword so that
// 1st character goes into low byte (as when reading macro tokens from script)
TOKEN Genome::GetToken()
{
	TOKEN result = TokenAt(myGenePointer);
	myGenePointer+=4;
	return result;
}



// Find the NEXT gene start marker. Set myGenePointer to point to its TYPE codon
// return false if reached end of genome or the start of a specified genome is found.
bool Genome::GetStart()
{
	int marker;

	do 
	{
		// read 4 bytes from genome
		marker = TokenAt(myGenePointer);		
		myEndHasBeenReached = (marker == ENDGENOMETOKEN);

		if (myEndHasBeenReached)
			return false;

		// Move on a byte.
		// Note: this has been moved until after 'gend' test. This allows
		// multiple calls to GetGeneType to return false.
		myGenePointer++;
	} 
	while (marker != GENETOKEN);					// repeat till found "gene" marker
	
	myGenePointer += 3;									// point to next codon (gene TYPE)
		
	return true;
}



bool Genome::TestCodonExtn(void)
{
	int marker = TokenAt( myGenePointer );
	if( marker == Tok('g','e','x','t') ) {
		myGenePointer += 4;
		return true;
	}
	else return false;
}

// Count the number of genes of the given type
int  Genome::CountGeneType(			 
				  int type,
				  int subtype,
				  int numsubs)
{
	Reset();	// Reset the pointer to the start of the Genome

	int count=0;

	while ((GetGeneType(type,subtype,numsubs,SWITCH_ALWAYS))!=false)
	{
		count++;
	}

	return (count);
}



// Find the NEXT gene of the given type & subtype, and, if sex-linked, the appropriate gender
// Skip genes if they don't switch on yet. 
// Sets myGenePointer to point to the first codon in the gene (after Mutability flags)
// Return false if reached end of genome.
bool Genome::GetGeneType(
					  int type,			// TYPE number (BRAINGENE etc)
					  int subtype,		// SUBTYPE number (CLUSTER, etc)
					  int numsubs,		// number of possible subtypes for this type (NUMBRAINSUBTYPES etc)
					  int flag, /*=0*/	// special switch-on condition (dflt is switch if correct age)
  					  int endType, /*= INVALIDGENE*/		// TYPE of gene to stop at.
					  int endSubType, /*= INVALIDSUBGENE*/	// SUBTYPE of gene to stop at
					  int otherEndType) /*= INVALIDGENE*/	// another TYPE of gene to stop at.

{
	byte* genePointerWhenWeCameIntoThisFunction = myGenePointer;

	// Parse genome until end found:
	while (GetStart())
    {
		int thisGeneType = GetCodon(0, NUMGENETYPES - 1);	// ***** Gene type.
		int thisGeneSubType = GetCodon(0, numsubs-1);		// ***** Sub-type.
		myGenePointer++;									// ***** (skip over ID#).
		myGenePointer++;									// ***** (skip over Generation#).
		myGeneAge = GetCodon(0, 255);
		int sexOfThisGene = GetCodon(0, 255);				// ***** Flags.
		myGenePointer++;									// ***** (skip over Mutability Weighting).
		int variantOfThisGene = GetCodon(0,NUM_BEHAVIOUR_VARIANTS);	// ***** variant


		// Check switch-on time - unless an organ:
		if ((!(thisGeneType == 3 && thisGeneSubType == 0)) && !TimeToSwitchOn(myGeneAge, flag))				// Not the Switch-on time?
			continue;

		// Check sex:
		if (!(sexOfThisGene & MIGNORE) &&
          (((sexOfThisGene & (LINKMALE|LINKFEMALE)) == 0) ||
		  ((sexOfThisGene & LINKMALE) && (mySex == MALE))||
		  ((sexOfThisGene & LINKFEMALE) && (mySex == FEMALE)))) {
		} else {
			// not the right sex:
			continue;
		}

		// Check variant:
		if (!(variantOfThisGene==0 ||					// express always?
			variantOfThisGene==myVariant))				// express if this variant?
			continue;


		// If reached endtype:
		if (thisGeneType==otherEndType || (thisGeneType==endType && 
			(endSubType==INVALIDGENE || endSubType==thisGeneSubType))) {
			// Reset reader back to the beginning of this gene.
			myGenePointer = genePointerWhenWeCameIntoThisFunction;
			break;
		}

		// If you find a gene of correct type...
		if (thisGeneType==type && thisGeneSubType==subtype)
			return true;
	}

	return false;										// false if end of genome
}


// Return true if it is Ok for given gene to switch on at this time
bool Genome::TimeToSwitchOn(byte switchontime,	// SwitchOnTime value from gene header
							 int flag)			// special switch-on condition

{
	switch (flag) {									// depending on condition...

	case SWITCH_AGE:								// switch if gene is timed to go off now
		return (switchontime == myAge) ? true : false;

	case SWITCH_ALWAYS:								// switched on every time genome is scanned
		return true;								// (e.g. to re-read Appearance genes)

	case SWITCH_EMBRYO:								// switch on if myAge=0, regardless of what is
		return (myAge == 0) ? true : false;	// in switchontime (which might have mutated)

    case SWITCH_UPTOAGE:
        return (switchontime <= myAge) ? true : false;
	}
	return true;
}

// NOTE: myGenePointer has just been set by GetGeneType() & therefore points to first codon
byte Genome::Generation()
{
	return myGenePointer[0 - GH_LENGTH + GH_GEN];	// look back to start marker then fwd to Gen#
}

byte Genome::GetGenus()
{
	Reset();
	byte genus = *(myGenePointer + GO_GENUS);
	Reset();
	return genus;
}


