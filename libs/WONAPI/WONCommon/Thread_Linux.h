#ifndef __WON_THREAD_LINUX_H__
#define __WON_THREAD_LINUX_H__
#include "WONShared.h"

#include "Platform_Linux.h"
#include "Event.h"

namespace WONAPI
{

class Thread : public ThreadBase
{
private:
	static void* StaticThreadFunc(void *pThread) 
	{ 
		Thread *aThread = (Thread*)pThread;
		aThread->ThreadFunc(); 
		aThread->mRunning = false;
		if(aThread->mAutoDelete)
			delete aThread;

		return NULL;
	}

protected:
	pthread_t mThreadHandle;


	virtual void StartHook()  { pthread_create(&mThreadHandle, NULL, StaticThreadFunc, this); }
	virtual void WaitForStopHook() { pthread_join(mThreadHandle, NULL); }
	virtual bool InThisThreadHook() { return pthread_equal(pthread_self(), mThreadHandle)!=0; }	
};

}; // namespace WONAPI

#endif
