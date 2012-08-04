//----------------------------------------------------------------------------------
// PerformanceLogMgr.h
//
// This class provides the interface that should be used by the game code for the 
// performance logging system. This system is setup in such a manner so that it can
// be completely removed in FINAL builds, and this manager handles this separation
//
//----------------------------------------------------------------------------------
#ifndef __PERFORMANCELOGMGR_H__
#define __PERFORMANCELOGMGR_H__

class CPerformanceLogMgr
{
public:

	//called to initialize the performance log system and create the default values. This
	//should be called at startup time
	static void Init();

	//called to shut down the performance logging system. This should be called prior to shutdown
	static void Term();

	//called to set the currently active level so that reports will go to the correct location
	static void SetCurrentLevel(const char* pszLevel);

	//called to update the frame logging system with the specified interval
	static void	Update(double fFrameTimeS);
};

#endif


