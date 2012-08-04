#include "AsyncOpContainer.h"
using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AsyncOpContainer::AsyncOpContainer() 
{
	mPurging = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
AsyncOpContainer::~AsyncOpContainer()
{
	PurgeOps();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AsyncOpContainer::PurgeOps() // kills all ops
{
	mPurging = true;
	while(!mOpQueue.empty())
	{
		mCurPumpOp = mOpQueue.front();
		mOpQueue.pop_front();
		if(mCurPumpOp->Pending())
			mCurPumpOp->Kill();
		
		mCurPumpOp->Complete();
	}
	mPurging = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AsyncOpContainer::QueueOp(AsyncOp *theOp)
{	
	AutoCrit aCrit(mDataCrit);
	if(theOp==mCurPumpOp.get())
	{
		if(!theOp->Pending() || mPurging) // this allows ops to be run again from within Complete
			return;
	}

	mOpQueue.push_back(theOp);
	if(mOpQueue.size()==1)
		mSignal.Set();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool AsyncOpContainer::WaitForOps(DWORD theTime)
{
	AutoCrit aCrit(mDataCrit);
	if(!mOpQueue.empty())
		return true;

	aCrit.Leave();
	return mSignal.WaitFor(theTime);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void AsyncOpContainer::Pump(DWORD theMaxTime, int theMaxOps)
{	
	AutoCrit aCrit(mDataCrit);

	DWORD aStartTick = GetTickCount();
	DWORD anElapsed;
	int aMaxOps = theMaxOps;
	if(aMaxOps<0)
		aMaxOps = mOpQueue.size();

	if(mOpQueue.empty() && theMaxTime>0)
	{
		aCrit.Leave();
		mSignal.WaitFor(theMaxTime);
		aCrit.Enter();
	}

	// Pump until either:
	// 1. OpQueue is empty
	// 2. theMaxTime is exceeded
	// 3. theMaxOps is exceeded
	while(!mOpQueue.empty())
	{
		mCurPumpOp = mOpQueue.front();
		mOpQueue.pop_front();
		
		aCrit.Leave();

		if(mCurPumpOp->Pending())
			mCurPumpOp->StartRunAsync();

		if(mCurPumpOp->AwaitingCompletion())
			mCurPumpOp->Complete();

		aCrit.Enter();
		
		// Check if exceeded the max number of ops
		if(aMaxOps>0) 
		{
			aMaxOps--;
			if(aMaxOps==0) 
				break;
		}

		// Check if exceeded the max amount of time
		if(theMaxTime>0)
		{
			anElapsed = GetTickCount() - aStartTick;
			if(anElapsed >= theMaxTime)
				break;
		}
	}

	mCurPumpOp = NULL;		
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
