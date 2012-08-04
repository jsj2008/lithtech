// PerfCounterManager.h
//  The Performance Counter Manager is the internal performance profiler for the engine.
// Code can add, start, and stop named counter which will be displayed (while in profile mode).

#ifndef __PERFCOUNTERMANAGER_H__
#define __PERFCOUNTERMANAGER_H__

#ifndef NO_PROFILE                                          // To Compile without any profiling code...

#pragma warning (disable:4530)
#pragma warning (disable:4786)                              // STL Warnings (Yeah, I know...But life goes on)...

#ifndef __VECTOR__
#include <vector>
#define __VECTOR__
#endif

#ifndef __MAP__
#include <map>
#define __MAP__
#endif

//#ifndef __CODEWARRIOR
using namespace std;
//#endif

#ifndef __LTINTEGER_H__
#include "ltinteger.h"
#endif


#if defined(__LINUX)
// uh, look it up by pressing f1 on msdev... 
struct LARGE_INTEGER 
{
    uint64 QuadPart ;
};
#endif

struct SCOUNTER_INFO {                                      // CDIPerfCounterMan stores a list of these structs (one for each counter)
    SCOUNTER_INFO(uint32 dwCntrGroup, const char* szName)   { strncpy(szCounterName,szName,32); FrameCount.QuadPart = 0; TmpStartTime.QuadPart = 0; MaxCount.QuadPart = 0; dwCounterGroup = dwCntrGroup; iCounterID = -1; }
    SCOUNTER_INFO()                                         { strncpy(szCounterName,"UnNamed",32); FrameCount.QuadPart = 0; TmpStartTime.QuadPart = 0; MaxCount.QuadPart = 0; dwCounterGroup = 0; iCounterID = -1; }
    uint32      dwCounterGroup;
    char        szCounterName[32];
    int         iCounterID;
    LARGE_INTEGER FrameCount;                               // Current running total
    LARGE_INTEGER MaxCount;
    LARGE_INTEGER TmpStartTime;                         // Used to keep track of start time (from StartCounter)
};

class CDIPerfCounterMan {
public:
    CDIPerfCounterMan();
    ~CDIPerfCounterMan();
    typedef     vector<SCOUNTER_INFO> COUNTER_LIST;

    int32       AddCounter(uint32 dwCounterGroup, const char* szCounterName);   // Add new counter (returns the ID, 0 is failure)
    int32       GetCounterID(uint32 dwCounterGroup, const char* szCounterName); // Forget your counter's ID? (Zero is failure)
    bool        DeleteCounter(uint32 uCounterID);           // Delete the counter

    void        StartCounter(uint32 uCounterID);            // Start timing your code.
    void        StopCounter(uint32 uCounterID);             // Stop timing your code.

    void        DrawCounters();                             // Go ahead and draw the graph of all counters...
private:
    COUNTER_LIST           m_Counters;                      // Map (of all the counters) from CounterID to the COUNTER_INFO structs 
    map<uint32,bool>       m_CounterGroupEnabledMap;        // Maps CounterGroup to enable/disabled (if we're worrying about these right now)
    LARGE_INTEGER          m_FrameTimeCount,m_FrameTimeTmp;
    LARGE_INTEGER          m_PerfCntFreq;
//  uint32                 m_ProfilerCounterID;
    uint32                 m_FramesTillNextSample;
    uint32                 m_FramesTillNextMaxValClear;
    float                  m_MaxDisplayNum;
    uint32                 m_DisplayCountUnder;
    uint32                 m_DisplayCountOver;

    void        SetupFrame();                               // Setup and some random stuff...
    void        FrameMarker();                              // Called once per frame, stash the frame count away here (do some tricks if it's currently running)...
    void        ClearCounters(bool bAllowMaxValueClear = false);

    virtual void GetPerfCounter(LARGE_INTEGER &iCounter)=0; // Pure virtual functions to be implemented by the instantiated device dependant version...
    virtual void GetPerfCounterFrequency(LARGE_INTEGER &iFreq)=0;
};

#endif // NO_PROFILE

#endif



