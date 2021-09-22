// Tract.h: interface for the Tract class.
//
//////////////////////////////////////////////////////////////////////
#ifndef Tract_H
#define Tract_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BrainComponent.h"
#include "Dendrite.h"
#include "Lobe.h"

class Genome;
class GenomeInitFailedException;

class Tract;
typedef std::vector<Tract*> Tracts;

class Tract : public BrainComponent {
	typedef BrainComponent base;
	friend class BrainAccess;
	CREATURES_DECLARE_SERIAL(Tract)
public:
	Tract(Genome& genome, Lobes& lobes); 
	Tract(std::istream &in, Lobes& lobes) ;
	Tract();
	virtual ~Tract();

	virtual void Initialise();

	inline operator const char*() const {		return myName.c_str();}
	inline const char* GetName() const {		return myName.c_str();}

	virtual void DoUpdate();
	void ClearActivity();
	void DoUpdateFromDesc(std::istream &in);
	void DoUpdateDendriteFromDesc(int i, std::istream &in);
	virtual void TraceDebugInformation();

	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);
	
	// sets used by appletts
	bool SetDendriteWeight( const int dendrite, const int var, const float value );
	bool SetSVFloat(int entryNo, float value);

	void DumpTract(std::ostream& out);
	bool DumpDendrite(int d, std::ostream& out);
	int DumpSize();

protected:
	std::string myName;
	Dendrites myDendrites;

	struct TractAttachmentDetails {
		Lobe* lobe;
		struct {int min;int max;} neuronRangeToUse;
		int noOfDendritesPerNeuronOnEachPass;
		Neurons neurons;
		// State variable to monitor that controls migration
		int neuralGrowthFactorStateVariableIndex;
	} mySrc, myDst;

	bool myDendritesAreRandomlyConnectedAndMigrate;
	bool myNoOfDendritesPerNeuronIsRandomUpToSpecifiedUpperBound;

	int GetNoOfDendritesConnectedTo(Neuron* neuron, bool checkSrcNotDst);
	bool DoesDendriteExistFromTo(Neuron* srcNeuron, Neuron* dstNeuron);
	void InitNeuronLists();

	/////////////////////////
	// Migration data and functions
	/////////////////////////

	// Soft Constants - defined in Brain.catalogue file 
	// (future potential for different settings for different tracts)
	byte myMaxMigrations;				// Max number of migrations per tract per update.
	byte myDendriteStrengthSVIndex;		// Index of state var in dendrites that represents their Strength

	Dendrites myWeakDendrites;			// List of dendrites to be migrated

	void UpdateWeakDendritesList( Dendrite &d );
	void MigrateWeakDendrites();
	Neurons FindNNeuronsWithHighestGivenState( Neurons searchNeurons, int stateIndex, int maxListSize );
	bool AttemptMigration( Neuron* dest, Neuron* source, int sourceStateIndex );
	Dendrite* GetDendriteIfExistingFromTo(Neuron* srcNeuron, Neuron* dstNeuron);
	void ProcessRewardAndPunishment( Dendrite &d );
	
public:

	float STtoLTRate;

	/////////////////////////
	// Reward/Punishment data and functions
	//	This mechanism increases or decreases weights of dendrites connected to a 
	//	winning neuron. When reinforcement takes place and the amount of reinforcement
	//	depends on biochemical levels and parameters that are set by SVRules.
	/////////////////////////
	class ReinforcementDetails {
	private:
		bool myDendritesSupportReinforcement;
		float myThreshold;						// Threshold of Reinforcement chemical req'd before Reinforcement process takes place
		float myRate;							// Amount of change that takes place (as does the level of Reinforcement chemical)
		byte myChemicalIndex;						// Index to Reinforcement chemical
	public:
		ReinforcementDetails() {
			myDendritesSupportReinforcement = false;
			myRate = 0;
			myThreshold = 0;
			myChemicalIndex = 0;
		}

		void SetDendritesSupportFlag( bool b ) { myDendritesSupportReinforcement = b; }
		void SetThreshold( float f ) { myThreshold = f; }
		void SetRate( float f ) { myRate = f; }
		void SetChemicalIndex( byte b ) { myChemicalIndex = b; };
		byte GetChemicalIndex() {return myChemicalIndex;};
		
		bool IsSupported() {return myDendritesSupportReinforcement;}
		void ReinforceAVariable( float levelOfReinforcement, float &variableToReinforce );

		virtual bool Write(CreaturesArchive &archive) const;
		virtual bool Read(CreaturesArchive &archive);
	} myReward, myPunishment;

};




#endif//Tract_H

