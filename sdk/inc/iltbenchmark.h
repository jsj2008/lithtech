/*!***************************************************************************
;
;   MODULE:     ILTBenchmark (.H)
;
;   PURPOSE:    General purpose functions for system benchmarking
;
;   HISTORY:    4-24-2000 [mds] File created.
;
***************************************************************************/

#ifndef __ILTBENCHMARK_H__
#define __ILTBENCHMARK_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


/*!  General purpose functions for system benchmarking.  */

/*!  Benchmark CPU Test Structure */

typedef struct LTBenchCPUTest_t
{
    uint32  m_dwSize;
    uint32  m_dwMinPerformanceIndex;
    uint32  m_dwMaxPerformanceIndex;
} LTBENCH_CPU_TEST;



/*!  Benchmark CPU Test Result Structure */

typedef struct LTBenchCPUResult_t
{
    uint32  m_dwSize;
    uint32  m_dwPerformanceIndex;
    LTFLOAT m_fTestDuration;
} LTBENCH_CPU_RESULT;



/*!  Benchmark Graphics Test Structure */

typedef struct LTBenchGraphicsInfo_t
{
    uint32  m_dwSize;
    uint32  m_dwMinFrameRate;
    uint32  m_dwMaxFrameRate;
    uint32  m_dwAverageFrameRate;
    uint32  m_dwMinPolyCount;
    uint32  m_dwMaxPolyCount;
    uint32  m_dwAveragePolyCount;
    LTFLOAT m_fTestDuration;
} LTBENCH_GRAPHICS_TEST;

/*!
The ILTBenchmarkMgr interface contains functions that are used to
determine the performance of the current graphics hardware.

Define a holder to get this interface like this:
\code
define_holder(ILTBenchmarkMgr, your_var);
\endcode
*/

class ILTBenchmarkMgr : public IBase {
public:
    interface_version(ILTBenchmarkMgr, 0);

/*!
Initialize the benchmark manager. This function must be called first.

Used for: Misc.
*/
    virtual LTRESULT    Init() = 0;

/*!
Terminate the benchmark manager. This function must be called last.

Used for: Misc.
*/
    virtual LTRESULT    Term() = 0;

/*!
\param  pTest Address of a \b LTBENCH_CPU_TEST structure that contains information about the test to be performed.
\param  pResult Address of a \b LTBENCH_CPU_RESULT structure that will contain information about the test that was performed.
\param  fTestDuration Time in seconds that the testing will be performed.
\param  bAllowEscape If this parameter is \b true, pressing the escape key during the test will abort the test.  If this parameter is \b false, the user will not be able to abort the test.

Execute the CPU benchmark test.

Used for: Misc.
*/
    virtual LTRESULT    DoCPUBenchmarking(LTBENCH_CPU_TEST* pTest,
                                            LTBENCH_CPU_RESULT* pResult,
                                            LTFLOAT fTestDuration = 1.0f,
                                            bool bAllowEscape = true) = 0;

/*!
\param  fTestDuration Time in seconds that the testing will be performed.  If this value is zero, the testing will continue until a call is made to \b StopGraphicsBenchmarking.  This optional parameter defaults to zero.

Start tracking graphics benchmark information.

Used for: Misc.
*/
    virtual LTRESULT    StartGraphicsBenchmarking(
        LTFLOAT fTestDuration = 0.0f) = 0;

/*!
\param  pTest Address of a \b LTBENCH_GRAPHICS_TEST structure that will contain information about the graphics test results.  This optional parameter defaults to \b NULL.

Stop tracking graphics benchmark information.

Used for: Misc.
*/
    virtual LTRESULT    StopGraphicsBenchmarking(
        LTBENCH_GRAPHICS_TEST* pTest = NULL) = 0;

/*!
\param  pbIsBenchmarking Address of a bool variable to return the result.

Determine if graphics benchmarking is in progress.


Used for: Misc.
*/
    virtual LTRESULT    IsGraphicsBenchmarking(bool* pbIsBenchmarking) = 0;

/*!
\param  pTest Address of a \b LTBENCH_GRAPHICS_TEST structure that contains information about the graphics test results.

Get graphics benchmarking information.

Used for: Misc.
*/
    virtual LTRESULT    GetGraphicsBenchmarkingInfo(
        LTBENCH_GRAPHICS_TEST* pTest) = 0;

/*!
Reset graphics benchmarking information.

Used for: Misc.
*/
    virtual LTRESULT    ClearGraphicsBenchmarkingInfo() = 0;
};

 
#endif //! __ILTBENCHMARK_H__

