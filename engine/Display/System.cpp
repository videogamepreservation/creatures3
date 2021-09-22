// This #define is needed for DirectX
// It must be present in exactly one
// translation unit (cpp file) which 
// includes DirectX stuff
#define		INITGUID

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "DisplayEngine.h"
#include "../Sound/MidiModule.h"

