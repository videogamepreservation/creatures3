#ifndef ROOMCA_H
#define ROOMCA_H
#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


class CARates;

void UpdateRoomCA(CARates const &rates, float inputFromObjectsInRoom, float &newValue, float &tempValue);

void UpdateDoorCA(float tempValue1, CARates const &rates1, float relativeDoorSize1,
				  float tempValue2, CARates const &rates2, float relativeDoorSize2,
				  float &newValue1, float &newValue2 );

#endif // ROOMCA_H
