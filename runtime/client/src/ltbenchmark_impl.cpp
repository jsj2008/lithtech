/****************************************************************************
;
;   MODULE:     LTBenchmark_Impl (.CPP)
;
;   PURPOSE:    General purpose functions for system benchmarking
;
;   HISTORY:    4-24-2000 [mds] File created.
;
;   NOTICE:     Copyright (c) 2000 LithTech, Inc.
;
***************************************************************************/


// Includes...
#include "bdefs.h"

#include "iltbenchmark.h"
#include "clientde_impl.h"
#include "ltinfo_impl.h"
#include "systimer.h"


// Macros...

#define IsKeyDown(key)  (GetAsyncKeyState(key) & 0x80000000)



class CLTBenchmarkMgr : public ILTBenchmarkMgr
{
public:
    declare_interface(CLTBenchmarkMgr);

    CLTBenchmarkMgr();
    ~CLTBenchmarkMgr();

    LTRESULT Clear();

public:
    //ILTBenchmark functions.
    LTRESULT Init();
    LTRESULT Term();
    LTRESULT DoCPUBenchmarking(LTBENCH_CPU_TEST* pTest,
                                            LTBENCH_CPU_RESULT* pResult,
                                            LTFLOAT fTestDuration = 1.0f,
                                            bool bAllowEscape = true);
    LTRESULT StartGraphicsBenchmarking(LTFLOAT fTestDuration = 0.0f);
    LTRESULT StopGraphicsBenchmarking(LTBENCH_GRAPHICS_TEST* pTest = NULL);
    LTRESULT IsGraphicsBenchmarking(bool* pbIsBenchmarking);
    LTRESULT GetGraphicsBenchmarkingInfo(LTBENCH_GRAPHICS_TEST* pTest);
    LTRESULT ClearGraphicsBenchmarkingInfo();

public:
    LTRESULT    GraphicsBenchmarkingUpdate();

protected:
    LTBENCH_CPU_TEST*           m_pCPUTest;
    LTBENCH_CPU_RESULT*         m_pCPUResult;
    LTFLOAT                     m_fCPUTestDuration;
    bool						m_bCPUAllowEscape;

    LTBENCH_GRAPHICS_TEST       m_ltBenchGraphicsTest;
    bool						m_bIsGraphicsBenchmarking;
    LTFLOAT                     m_fGraphicsTestDuration;
    uint32                      m_dwGraphicsTestStartTime;
    uint32                      m_dwGraphicsTestCurrentTime;
    uint32                      m_dwGraphicsTestEndTime;
    uint32                      m_dwUpdateCount;
    uint32                      m_dwFrameCount;
    uint32                      m_dwPolyCount;
};

//instantiate our implemenation class.
define_interface(CLTBenchmarkMgr, ILTBenchmarkMgr);

//pointer to the implementation class.
static CLTBenchmarkMgr *ilt_benchmark_impl = NULL;

void GraphicsBenchmarkingUpdate() {
    IFBREAKRETURN(ilt_benchmark_impl == NULL);

    //call the function in the interface implemenatation class.
    ilt_benchmark_impl->GraphicsBenchmarkingUpdate();
}

// Functions...

//-----------------------------------------------------------------------------
// CLTBenchmarkMgr member functions
//-----------------------------------------------------------------------------
CLTBenchmarkMgr::CLTBenchmarkMgr() {
    ilt_benchmark_impl = this;

    Clear();
}

//-----------------------------------------------------------------------------
CLTBenchmarkMgr::~CLTBenchmarkMgr() {
    Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::Clear() {
    m_pCPUTest = NULL;
    m_pCPUResult = NULL;
    m_fCPUTestDuration = 0.0f;
    m_bCPUAllowEscape = LTFALSE;

    memset(&m_ltBenchGraphicsTest, 0, sizeof(LTBENCH_GRAPHICS_TEST));
    m_ltBenchGraphicsTest.m_dwSize = sizeof(LTBENCH_GRAPHICS_TEST);
    m_bIsGraphicsBenchmarking = LTFALSE;
    m_fGraphicsTestDuration = 0.0f;
    m_dwGraphicsTestStartTime = 0;
    m_dwGraphicsTestCurrentTime = 0;
    m_dwGraphicsTestEndTime = 0;
    m_dwUpdateCount = 0;
    m_dwFrameCount = 0;
    m_dwPolyCount = 0;

    return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::Init()
{
    Clear();

    return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::Term()
{
    Clear();

    return LT_OK;
}

//-----------------------------------------------------------------------------
#pragma optimize("", off)
LTRESULT CLTBenchmarkMgr::DoCPUBenchmarking(   LTBENCH_CPU_TEST* pTest,
                                                LTBENCH_CPU_RESULT* pResult,
                                                LTFLOAT fTestDuration,
                                                bool bAllowEscape)
{
    LTRESULT result = LT_OK;

    // First, benchmarking is not asynchronous
    if (m_pCPUTest)
        return LT_ERROR;

    // Make sure structure pointers are not null
    if (!pTest || !pResult)
        return LT_ERROR;

    // Make sure structure size is what we're expecting
    if (sizeof(LTBENCH_CPU_TEST) != pTest->m_dwSize)
        return LT_ERROR;

    // Make sure structure size is what we're expecting
    if (sizeof(LTBENCH_CPU_RESULT) != pResult->m_dwSize)
        return LT_ERROR;

    // Refresh
    Clear();

    // Assign values
    m_pCPUTest = pTest;
    m_pCPUResult = pResult;
    m_fCPUTestDuration = fTestDuration;
    m_bCPUAllowEscape = bAllowEscape;

    // Convert the duration from seconds to milliseconds and test
    uint32 dwTestDuration = (uint32)(fTestDuration * 1000.0f);
    if (dwTestDuration > 0)
    {
        uint32 dwCurrentTime = (uint32)time_GetTime();
        uint32 dwEndTime = dwCurrentTime + dwTestDuration;
        while (dwCurrentTime <= dwEndTime)
        {
            // Do some math to take up time
            double dX = 1024.1968;
            double dY = dX * dX;
            double dZ = dY * dX;
            dX = sqrt(dZ);
            dY = dX / 3.141592;
            dZ = sqrt(dY);
            dX = dZ * 3.141592;
            dY = sqrt(dX);
            dZ = dX / dY;

            // Update the index
            pResult->m_dwPerformanceIndex++;
            if (MAX_DWORD == pResult->m_dwPerformanceIndex)
                break;

            // Update time          
            dwCurrentTime = (uint32)time_GetTime();

            // Check for abort
#ifndef __XBOX
            if (bAllowEscape && IsKeyDown(VK_ESCAPE))
            {
                result = LT_ESCABORT;
                break;
            }
#endif
        }
        if (dwCurrentTime > dwEndTime)
            dwCurrentTime = dwEndTime;
        pResult->m_fTestDuration = fTestDuration - (((LTFLOAT)dwEndTime - (LTFLOAT)dwCurrentTime) / (LTFLOAT)1000);
    }

    // Scale the value
    if (pResult->m_dwPerformanceIndex > 0)
        pResult->m_dwPerformanceIndex /= 1000;

    // Clamp value?  We only clamp if passed in a max greater than zero...
    if (pTest->m_dwMaxPerformanceIndex > 0)
    {
        if (pResult->m_dwPerformanceIndex < pTest->m_dwMinPerformanceIndex)
            pResult->m_dwPerformanceIndex = pTest->m_dwMinPerformanceIndex;

        if (pResult->m_dwPerformanceIndex > pTest->m_dwMaxPerformanceIndex)
            pResult->m_dwPerformanceIndex = pTest->m_dwMaxPerformanceIndex;
    }

    // Refresh
    Clear();

    return result;
}
#pragma optimize("", on)

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::StartGraphicsBenchmarking(LTFLOAT fTestDuration)
{
    // If we're already benchmarking, return
    if (m_bIsGraphicsBenchmarking)
        return LT_ERROR;

    // Re-init our results struct
    ClearGraphicsBenchmarkingInfo();
    m_ltBenchGraphicsTest.m_dwMinFrameRate = MAX_DWORD;
    m_ltBenchGraphicsTest.m_dwMinPolyCount = MAX_DWORD;

    // Keep track of our state
    m_bIsGraphicsBenchmarking = LTTRUE;

    // Determine when we need to end
    m_fGraphicsTestDuration = fTestDuration;
    m_dwGraphicsTestCurrentTime = (uint32)time_GetTime();
    m_dwGraphicsTestStartTime = m_dwGraphicsTestCurrentTime;
    m_dwGraphicsTestEndTime = 0;
    if (m_fGraphicsTestDuration > 0.0f)
        m_dwGraphicsTestEndTime = m_dwGraphicsTestStartTime + (uint32)(1000 * fTestDuration);

    m_dwUpdateCount = 0;
    m_dwFrameCount = 0;
    m_dwPolyCount = 0;
    
    return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::StopGraphicsBenchmarking(LTBENCH_GRAPHICS_TEST* pTest)
{
    // If we're not benchmarking, return
    if (!m_bIsGraphicsBenchmarking)
        return LT_ERROR;

    // Keep track of our state
    m_bIsGraphicsBenchmarking = LTFALSE;

    // Pass back info?
    if (pTest)
        *pTest = m_ltBenchGraphicsTest;

    return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::IsGraphicsBenchmarking(bool* pbIsBenchmarking)
{
    // Valid pointer?
    if (!pbIsBenchmarking)
        return LT_ERROR;

    *pbIsBenchmarking = m_bIsGraphicsBenchmarking;

    return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::GetGraphicsBenchmarkingInfo(LTBENCH_GRAPHICS_TEST* pTest)
{
    // Valid pointer?
    if (!pTest)
        return LT_ERROR;

    *pTest = m_ltBenchGraphicsTest;

    return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::ClearGraphicsBenchmarkingInfo()
{
    Clear();

    return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTBenchmarkMgr::GraphicsBenchmarkingUpdate()
{
    // Bail if we're not benchmarking
    if (!m_bIsGraphicsBenchmarking)
        return LT_OK;

    // Grab the current performance info...
    LTPERFORMANCEINFO info;

    // Initialize our structure
    memset(&info, 0, sizeof(LTPERFORMANCEINFO));
    info.m_dwSize = sizeof(LTPERFORMANCEINFO);

    LTRESULT ltResult = in_GetPerformanceInfo(&info);
    if (LT_OK != ltResult)
        return LT_ERROR;

    // Bump the count
    m_dwUpdateCount++;

    // Update test object members...
    uint32 dwFramerate = info.m_dwFps;
    if (dwFramerate < m_ltBenchGraphicsTest.m_dwMinFrameRate)
        m_ltBenchGraphicsTest.m_dwMinFrameRate = dwFramerate;
    if (dwFramerate > m_ltBenchGraphicsTest.m_dwMaxFrameRate)
        m_ltBenchGraphicsTest.m_dwMaxFrameRate = dwFramerate;
    if (dwFramerate > 0)
    {
        m_dwFrameCount += dwFramerate;
        m_ltBenchGraphicsTest.m_dwAverageFrameRate = (uint32)(((LTFLOAT)m_dwFrameCount / (LTFLOAT)m_dwUpdateCount) + 0.5f);
    }
    
    uint32 dwPolys = info.m_dwNumWorldPolys + info.m_dwNumModelPolys;
    if (dwPolys < m_ltBenchGraphicsTest.m_dwMinPolyCount)
        m_ltBenchGraphicsTest.m_dwMinPolyCount = dwPolys;
    if (dwPolys > m_ltBenchGraphicsTest.m_dwMaxPolyCount)
        m_ltBenchGraphicsTest.m_dwMaxPolyCount = dwPolys;
    if (dwPolys > 0)
    {
        m_dwPolyCount += dwPolys;
        m_ltBenchGraphicsTest.m_dwAveragePolyCount = (uint32)(((LTFLOAT)m_dwPolyCount / (LTFLOAT)m_dwUpdateCount) + 0.5f);
    }

    // Update values (whether we were passed a duration or not so we always have the info)
    m_dwGraphicsTestCurrentTime = (uint32)time_GetTime();

    // Quit?  We only quit when we have a duration...
    if (m_fGraphicsTestDuration > 0.0f)
    {
        // How long have we been running?
        if (m_dwGraphicsTestCurrentTime > m_dwGraphicsTestEndTime)
            m_dwGraphicsTestEndTime = m_dwGraphicsTestCurrentTime;
        m_ltBenchGraphicsTest.m_fTestDuration = m_fGraphicsTestDuration - (((LTFLOAT)m_dwGraphicsTestEndTime - (LTFLOAT)m_dwGraphicsTestCurrentTime) / (LTFLOAT)1000);

        // Time's up...
        if (m_dwGraphicsTestCurrentTime >= m_dwGraphicsTestEndTime)
            StopGraphicsBenchmarking(NULL);
    }
    else
    {
        // We're running indefinitely... update the time
        LTFLOAT fCurrentTime = (LTFLOAT)m_dwGraphicsTestCurrentTime / 1000.0f;
        LTFLOAT fStartTime = (LTFLOAT)m_dwGraphicsTestStartTime / 1000.0f;
        m_ltBenchGraphicsTest.m_fTestDuration = fCurrentTime - fStartTime;
    }

    return LT_OK;
}
