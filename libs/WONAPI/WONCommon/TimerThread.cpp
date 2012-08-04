#include "TimerThread.h"
using namespace WONAPI;

/*
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::Destroy()
{
	TimerThread *aThread = mTimerThread;
	mTimerThread = NULL;

	aThread->Stop();
	delete aThread;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::Pump()
{
	mTimerThread->PumpPrv();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD TimerThread::GetWaitTime()
{
	return mTimerThread->GetWaitTimePrv();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::AddTimerOp(AsyncOp *theOp, DWORD theTimeout)
{
	if(mTimerThread!=NULL)
		mTimerThread->AddTimerOpPrv(theOp,theTimeout);
	else
		theOp->Kill();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::RemoveTimerOp(AsyncOp *theOp)
{
	if(mTimerThread!=NULL)
		mTimerThread->RemoveTimerOpPrv(theOp);
}
*/

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TimerThread::TimerThread()
{
	mLastTick = 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TimerThread::~TimerThread()
{
	PurgeOps();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::PurgeOps()
{
	AutoCrit aCrit(mDataCrit);

	TimerMap::iterator anItr = mTimerMap.begin();
	while(anItr!=mTimerMap.end())
	{
		AsyncOp *anOp = anItr->second;
		anOp->RemoveFromTimerMap(false);
		anOp->Kill();
		++anItr;
	}

	anItr = mWrapAroundMap.begin();
	while(anItr!=mWrapAroundMap.end())
	{
		AsyncOp *anOp = anItr->second;
		anOp->RemoveFromTimerMap(false);
		anOp->Kill();
		++anItr;
	}

	mTimerMap.clear();
	mWrapAroundMap.clear();
	mLastTick = 0; 
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::AddTimerOp(AsyncOp *theOp, DWORD theTimeout)
{
	AutoCrit aCrit(mDataCrit);

	DWORD aTick = GetTickCount();
	DWORD aWrapDist = 0xffffffff - aTick;
	if(theTimeout > aWrapDist) // wrap around problem
	{
		DWORD aWrapTick = theTimeout - aWrapDist; 
		theOp->AddToTimerMap(mWrapAroundMap,aWrapTick);
	}
	else
		theOp->AddToTimerMap(mTimerMap,aTick+theTimeout);

	Signal();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::RemoveTimerOp(AsyncOp *theOp)
{
	AutoCrit aCrit(mDataCrit);
	theOp->RemoveFromTimerMap(true);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::Pump()
{
	AutoCrit aCrit(mDataCrit);

	DWORD aNewTick = GetTickCount();

	TimerMap::iterator anItr;
	if(aNewTick < mLastTick) //wrap around
	{
		anItr = mTimerMap.begin();
		while(anItr!=mTimerMap.end())
		{
			AsyncOp *anOp = anItr->second;
			anOp->RemoveFromTimerMap(false);
			anOp->Finish(WS_TimedOut);
			++anItr;
		}

		mTimerMap = mWrapAroundMap;
		mWrapAroundMap.clear();
	}

	mLastTick = aNewTick;
	anItr = mTimerMap.begin();
	while(anItr!=mTimerMap.end())
	{
		if(anItr->first<=mLastTick)
		{
			AsyncOp* anOp = anItr->second;
			anOp->RemoveFromTimerMap(false);
			anOp->Finish(WS_TimedOut);
			mTimerMap.erase(anItr++);
		}
		else
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD TimerThread::GetWaitTime()
{
	AutoCrit aCrit(mDataCrit);

	// Set thread to wake up when first timer goes off or when wrap around occurs
	DWORD aWaitTime = 0xffffffff - mLastTick + 10;

	if(!mTimerMap.empty()) 
	{
		TimerMap::iterator anItr = mTimerMap.begin();
		if(anItr->first <= mLastTick)
			aWaitTime = 0;
		else
			aWaitTime = anItr->first - mLastTick;
			
	}

	return aWaitTime;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimerThread::ThreadFunc()
{
	mLastTick = GetTickCount();

	while(true)
	{
		DWORD aWaitTime = GetWaitTime();

		// Wait for wakeup event, stop event, and timeout when timers should start going off
		mSignalEvent.WaitFor(aWaitTime);
		if(mStopped) // stop
			break;

		Pump();
	}
}

	
