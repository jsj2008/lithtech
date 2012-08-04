// ------------------------------------------------------------------------- //
//
// FILE      : U N D A T A . C P P
//
// CREATED   : 11/01/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : Loosly based upon struct banks and moArray's
//
// ------------------------------------------------------------------------- //

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "stdlterror.h"

#include <stdio.h>
#include <assert.h>

#include "undata.h"

// ------------------------------------------------------------------------- //
// Globals
// ------------------------------------------------------------------------- //
// choose the correct type for the memory class
#ifdef __LINUX
#include "linuxundata.h"
CLINUXUnboundData UnboundData;

#elif __WIN32
#include "winundata.h"
CWIN32UnboundData UnboundData;
#endif

CGenUnboundData *g_pUnboundData = &UnboundData;

// ------------------------------------------------------------------------- //
// Class implementation
// ------------------------------------------------------------------------- //

ESTDLTResults
CGenUnboundData::Init () {
	uint32* tmpPtr;
	uint32  blockSizeInWords;

	// Figure out a better way to deal with dynamic block Size
	CurBlockSize = 512; // 1/2 K block size
	blockSizeInWords = CurBlockSize/4;

	// init stat counters
	GetCnt = RetCnt = 0;

	// first grab a big chunck of memory and thread it together to form
	// your freelist
	if (initUnboundHeap(CurBlockSize, &heapStart, &heapEnd) != STDLT_OK) {
		CRITICAL_ERROR("undata.cpp", "Couldn't allocate heap\n");
	}

	// Assumes that heapStart addess is numerically lower than heapEnd
	for (tmpPtr = heapEnd-blockSizeInWords, FreeListHead = NULL, FreeBlocks = 0;
	     tmpPtr >= heapStart;
		 tmpPtr -= blockSizeInWords) {
		*tmpPtr = (uint32) FreeListHead;
		FreeListHead = tmpPtr;
		FreeBlocks++;
	}
	minFreeBlocks = FreeBlocks;

	assert (FreeListHead != NULL);
	return (STDLT_OK);
}

void
CGenUnboundData::PrintUndataStats () {
	printf ("Unbound data memory stats\n");
	printf ("\tTotal free blocks    = %d\n", FreeBlocks);
	printf ("\tGetNewBlock calls    = %d\n", GetCnt);
	printf ("\tReturnOldBlock calls = %d\n", RetCnt);
}

