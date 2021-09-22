/*********************************************************************
* File:     Receptor.h
* Created:  13/01/98
* Author:   Robin E. Charlton
* 
*********************************************************************/

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "Receptor.h"


CREATURES_IMPLEMENT_SERIAL(Receptor)


/*********************************************************************
* Public: Default constructor.
*********************************************************************/
Receptor::Receptor()
{
	Chem =  Effect = 0;
	Threshold = Nominal = Gain = 0.0f;
	Dest = NULL;
	isClockRateReceptor = false;
}


// ----------------------------------------------------------------------
// Method:		Write
// Arguments:	archive - archive being written to
// Returns:		true if successful
// Description:	Overridable function - writes details to archive,
//				taking serialisation into account
// ----------------------------------------------------------------------
bool Receptor::Write(CreaturesArchive &archive) const {

	archive << IDOrgan;
	archive << IDTissue;
	archive << IDLocus;

	archive << Chem;
	archive << Threshold;
	archive << Nominal;
	archive << Gain;
	archive << Effect;

	archive.WriteFloatRef( Dest );
	archive << isClockRateReceptor;
	return true;
}

// ----------------------------------------------------------------------
// Method:		Read
// Arguments:	archive - archive being read from
// Returns:		true if successful
// Description:	Overridable function - reads detail of class from archive
// ----------------------------------------------------------------------
bool Receptor::Read(CreaturesArchive &archive)	{


	int32 version = archive.GetFileVersion();

	if(version >= 3)
	{

		archive >> IDOrgan;
		archive >> IDTissue;
		archive >> IDLocus;

		archive >> Chem;
		archive >> Threshold;
		archive >> Nominal;
		archive >> Gain;
		archive >> Effect;

		archive.ReadFloatRef( Dest );
		archive >> isClockRateReceptor;
	}
	else
	{
		_ASSERT(false);
		return false;
	}
	return true;
}
