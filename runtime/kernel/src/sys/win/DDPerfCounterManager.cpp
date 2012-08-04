// Win32_PerfCounterManager.cpp

#include "bdefs.h"
#include "DDPerfCounterManager.h"

#ifndef NO_PROFILE 

CWin32PerfCounterMan g_PerfCounterManager;			// There should only be one of these guys around - use the access functions to get at him...
CDIPerfCounterMan* GetPerfCounterMan()				{ return &g_PerfCounterManager; }

void CWin32PerfCounterMan::GetPerfCounter(LARGE_INTEGER &iCounter)
{
	ASSERT(this == &g_PerfCounterManager);			// See note above...
//	QueryPerformanceCounter(&iCounter);

	DWORD dwLow,dwHigh;
	__asm {
		rdtsc
		mov	dwLow, eax
		mov	dwHigh, edx
	}
	iCounter.QuadPart = ((unsigned __int64)dwHigh << 32) | (unsigned __int64)dwLow;
}

void CWin32PerfCounterMan::GetPerfCounterFrequency(LARGE_INTEGER &iFreq)
{
	ASSERT(this == &g_PerfCounterManager);			// See note above...
//	QueryPerformanceFrequency(&iFreq);

	if (m_iFreq.QuadPart != 0) { iFreq = m_iFreq; return; }
	DWORD dwLow,dwHigh;
	__asm {
		rdtsc
		mov	dwLow, eax
		mov	dwHigh, edx
	}
	unsigned __int64 ST = ((unsigned __int64)dwHigh << 32) | (unsigned __int64)dwLow;
	Sleep(500);
	__asm {
		rdtsc
		mov	dwLow, eax
		mov	dwHigh, edx
	} 
	unsigned __int64 ET = ((unsigned __int64)dwHigh << 32) | (unsigned __int64)dwLow;
	m_iFreq.QuadPart = 2 * (ET - ST); if (m_iFreq.QuadPart==0) m_iFreq.QuadPart = 1;
}

#endif

// External functions (exported to other DLLs - like the renderer) - these can't be inline...
uint32 r_AddandStartProfileCounter(uint32 dwCounterGroup, const char* szCounterName)
{
    #ifdef NO_PROFILE
	ASSERT(0 && "Profiling is not enabled in this version of the engine"); return 0;
    #else
	uint32 iCounterID = GetPerfCounterMan()->AddCounter(dwCounterGroup,szCounterName);
	if (iCounterID) { GetPerfCounterMan()->StartCounter(iCounterID); return iCounterID; }
	else return iCounterID;
    #endif
}

uint32 r_AddProfileCounter(uint32 dwCounterGroup, const char* szCounterName)
{
    #ifdef NO_PROFILE
	ASSERT(0 && "Profiling is not enabled in this version of the engine"); return 0;
    #else
	return GetPerfCounterMan()->AddCounter(dwCounterGroup,szCounterName);
    #endif
}

void   r_StartProfileCounter(uint32 iCounterID)
{
    #ifdef NO_PROFILE
	ASSERT(0 && "Profiling is not enabled in this version of the engine");
    #else
	GetPerfCounterMan()->StartCounter(iCounterID); 
    #endif
}

void   r_StopProfileCounter(uint32 iCounterID)
{
    #ifdef NO_PROFILE
	ASSERT(0 && "Profiling is not enabled in this version of the engine");
    #else
	GetPerfCounterMan()->StopCounter(iCounterID); 
    #endif
}


//
//PC implementation of the ILTPerfCounter interface.
//

// The Public Interface...
class CLTPerfCounterPC : public ILTPerfCounter {
public:
    declare_interface(CLTPerfCounterPC);

	// Add new counter (returns the ID, 0 is failure)
	uint32 AddCounter(uint32 dwCounterGroup, const char* szCounterName) 
	{
        #ifndef NO_PROFILE
		return GetPerfCounterMan()->AddCounter(dwCounterGroup,szCounterName);
        #else
		return NULL;
        #endif
	}

	// Forget your counter's ID? (Zero is failure)
	uint32 GetCounterID(uint32 dwCounterGroup, const char* szCounterName)
	{
        #ifndef NO_PROFILE
		return GetPerfCounterMan()->GetCounterID(dwCounterGroup,szCounterName);
        #else
		return NULL;
        #endif
	}

	// Delete the counter
	bool DeleteCounter(uint32 uCounterID)
	{
        #ifndef NO_PROFILE
		return GetPerfCounterMan()->DeleteCounter(uCounterID);
        #else
		return false;
        #endif
	}

	// Start timing your code.
	void StartCounter(uint32 uCounterID)
	{
        #ifndef NO_PROFILE
		GetPerfCounterMan()->StartCounter(uCounterID);
        #endif
	}

	// Stop timing your code.
	void StopCounter(uint32 uCounterID)
	{
        #ifndef NO_PROFILE
		GetPerfCounterMan()->StopCounter(uCounterID);
        #endif
	}
};

//instantiate our ILTPerfCounter implemenation and add it to the interface mgr.
define_interface(CLTPerfCounterPC, ILTPerfCounter);

