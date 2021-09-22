#ifndef __instincthdr
#define __instincthdr

class Genome;
class Brain;
const int MAX_INSTINCT_INPUTS = 3;		// max # of dendrites for an instinct


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../../PersistentObject.h"

#include <vector>
typedef std::vector<class Instinct*> Instincts;

class Instinct : public PersistentObject {
	CREATURES_DECLARE_SERIAL( Instinct )
public:
	Instinct();
	// construct by reading data from a gene:
	Instinct(Genome& g, Brain* brain);

	// process another phase of this instinct, return TRUE when it's finished
	bool Process();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);


private:
	Brain* myBrain;

	struct InstinctInput {
		int neuronId;					// neuron in that lobe to be stimulated
		std::string name;
	} myInputs[MAX_INSTINCT_INPUTS];

	int myDecisionId;					// 'decision' to be taken
	struct ReinforcementDetails {
		int driveId;					// reinforcement chemical (eg. Reward/Punish)
		float amount;					// amount of reinforcement to apply
	} myReinforcement;

	int myInstinctTick;					// where in the instinct-processing we've got to so far
};
#endif //__instincthdr