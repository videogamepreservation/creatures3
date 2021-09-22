#ifndef TINTMANAGER_H
#define TINTMANAGER_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../PersistentObject.h"
#include "../../common/C2eTypes.h"

class Bitmap;

class TintManager : public PersistentObject
{
	CREATURES_DECLARE_SERIAL( TintManager )

public:

	TintManager();
	~TintManager();

	virtual bool Write( CreaturesArchive &ar ) const;
	virtual bool Read( CreaturesArchive &ar );

	void BuildTintTable(int16 redTint, int16 greenTint, int16 blueTint, int16 rotation, int16 swap);
	void TintBitmap(Bitmap* thisBitmap) const;

	const uint16* GetMyTintTable() const { return myTintTable; }
private:

	// My Tint Tables

	uint16 myTintTable[65536];

	// Specifications for tint table for use in serialisation etc.

	uint8 myRedTint,myGreenTint,myBlueTint,myRotation,mySwap;

	friend CreaturesArchive& operator<<( CreaturesArchive &ar, TintManager const &thisManager );
	friend CreaturesArchive& operator>>( CreaturesArchive &ar, TintManager &thisManager );
};

CreaturesArchive& operator<<( CreaturesArchive &ar, TintManager const &thisManager );

CreaturesArchive& operator>>( CreaturesArchive &ar, TintManager &thisManager );

#endif // TINTMANAGER_H