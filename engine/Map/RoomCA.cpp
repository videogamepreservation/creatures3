#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "RoomCA.h"
#include "CARates.h"
#include <math.h>

void UpdateRoomCA
	(CARates const &rates, 
	 float inputFromObjectsInRoom, 
	 float &newValue,
	 float &tempValue)
{
	// Ensure input used between 0.0 and 1.0
	if( inputFromObjectsInRoom < 0.0f ) 
		inputFromObjectsInRoom = 0.0f;
	float adjustedInput = 1.0f - 1.0f / (inputFromObjectsInRoom + 1.0f);
	float lossOrGainRate = newValue>adjustedInput ?
		rates.GetLoss() :
		rates.GetGain();
	tempValue =
		lossOrGainRate*adjustedInput +
		(1.0f-lossOrGainRate)*newValue;
	newValue = 0.0f;
}

void UpdateDoorCA(float tempValue1, CARates const &rates1, float relativeDoorSize1,
				  float tempValue2, CARates const &rates2, float relativeDoorSize2,
				  float &newValue1, float &newValue2 )
{
	register float diffusionRate =
		rates1.GetDiffusionRoot()*rates2.GetDiffusionRoot()/* * opening*/;
	register float averageValue = (tempValue1+tempValue2)/2.0f;
	register float diffusionRateInverted = 1.0f-diffusionRate;
	register float temp = averageValue*diffusionRate;
	newValue1 +=
		relativeDoorSize1*(tempValue1*diffusionRateInverted + temp);
	newValue2 +=
		relativeDoorSize2*(tempValue2*diffusionRateInverted + temp);
}