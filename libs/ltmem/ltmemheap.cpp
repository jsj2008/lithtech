
#include "stdafx.h"
#include "ltmem.h"
#include "ltmemheap.h"
#include "generalheapgroup.h"

// define if we are using the simple heaps
#define LTMEMUSESIMPLEHEAP

// define if we are using the general heap
#define LTMEMUSEGENERALHEAP


///////////////////////////////////////////////////////////////////////////////////////////
// ltheap global variables
///////////////////////////////////////////////////////////////////////////////////////////

// true if lt global heap is initialized
bool g_bLTMemHeapInitialized = false;


///////////////////////////////////////////////////////////////////////////////////////////
// simple heaps
///////////////////////////////////////////////////////////////////////////////////////////

#define LTMEMHEAPNUMSIMPLEHEAPSIZES 4

// size of the simple heaps 
uint32 g_nSimpleHeapSizes[] = { 16, 32, 48, 64 };

// initial number of items in each of the simple heap 
uint32 g_nSimpleHeapNumItems[] = { 10000, 5000, 5000, 5000 };

// grow number of items in each of the simple heap 
uint32 g_nSimpleHeapGrowItems[] = { 10000, 5000, 5000, 5000 };

// array of simple heaps
CLilFixedHeapGroup g_arySimpleHeaps[4];


///////////////////////////////////////////////////////////////////////////////////////////
// general heap
///////////////////////////////////////////////////////////////////////////////////////////

// size of the general heap
//const uint32 g_nGeneralHeapSize = 0; // if general heap size is 0 don't use the general heap
const uint32 g_nGeneralHeapSize = 1024*1024*64;
const uint32 g_nGeneralHeapGrowSize = 1024*1024*16;

// alignment of the general heap
const uint32 g_nGeneralHeapAlign = 16;

// general heap
CGeneralHeapGroup g_GeneralHeap;


// Initialize the LTMemHeap
void LTMemHeapInit()
{
	// set initialized flag
	g_bLTMemHeapInitialized = true;

	// initialize the simple heaps
	{
		for (uint32 n = 0; n < LTMEMHEAPNUMSIMPLEHEAPSIZES; n++)
		{
			g_arySimpleHeaps[n].Init(g_nSimpleHeapSizes[n],g_nSimpleHeapNumItems[n],g_nSimpleHeapGrowItems[n]);
		}
	}

	// if we are using the general heap
	if (g_nGeneralHeapSize > 0)
	{
		// initialize the general heap
		g_GeneralHeap.Init(g_nGeneralHeapSize, g_nGeneralHeapGrowSize, g_nGeneralHeapAlign);
	}
}


// Terminate the LTMemHeap
void LTMemHeapTerm()
{
	// terminate the simple heaps
	{
		for (uint32 n = 0; n < LTMEMHEAPNUMSIMPLEHEAPSIZES; n++)
		{
			g_arySimpleHeaps[n].Term();
		}
	}

	// terminate the general heap
	g_GeneralHeap.Term();

	// we are no longer initialized
	g_bLTMemHeapInitialized = false;
}


// Allocate memory
void* LTMemHeapAlloc(uint32 nSize)
{
	// memory pointer to return
	void* pMem = NULL;

	// see if this memory fits in a simple heap
	{
		for (uint32 n = 0; n < LTMEMHEAPNUMSIMPLEHEAPSIZES; n++)
		{
			if (nSize <= g_nSimpleHeapSizes[n])
			{
				pMem = g_arySimpleHeaps[n].Alloc();
			}
		}
	}

	// if we didn't allocate in simple heap try general heap
	if ((pMem == NULL) && (g_nGeneralHeapSize > 0))
	{
		pMem = g_GeneralHeap.Alloc(nSize);
	}

	return pMem;
}


// Free memory
void LTMemHeapFree(void* pMem)
{
	// if memory is null don't free it
	if (pMem == NULL) return;
	
	// try to free from simple heap
	{
		for (uint32 n = 0; n < LTMEMHEAPNUMSIMPLEHEAPSIZES; n++)
		{
			// the simple heap free will check if it is in the heap
			// and free it if it can
			if (g_arySimpleHeaps[n].Free(pMem))
			{
				// we succeeded so exit
				return;
			}
		}
	}

	// if we get here then memory was not in the simple heap so check the general heap
	if (g_nGeneralHeapSize > 0)
	{
		if (g_GeneralHeap.InHeap(pMem))
		{
			g_GeneralHeap.Free(pMem);

			return;
		}
	}

	// This memory is not from LTMemHeap !!!
	ASSERT(false);
}


// resize memory
void* LTMemHeapReAlloc(void* pMem, uint32 nNewSize)
{
	// get size of old memory
	uint32 nOldSize = 0;
	{
		for (uint32 n = 0; n < LTMEMHEAPNUMSIMPLEHEAPSIZES; n++)
		{
			if (g_arySimpleHeaps[n].InHeap(pMem))
			{
				nOldSize = g_nSimpleHeapSizes[n];

				// done searching
				break;
			}
		}
	}
	if (nOldSize == 0)
	{
		if (g_GeneralHeap.InHeap(pMem)) 
		{
			// get size from general heap
			nOldSize = g_GeneralHeap.GetSize(pMem);
		}
		else
		{
			// This memory is not from LTMemHeap !!!
			ASSERT(false);
		}
	}
	
	// allocate new memory
	void* pNewMem = LTMemHeapAlloc(nNewSize);
	if (pNewMem == NULL) return NULL;

	// figure out amount to copy
	uint32 nCopyAmount;
	if (nNewSize > nOldSize) nCopyAmount = nOldSize;
	else nCopyAmount = nNewSize;

	// copy data over
	memcpy(pNewMem, pMem, nCopyAmount);

	// free old memory
	LTMemHeapFree(pMem);	

	// return value
	return pMem;
}


// get the size of this piece of memory
uint32 LTMemHeapGetSize(void* pMem)
{
	// get size of old memory
	uint32 nSize = 0;
	{
		for (uint32 n = 0; n < LTMEMHEAPNUMSIMPLEHEAPSIZES; n++)
		{
			if (g_arySimpleHeaps[n].InHeap(pMem))
			{
				nSize = g_nSimpleHeapSizes[n];

				// done searching
				break;
			}
		}
	}
	if (nSize == 0)
	{
		if (g_GeneralHeap.InHeap(pMem)) 
		{
			// get size from general heap
			nSize = g_GeneralHeap.GetSize(pMem);
		}
		else
		{
			// This memory is not from LTMemHeap !!!
			ASSERT(false);
		}
	}

	return nSize;
}
