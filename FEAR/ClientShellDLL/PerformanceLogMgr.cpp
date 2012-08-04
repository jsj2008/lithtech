#include "stdafx.h"
#include "PerformanceLogMgr.h"

#ifndef _FINAL
#include "PerformanceLog.h"
#endif

//called to initialize the performance log system and create the default values. This
//should be called at startup time
void CPerformanceLogMgr::Init()
{
#ifndef _FINAL
	CPerformanceLog::Singleton().Init();
#endif
}

//called to shut down the performance logging system. This should be called prior to shutdown
void CPerformanceLogMgr::Term()
{
#ifndef _FINAL
	CPerformanceLog::Singleton().Term();
#endif
}


//called to set the currently active level so that reports will go to the correct location
void CPerformanceLogMgr::SetCurrentLevel(const char* pszLevel)
{
#ifndef _FINAL
	CPerformanceLog::Singleton().SetCurrentLevel(pszLevel);
#endif
}


//called to update the frame logging system with the specified interval
void CPerformanceLogMgr::Update(double fFrameTimeS){
#ifndef _FINAL
	CPerformanceLog::Singleton().Update(fFrameTimeS);
#endif
}

