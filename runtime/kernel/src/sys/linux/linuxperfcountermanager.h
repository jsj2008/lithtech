// LinuxPerfCounterManager.h
//	Linux Implementation of the PerfCounterManager
// Note: This needs to implemented on all platforms (getting the CPU Perf Counter)

#ifndef __LINUXCOUNTERMANAGER_H__
#define __LINUXCOUNTERMANAGER_H__


#ifndef __PERFCOUNTERMANAGER_H__
#include "perfcountermanager.h"
#endif

#ifndef __ILTPERFCOUNTER_H__
#include "iltperfcounter.h"
#endif

#ifndef NO_PROFILE											// Define to compile out all profiling...

// Win32 Version of the DI PerfCounterManager...
class CLINUXPerfCounterMan : public CDIPerfCounterMan {
public:
	CLINUXPerfCounterMan()									{ m_iFreq.QuadPart = 0; }
private:
	void GetPerfCounter(LARGE_INTEGER &iCounter);
	void GetPerfCounterFrequency(LARGE_INTEGER &iFreq);

	LARGE_INTEGER	m_iFreq;
};

extern CLINUXPerfCounterMan g_PerfCounterManager;			// There should only be one of these guys around - use the access functions to get at him...

// Access functions to CDIPerfCounterMan...
CDIPerfCounterMan* GetPerfCounterMan();

#endif //NO_PROFILE

// ACCESSOR FUNCTIONS 
//	Use these functions to add/start/stop counters - they compile to nothing if NO_PROFILE is defined

// Add new counter (or find if the counter already exists) and start it...
inline uint32 AddandStartProfileCounter(uint32 dwCounterGroup, const char* szCounterName)
{
    #ifndef NO_PROFILE
	int32 iCounterID = GetPerfCounterMan()->AddCounter(dwCounterGroup,szCounterName);
	if (iCounterID) { GetPerfCounterMan()->StartCounter(iCounterID); return iCounterID; }
	else return iCounterID;
    #endif
	return 0;
}

// Add new counter (returns the ID, 0 is failure)
inline uint32 AddProfileCounter(uint32 dwCounterGroup, const char* szCounterName)
{
    #ifndef NO_PROFILE
	return GetPerfCounterMan()->AddCounter(dwCounterGroup,szCounterName);
    #endif
	return 0;
}

// Start a counter (if you don't know it's ID)
inline bool StartProfileCounter(uint32 dwCounterGroup, const char* szCounterName)
{
    #ifndef NO_PROFILE
	int32 iCounterID = GetPerfCounterMan()->GetCounterID(dwCounterGroup,szCounterName);
	if (iCounterID) { GetPerfCounterMan()->StartCounter(iCounterID); return true; }
	else return false;
    #endif
	return false;
}

inline void StartProfileCounter(uint32 iCounterID)
{
    #ifndef NO_PROFILE
	GetPerfCounterMan()->StartCounter(iCounterID); 
    #endif
}

inline bool StopProfileCounter(uint32 dwCounterGroup, const char* szCounterName)
{
    #ifndef NO_PROFILE
	int32 iCounterID = GetPerfCounterMan()->GetCounterID(dwCounterGroup,szCounterName);
	if (iCounterID) { GetPerfCounterMan()->StopCounter(iCounterID); return true; }
	else return false;
    #endif
	return false;
}

inline void StopProfileCounter(uint32 iCounterID)
{
    #ifndef NO_PROFILE
	GetPerfCounterMan()->StopCounter(iCounterID); 
    #endif
}

inline void DrawProfileCounters()
{
    #ifndef NO_PROFILE
	GetPerfCounterMan()->DrawCounters(); 
    #endif
}

// External functions (exported to other DLLs - like the renderer) - these can't be inline...
uint32 r_AddandStartProfileCounter(uint32 dwCounterGroup, const char* szCounterName);
uint32 r_AddProfileCounter(uint32 dwCounterGroup, const char* szCounterName);
void   r_StartProfileCounter(uint32 iCounterID);
void   r_StopProfileCounter(uint32 iCounterID);

// ProfileCounter can be used as a nice way of starting a counter and then having it stop automatically when it's destructor is called...
class ProfileCounter {
public:
	inline ProfileCounter(uint32 dwCounterGroup, const char* szCounterName, bool bStartIt = true) {
        #ifndef NO_PROFILE
		m_iCounterID = GetPerfCounterMan()->AddCounter(dwCounterGroup,szCounterName);
		if (bStartIt) GetPerfCounterMan()->StartCounter(m_iCounterID);
        #endif
	}
	inline ~ProfileCounter() {
        #ifndef NO_PROFILE
		GetPerfCounterMan()->StopCounter(m_iCounterID);
        #endif
	}

	inline void	StartCounter() {
        #ifndef NO_PROFILE
		GetPerfCounterMan()->StartCounter(m_iCounterID);
        #endif
	}
	inline void	StopCounter() {
        #ifndef NO_PROFILE
		GetPerfCounterMan()->StopCounter(m_iCounterID);
        #endif
	}

private:
    #ifndef NO_PROFILE
	uint32 m_iCounterID;
    #endif
};

#endif



