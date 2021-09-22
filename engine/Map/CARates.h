#ifndef  CARATES_H
#define  CARATES_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../PersistentObject.h"

class CARates : public PersistentObject {
	CREATURES_DECLARE_SERIAL(CARates)
public:
	CARates()
		: myGain( 0.0f ), myLoss( 0.0f ), myDiffusion( 0.0f ), myDiffusionRoot( 0.0f ) {}
	CARates( float gain, float loss, float diffusion )
		: myGain( gain ), myLoss( loss ), myDiffusion( diffusion ), myDiffusionRoot( sqrtf(diffusion) ) {}

	virtual inline bool Write(CreaturesArchive &ar) const
	{
		ar << myGain;
		ar << myLoss;
		ar << myDiffusion;
		ar << myDiffusionRoot;
		return true;
	}

	virtual inline bool Read(CreaturesArchive &ar) 
	{
		ar >> myGain;
		ar >> myLoss;
		ar >> myDiffusion;
		ar >> myDiffusionRoot;
		return true;
	}

	inline float GetGain() const { return myGain; }
	inline float GetLoss() const { return myLoss; }
	inline float GetDiffusion() const { return myDiffusion; }
	inline float GetDiffusionRoot() const { return myDiffusionRoot; }

	inline void SetGain( float gain ) { myGain = gain; }
	inline void SetLoss( float loss ) { myLoss = loss; }
	inline void SetDiffusion( float diffusion ) { myDiffusion = diffusion; myDiffusionRoot = sqrtf(diffusion); }

private:
	float myGain;
	float myLoss;
	float myDiffusion;
	float myDiffusionRoot;
};

#endif
