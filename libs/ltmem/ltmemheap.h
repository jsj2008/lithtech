
#ifndef __LTMEMHEAP_H__
#define __LTMEMHEAP_H__

#ifndef __GENERALHEAPGROUP_H__
#include "generalheapgroup.h"
#endif

#ifndef __LILFIXEDHEAPGROUP_H__
#include "lilfixedheapgroup.h"
#endif

// define this to use the LT mem system
// if this is not defined then just standard malloc and free are available
//#define USELTMEM

// Init the LTMemHeap
void LTMemHeapInit();

// Terminate the LTMemHeap
void LTMemHeapTerm();

// Allocate memory
void* LTMemHeapAlloc(uint32 nSize);

// Free memory
void LTMemHeapFree(void* pMem);

// Resize memory
void* LTMemHeapReAlloc(void* pMem, uint32 nRequestedSize);

// Get the size of the allocated memory
uint32 LTMemHeapGetSize(void* pMem);

#endif
