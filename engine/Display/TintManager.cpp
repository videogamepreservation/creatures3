#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "TintManager.h"

#include "Bitmap.h"
#include "DisplayEngine.h"
#include "../C2eServices.h"

#ifdef _WIN32
#include "../../common/zlib113/zlib.h"
#else
#include <zlib.h>
#endif

#define BOUND(c) if(c<0)c=0;else if (c>255)c=255;

CREATURES_IMPLEMENT_SERIAL( TintManager )

TintManager::TintManager()
{
	myRedTint = myBlueTint = myGreenTint = mySwap = myRotation = 128;
	for(int colour = 0;colour < 65536; colour++)
		myTintTable[colour] = colour;
}

TintManager::~TintManager()
{

}

CreaturesArchive& operator<<( CreaturesArchive &ar, TintManager const &thisManager )
{
	thisManager.Write(ar);
	return ar;
}

CreaturesArchive& operator>>( CreaturesArchive &ar, TintManager &thisManager )
{
	thisManager.Read(ar);
	return ar;
}

bool TintManager::Write( CreaturesArchive &ar ) const
{
	// New style...
	uint16* myCompressedSpace = new uint16[128*1024];
	uint32 csize = 128*1024*sizeof(uint16);
	uint32 temp;
	if ( compress((uint8*)myCompressedSpace,&csize,(uint8*)myTintTable,65536*sizeof(uint16)) != Z_OK)
	{
		temp = 1;
		ar << temp;
		ar << myRedTint << myGreenTint << myBlueTint << myRotation << mySwap;
	}
	else
	{
		temp = 2;
		ar << temp;
		ar << csize;
		ar.Write(myCompressedSpace,csize);
		ar << myRedTint << myGreenTint << myBlueTint << myRotation << mySwap;
	}
	delete [] myCompressedSpace;
	
	return true;
}

bool TintManager::Read( CreaturesArchive &ar )
{
	int32 version = ar.GetFileVersion();
	
	if(version >= 3)
	{
		uint32 size;
		ar >> size;
		if (size == 2)
		{
			uint16* myCompressedData = new uint16[128*1024];
			uint32 csize;
			ar >> csize;
			ar.Read(myCompressedData,csize);
			ar >> myRedTint >> myGreenTint >> myBlueTint >> myRotation >> mySwap;
			size = 65536 * sizeof(uint16);
			if ( uncompress((uint8*)myTintTable,&size,(uint8*)myCompressedData,csize) != Z_OK )
			{
				// Oh bugger - fecked tinttable on load :(:(
				BuildTintTable(myRedTint,myGreenTint,myBlueTint,myRotation,mySwap);
			}
			delete [] myCompressedData;
		}
		else if (size == 1)
		{
			ar >> myRedTint >> myGreenTint >> myBlueTint >> myRotation >> mySwap;
			BuildTintTable(myRedTint,myGreenTint,myBlueTint,myRotation,mySwap);
		}
	}
	else
	{
		_ASSERT(false);
		return false;
	}

	return true;
}


void TintManager::BuildTintTable(int16 redTint, int16 greenTint, int16 blueTint, int16 rotation, int16 swap)
{
	// Remember to manage by 565/555

	myRedTint = redTint;
	myGreenTint = greenTint;
	myBlueTint = blueTint;
	myRotation = rotation;
	mySwap = swap;

	if (redTint == 128 && blueTint == 128 && greenTint == 128 && rotation == 128 && swap == 128)
	{
		for(int colour = 0;colour < 65536; colour++)
			myTintTable[colour] = colour;
		return;
	}


	// OutputFormattedDebugString("Building of Tint table of (%d,%d,%d) : %d : %d ...",redTint,greenTint,blueTint,rotation,swap);

	bool is565 = DisplayEngine::theRenderer().GetMyPixelFormat() == RGB_565;

	uint16 absRot = (rotation >= 128)? rotation - 128:128 - rotation;
	uint16 invRot = 127 - absRot;
	uint16 absSwap = (swap >= 128)? swap - 128:128 - swap;
	uint16 invSwap = 127 - absSwap;

	uint16 destinationColour;
	uint8 ar, ag, ab =0;
	int32 r,g,b = 0;
	int32 rr,rg,rb = 0;
	int32 sr,sb = 0;

	myTintTable[0] = 0;

	redTint -= 128;
	greenTint -= 128;
	blueTint -= 128;

	for(int32 colour = 1; colour < (is565?65536:32678); colour++)
	{
		
		if (is565)
		{
			P565_TO_RGB(colour,ar,ag,ab)
		}
		else
		{
			P555_TO_RGB(colour,ar,ag,ab)
		}

		// Now we have a colour. Let's tint it :)

		r = ar + redTint;
		g = ag + greenTint;
		b = ab + blueTint;

		// Woohoo, tinted :):)

		// Ceiling / Floor it...
		BOUND(r)
		BOUND(g)
		BOUND(b)

		// Next the task is to make them HSV? do we need this? Rot/Swap
		// Perhaps the trick will be to make it cunningly without H,S & V.

		
		// Uses C2 (fecked?) code for now :)
		if ( rotation < 128 )
		{
			rr = ( (absRot * b) + (invRot * r) ) >> 7;
			rg = ( (absRot * r) + (invRot * g) ) >> 7;
			rb = ( (absRot * g) + (invRot * b) ) >> 7;
		}	
		else
		{
			rr = ( (absRot * g) + (invRot * r) ) >> 7;
			rg = ( (absRot * b) + (invRot * g) ) >> 7;
			rb = ( (absRot * r) + (invRot * b) ) >> 7;
		}
		sr = ( (absSwap * rb) + (invSwap * rr) ) >> 7;
		sb = ( (absSwap * rr) + (invSwap * rb) ) >> 7;

		BOUND(sr)
		BOUND(rg)
		BOUND(sb)

		if (is565)
		{
			RGB_TO_565(sr,rg,sb,destinationColour)
		}
		else
		{
			RGB_TO_555(sr,rg,sb,destinationColour)
		}
		
		myTintTable[colour] = (destinationColour == 0)?1:destinationColour;
	}
	// OutputFormattedDebugString("Done\n");	
}

void TintManager::TintBitmap(Bitmap* thisBitmap) const
{
	// Really easy :):)
	int32 w,h;
	w = thisBitmap->GetWidth();
	h = thisBitmap->GetHeight();

	uint16* bitmapData = thisBitmap->GetData();

	for(int i = 0; i < (w*h); i++)
		*bitmapData++ = myTintTable[*bitmapData];	
}
