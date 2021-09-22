/*********************************************************************
* File:     CPUID.h
* Created:  10/6/98
* Author:   Robin E. Charlton
* 
* Originates from Intel Corp.
* Possibly placed in "WinMain":
*	gProcessorType = GetProcessorType();
*	gHasMMXTechnology = CheckMMXTechnology();
*
*********************************************************************/

#ifndef _WIN32
#error non-win32 version needs work...
#endif

#ifndef CPUID_H
#define CPUID_H

const DWORD CPU_TYPE =      0x00003000;
const DWORD CPU_FAMILY =    0x00000f00;
const DWORD CPU_MODEL =     0x000000f0;
const DWORD CPU_STEPPING =  0x0000000f;

extern DWORD GetProcessorType(void);
extern BOOL CheckMMXTechnology(void);

#endif // CPUID_H
