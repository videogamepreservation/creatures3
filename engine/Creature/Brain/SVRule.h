// SVRule.h: interface for the SVRule class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SVRule_H
#define SVRule_H


#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "BrainConstants.h"
#include "../../PersistentObject.h"
#include "../../Maths.h"
#include "../../Map/Map.h" // FastFloatToInteger
#include "../Biochemistry/BiochemistryConstants.h"

enum NeuronVariableNames {
	STATE_VAR,
	INPUT_VAR,
	OUTPUT_VAR,
	THIRD_VAR,
	FOURTH_VAR,
	FIFTH_VAR,
	SIXTH_VAR,
	NGF_VAR				// Neural Growth Factor - controls dendrite migration
};

enum DendriteVariableNames {
	WEIGHT_SHORTTERM_VAR,
	WEIGHT_LONGTERM_VAR,
	SECOND_DENDRITE_VAR,
	THIRD_DENDRITE_VAR,
	FOURTH_DENDRITE_VAR,
	FIFTH_DENDRITE_VAR,
	SIXTH_DENDRITE_VAR,
	STRENGTH_VAR		// Determines how 'permanent' the dendrite is (i.e. unwilling to migrate)
};

struct SVRuleEntry {
	int opCode;
	int operandVariable;
	int arrayIndex;
	float floatValue;
};

// used to divide an operand to get a float
const int FloatDivisor = 248;

class Genome;
class BrainComponent;

class SVRule : public PersistentObject {
	friend class BrainAccess;
	CREATURES_DECLARE_SERIAL(SVRule)
public:
	enum {
		noOfVariables=NUM_SVRULE_VARIABLES
	};
	enum {length=16};
	static SVRuleVariables invalidVariables;
	enum OpCodes {
		stopImmediately,				blankOperand,
		storeAccumulatorInto,			loadAccumulatorFrom,

		ifEqualTo,						ifNotEqualTo,
		ifGreaterThan,					ifLessThan,
		ifGreaterThanOrEqualTo,			ifLessThanOrEqualTo,

		ifZero,							ifNonZero,
		ifPositive,						ifNegative,
		ifNonNegative,					ifNonPositive,

		add,							subtract,
		subtractFrom,					multiplyBy,
		divideBy,						divideInto,
		minIntoAccumulator,				maxIntoAccumulator,

		setTendRate,					tendAccumulatorToOperandAtTendRate,
		negateOperandIntoAccumulator,	loadAbsoluteValueOfOperandIntoAccumulator,
		getDistanceTo,					flipAccumulatorAround,

		noOperation,					setToSpareNeuron,
		boundInZeroOne,					boundInMinusOnePlusOne,
		addAndStoreIn,					tendToAndStoreIn,

		// Some C2 style sliders:
		doNominalThreshold,				doLeakageRate,
		doRestState,					doInputGainLoHi,
		doPersistence,					doSignalNoise,
		doWinnerTakesAll,				doSetSTtoLTRate,

		doSetLTtoSTRateAndDoWeightSTLTWeightConvergence,	storeAbsInto,

		ifZeroStop,						ifNZeroStop,
		ifZeroGoto,						ifNZeroGoto,

		divideAndAddToNeuronInput,		mulitplyAndAddToNeuronInput,
		gotoLine,						
		
		ifLessThanStop,					ifGreaterThanStop,
		ifLessThanOrEqualStop,			ifGreaterThanOrEqualStop,

		setRewardThreshold,				setRewardRate,
		setRewardChemicalIndex,			setPunishmentThreshold,			
		setPunishmentRate,				setPunishmentChemicalIndex,

		//doSigmoidOnAccumulator,

		preserveVariable,				restoreVariable,
		preserveSpareVariable,			restoreSpareVariable,

		ifNegativeGoto,						ifPositiveGoto,

		noOfOpCodes
	};
	enum OpCodeType {
		operationTakesNoOperand, operationReadsFromAnOperand, operationWritesToAnOperand,
	};
	static OpCodeType operationTypes[noOfOpCodes];
	enum OperandCodes {
		accumulatorCode,

		inputNeuronCode,
		dendriteCode,
		neuronCode,
		spareNeuronCode,

		randomCode,

		chemicalIndexedBySourceNeuronIdCode,
		chemicalCode,
		chemicalIndexedByDestinationNeuronIdCode,

		zeroCode,
		oneCode,

		valueCode,
		negativeValueCode,
		valueTenCode,
		valueTenthCode,
		valueIntCode,

		noOfOperands
	};
	enum OperandCodeType {
		operandTakesNoIndex, operandTakesAChemicalIndex, operandTakesAVariableArrayIndex, operandIsAFloatValue,
	};
	static OperandCodeType operandTypes[noOfOperands];

	enum ProcessReturnCode {
		doNothing,
		setSpareNeuronToCurrent
	};

	SVRule();
	virtual ~SVRule();

	inline ProcessReturnCode ProcessGivenVariables(SVRuleVariables& inputVariables, SVRuleVariables& dendriteVariables, SVRuleVariables& neuronVariables, SVRuleVariables& spareNeuronVariables, int srcNeuronId, int dstNeuronId, BrainComponent* myOwner = NULL);
	void InitFromGenome(Genome& genome);
	void InitFromDesc(std::istream &in);


	virtual bool Write(CreaturesArchive &archive) const;
	virtual bool Read(CreaturesArchive &archive);

	void Dump(std::ostream& out);
	int DumpSize();

	bool SetFloat(int entryNo, float value);
	inline void RegisterBiochemistry(float* chemicals) {
		myPointerToChemicals = chemicals;
	}

protected:
	SVRuleEntry myRule[length+1];
	float *myPointerToChemicals;

	static int InitVariables();
	static int dummyVariableToGetInitVariablesCalled;

	// Reward/Punishment SVRules handled out of inline.
	void HandleSetRewardThreshold( BrainComponent* myOwner, float operand );
	void HandleSetRewardRate( BrainComponent* myOwner, float operand );
	void HandleSetRewardChemicalIndex( BrainComponent* myOwner, byte operand );
	void HandleSetPunishmentThreshold( BrainComponent* myOwner, float operand );
	void HandleSetPunishmentRate( BrainComponent* myOwner, float operand );
	void HandleSetPunishmentChemicalIndex( BrainComponent* myOwner, byte operand );
	void HandleSetSTtoLTRate( BrainComponent* myOwner, float operand );
	void HandleSetLTtoSTRateAndDoCalc( BrainComponent* myOwner, float operand, SVRuleVariables& dendriteVariables );
};



#ifdef _MSC_VER
__forceinline
#else
inline 
#endif

SVRule::ProcessReturnCode SVRule::ProcessGivenVariables(SVRuleVariables& inputVariables, 
														SVRuleVariables& dendriteVariables, 
														SVRuleVariables& neuronVariables, 
														SVRuleVariables& spareNeuronVariables, 
														int srcNeuronId, 
														int dstNeuronId, 
														BrainComponent* myOwner) 
{
	float accumulator;
	float operand;
	float* pointerOperand;
	float tendRate = 0.0f;

	ProcessReturnCode returnCode = doNothing;

	accumulator = inputVariables[0];
	for (int i=0; i<length; i++) {
		SVRuleEntry& svRuleEntry = myRule[i];

		int opCode = svRuleEntry.opCode;
		switch (operationTypes[opCode])
		{

			case operationTakesNoOperand:
				switch (opCode)
				{
					case noOperation:
						break;
					case setToSpareNeuron:
						returnCode = setSpareNeuronToCurrent;
						break;
					case stopImmediately:
						return returnCode;

					case doWinnerTakesAll:
						if (neuronVariables[STATE_VAR] >= spareNeuronVariables[STATE_VAR])
						{				// this neuron the winner so far?
							spareNeuronVariables[OUTPUT_VAR] = 0.0f;
							neuronVariables[OUTPUT_VAR] = neuronVariables[STATE_VAR];
							returnCode = setSpareNeuronToCurrent;
						}
						break;
					default:
						ASSERT(false);
				}
				continue;				// to loop outside



			case operationWritesToAnOperand:
				switch (svRuleEntry.operandVariable)
				{
					case inputNeuronCode:
						pointerOperand = &inputVariables[svRuleEntry.arrayIndex];
						break;
					case dendriteCode:
						pointerOperand = &dendriteVariables[svRuleEntry.arrayIndex];
						break;
					case neuronCode:
						pointerOperand = &neuronVariables[svRuleEntry.arrayIndex];
						break;
					case spareNeuronCode:
						pointerOperand = &spareNeuronVariables[svRuleEntry.arrayIndex];
						break;
					default:
						continue;		// to loop outside
				}
				switch (svRuleEntry.opCode)
				{
					case storeAccumulatorInto:
						*pointerOperand = BoundIntoMinusOnePlusOne(accumulator);
						break;
					case addAndStoreIn:
						*pointerOperand = BoundIntoMinusOnePlusOne(accumulator+*pointerOperand);
						break;
					case blankOperand:
						*pointerOperand = 0.0f;
						break;
					case tendToAndStoreIn:
						*pointerOperand = BoundIntoMinusOnePlusOne(
							accumulator*(1.0f-tendRate) +
							(*pointerOperand)*tendRate
						);
						break;
					case storeAbsInto:
						{
							float tmpF = accumulator<0 ? -accumulator : accumulator;
							*pointerOperand = BoundIntoZeroOne( tmpF );
							break;
						}
					default:
						ASSERT(false);
				}
				continue;



			case operationReadsFromAnOperand:
				switch (svRuleEntry.operandVariable)
				{
					case accumulatorCode:
						operand = accumulator;
						break;
					case inputNeuronCode:
						operand = inputVariables[svRuleEntry.arrayIndex];
						break;
					case dendriteCode:
						operand = dendriteVariables[svRuleEntry.arrayIndex];
						break;
					case neuronCode:
						operand = neuronVariables[svRuleEntry.arrayIndex];
						break;
					case spareNeuronCode:
						operand = spareNeuronVariables[svRuleEntry.arrayIndex];
						break;
					case randomCode:
						operand = RndFloat();
						break;

					case chemicalIndexedBySourceNeuronIdCode:
						operand = myPointerToChemicals[(svRuleEntry.arrayIndex+srcNeuronId)%NUMCHEM];
						break;
					case chemicalCode:
						operand = myPointerToChemicals[(svRuleEntry.arrayIndex)%NUMCHEM];
						break;
					case chemicalIndexedByDestinationNeuronIdCode:
						operand = myPointerToChemicals[(svRuleEntry.arrayIndex+dstNeuronId)%NUMCHEM];
						break;

					case zeroCode:
						operand = 0.0f;
						break;
					case oneCode:
						operand = 1.0f;
						break;

					case negativeValueCode:
						operand = -svRuleEntry.floatValue;
						break;
					case valueCode:
						operand = svRuleEntry.floatValue;
						break;
					case valueTenCode:
						operand = svRuleEntry.floatValue * 10.0f;
						break;
					case valueTenthCode:
						operand = svRuleEntry.floatValue / 10.0f;
						break;
					case valueIntCode:
						operand = (float) ( Map::FastFloatToInteger ((svRuleEntry.floatValue * FloatDivisor)) );
						break;
					default:
						operand = 0.0f;
						break;
				}

				switch (svRuleEntry.opCode)
				{
					case loadAccumulatorFrom:
						accumulator = operand;
						break;

					case ifNotEqualTo:
						if (accumulator==operand) i++;
						break;
					case ifEqualTo:
						if (accumulator!=operand) i++;
						break;
					case ifGreaterThanOrEqualTo:
						if (accumulator<operand) i++;
						break;
					case ifLessThanOrEqualTo:
						if (accumulator>operand) i++;
						break;
					case ifGreaterThan:
						if (accumulator<=operand) i++;
						break;
					case ifLessThan:
						if (accumulator>=operand) i++;
						break;

					case ifNonZero:
						if (operand==0.0f) i++;
						break;
					case ifZero:
						if (operand!=0.0f) i++;
						break;
					case ifNonNegative:
						if (operand<0.0f) i++;
						break;
					case ifNonPositive:
						if (operand>0.0f) i++;
						break;
					case ifPositive:
						if (operand<=0.0f) i++;
						break;
					case ifNegative:
						if (operand>=0.0f) i++;
						break;

					case ifZeroStop:
						if (operand==0)	return returnCode;
						break;
					case ifNZeroStop:
						if (operand!=0)	return returnCode;
						break;

					case ifLessThanStop:
						if (accumulator<operand)	return returnCode;
						break;
					case ifGreaterThanStop:
						if (accumulator>operand)	return returnCode;
						break;
					case ifLessThanOrEqualStop:
						if (accumulator<=operand)	return returnCode;
						break;
					case ifGreaterThanOrEqualStop:
						if (accumulator>=operand)	return returnCode;
						break;

					case ifZeroGoto:
						if (accumulator==0)
						{
							int newLoc = Map::FastFloatToInteger(operand) - 1;

							// Do not over-run the end of the SVRule
							if (newLoc > length)
								return returnCode;

							// Or undershoot (only jump when line is later than current)
							if (newLoc > i) 
								i = newLoc - 1;
						}
						break;
					case ifNZeroGoto:
						if (accumulator!=0)
						{
							int newLoc = Map::FastFloatToInteger(operand) - 1;

							// Do not over-run the end of the SVRule
							if (newLoc > length)
								return returnCode;

							// Or undershoot (only jump when line is later than current)
							if (newLoc > i) 
								i = newLoc - 1;
						}
						break;
					case ifNegativeGoto:
						if (accumulator<0)
						{
							int newLoc = Map::FastFloatToInteger(operand) - 1;

							// Do not over-run the end of the SVRule
							if (newLoc > length)
								return returnCode;

							// Or undershoot (only jump when line is later than current)
							if (newLoc > i) 
								i = newLoc - 1;
						}
						break;
					case ifPositiveGoto:
						if (accumulator>0)
						{
							int newLoc = Map::FastFloatToInteger(operand) - 1;

							// Do not over-run the end of the SVRule
							if (newLoc > length)
								return returnCode;

							// Or undershoot (only jump when line is later than current)
							if (newLoc > i) 
								i = newLoc - 1;
						}
						break;
					case gotoLine:
						{
							int newLoc = Map::FastFloatToInteger(operand) - 1;

							// Do not over-run the end of the SVRule
							if (newLoc > length)
								return returnCode;

							// Or undershoot (only jump when line is later than current)
							if (newLoc > i) 
								i = newLoc - 1;
							// -1 coz i is incremented by for loop
						}
						break;
					case divideAndAddToNeuronInput:
						// Divide accumulator by operand and add result to neuron input variable
						if (operand!=0)
						{
							accumulator /= operand;
							pointerOperand = &neuronVariables[INPUT_VAR];
							*pointerOperand = BoundIntoMinusOnePlusOne(accumulator+*pointerOperand);
						}
						break;
					case mulitplyAndAddToNeuronInput:
						// Multiply accumulator by operand and add result to neuron input variable
						accumulator *= operand;
						pointerOperand = &neuronVariables[INPUT_VAR];
						*pointerOperand = BoundIntoMinusOnePlusOne(accumulator+*pointerOperand);
						break;

					case setRewardThreshold:
						HandleSetRewardThreshold( myOwner, operand );
						break;
					case setRewardRate:
						HandleSetRewardRate( myOwner, operand );
						break;
					case setRewardChemicalIndex:
						HandleSetRewardChemicalIndex( myOwner, operand );
						break;
					case setPunishmentThreshold:
						HandleSetPunishmentThreshold( myOwner, operand );
						break;
					case setPunishmentRate:
						HandleSetPunishmentRate( myOwner, operand );
						break;
					case setPunishmentChemicalIndex:
						HandleSetPunishmentChemicalIndex( myOwner, operand );
						break;


					case add:
						accumulator += operand;
						break;
					case subtract:
						accumulator -= operand;
						break;
					case subtractFrom:
						accumulator = operand - accumulator;
						break;
					case multiplyBy:
						accumulator *= operand;
						break;
					case divideBy:
						if (operand!=0)
							accumulator /= operand;
						break;
					case divideInto:
						if (accumulator!=0)
							accumulator = operand/accumulator;
						break;
					case maxIntoAccumulator:
						if (operand>accumulator) accumulator=operand;
						break;
					case minIntoAccumulator:
						if (operand<accumulator) accumulator=operand;
						break;

					case tendAccumulatorToOperandAtTendRate:
						accumulator =
							accumulator*(1.0f-tendRate) +
							operand*tendRate;
						break;
					case setTendRate:
						tendRate = fabs(operand);
						break;

					case negateOperandIntoAccumulator:
						accumulator = -operand;
						break;
					case loadAbsoluteValueOfOperandIntoAccumulator:
						accumulator = operand<0 ? -operand : operand;
						break;
					case getDistanceTo:
						accumulator = accumulator>operand ?
							accumulator-operand :
							operand-accumulator;
						break;
					case flipAccumulatorAround:
						accumulator = operand - accumulator;
						break;

					case boundInZeroOne:
						accumulator = BoundIntoZeroOne(operand);
						break;
					case boundInMinusOnePlusOne:
						accumulator = BoundIntoMinusOnePlusOne(operand);
						break;
					/*
					case doSigmoidOnAccumulator:
						if( operand >= 0 ) {
							float exponent = exp( -operand * accumulator );
							//accumulator = (1.0 - exponent) / (1.0 + exponent);
							accumulator = (float) ( 2.0 / ( 1.0 + exponent ) );
						}
						break;
					*/
					case preserveVariable:
						{
							int varIdx = Map::FastFloatToInteger(operand) % NUM_SVRULE_VARIABLES;
							neuronVariables[FOURTH_VAR] = neuronVariables[varIdx];
							break;
						}
					case restoreVariable:
						{
							int varIdx = Map::FastFloatToInteger(operand) % NUM_SVRULE_VARIABLES;
							neuronVariables[varIdx] = neuronVariables[FOURTH_VAR];
							break;
						}
					case preserveSpareVariable:
						{
							int varIdx = Map::FastFloatToInteger(operand) % NUM_SVRULE_VARIABLES;
							spareNeuronVariables[FOURTH_VAR] = spareNeuronVariables[varIdx];
							break;
						}
					case restoreSpareVariable:
						{
							int varIdx = Map::FastFloatToInteger(operand) % NUM_SVRULE_VARIABLES;
							spareNeuronVariables[varIdx] = spareNeuronVariables[FOURTH_VAR];
							break;
						}


					// C2-style sliders:
					case doNominalThreshold:								// Threshold the input:
						if (neuronVariables[INPUT_VAR] < operand)
							neuronVariables[INPUT_VAR] = 0.0f;
						break;
					case doLeakageRate:										// Leakage (relaxation) rate:
						tendRate = operand;
						break;
					case doRestState:										// Rest state:
						neuronVariables[INPUT_VAR] =
							neuronVariables[INPUT_VAR]*(1.0f-tendRate) + 
							operand*tendRate;
						break;
					case doInputGainLoHi:									// Input attentuation:
						neuronVariables[INPUT_VAR] *= operand;
						break;

					case doPersistence:
						neuronVariables[STATE_VAR] =
							neuronVariables[INPUT_VAR]*(1.0f-operand) + 
							neuronVariables[STATE_VAR]*operand;
						break;
					case doSignalNoise:
						neuronVariables[STATE_VAR] +=
							operand * RndFloat();
						break;

					case doSetSTtoLTRate:
						HandleSetSTtoLTRate( myOwner, operand );
						break;
					case doSetLTtoSTRateAndDoWeightSTLTWeightConvergence:
						HandleSetLTtoSTRateAndDoCalc( myOwner, operand, dendriteVariables );
						break;

					default:
						ASSERT(FALSE);
						break;
				}
				continue;

			default:
				ASSERT(false);
		}
	}
	return returnCode;
}

#endif//SVRule_H

