
#include "stdafx.h"
#ifdef _WIN32
#include "windows.h"
#endif
#include "ltmem.h"
#include "ltmemheap.h"
#include "ltmemdebug.h"
#include "ltmemtrack.h"

///////////////////////////////////////////////////////////////////////////////////////////
// ltheap global variables
///////////////////////////////////////////////////////////////////////////////////////////

// true if lt mem system is initialized
bool g_bLTMemInitialized = false;

#ifdef USELTMEM

// critical section to make heap thread safe
CRITICAL_SECTION g_LTMemCriticalSection; 

#endif
///////////////////////////////////////////////////////////////////////////////////////////
// Initializer class for ltmem
///////////////////////////////////////////////////////////////////////////////////////////
class CLTMemInitialize
{
public:
	CLTMemInitialize();
	~CLTMemInitialize();
} g_LTMemInitialize;

CLTMemInitialize::CLTMemInitialize()
{
	LTMemInit();
}

CLTMemInitialize::~CLTMemInitialize()
{
	LTMemTerm();
}


///////////////////////////////////////////////////////////////////////////////////////////
// LTMem functions
///////////////////////////////////////////////////////////////////////////////////////////

// Initialize
void LTMemInit()
{
#ifdef USELTMEM
	// don't need to initialize if we already are
	if (g_bLTMemInitialized == true) return;

	// Initialize the LTMem critical section.
	InitializeCriticalSection(&g_LTMemCriticalSection); 

	// Request ownership of the LTMem critical section.
	EnterCriticalSection(&g_LTMemCriticalSection); 

	// initialize memory tracking
	LTMemTrackInit();

	// initialize memory debugging
	LTMemDebugInit();

	// initialize heap
	LTMemHeapInit();

	// set initialized flag
	g_bLTMemInitialized = true;

	LTMemTrackSetupMemTypesToStrings();

    // Release ownership of the LTMem critical section.
    LeaveCriticalSection(&g_LTMemCriticalSection);
#endif
}


// Terminate
void LTMemTerm()
{
#ifdef USELTMEM
	// don't need to terminate if we already are
	if (g_bLTMemInitialized == false) return;

	// Request ownership of the LTMem critical section.
	EnterCriticalSection(&g_LTMemCriticalSection); 

	// term heap
	LTMemHeapTerm();

	// term memory tracking
	LTMemTrackTerm();

	// term memory debugging
	LTMemDebugTerm();

	// we are no longer initialized
	g_bLTMemInitialized = false;

    // Release ownership of the LTMem critical section.
    LeaveCriticalSection(&g_LTMemCriticalSection);

    // Release resources used by the LTMem critical section object.
	DeleteCriticalSection(&g_LTMemCriticalSection);
#endif
}


// LTMem Allocation function
void* LTMemAlloc(uint32 nSize)
{
#ifdef USELTMEM
	void* pRet;

	// make sure memory system is initialize
	if (g_bLTMemInitialized == false) LTMemInit();

	// Request ownership of the LTMem critical section.
	EnterCriticalSection(&g_LTMemCriticalSection); 

#ifdef LTMEMDEBUG
	pRet = LTMemDebugAlloc(nSize);
#else
#ifdef LTMEMTRACK
	pRet = LTMemTrackAlloc(nSize);
#else
	pRet = LTMemHeapAlloc(nSize);
#endif
#endif

    // Release ownership of the LTMem critical section.
    LeaveCriticalSection(&g_LTMemCriticalSection);

	return pRet;

#else
	return malloc(nSize);
#endif
}


// LTMem Free function
void LTMemFree(void* pMem)
{
#ifdef USELTMEM
	// Request ownership of the LTMem critical section.
	EnterCriticalSection(&g_LTMemCriticalSection); 

#ifdef LTMEMDEBUG
	LTMemDebugFree(pMem);
#else
#ifdef LTMEMTRACK
	LTMemTrackFree(pMem);
#else
	LTMemHeapFree(pMem);
#endif
#endif

    // Release ownership of the LTMem critical section.
    LeaveCriticalSection(&g_LTMemCriticalSection);

#else
	free(pMem);
#endif
}


// LTMem system memory re-size function
void* LTMemReAlloc(void* pOldMem, uint32 nNewSize)
{
#ifdef USELTMEM
	void* pRet;

	// Request ownership of the LTMem critical section.
	EnterCriticalSection(&g_LTMemCriticalSection); 

#ifdef LTMEMDEBUG
	pRet = LTMemDebugReAlloc(pOldMem, nNewSize);
#else
#ifdef LTMEMTRACK
	pRet = LTMemTrackReAlloc(pOldMem, nNewSize);
#else
	pRet = LTMemHeapReAlloc(pOldMem, nNewSize);
#endif
#endif

    // Release ownership of the LTMem critical section.
    LeaveCriticalSection(&g_LTMemCriticalSection);

	return pRet;

#else
	return realloc(pOldMem, nNewSize);
#endif
}


