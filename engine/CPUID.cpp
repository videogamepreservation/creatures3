/*********************************************************************
* File:     CPUID.cpp
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
#error not ported
#endif

#ifdef _MSC_VER

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif


#include "../common/C2eTypes.h"


// Globals:
BOOL gIsPentiumProcessor = FALSE;
BOOL gIsPentiumProProcessor = FALSE;
BOOL gIsPentiumIIProcessor = FALSE;
BOOL gHasMMXTechnology = FALSE;
DWORD gProcessorType;

/*********************************************************************
* Global: GetProcessorType.
* return values: 
*	Type (bits 13-12), Family (bits 11-8), Model (bits 7-4), Stepping (bits 3-0)
*
*	T = 00, F = 0101, M = 0001, for Pentium Processors (60, 66 MHz)
*	T = 00, F = 0101, M = 0010, for Pentium Processors (75, 90, 100, 120, 133, 150, 166, 200 MHz)
*	T = 00, F = 0101, M = 0100, for Pentium Processors with MMX technology
*
*	T = 00, F = 0110, M = 0001, for Pentium Pro Processor
*	T = 00, F = 0110, M = 0011, for Pentium II Processor
*
*	T = 00 for original OEM processor
*	T = 01 for Intel OverDrive Processor
*	T = 10 for dual processor
*	T = 11 is reserved
*********************************************************************/
DWORD GetProcessorType(void)
{
	DWORD retval;
	__try { 
		_asm {
            push ebx
			mov eax, 1		// set up CPUID to return processor version and features
						    //	0 = vendor string, 1 = version info, 2 = cache info
		    __asm _emit 0x0f    // CPUID
		    __asm _emit 0xa2
			and eax, 03fffh		// type, family, model, stepping returned in eax
			mov retval, eax
            pop ebx
		}
	} __except(EXCEPTION_EXECUTE_HANDLER) {retval = 0;}

	return retval;
}


/*********************************************************************
* Global: CheckMMXTechnology.
*********************************************************************/
BOOL CheckMMXTechnology(void)
{
	BOOL retval = TRUE;
	DWORD RegEDX;

	__try {
		_asm {
            push ebx
			mov eax, 1	// set up CPUID to return processor version and features
					//	0 = vendor string, 1 = version info, 2 = cache info
		    __asm _emit 0x0f    // CPUID
		    __asm _emit 0xa2
			mov RegEDX, edx	// features returned in edx
            pop ebx
	   	}

   	} __except(EXCEPTION_EXECUTE_HANDLER) { retval = FALSE; }


	if (retval == FALSE)
		return FALSE;        	// processor does not support CPUID


	if (RegEDX & 0x800000) 		// bit 23 is set for MMX technology
	{
	   __try { _asm emms } 		// try executing the MMX instruction "emms"
	   __except(EXCEPTION_EXECUTE_HANDLER) { retval = FALSE; }
	}
   	else
		return FALSE;        	// processor supports CPUID but does not support MMX technology

	// if retval == 0 here, it means the processor has MMX technology but
	// floating-point emulation is on; so MMX technology is unavailable

	return retval;
}


/*********************************************************************
* Global: EnumerateProcessorType.
*********************************************************************/
void EnumerateProcessorType(void)
{
	DWORD type, family, model, stepping;

	type     = (gProcessorType>>12) & 0x3;
	family   = (gProcessorType>>8)  & 0xf;
	model    = (gProcessorType>>4)  & 0xf;
	stepping =  gProcessorType      & 0xf;

	if (family == 5)
		gIsPentiumProcessor = TRUE;
	else if (family == 6 && model == 1)
		gIsPentiumProProcessor = TRUE;
	else if (family == 6 && model == 3)
		gIsPentiumIIProcessor = TRUE;

}

#else
// non visual-c version

#warning CPUID functions not implemented

DWORD GetProcessorType(void) { return 0; }
BOOL CheckMMXTechnology(void) { return FALSE; }

#endif // MSC_VER

