// BrainComponent.cpp: implementation of the BrainComponent class.
//
//////////////////////////////////////////////////////////////////////

#include "BrainComponent.h"
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


CREATURES_IMPLEMENT_SERIAL(BrainComponent)


// ------------------------------------------------------------------------
// Function:    (constructor)
// Class:       BrainComponent
// Description: 
// ------------------------------------------------------------------------
BrainComponent::BrainComponent()
{
	mySupportReinforcementFlag = false;
	myRunInitRuleAlwaysFlag = false;
	myIdInList = -1;

	myInitialised = false;
}

// ------------------------------------------------------------------------
// Function:    (destructor)
// Class:       BrainComponent
// Description: 
// ------------------------------------------------------------------------
BrainComponent::~BrainComponent() {}


// ------------------------------------------------------------------------
// Function:    RegisterBiochemistry
// Class:       BrainComponent
// Description: 
// Arguments:   float* chemicals = 
// ------------------------------------------------------------------------
void BrainComponent::RegisterBiochemistry(float* chemicals)
{
	myPointerToChemicals = chemicals;
	myInitRule.RegisterBiochemistry(chemicals);
	myUpdateRule.RegisterBiochemistry(chemicals);
}



// ------------------------------------------------------------------------
// Function:    xIsProcessedBeforeY
// Class:       BrainComponent
// Description: 
// Arguments:   BrainComponent* x = 
//              BrainComponent* y = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool BrainComponent::xIsProcessedBeforeY(BrainComponent* x, BrainComponent* y) {
	return x->myUpdateAtTime < y->myUpdateAtTime;
}


// ------------------------------------------------------------------------
// Function:    GetUpdateAtTime
// Class:       BrainComponent
// Description: 
// Returns:     int = 
// ------------------------------------------------------------------------
int BrainComponent::GetUpdateAtTime()
{
	return myUpdateAtTime;
}

// ------------------------------------------------------------------------
// Function:    Write
// Class:       BrainComponent
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool BrainComponent::Write(CreaturesArchive &archive) const
{
	archive << myIdInList;
	archive << myUpdateAtTime;

	archive << mySupportReinforcementFlag;
	archive << myRunInitRuleAlwaysFlag;
	archive << myInitialised;

	myInitRule.Write( archive );
	myUpdateRule.Write( archive );
	return true;
}

// ------------------------------------------------------------------------
// Function:    Read
// Class:       BrainComponent
// Description: 
// Arguments:   CreaturesArchive &archive = 
// Returns:     bool = 
// ------------------------------------------------------------------------
bool BrainComponent::Read(CreaturesArchive &archive)
{
	int32 version = archive.GetFileVersion();


	if(version >= 3)
	{
		archive >> myIdInList;
		archive >> myUpdateAtTime;

		archive >> mySupportReinforcementFlag;
		archive >> myRunInitRuleAlwaysFlag;
		archive >> myInitialised;

		if(!myInitRule.Read( archive ))
			return false;

		if(!myUpdateRule.Read( archive ))
			return false;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}


