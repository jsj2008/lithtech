
#include "bdefs.h"
#ifdef _MSC_VER
#include <new.h>
#else
#include <new>
#endif
#include "de_memory.h"
#include "build_options.h"
#include "sysdebugging.h"
#include "dsys_interface.h"
#include "syscounter.h"
#include "ltmem.h"

//#define TRACK_MEMORY_USAGE

class MemTrack
{
public:
	uint32	m_AllocSize;
	uint32	m_AllocNumber;
};


static uint32 g_MemoryUsage;
static uint32 g_nAllocations;

uint32 g_nTotalAllocations, g_nTotalFrees;

static int g_MemRefCount=0;
#ifdef _MSC_VER
static _PNH g_OldNewHandler;
#else
static std::new_handler g_OldNewHandler;
#define _set_new_handler std::set_new_handler
#endif

// PlayDemo profile info.
uint32 g_PD_Malloc=0;
uint32 g_PD_Free=0;


// Hook Stdlith's base allocators.
void* DefStdlithAlloc(uint32 size)
{
	return dalloc(size);
}

void DefStdlithFree(void *ptr)
{
	dfree(ptr);
}


void dm_HeapCompact()
{
	HANDLE hHeap;

	if(hHeap = GetProcessHeap())
	{
		HeapCompact(hHeap, 0);
	}
}

#ifdef _MSC_VER
static int dm_NewHandler(size_t size)
{
	dsi_OnMemoryFailure();
	return 0;
}
#else
static void dm_NewHandler()
{
	dsi_OnMemoryFailure();
}
#endif


void dm_Init()
{
	if(g_MemRefCount == 0)
	{
		g_OldNewHandler = _set_new_handler(dm_NewHandler);

		g_MemoryUsage = 0;
		g_nAllocations = 0;
		g_nTotalAllocations = g_nTotalFrees = 0;
	}

	g_MemRefCount++;
}


void dm_Term()
{

	g_MemRefCount--;
	if(g_MemRefCount == 0)
	{
		// Restore the old new handler.
		_set_new_handler(g_OldNewHandler);

		g_MemoryUsage = 0;
		g_nAllocations = 0;
	}
}


uint32 dm_GetBytesAllocated()
{
	return g_MemoryUsage;
}


uint32 dm_GetNumAllocations()
{
	return g_nAllocations;
}

void* dalloc(uint32 size)
{
	//[DLK] Removed to avoid allocation errors with D3DXEffectCompiler
	/*
	if(size == 0)
	{
		return NULL;
	}
	*/

	unsigned long fullAllocSize;

	// Add 4 bytes if we're tracking memory usage.
	#ifdef TRACK_MEMORY_USAGE
		MemTrack *pMemTrack;
		fullAllocSize = size + sizeof(MemTrack);
	#else
		fullAllocSize = size;
	#endif

	
	char *ptr;
	// Try to allocate the memory.
	{
		CountAdder cntAdd(&g_PD_Malloc);
		LT_MEM_TRACK_ALLOC(ptr = (char*)LTMemAlloc((size_t)fullAllocSize),LT_MEM_TYPE_UNKNOWN);
	}

	if(!ptr)
	{
		dsi_OnMemoryFailure();
	}

	// Store the size in there if we're tracking memory usage.
	#ifdef TRACK_MEMORY_USAGE
		g_MemoryUsage += size;
		pMemTrack = (MemTrack*)ptr;
		pMemTrack->m_AllocSize = size;
		pMemTrack->m_AllocNumber = g_nAllocations;
		ptr += sizeof(MemTrack);
	#endif

	++g_nAllocations;
	++g_nTotalAllocations;

	return ptr;
}


void* dalloc_z(uint32 size)
{
	void *ret;

	ret = dalloc(size);
	if(ret)
	{
		memset(ret, 0, size);
	}

	return ret;
}


void dfree(void *ptr)
{
	if(!ptr)
		return;

	char *pCharPtr = (char*)ptr;

	#ifdef TRACK_MEMORY_USAGE
		MemTrack *pMemTrack;

		pCharPtr -= sizeof(MemTrack);
		pMemTrack = (MemTrack*)pCharPtr;

		if(pMemTrack->m_AllocSize > g_MemoryUsage)
		{
			dsi_PrintToConsole("Error: engine freed more memory than allocated!");
		}

		g_MemoryUsage -= pMemTrack->m_AllocSize;
	#endif

	--g_nAllocations;
	++g_nTotalFrees;
	
	{
		CountAdder cntAdd(&g_PD_Free);
		LTMemFree(pCharPtr);
	}
}

void* operator new(size_t size, void *ptr, char z)
{
	return dalloc(size);
}

void* operator new(size_t size)
{
	return dalloc(size);
}

void operator delete(void *ptr)
{
	dfree(ptr);
}
