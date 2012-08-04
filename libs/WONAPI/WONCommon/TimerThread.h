#ifndef __WON_TIMERTHREAD_H__
#define __WON_TIMERTHREAD_H__
#include "WONShared.h"

#include "BiMap.h"
#include "Thread.h"
#include "AsyncOp.h"
#include "CriticalSection.h"

namespace WONAPI
{

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class TimerThread : public Thread
{
private:
	CriticalSection mDataCrit;

	typedef AsyncOp::TimerMap TimerMap;

	TimerMap mTimerMap;
	TimerMap mWrapAroundMap;
	DWORD mLastTick;

private:
	virtual void ThreadFunc();	

public:
	TimerThread();
	~TimerThread();

	void AddTimerOp(AsyncOp *theOp, DWORD theTimeout);
	void RemoveTimerOp(AsyncOp *theOp);
	void Pump();
	DWORD GetWaitTime();

	void PurgeOps();
};


}; // namespace WONAPI

#endif
