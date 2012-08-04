// ltmemstats.cpp 
//

#include "stdafx.h"
#include "ltmem.h"
#include "ltmemheap.h"
#include "ltmemdebug.h"
#include "ltmemtrack.h"

#define LTMEMDEBUG_HEADER_CHECK_VAL		0x12DEBC34
#define LTMEMDEBUG_FOOTER_CHECK_VAL		0x12DEBCF4
#define LTMEMDEBUG_FREED_CHECK_VAL		0x12FEED34

// footer to go at end of memory chunk
struct CLTMemDebugFooter
{
	uint32 nCheck;
};

// footer to go at start of memory chunk
struct CLTMemDebugHeader
{
	uint32 nCheck;
	CLTMemDebugFooter* pFooter;
};


// init debug memory system
void LTMemDebugInit()
{
}


// term debug memory system
void LTMemDebugTerm()
{
}


// allocate memory
void* LTMemDebugAlloc(uint32 nRequestedSize)
{
	// memory we have allocated
	uint8* pMem;

	// adjust size to allocate
	uint32 nAdjustedSize = nRequestedSize + sizeof(CLTMemDebugHeader) + sizeof(CLTMemDebugFooter);

	// allocate memory
#ifdef LTMEMTRACK
	pMem = (uint8*)LTMemTrackAlloc(nAdjustedSize);
#else
	pMem = (uint8*)LTMemHeapAlloc(nAdjustedSize);
#endif

	// make sure allocation succeeded
	if (pMem == NULL) return NULL;

	// figure out header location
	CLTMemDebugHeader* pHeader = (CLTMemDebugHeader*)pMem;

	// figure out footer location
	CLTMemDebugFooter* pFooter = (CLTMemDebugFooter*)(pMem + sizeof(CLTMemDebugHeader) + nRequestedSize);

	// set up values for header and footer
	pHeader->nCheck = LTMEMDEBUG_HEADER_CHECK_VAL;
	pHeader->pFooter = pFooter;
	pFooter->nCheck = LTMEMDEBUG_FOOTER_CHECK_VAL;

	// adjust mem pointer to point after our header
	pMem += sizeof(CLTMemDebugHeader);

	// return allocated pointer
	return pMem;
}

bool VerifyDebugChecks(CLTMemDebugHeader* pHeader)
{
	//first off, we want to see if the headers and footers match
	if(	(pHeader->nCheck == LTMEMDEBUG_HEADER_CHECK_VAL) && 
		(pHeader->pFooter) && 
		(pHeader->pFooter->nCheck == LTMEMDEBUG_FOOTER_CHECK_VAL))
	{
		//a valid block
		return true;
	}

	//we have an invalid block, lets see if it is a double free
	if(pHeader->nCheck == LTMEMDEBUG_FREED_CHECK_VAL)
	{
		assert(!"Critical Error: Double free found.");
	}
	else
	{
		//just standard corruption, lets see the form
		if(pHeader->nCheck != LTMEMDEBUG_HEADER_CHECK_VAL)
		{
			assert(!"Critical Error: Corruption before memory block");
		}
		else 
		{
			assert(!"Critical Error: Corruption after memory block");
		}
	}

	return false;
}

// free memory
void LTMemDebugFree(void* pMem)
{
	// adjust pointer to actual position
	uint8* pMemActual = (uint8*)pMem;
	pMemActual -= sizeof(CLTMemDebugHeader);

	// pointer to header
	CLTMemDebugHeader* pInfo = (CLTMemDebugHeader*)pMemActual;

	if(VerifyDebugChecks(pInfo))
	{
		pInfo->nCheck			= LTMEMDEBUG_FREED_CHECK_VAL;
		pInfo->pFooter->nCheck	= LTMEMDEBUG_FREED_CHECK_VAL;
	}

	// free the memory
#ifdef LTMEMTRACK
	LTMemTrackFree(pMemActual); 
#else
	LTMemHeapFree(pMemActual); 
#endif
}


// re allocate memory
void* LTMemDebugReAlloc(void* pMemOld, uint32 nRequestedSize)
{
	// adjust pointer to actual position
	uint8* pMemActual = (uint8*)pMemOld;
	pMemActual -= sizeof(CLTMemDebugHeader);

	// adjust size to allocate
	uint32 nAdjustedSize = nRequestedSize + sizeof(CLTMemDebugHeader) + sizeof(CLTMemDebugFooter);

	// allocate memory
#ifdef LTMEMTRACK
	uint8* pMem = (uint8*)LTMemTrackReAlloc(pMemActual, nAdjustedSize);
#else
	uint8* pMem = (uint8*)LTMemHeapReAlloc(pMemActual, nAdjustedSize);
#endif

	// make sure allocation succeeded
	if (pMem == NULL) return NULL;

	// figure out header location
	CLTMemDebugHeader* pHeader = (CLTMemDebugHeader*)pMem;

	// figure out footer location
	CLTMemDebugFooter* pFooter = (CLTMemDebugFooter*)(pMem + sizeof(CLTMemDebugHeader) + nRequestedSize);

	// set up values for header and footer
	pHeader->nCheck = LTMEMDEBUG_HEADER_CHECK_VAL;
	pHeader->pFooter = pFooter;
	pFooter->nCheck = LTMEMDEBUG_FOOTER_CHECK_VAL;

	// adjust mem pointer to point after our header
	pMem += sizeof(CLTMemDebugHeader);

	// return allocated pointer
	return pMem;
}


// get size of memory
uint32 LTMemDebugGetSize(void* pMem)
{
	// adjust pointer to actual position
	uint8* pMemActual = (uint8*)pMem;
	pMemActual -= sizeof(CLTMemDebugHeader);

#ifdef LTMEMTRACK
	return LTMemTrackGetSize(pMemActual); 
#else
	return LTMemHeapGetSize(pMemActual); 
#endif
}

