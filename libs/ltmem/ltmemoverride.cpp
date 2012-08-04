// ltmemoverride.cpp 
//

#include "stdafx.h"
#include "ltmem.h"
#include "ltmemheap.h"

/*
void* malloc(uint32 n)
{
	return LTMemAlloc(n);
}

void free(void* p)
{
	LTMemFree(p);
}
*/

/*
// User-defined operator new.
void *operator new( size_t stAllocateBlock )
{
	return LTMemAlloc(stAllocateBlock);
}

// User-defined operator delete.
void operator delete( void *pvMem )
{
	LTMemFree(pvMem);
}
*/
