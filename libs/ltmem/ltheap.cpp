
#include "stdafx.h"
#include "ltheap.h"

///////////////////////////////////////////////////////////////////////////////////////////
// ltheap global variables
///////////////////////////////////////////////////////////////////////////////////////////

// true if lt global heap is initialized
bool g_bLTMemInitialized = false;


///////////////////////////////////////////////////////////////////////////////////////////
// simple heaps
///////////////////////////////////////////////////////////////////////////////////////////

// size of the simple heaps last item must be 0
// if you don't want to use simple heaps just make the first item 0
uint32 g_nSimpleHeapSizes[] = { 16, 32, 48, 64, 0 };

// initial number of items in each of the simple heap last item must be 0
uint32 g_nSimpleHeapNumItems[] = { 10000, 5000, 5000, 5000, 0 };

// grow number of items in each of the simple heap last item must be 0
uint32 g_nSimpleHeapGrowItems[] = { 10000, 5000, 5000, 5000, 0 };

// this is calculated from the simple heap sizes array
uint32 g_nNumSimpleHeapSizes;

// array of simple heaps
CLilFixedHeapGroup* g_pSimpleHeaps;


///////////////////////////////////////////////////////////////////////////////////////////
// general heap
///////////////////////////////////////////////////////////////////////////////////////////

CGeneralHeap* g_pGeneralHeap;


///////////////////////////////////////////////////////////////////////////////////////////
// memory tracking information
///////////////////////////////////////////////////////////////////////////////////////////

CLTMemoryTrackingInfo* g_pLTMemTrackInfo;


///////////////////////////////////////////////////////////////////////////////////////////
// debugging memory structures
///////////////////////////////////////////////////////////////////////////////////////////



void lt_mem_init()
{
	// figure out the number of different sizes of the simple heaps
	while (g_nNumSimpleHeapSizes = 0; g_nSimpleHeapSize[g_nNumSimpleHeapSizes] != 0; g_nNumSimpleHeapSizes++);

	// initialize the simple heaps

	// initialize the general heap

	// initialize memory tracking information

	// initialize memory debugging information

	// set initialized flag
	g_bLTMemInitialized = true;
}


void lt_mem_term()
{
	g_bLTMemInitialized = false;
}


void* lt_mem_malloc_internal(uint32 nSize)
{
}


void lt_mem_free_internal(void* pMem)
{
}


void lt_mem_realloc_internal(void* pMem, nNewSize)
{
}