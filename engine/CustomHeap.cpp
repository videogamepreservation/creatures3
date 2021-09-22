////////////////////////////////////////////////////////////////////////////////
// Filename:	CustomHeap.cpp
//
// Description: Overrides operator new and delete, making all (most?) heap 
//              memory allocation in the program go through our own functions.
//              Provides a global memory allocated on the heap function to
//              allow monitoring during debugging, to test memory usage.
//
//              Define CUSTOM_HEAP to use it.  Should be undefined for release.
//
// Author:		Francis Irving
////////////////////////////////////////////////////////////////////////////////

#include "CustomHeap.h"

//#define CUSTOM_HEAP

#ifdef CUSTOM_HEAP

	#define MONITOR_HEAP

	#include <windows.h>
	#include "C2eServices.h"

	#ifndef _WIN32
		#warning This would be totally different on another OS?
	#endif

	#ifdef MONITOR_HEAP
		int allocatedMemory = 0;
	#endif

	void* operator new(unsigned int size)
	{
		void *location = HeapAlloc(GetProcessHeap(), 0, size);
		int alloced = HeapSize(GetProcessHeap(), 0, location);
	#ifdef MONITOR_HEAP
		allocatedMemory += alloced;
	#endif

		ASSERT(location != NULL);

		return location;
	}

	void operator delete(void *location)
	{
		if (location == NULL)
			return;

	#ifdef MONITOR_HEAP
		allocatedMemory -= HeapSize(GetProcessHeap(), 0, location);
		ASSERT(allocatedMemory >= 0);
	#endif

		BOOL result = HeapFree(GetProcessHeap(), 0, location);
		ASSERT(result != 0);
	}

	int GetCustomHeapSize()
	{
		return allocatedMemory;
	}
#else
	int GetCustomHeapSize()
	{
		return -1;
	}
#endif // CUSTOM_HEAP
