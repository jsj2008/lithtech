#ifndef __WON_THREAD_H__
#define __WON_THREAD_H__
#include "WONShared.h"
#include "Event.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class ThreadBase
{
protected:
	bool mStopped;
	bool mRunning;
	bool mAutoDelete;
	Event mSignalEvent;

	virtual void StartHook() { mRunning = false; }
	virtual void WaitForStopHook() { }
	virtual bool InThisThreadHook() { return false; }

	virtual void ThreadFunc()=0;

public:
	ThreadBase();
	virtual ~ThreadBase();

	void Start();
	void Stop(bool waitForStop = true);
	void WaitForStop(); 
	bool InThisThread();
	bool IsRunning() { return mRunning; }
	void SetAutoDelete(bool autoDelete) { mAutoDelete = autoDelete; }

	virtual void Signal();
};

}; // namespace WONAPI

#ifndef __WON_SINGLETHREADED__

#if defined(WIN32)
#include "Thread_Windows.h"
#elif defined(_LINUX)
#include "Thread_Linux.h"
#elif defined(macintosh) && (macintosh == 1)
#include "Thread_Mac.h"
#else
#error No Thread class defined. (Need WIN32, _LINUX or macintosh defined)
#endif

#else  // __WON_SINGLETHREADED__

namespace WONAPI
{
	typedef ThreadBase Thread;
};

#endif  // __WON_SINGLETHREADED__

#endif
