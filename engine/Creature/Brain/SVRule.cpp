// SVRule.cpp: implementation of the SVRule class.
//
//////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "SVRule.h"
#include "BrainIO.h"
#include "../Genome.h"
#include "Tract.h"



SVRule::OpCodeType SVRule::operationTypes[noOfOpCodes] =
{
	operationTakesNoOperand,		operationWritesToAnOperand,
	operationWritesToAnOperand,		operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	operationTakesNoOperand,		operationTakesNoOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationWritesToAnOperand,		operationWritesToAnOperand,

// C2 style ones:
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationTakesNoOperand,		operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationWritesToAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	//operationReadsFromAnOperand,	
	
	operationReadsFromAnOperand,	operationReadsFromAnOperand,
	operationReadsFromAnOperand,	operationReadsFromAnOperand,

	operationReadsFromAnOperand,	operationReadsFromAnOperand
};

SVRule::OperandCodeType SVRule::operandTypes[noOfOperands] =
{
	operandTakesNoIndex,

	operandTakesAVariableArrayIndex,
	operandTakesAVariableArrayIndex,
	operandTakesAVariableArrayIndex,
	operandTakesAVariableArrayIndex,

	operandTakesNoIndex,

	operandTakesAChemicalIndex,
	operandTakesAChemicalIndex,
	operandTakesAChemicalIndex,

	operandTakesNoIndex,
	operandTakesNoIndex,

	operandIsAFloatValue,
	operandIsAFloatValue,
	operandIsAFloatValue
};


CREATURES_IMPLEMENT_SERIAL(SVRule)

/*****************************************/
/*******       SVRule class         ******/
/*****************************************/

SVRuleVariables SVRule::invalidVariables;
int SVRule::dummyVariableToGetInitVariablesCalled = InitVariables();
// ------------------------------------------------------------------------
// Function:    InitVariables
// Class:       SVRule
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int SVRule::InitVariables()
{
	for (int i=0; i<NUM_SVRULE_VARIABLES; i++)
	{
		invalidVariables[i] = 0.0f;
	}
	return 0;
}

// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       SVRule
// Description: 
// ------------------------------------------------------------------------
SVRule::SVRule()
{
	myRule[length].opCode = stopImmediately;	// just in case
}
// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       SVRule
// Description: 
// ------------------------------------------------------------------------
SVRule::~SVRule() {}


// ------------------------------------------------------------------------
// Function:    InitFromGenome
// Class:       SVRule
// Description: 
// Arguments:   Genome& genome = 
// ------------------------------------------------------------------------
void SVRule::InitFromGenome(Genome& genome)
{
	for	(int i=0; i<length; i++) {
		SVRuleEntry& e = myRule[i];


		// get opcode:
		e.opCode = genome.GetCodonLessThan(noOfOpCodes);

		// operand:
		e.operandVariable = genome.GetCodonLessThan(noOfOperands);
		ASSERT(e.operandVariable < noOfOperands);

		// array index (even if there isn't one really):
		OperandCodeType t = operandTypes[e.operandVariable];
		e.arrayIndex = genome.GetCodonLessThan(
			t==operandTakesAVariableArrayIndex ? NUM_SVRULE_VARIABLES : 256
		);

		// float value (even if not needed)
		e.floatValue = ((float)e.arrayIndex)/FloatDivisor;
		if (e.floatValue>1.0f)
			e.floatValue = 1.0f;
	}
}


// ------------------------------------------------------------------------
// Function:    InitFromDesc
// Class:       SVRule
// Description: 
// Arguments:   std::istream &in = 
// ------------------------------------------------------------------------
void SVRule::InitFromDesc(std::istream &in)
{
	for	(int i=0; i<length; i++)
	{
		SVRuleEntry& e = myRule[i];

		in.get((char &)e.opCode);
		in.get((char &)e.operandVariable);
		in.get((char &)e.arrayIndex);
		ReadDesc(&e.floatValue, in);
	}
}


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Write
// Class:       SVRule
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool SVRule::Write(CreaturesArchive &archive) const
{
	for	(int i=0; i<length; i++) {
		const SVRuleEntry& e = myRule[i];
		archive << e.opCode << e.operandVariable <<	e.arrayIndex << e.floatValue;
	}
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
// ------------------------------------------------------------------------
// Function:    Read
// Class:       SVRule
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool SVRule::Read(CreaturesArchive &archive) 
{
	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		for	(int i=0; i<length; i++) {
			SVRuleEntry& e = myRule[i];
			archive >> e.opCode >> e.operandVariable >>	e.arrayIndex >> e.floatValue;
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}

// ------------------------------------------------------------------------
// Function:    Dump
// Class:       SVRule
// Description: 
// Arguments:   std::ostream& out = 
// ------------------------------------------------------------------------
void SVRule::Dump(std::ostream& out)
{
	for	(int i=0; i<length; i++) {
		SVRuleEntry& e = myRule[i];

		out.put(e.opCode);
		out.put(e.operandVariable);
		out.put(e.arrayIndex);
		WriteDesc(&e.floatValue, out);
	}
};


// ------------------------------------------------------------------------
// Function:    DumpSize
// Class:       SVRule
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int SVRule::DumpSize()
{
	int length = 0;
	char string[20];

	for	(int i=0; i<length; i++) {
		SVRuleEntry& e = myRule[i];
#ifdef MSC_VER
		length += 1+strlen(_itoa(e.opCode, string, 10));
		length += 1+strlen(_itoa(e.operandVariable, string, 10));
		length += 1+strlen(_itoa(e.arrayIndex, string, 10));
#else
		// is itoa() standard? not sure...
		sprintf( string, "%d", e.opCode );
		length += 1+strlen( string );
		sprintf( string, "%d", e.operandVariable );
		length += 1+strlen( string );
		sprintf( string, "%d", e.arrayIndex );
		length += 1+strlen( string );
#endif
		char floatStr[7];
		sprintf(floatStr, "%1.3f", e.floatValue);
		length = 1+strlen(floatStr);
	}

	return length;
};

// ------------------------------------------------------------------------
// Function:    SetFloat
// Class:       SVRule
// Description: 
// Arguments:   int entryNo = 
//              float value = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool SVRule::SetFloat(int entryNo, float value)
{
	if(entryNo < 0 || entryNo > length || value < -1 || value > 1)
		return false;

	myRule[entryNo].floatValue = value;
	return true;
}

// SVRules for reward and punishment have to be outside the inline function because they
// access BrainComponent member functions and BrainComponents need to have SVRule defined
// before they are.
// ------------------------------------------------------------------------
// Function:    HandleSetRewardThreshold
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              float operand  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetRewardThreshold( BrainComponent* myOwner, float operand )
{
	if( myOwner )
	{
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			ownerTract->myReward.SetThreshold( BoundIntoMinusOnePlusOne(operand) );
		}
	}
}

// ------------------------------------------------------------------------
// Function:    HandleSetRewardRate
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              float operand  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetRewardRate( BrainComponent* myOwner, float operand )
{
	if( myOwner )
	{
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			ownerTract->myReward.SetRate( BoundIntoMinusOnePlusOne(operand) );
		}
	}
}

// ------------------------------------------------------------------------
// Function:    HandleSetRewardChemicalIndex
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              byte operand  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetRewardChemicalIndex( BrainComponent* myOwner, byte operand )
{
	if( myOwner )
	{
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			ownerTract->myReward.SetChemicalIndex( operand % NUMCHEM );
			ownerTract->myReward.SetDendritesSupportFlag(true);
		}
	}
}

// ------------------------------------------------------------------------
// Function:    HandleSetPunishmentThreshold
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              float operand  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetPunishmentThreshold( BrainComponent* myOwner, float operand )
{
	if( myOwner )
	{
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			ownerTract->myPunishment.SetThreshold( BoundIntoMinusOnePlusOne(operand) );
		}
	}
}

// ------------------------------------------------------------------------
// Function:    HandleSetPunishmentRate
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              float operand  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetPunishmentRate( BrainComponent* myOwner, float operand )
{
	if( myOwner )
	{
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			ownerTract->myPunishment.SetRate( BoundIntoMinusOnePlusOne(operand) );
		}
	}
}

// ------------------------------------------------------------------------
// Function:    HandleSetPunishmentChemicalIndex
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              byte operand  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetPunishmentChemicalIndex( BrainComponent* myOwner, byte operand )
{
	if( myOwner ) {
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			ownerTract->myPunishment.SetChemicalIndex( operand % NUMCHEM );
			ownerTract->myPunishment.SetDendritesSupportFlag(true);
		}
	}
}

// ------------------------------------------------------------------------
// Function:    HandleSetSTtoLTRate
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              float operand  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetSTtoLTRate( BrainComponent* myOwner, float operand )
{
	if( myOwner )
	{
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			ownerTract->STtoLTRate = fabs(operand);
		}
	}
}

// ------------------------------------------------------------------------
// Function:    HandleSetLTtoSTRateAndDoCalc
// Class:       SVRule
// Description: 
// Arguments:   BrainComponent* myOwner = 
//              float operand = 
//              SVRuleVariables& dendriteVariables  = 
// ------------------------------------------------------------------------
void SVRule::HandleSetLTtoSTRateAndDoCalc( BrainComponent* myOwner, float operand, SVRuleVariables& dendriteVariables )
{
	if( myOwner )
	{
		if( myOwner->SupportsReinforcement() )
		{
			Tract* ownerTract = (Tract*) myOwner;
			// Tend STW towards LTW are rate STtoLTRate
			// Tend LTW towards STW are rate LTtoSTRate
			float LTtoSTRate = operand;
			float oldSTW = dendriteVariables[WEIGHT_SHORTTERM_VAR];
			float oldLTW = dendriteVariables[WEIGHT_LONGTERM_VAR];
			dendriteVariables[WEIGHT_SHORTTERM_VAR] += (oldLTW - oldSTW) * ownerTract->STtoLTRate;
			dendriteVariables[WEIGHT_LONGTERM_VAR] += (oldSTW - oldLTW) * LTtoSTRate;
		}
	}
}
