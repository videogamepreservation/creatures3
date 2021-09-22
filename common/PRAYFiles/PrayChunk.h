// --------------------------------------------------------------------------------------
// Filename:	PrayChunk.h
// Class:		PrayChunk
// Purpose:		To encapsulate the chunks of data in Pray Files
// Description:
//  The PrayChunks are returned by the PrayManager when asked for. They are managed as
//  a list in the praymanager, and are cached thusly.
//  When you "delete" a PrayChunk its reference count is decremented. When its count
//  reaches zero, it is marked for cleaning out.
//  Should it be deleted when its refcount is zero, the chunk is removed immediately
//  from the list.
//
// History:
//  21Jun99	DanS	Initial Version
// --------------------------------------------------------------------------------------

#ifndef PRAYCHUNK_H
#define PRAYCHUNK_H

#ifdef _MSC_VER
#pragma warning(disable:4786 4503)
#endif

#include "../C2eTypes.h"

class PrayManager;

class PrayChunk
{
	// The PrayManager is our friend, it gives us our data :)
	friend class PrayManager;
	// The PrayChunkPtr is also our friend - It's the only way to make us :)
	friend class PrayChunkPtr;

public:

	// ----------------------------------------------------------------------------------
	// Method:		GetData
	// Arguments:	(None)
	// Returns:		Pointer to the data owned by the chunk.
	// Description:	This method returns the data owned by the chunk. It is not processed
	//				in any way (other than decompression) before being passed to you.
	//				You may not do anything other than read this data :)
	// ----------------------------------------------------------------------------------
	const uint8* GetData() const;

	// ----------------------------------------------------------------------------------
	// Method:		GetSize
	// Arguments:	(None)
	// Returns:		Size of the data owned by the chunk.
	// Description:	This method returns the size of the data owned by the chunk. It is 
	//				not processed in any way (other than decompression) before being 
	//				passed to you. You may not do anything other than read this data :)
	// ----------------------------------------------------------------------------------
	const int GetSize() const;

private:
	int myCount;
	PrayManager* myManager;
	uint8* myPtr;
	int mySize;

	// ----------------------------------------------------------------------------------
	// Constructor
	// Arguments:	sizeOf - the size of the data to manage
	//				manager - The PrayManager which owns this PrayChunk
	// Returns:		(None)
	// Description: This constructor is held private as you should never call it.
	//              What you should do is instantiate a PrayChunkPtr with it :)
	// ----------------------------------------------------------------------------------
	PrayChunk(uint32 sizeOf, PrayManager* manager);

	PrayChunk() { myPtr = NULL; myCount = mySize = 0; myManager = NULL; }

	~PrayChunk() { delete [] myPtr; }
};

// This is a refcounter class
// Lifted pretty much from http://www.cerfnet.com/~mpcline/c++-faq-lite/freestore-mgmt.html

class PrayChunkPtr
{
public:
	PrayChunk* operator-> () const 
	{ 
		return pc_; 
	}
	
	PrayChunk* operator*  () const 
	{ 
		return pc_; 
	}

	PrayChunkPtr(uint32 size, PrayManager* pm) 
	{ 
		pc_ = new PrayChunk(size,pm); 
		pc_->myCount++;
	}
	
	PrayChunkPtr(PrayChunk* ptr)
	{ 
		pc_ = ptr;
		if (pc_)
			pc_->myCount++; 
	}

	~PrayChunkPtr()			 
	{ 
		if (--pc_->myCount == 0) 
			delete pc_; 
	}
	
	PrayChunkPtr(const PrayChunkPtr& p) 
	{ 
		pc_ = p.pc_;
		if (pc_)
			++pc_->myCount; 
	}

	PrayChunkPtr& operator= (const PrayChunkPtr& p)
	{
		if (p.pc_)
			++p.pc_->myCount;
		if (--pc_->myCount == 0)
		{
			delete pc_;
		}
		pc_ = p.pc_;
		return *this;
	}

	PrayChunkPtr() 
	{ 
		pc_ = new PrayChunk(); 
		pc_->myCount++;
	}

private:
	PrayChunk* pc_;
};

#endif //PRAYCHUNK_H
