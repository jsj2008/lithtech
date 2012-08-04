#ifndef __WON_THREAD_WINDOWS_H__
#define __WON_THREAD_WINDOWS_H__
#include "WONShared.h"

#include <process.h>
#include "Event.h"

namespace WONAPI
{

class Thread : public ThreadBase
{
private:
	static void StaticThreadFunc(void *pThread) 
	{ 
		Thread *aThread = (Thread*)pThread;
		aThread->mThreadId = GetCurrentThreadId();
		aThread->ThreadFunc();
		aThread->mRunning = false;
		if(aThread->mAutoDelete)
			delete aThread;
	}

protected:
	HANDLE mThreadHandle;
	DWORD mThreadId;

	virtual void StartHook()  { mThreadHandle = (HANDLE)_beginthread(StaticThreadFunc,0,this); }
	virtual void WaitForStopHook() { WaitForSingleObject(mThreadHandle,INFINITE); }
	virtual bool InThisThreadHook() { return GetCurrentThreadId()==mThreadId; }	
};

}; // namespace WONAPI

#endif
